/******************************************************************************
* @file  rsi_udma.c
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
******************************************************************************/

#include "rsi_ccp_user_config.h"
#ifndef UDMA_ROMDRIVER_PRESENT

#include "rsi_rom_udma.h"
#include "rsi_udma.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================*/
/**
 * @fn          RSI_DRIVER_VERSION_M4 RSI_UDMA_GetVersion(void)
 * @brief		This API is used to get version information of the driver implementation
 * @return 		vsDriverVersion - if success 
 *              RSI_UDMA_API_VERSION : Version of the CMSIS-Driver specification used to implement this driver.
 *              RSI_UDMA_DRV_VERSION : UDMA peripheral source code version of the actual driver implementation.
 */
RSI_DRIVER_VERSION_M4 RSI_UDMA_GetVersion(void)
{
  RSI_DRIVER_VERSION_M4 vsDriverVersion;

  vsDriverVersion.api = RSI_UDMA_API_VERSION;
  vsDriverVersion.drv = RSI_UDMA_DRV_VERSION;
  return vsDriverVersion;
}

/*==============================================*/
/**
 * @fn          RSI_UDMA_CAPABILITIES RSI_UDMA_GetCapabilities(void)
 * @brief		The function RSI_UDMA_GetCapabilities returns information about capabilities of this driver implementation.
 * @return 		vsDriverCapabilities - if success
 *              NoOfChannels: Number of DMA channels
 *				MaxNoOfTxferPerCycle  : Maximum number of transfers per DMA cycle
 */
RSI_UDMA_CAPABILITIES_T RSI_UDMA_GetCapabilities(void)
{
  RSI_UDMA_CAPABILITIES_T vsDriverCapabilities;

  vsDriverCapabilities.maxNoOfTxferPerCycle = 1024;
  vsDriverCapabilities.noOfChannels         = 32;
  return vsDriverCapabilities;
}

/*==============================================*/
/**
 * @fn          RSI_UDMA_HANDLE_T udma_init(void *mem, const RSI_UDMA_INIT_T *pInit)
 * @brief		This API is used to initialize driver context parameters
 * @param[in]	mem 	:  Pointer to memory
 * @param[in]	pInit	:  Pointer to UDMA initialization
 * @return 		value of pDrv - if success
 */
RSI_UDMA_HANDLE_T udma_init(void *mem, const RSI_UDMA_INIT_T *pInit)
{
  RSI_UDMA_DATACONTEXT_T *pDrv;

  /* Verify alignment is at least 4 bytes */
  if (((uint32_t)mem & 0x3) != 0) {
    return NULL;
  }

  pDrv = (RSI_UDMA_DATACONTEXT_T *)mem;
  memset(pDrv, 0, sizeof(RSI_UDMA_DATACONTEXT_T));

  // Save pointer to user data
  pDrv->pUserData = pInit->pUserData;
  pDrv->base      = (RSI_UDMA_T *)pInit->base;
  pDrv->sramBase  = (RSI_UDMA_DESC_T *)pInit->sramBase;

  if ((pInit->sramBase & ((uint32_t)(~0x3FF))) == pInit->sramBase) {
    pDrv->base->CTRL_BASE_PTR = pInit->sramBase;
  } else {
    return (RSI_UDMA_HANDLE_T)ERROR_UDMA_CTRL_BASE_INVALID;
  }
  return (RSI_UDMA_HANDLE_T)pDrv;
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_UDMA_ChannelControlsDisable(RSI_UDMA_HANDLE_T pHandle, RSI_UDMA_CHA_CFG_T *pCfg)
 * @brief		This API is used to disable the required UDMA channel attribute
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @param[in]	pCfg	:  Pointer to channel configuration structure
 * @return 	    RSI_OK - if success
 */
rsi_error_t RSI_UDMA_ChannelControlsDisable(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  if (pCfg->dmaCh <= CHNL_31) {
    // Clear the useburst bit for this channel
    if (pCfg->burstReq == 0) {
      pDrv->base->CHNL_USEBURST_CLR = SET_BIT(pDrv->dmaCh);
    }
    // Clear the alternate control select bit for this channel
    if (pCfg->altStruct == 0) {
      pDrv->base->CHNL_PRI_ALT_CLR = SET_BIT(pDrv->dmaCh);
    }
    // Clear the high priority bit for this channel
    if (pCfg->channelPrioHigh == 0) {
      pDrv->base->CHNL_PRIORITY_CLR = SET_BIT(pDrv->dmaCh);
    }
    // Clear the request mask bit for this channel
    if (pCfg->reqMask == 0) {
      pDrv->base->CHNL_REQ_MASK_CLR = SET_BIT(pDrv->dmaCh);
    }
  } else {
    return ERROR_UDMA_INVALIDCHNLNUM;
  }
  return RSI_OK;
}
/*==============================================*/
/**
 * @fn          rsi_error_t udma_setup_channel_transfer(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg, RSI_UDMA_CHA_CONFIG_DATA_T vsUdmaChaConfigData, void *pSrcAddr, void *pDstAddr)
 * @brief		This API is used to control parameters for a UDMA channel control structure
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @param[in]	pCfg	:  Pointer to channel configuration structure
 * @param[in]	vsUDMAChaConfigData	: micro DMA channel for structure configuration
 * @param[out]	pSrcAddr:  Pointer to source address
 * @param[out]	pDstAddr:  Pointer to destination address
 * @return 		
 *         - ERROR_UDMA_INVALID_ARG - if fails 
 *         - RSI_OK - if success
 */
rsi_error_t udma_setup_channel_transfer(RSI_UDMA_HANDLE_T pHandle,
                                        const RSI_UDMA_CHA_CFG_T *pCfg,
                                        RSI_UDMA_CHA_CONFIG_DATA_T vsUdmaChaConfigData,
                                        void *pSrcAddr,
                                        volatile void *pDstAddr)
{
  uint32_t channelTableIndex = 0;
  uint32_t srcInc            = 0;
  uint32_t dstInc            = 0;
  uint32_t length            = 0;
  RSI_UDMA_DESC_T *pUDMAChaCtrlDataStruct;

  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  // Enable skip descriptor fetch
  pDrv->base->UDMA_SKIP_DESC_FETCH_REG |= (1 << pDrv->dmaCh);

  /* Alternate Data structure */
  if (pCfg->altStruct == 1) {
    if (pDrv->base == UDMA0) {
      channelTableIndex = (pDrv->dmaCh) + UDMA_ALT_SELECT;
    }
    if (pDrv->base == UDMA1) {
      channelTableIndex = (pDrv->dmaCh) + UDMA_ULPALT_SELECT;
    }
  }
  // Primary Data Structure
  if (pCfg->altStruct == 0) {
    channelTableIndex = (pDrv->dmaCh) + UDMA_PRI_SELECT;
  }

  if ((channelTableIndex & 0xffff) < 64) {
    channelTableIndex &= 0x3f;
    pUDMAChaCtrlDataStruct = (RSI_UDMA_DESC_T *)(pDrv->base->CTRL_BASE_PTR);

    // Sets all configurable descriptor parameters
    // Set transfer mode
    if ((vsUdmaChaConfigData.transferType == UDMA_MODE_BASIC) || (vsUdmaChaConfigData.transferType == UDMA_MODE_AUTO)
        || (vsUdmaChaConfigData.transferType == UDMA_MODE_PINGPONG)) {
      pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.transferType = vsUdmaChaConfigData.transferType;
    } else {
      return ERROR_UDMA_INVALID_XFERMODE;
    }

    /* Set Next burst */
    pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.nextBurst = vsUdmaChaConfigData.nextBurst;

    /* Set transfer length */
    if (vsUdmaChaConfigData.totalNumOfDMATrans <= UDMA_CHCTL_XFERSIZE_M) {
      pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.totalNumOfDMATrans =
        vsUdmaChaConfigData.totalNumOfDMATrans;
    } else {
      return ERROR_UDMA_INVALID_TRANS_LEN;
    }
    // Set arbitration
    if (vsUdmaChaConfigData.rPower <= ARBSIZE_1024) {
      pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.rPower = vsUdmaChaConfigData.rPower;
    } else {
      return ERROR_UDMA_INVALID_ARG;
    }
    // Set protection control
    pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.srcProtCtrl = vsUdmaChaConfigData.srcProtCtrl;
    pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.dstProtCtrl = vsUdmaChaConfigData.dstProtCtrl;

    // set source size
    if ((vsUdmaChaConfigData.srcSize == SRC_SIZE_8) || (vsUdmaChaConfigData.srcSize == SRC_SIZE_16)
        || (vsUdmaChaConfigData.srcSize == SRC_SIZE_32)) {
      pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.srcSize = vsUdmaChaConfigData.srcSize;
    } else {
      return ERROR_UDMA_INVALID_ARG;
    }
    // set source increment
    if ((vsUdmaChaConfigData.srcInc == SRC_INC_8) || (vsUdmaChaConfigData.srcInc == SRC_INC_16)
        || (vsUdmaChaConfigData.srcInc == SRC_INC_32) || (vsUdmaChaConfigData.srcInc == SRC_INC_NONE)) {
      pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.srcInc = vsUdmaChaConfigData.srcInc;
    } else {
      return ERROR_UDMA_INVALID_ARG;
    }
    // set destination size
    if ((vsUdmaChaConfigData.dstSize == DST_SIZE_8) || (vsUdmaChaConfigData.dstSize == DST_SIZE_16)
        || (vsUdmaChaConfigData.dstSize == DST_SIZE_32)) {
      pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.dstSize = vsUdmaChaConfigData.dstSize;
    } else {
      return ERROR_UDMA_INVALID_ARG;
    }

    if ((vsUdmaChaConfigData.dstInc == DST_INC_8) || (vsUdmaChaConfigData.dstInc == DST_INC_16)
        || (vsUdmaChaConfigData.dstInc == DST_INC_32) || (vsUdmaChaConfigData.dstInc == DST_INC_NONE)) {
      pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.dstInc = vsUdmaChaConfigData.dstInc;
    } else {
      return ERROR_UDMA_INVALID_ARG;
    }

    // Adjust the mode if the alt control structure is selected
    if ((channelTableIndex & UDMA_ALT_SELECT)
        && (vsUdmaChaConfigData.transferType == UDMA_MODE_MEM_SCATTER_GATHER
            || vsUdmaChaConfigData.transferType == UDMA_MODE_PER_SCATTER_GATHER)) {
      vsUdmaChaConfigData.transferType |= UDMA_MODE_ALT_SELECT;
    }
    // Calculate soure end address based on transfer mode
    srcInc = (vsUdmaChaConfigData.srcInc);
    if (srcInc != UDMA_SRC_INC_NONE) {
      length   = (uint16_t)((vsUdmaChaConfigData.totalNumOfDMATrans + 1) << srcInc);
      pSrcAddr = (void *)((uint32_t)pSrcAddr + length - 1);
    }

    pUDMAChaCtrlDataStruct[channelTableIndex].pSrcEndAddr = pSrcAddr;

    // calculate destination end address based on mode
    dstInc = (vsUdmaChaConfigData.dstInc);
    if (dstInc != UDMA_DST_INC_NONE) {
      if ((vsUdmaChaConfigData.transferType == UDMA_MODE_MEM_SCATTER_GATHER)
          || (vsUdmaChaConfigData.transferType == UDMA_MODE_PER_SCATTER_GATHER)) {
        pDstAddr = (volatile void *)&pUDMAChaCtrlDataStruct[channelTableIndex | UDMA_ALT_SELECT].Spare;
      } else {
        length   = (uint16_t)((vsUdmaChaConfigData.totalNumOfDMATrans + 1) << dstInc);
        pDstAddr = (volatile void *)((volatile uint8_t *)pDstAddr + length - 1);
      }
    }

    pUDMAChaCtrlDataStruct[channelTableIndex].pDstEndAddr = pDstAddr;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t udma_set_channel_scatter_gather_transfer(RSI_UDMA_HANDLE_T pHandle,
 *                                                uint8_t dmaCh,
 *                                                uint32_t taskCount,
 *                                                void *pTaskList,
 *                                                uint32_t transferType)
 * @brief		This API is used to configures a UDMA channel for scatter-gather mode
 * @param[in]	pHandle	 : Pointer to driver context handle
 * @param[in]	dmaCh	 :  DMA channel
 * @param[in]	taskcount: task count
 * @param[in]	pTastList: Pointer to task list 
 * @param[in]	transferType: Set transfer mode
 * @return 		RSI_OK - if success
 */
rsi_error_t udma_set_channel_scatter_gather_transfer(RSI_UDMA_HANDLE_T pHandle,
                                                     uint8_t dmaCh,
                                                     uint32_t taskCount,
                                                     void *pTaskList,
                                                     uint32_t transferType)
{
  RSI_UDMA_DESC_T *psTaskTable;
  RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData;
  RSI_UDMA_DESC_T *pUDMAChaCtrlDataStruct;

  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  if ((dmaCh < 32) && (taskCount <= 1024) && (taskCount != 0)) {
    pUDMAChaCtrlDataStruct = (RSI_UDMA_DESC_T *)(pDrv->base->CTRL_BASE_PTR);
    psTaskTable            = (RSI_UDMA_DESC_T *)pTaskList;

    // Set source end pointer address
    pUDMAChaCtrlDataStruct[dmaCh].pSrcEndAddr = &psTaskTable[taskCount - 1].Spare;

    if (pDrv->base == UDMA0) {
      // Set dest end pointer address
      pUDMAChaCtrlDataStruct[dmaCh].pDstEndAddr = &(pUDMAChaCtrlDataStruct[dmaCh | UDMA_ALT_SELECT].Spare);
    }
    if (pDrv->base == UDMA1) {
      // Set dest end pointer address
      pUDMAChaCtrlDataStruct[dmaCh].pDstEndAddr = &(pUDMAChaCtrlDataStruct[dmaCh | UDMA_ULPALT_SELECT].Spare);
    }

    // Set primary structure configurations
    vsUDMAChaConfigData.dstInc             = DST_INC_32;
    vsUDMAChaConfigData.dstSize            = DST_SIZE_32;
    vsUDMAChaConfigData.srcInc             = SRC_INC_32;
    vsUDMAChaConfigData.srcSize            = SRC_SIZE_32;
    vsUDMAChaConfigData.rPower             = ARBSIZE_4;
    vsUDMAChaConfigData.transferType       = (unsigned int)(transferType & 0x07);
    vsUDMAChaConfigData.totalNumOfDMATrans = (unsigned int)(((taskCount * 4) - 1) & 0x03FF);
    vsUDMAChaConfigData.srcProtCtrl        = 0x0;
    vsUDMAChaConfigData.dstProtCtrl        = 0x0;
    vsUDMAChaConfigData.nextBurst          = 0x0;

    pUDMAChaCtrlDataStruct[dmaCh].vsUDMAChaConfigData1 = vsUDMAChaConfigData;
  } else {
    return ERROR_UDMA_INVALID_ARG;
  }
  pDrv->base->CHNL_PRI_ALT_CLR = SET_BIT(dmaCh);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          uint32_t udma_get_channel_transfer_length(RSI_UDMA_HANDLE_T pHandle,
 *                                         const RSI_UDMA_CHA_CFG_T *pCfg,
 *                                         RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData)
 * @brief		This API Gets the current transfer size for a UDMA channel control structure
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @param[in]	pCfg	:  Pointer to channel configuration structure
 * @param[in]	vsUDMAChaConfigData	: micro DMA channel for structure configuration
 * @return 		RSI_OK - if success
 */
uint32_t udma_get_channel_transfer_length(RSI_UDMA_HANDLE_T pHandle,
                                          const RSI_UDMA_CHA_CFG_T *pCfg,
                                          RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData)
{
  UNUSED_PARAMETER(vsUDMAChaConfigData);
  uint32_t channelTableIndex = 0;
  uint32_t transferSize      = 0;
  const RSI_UDMA_DESC_T *pUDMAChaCtrlDataStruct;

  const RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  /* checks Alternate Data structure */
  if (pCfg->altStruct == 1) {
    channelTableIndex = (pDrv->dmaCh) + UDMA_ALT_SELECT;
  }
  // checks Primary Data structure
  if (pCfg->altStruct == 0) {
    channelTableIndex = (pDrv->dmaCh) + UDMA_PRI_SELECT;
  }
  if ((channelTableIndex & 0xffff) < 64) {
    channelTableIndex &= 0x3f;
    // Assigns control base pointer to the desciptor
    pUDMAChaCtrlDataStruct = (RSI_UDMA_DESC_T *)(pDrv->base->CTRL_BASE_PTR);
    // Gets transfer size
    transferSize = pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.totalNumOfDMATrans;

    // the number of items remaining to transfer
    return (transferSize + 1);
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          uint32_t udma_get_channel_transfer_mode(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
 * @brief		This API Gets the current transfer size for a UDMA channel control structure
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @param[in]	pCfg	:  Pointer to channel configuration structure
 * @return 		RSI_OK - if success
 */
uint32_t udma_get_channel_transfer_mode(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
{
  uint32_t channelTableIndex = 0;
  uint32_t transferMode      = 0;
  const RSI_UDMA_DESC_T *pUDMAChaCtrlDataStruct;

  const RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  // checks Alternate Data structure
  if (pCfg->altStruct == 1) {
    channelTableIndex = (pDrv->dmaCh) + UDMA_ALT_SELECT;
  }
  // checks Primary Data structure
  if (pCfg->altStruct == 0) {
    channelTableIndex = (pDrv->dmaCh) + UDMA_PRI_SELECT;
  }
  if ((channelTableIndex & 0xffff) < 64) {
    channelTableIndex &= 0x3f;

    // assigns control base pointer to the desciptor
    pUDMAChaCtrlDataStruct = (RSI_UDMA_DESC_T *)(pDrv->base->CTRL_BASE_PTR);

    // Get the transfer mode from the control memory data
    transferMode = pUDMAChaCtrlDataStruct[channelTableIndex].vsUDMAChaConfigData1.transferType;

    return transferMode;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          void RSI_UDMA_SetSingleRequest(RSI_UDMA_HANDLE_T pHandle)
 * @brief		This API is used to set the request for single transfer
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @return 		none
 */
void RSI_UDMA_SetSingleRequest(RSI_UDMA_HANDLE_T pHandle)
{
  RSI_UDMA_DATACONTEXT_T *pDrv                             = (RSI_UDMA_DATACONTEXT_T *)pHandle;
  pDrv->base->UDMA_CONFIG_CTRL_REG_b.SINGLE_REQUEST_ENABLE = ENABLE;
}
/*==============================================*/
/**
 * @fn      rsi_error_t udma_interrupt_enable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
 * @brief		This API is used to enable the done interrupt to processor. 
 * @param[in]	pHandle	   : Pointer to driver context handle
 * @param[in]	dmaCh	     :  DMA channel
 * @return 		RSI_OK - if success
 */
rsi_error_t udma_interrupt_enable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  if ((dmaCh < 32) && (pDrv->base == UDMA0)) {
    M4SS_UDMA_INTR_SEL |= SET_BIT(dmaCh);
  } else if ((dmaCh < 12) && (pDrv->base == UDMA1)) {
    pDrv->base->UDMA_INTR_MASK_REG |= SET_BIT(dmaCh);
  } else {
    return ERROR_UDMA_INVALIDCHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          void RSI_UDMA_AckEnable(const void *pHandle, uint32_t peripheral)
 * @brief		This API is used to enable ACK from DMA to the peripheral while transfers . 
 * @param[in]	pHandle	   : Pointer to driver context handle
 * @param[in]   peripheral : ACK enable for required peripheral
 * @return 		none
 */
void RSI_UDMA_AckEnable(const void *pHandle, uint32_t peripheral)
{
  UNUSED_PARAMETER(pHandle);
  switch (peripheral) {
    case USART0_ACK:
      PERIPHERAL_UDMA_DMA_SEL |= SET_BIT(0);
      break;

    case UART1_ACK:
      PERIPHERAL_UDMA_DMA_SEL |= SET_BIT(1);
      break;

    case UART3_ACK:
      PERIPHERAL_UDMA_DMA_SEL |= SET_BIT(3);
      break;

    case SSISLAVE_ACK:
      PERIPHERAL_UDMA_DMA_SEL |= SET_BIT(4);
      break;

    case SSIMASTER_ACK:
      PERIPHERAL_UDMA_DMA_SEL |= SET_BIT(5);
      break;

    case SSISLAVE1_ACK:
      PERIPHERAL_UDMA_DMA_SEL |= SET_BIT(6);
      break;

    case I2C_ACK:
      PERIPHERAL_UDMA_DMA_SEL |= SET_BIT(7);
      break;
    default:
      break;
  }
}

/*==============================================*/
/**
 * @fn          rsi_error_t udma_setup_channel(RSI_UDMA_HANDLE_T pHandle, RSI_UDMA_CHA_CFG_T *pCfg)
 * @brief		This API is used to configures required parameters for a channel
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @param[in]	pCfg	:  Pointer to channel configuration structure
 * @return 		RSI_OK - if success
 */
rsi_error_t udma_setup_channel(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  pDrv->dmaCh = pCfg->dmaCh;

  if (pDrv->dmaCh > 31) {
    return ERROR_UDMA_INVALIDCHNLNUM;
  } else {
    // Error Clear
    RSI_UDMA_ErrorStatusClear(pHandle);
    // Enable interrupts
    udma_interrupt_enable(pHandle, ((uint8_t)(pDrv->dmaCh)));

    // Set alternate control structure
    if (pCfg->altStruct == 1) {
      pDrv->base->CHNL_PRI_ALT_SET = SET_BIT(pDrv->dmaCh);
    }
    // Set burst or single request
    if (pCfg->burstReq == 1) {
      pDrv->base->CHNL_USEBURST_SET = SET_BIT(pDrv->dmaCh);
    }
    // Single request enable should be there
    RSI_UDMA_SetSingleRequest(pHandle);

    // Masks DMA requests
    if (pCfg->reqMask == 1) {
      pDrv->base->CHNL_REQ_MASK_SET = SET_BIT(pDrv->dmaCh);
    }
    // Set high priority to channel
    if (pCfg->channelPrioHigh == 1) {
      pDrv->base->CHNL_PRIORITY_SET = SET_BIT(pDrv->dmaCh);
    }
    if (pCfg->periphReq == 1) {
      RSI_UDMA_AckEnable(pHandle, pCfg->periAck);
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          void udma_deInit(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
 * @brief		This API is used to initialize driver context parameters
 * @param[in]   pHandle	: Pointer to driver context handle
 * @param[in]   pCfg	: Pointer to DMA channel configuration structure
 * @return 		none
 */
void udma_deInit(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
{
  RSI_UDMA_InterruptClear(pHandle, ((uint8_t)(pCfg->dmaCh)));

  RSI_UDMA_ChannelControlsDisable(pHandle, pCfg);

  RSI_UDMA_ErrorStatusClear(pHandle);

  RSI_UDMA_ChannelDisable(pHandle, ((uint8_t)(pCfg->dmaCh)));
  RSI_UDMA_UDMADisable(pHandle);
}

/*==============================================*/
/**
 * @fn          void udma_interrupt_handler(RSI_UDMA_HANDLE_T pHandle) 
 * @brief		This API is used to handle all interrupt and error flags in interrupt context 
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @return 		none
 */
void udma_interrupt_handler(RSI_UDMA_HANDLE_T pHandle)
{
  uint32_t intr = 0;
  RSI_UDMA_DESC_T pDesc;
  const RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  if (pDrv->udmaCompCB != NULL) {
    intr = pDrv->base->UDMA_DONE_STATUS_REG;
    if (intr & CHNL0_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 0);
      pDrv->udmaCompCB(pHandle, &pDesc, 0);
    }
    if (intr & CHNL1_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 1);
      pDrv->udmaCompCB(pHandle, &pDesc, 1);
    }
    if (intr & CHNL2_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 2);
      pDrv->udmaCompCB(pHandle, &pDesc, 2);
    }
    if (intr & CHNL3_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 3);
      pDrv->udmaCompCB(pHandle, &pDesc, 3);
    }
    if (intr & CHNL4_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 4);
      pDrv->udmaCompCB(pHandle, &pDesc, 4);
    }
    if (intr & CHNL5_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 5);
      pDrv->udmaCompCB(pHandle, &pDesc, 5);
    }
    if (intr & CHNL6_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 6);
      pDrv->udmaCompCB(pHandle, &pDesc, 6);
    }
    if (intr & CHNL7_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 7);
      pDrv->udmaCompCB(pHandle, &pDesc, 7);
    }
    if (intr & CHNL8_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 8);
      pDrv->udmaCompCB(pHandle, &pDesc, 8);
    }
    if (intr & CHNL9_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 9);
      pDrv->udmaCompCB(pHandle, &pDesc, 9);
    }
    if (intr & CHNL10_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 10);
      pDrv->udmaCompCB(pHandle, &pDesc, 10);
    }
    if (intr & CHNL11_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 11);
      pDrv->udmaCompCB(pHandle, &pDesc, 11);
    }
    if (intr & CHNL12_INTR) {
      RSI_UDMA_InterruptClear(pHandle, 12);
      pDrv->udmaCompCB(pHandle, &pDesc, 12);
    }
    if (pDrv->base == UDMA0) {
      if (intr & CHNL13_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 13);
        pDrv->udmaCompCB(pHandle, &pDesc, 13);
      }
      if (intr & CHNL14_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 14);
        pDrv->udmaCompCB(pHandle, &pDesc, 14);
      }
      if (intr & CHNL15_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 15);
        pDrv->udmaCompCB(pHandle, &pDesc, 15);
      }
      if (intr & CHNL16_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 16);
        pDrv->udmaCompCB(pHandle, &pDesc, 16);
      }
      if (intr & CHNL17_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 17);
        pDrv->udmaCompCB(pHandle, &pDesc, 17);
      }
      if (intr & CHNL18_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 18);
        pDrv->udmaCompCB(pHandle, &pDesc, 18);
      }
      if (intr & CHNL19_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 19);
        pDrv->udmaCompCB(pHandle, &pDesc, 19);
      }
      if (intr & CHNL20_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 20);
        pDrv->udmaCompCB(pHandle, &pDesc, 20);
      }
      if (intr & CHNL21_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 21);
        pDrv->udmaCompCB(pHandle, &pDesc, 21);
      }
      if (intr & CHNL22_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 22);
        pDrv->udmaCompCB(pHandle, &pDesc, 22);
      }
      if (intr & CHNL23_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 23);
        pDrv->udmaCompCB(pHandle, &pDesc, 23);
      }
      if (intr & CHNL24_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 24);
        pDrv->udmaCompCB(pHandle, &pDesc, 24);
      }
      if (intr & CHNL25_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 25);
        pDrv->udmaCompCB(pHandle, &pDesc, 25);
      }
      if (intr & CHNL26_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 26);
        pDrv->udmaCompCB(pHandle, &pDesc, 26);
      }
      if (intr & CHNL27_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 27);
        pDrv->udmaCompCB(pHandle, &pDesc, 27);
      }
      if (intr & CHNL28_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 28);
        pDrv->udmaCompCB(pHandle, &pDesc, 28);
      }
      if (intr & CHNL29_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 29);
        pDrv->udmaCompCB(pHandle, &pDesc, 29);
      }
      if (intr & CHNL30_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 30);
        pDrv->udmaCompCB(pHandle, &pDesc, 30);
      }
      if (intr & CHNL31_INTR) {
        RSI_UDMA_InterruptClear(pHandle, 31);
        pDrv->udmaCompCB(pHandle, &pDesc, 31);
      }
    }
  } else {
    return;
  }
}

#ifdef __cplusplus
}
#endif
#else
typedef int dummy; // To remove empty translation unit warning.
#endif //UDMA_ROMDRIVER_PRESENT
