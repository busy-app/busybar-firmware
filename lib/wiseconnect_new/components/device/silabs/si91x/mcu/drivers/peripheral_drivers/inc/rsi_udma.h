/*******************************************************************************
* @file  rsi_udma.h
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
//Include Files

#include "rsi_ccp_common.h"
#include "rsi_error.h"
#include "rsi_packing.h"

#ifndef RSI_UDMA_H
#define RSI_UDMA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER

#define RSI_UDMA_DRV_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(02, 00) // driver version
#define RSI_UDMA_API_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(00, 01) // API version
#define UDMA_CHANNEL_NUM     32

#define M4SS_UDMA_INTR_SEL       (*((uint32_t volatile *)(0x46110000 + 0x0C)))
#define ULP_DYN_CLK_CTRL_DIS_REG (*((uint32_t volatile *)(0x24041400 + 0xA0)))
#define ULP_UDMA_STATIC_CLK_EN   (1UL << 17)
#define PERIPHERAL_UDMA_DMA_SEL  (*((uint32_t volatile *)(0x46008000UL + 0x58)))

#define UDMA_ATTR_USEBURST      0x1
#define UDMA_ATTR_ALTSELECT     0x2
#define UDMA_ATTR_HIGH_PRIORITY 0x4
#define UDMA_ATTR_REQMASK       0x8
#define UDMA_ATTR_ALL           0xF

// DMA control modes

#define CHNL0_INTR  (1UL << 0)  // Channel0 interrupt flag
#define CHNL1_INTR  (1UL << 1)  // Channel1 interrupt flag
#define CHNL2_INTR  (1UL << 2)  // Channel2 interrupt flag
#define CHNL3_INTR  (1UL << 3)  // Channel3 interrupt flag
#define CHNL4_INTR  (1UL << 4)  // Channel4 interrupt flag
#define CHNL5_INTR  (1UL << 5)  // Channel5 interrupt flag
#define CHNL6_INTR  (1UL << 6)  // Channel6 interrupt flag
#define CHNL7_INTR  (1UL << 7)  // Channel7 interrupt flag
#define CHNL8_INTR  (1UL << 8)  // Channel8 interrupt flag
#define CHNL9_INTR  (1UL << 9)  // Channel9 interrupt flag
#define CHNL10_INTR (1UL << 10) // Channel10 interrupt flag
#define CHNL11_INTR (1UL << 11) // Channel11 interrupt flag
#define CHNL12_INTR (1UL << 12) // Channel12 interrupt flag
#define CHNL13_INTR (1UL << 13) // Channel13 interrupt flag
#define CHNL14_INTR (1UL << 14) // Channel14 interrupt flag
#define CHNL15_INTR (1UL << 15) // Channel15 interrupt flag
#define CHNL16_INTR (1UL << 16) // Channel16 interrupt flag
#define CHNL17_INTR (1UL << 17) // Channel17 interrupt flag
#define CHNL18_INTR (1UL << 18) // Channel18 interrupt flag
#define CHNL19_INTR (1UL << 19) // Channel19 interrupt flag
#define CHNL20_INTR (1UL << 20) // Channel20 interrupt flag
#define CHNL21_INTR (1UL << 21) // Channel21 interrupt flag
#define CHNL22_INTR (1UL << 22) // Channel22 interrupt flag
#define CHNL23_INTR (1UL << 23) // Channel23 interrupt flag
#define CHNL24_INTR (1UL << 24) // Channel24 interrupt flag
#define CHNL25_INTR (1UL << 25) // Channel25 interrupt flag
#define CHNL26_INTR (1UL << 26) // Channel26 interrupt flag
#define CHNL27_INTR (1UL << 27) // Channel27 interrupt flag
#define CHNL28_INTR (1UL << 28) // Channel28 interrupt flag
#define CHNL29_INTR (1UL << 29) // Channel29 interrupt flag
#define CHNL30_INTR (1UL << 30) // Channel30 interrupt flag
#define CHNL31_INTR (1UL << 31) // Channel31 interrupt flag

#define UDMA_MODE_STOP                   0x0
#define UDMA_MODE_BASIC                  0x1
#define UDMA_MODE_AUTO                   0x2
#define UDMA_MODE_PINGPONG               0x3
#define UDMA_MODE_MEM_SCATTER_GATHER     0x4
#define UDMA_MODE_MEM_ALT_SCATTER_GATHER 0x5
#define UDMA_MODE_PER_SCATTER_GATHER     0x6
#define UDMA_MODE_ALT_SELECT             0x1
#define UDMA_SOFTWARE_TRIGG              0X2

#define UDMA_DST_INC_NONE 0x3
#define UDMA_SRC_INC_NONE 0x3

#define UDMA_PRI_SELECT    0x00
#define UDMA_ALT_SELECT    0x20
#define UDMA_ULPALT_SELECT 0x10

#define UDMA_CHCTL_XFERSIZE_M 0x3FF
#define UDMA_CHCTL_XFERMODE_M 0x07
#define UDMA_CHCTL_XFERSIZE_S 0x04

#define SRC_SIZE_32 0x2
#define SRC_SIZE_16 0x1
#define SRC_SIZE_8  0x0

#define DST_SIZE_32 0x2
#define DST_SIZE_16 0x1
#define DST_SIZE_8  0x0

#define SRC_INC_NONE 0x3
#define SRC_INC_32   0x2
#define SRC_INC_16   0x1
#define SRC_INC_8    0x0

#define DST_INC_NONE 0x3
#define DST_INC_32   0x2
#define DST_INC_16   0x1
#define DST_INC_8    0x0

#define ARBSIZE_1    0x00
#define ARBSIZE_2    0x01
#define ARBSIZE_4    0x02
#define ARBSIZE_8    0x03
#define ARBSIZE_16   0x04
#define ARBSIZE_32   0x05
#define ARBSIZE_64   0x06
#define ARBSIZE_128  0x07
#define ARBSIZE_256  0x08
#define ARBSIZE_512  0x09
#define ARBSIZE_1024 0x0A

#define DMA_XFERS_1    0x0
#define DMA_XFERS_2    0x1
#define DMA_XFERS_3    0x2
#define DMA_XFERS_4    0x3
#define DMA_XFERS_5    0x4
#define DMA_XFERS_6    0x5
#define DMA_XFERS_7    0x6
#define DMA_XFERS_8    0x7
#define DMA_XFERS_9    0x8
#define DMA_XFERS_10   0x9
#define DMA_XFERS_11   0xA
#define DMA_XFERS_16   0xF
#define DMA_XFERS_31   0x1F
#define DMA_XFERS_101  0x64
#define DMA_XFERS_256  0xFF
#define DMA_XFERS_1024 0x3FF

#define USART0_ACK    0x1
#define UART1_ACK     0x2
#define UART3_ACK     0x3
#define SSISLAVE_ACK  0x4
#define SSIMASTER_ACK 0x5
#define SSISLAVE1_ACK 0x6
#define I2C_ACK       0x7
#define CHNL_0        0
#define CHNL_31       31

#define PRIMARY   0
#define ALTERNATE 1
#define CHNL_0    0
#define CHNL_1    1

#define SRC_SIZE_32 0x2
#define SRC_SIZE_16 0x1
#define SRC_SIZE_8  0x0

#define DST_SIZE_32 0x2
#define DST_SIZE_16 0x1
#define DST_SIZE_8  0x0

#define SRC_INC_32 0x2
#define SRC_INC_16 0x1
#define SRC_INC_8  0x0

#define DST_INC_32 0x2
#define DST_INC_16 0x1
#define DST_INC_8  0x0

#define SRC_DST_SIZE_32 2
#define SRC_DST_SIZE_16 1
#define SRC_DTS_SIZE_8  0

#define MEM_SG  0
#define PERI_SG 1

typedef UDMA0_Type RSI_UDMA_T;

// brief RPDMA handle type
typedef void *RSI_UDMA_HANDLE_T;

// brief UDMA Driver Capabilities.
typedef struct {
  uint32_t noOfChannels;         // Number of DMA channels
  uint32_t maxNoOfTxferPerCycle; // Maximum number of transfers per DMA cycle
} RSI_UDMA_CAPABILITIES_T;

// brief  UDMA Channel Config structure
typedef struct {
  unsigned int transferType : 3; // The operating mode of the DMA cycle
  unsigned int
    nextBurst : 1; // Used to force the channel to only respond to burst requests at the tail end of a scatter-gather transfer
  unsigned int totalNumOfDMATrans : 10; // total number of DMA transfers that the DMA cycle contains
  unsigned int rPower : 4;              // Number of DMA transfers can occur before the controller rearbitrates
  unsigned int srcProtCtrl : 3;         // Performs control operation when the controller reads the source data
  unsigned int dstProtCtrl : 3;         // Performs control operation when the controller writes the destination data
  unsigned int srcSize : 2;             // Source data size
  unsigned int srcInc : 2;              // Source address increment
  unsigned int dstSize : 2;             // Destination data size
  unsigned int dstInc : 2;              // Destination address increment
} RSI_UDMA_CHA_CONFIG_DATA_T;

// brief  UDMA Control Structure.
typedef struct {
  volatile void *pSrcEndAddr;                               // The ending source address of the data transfer
  volatile void *pDstEndAddr;                               // The ending destination address of the data transfer
  volatile RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData1; // The Channel Config Structure
  volatile uint32_t Spare;                                  // An unused location
} RSI_UDMA_DESC_T;

typedef void (*udmaTransferCompleteCB)(RSI_UDMA_HANDLE_T udmaHandle, RSI_UDMA_DESC_T *pTranDesc, uint32_t dmaCh);

// brief RPDMA controller callback IDs
typedef enum {
  ROM_UDMA_XFERCOMPLETE_CB = 0, // Callback ID for UDMA transfer descriptor chain complete
} ROM_UDMA_CALLBACK_T;

// Private data structure used for the RPDMA controller driver, holds the driver and peripheral context
typedef struct {
  void *pUserData;                   // Pointer to user data used by driver instance, use NULL if not used
  RSI_UDMA_T *base;                  // UDMA base address
  RSI_UDMA_DESC_T *sramBase;         // SRAM descriptor table (all channels)
  RSI_UDMA_DESC_T *desc;             // descriptor structure
  udmaTransferCompleteCB udmaCompCB; // Transfer complete callback
  uint32_t dmaCh;
} RSI_UDMA_DATACONTEXT_T;

// RPDMA Init structure
typedef PRE_PACK struct POST_PACK {
  void *pUserData;   // Pointer to user data used by driver instance, use NULL if not used
  uint32_t base;     // Pointer to RPDMA global register instance
  uint32_t sramBase; // Pointer to memory used for RPDMA descriptor storage, must be 512 byte aligned
} RSI_UDMA_INIT_T;

// brief RPDMA transfer channel setup structure (use this structure as const if possible)
typedef PRE_PACK struct POST_PACK {
  uint32_t channelPrioHigh; // Channel priority level
  uint32_t altStruct;       // Primary or alternate control structure
  uint32_t burstReq;        // Burst request
  uint32_t reqMask;         // Mask channel request
  uint32_t periphReq;       // Peripheral request
  uint32_t periAck;         // dma ACK for peripheral
  uint32_t dmaCh;
} RSI_UDMA_CHA_CFG_T;
/** @addtogroup SOC18
* @{
*/

/*===================================================*/
/**
 * @fn          uint32_t RSI_UDMA_GetMemSize(void)
 * @brief		This API is used to Get memory size in bytes needed for UDMA controller driver context
 * @return 		Size in bytes needed for the ROM driver
 */
STATIC INLINE uint32_t RSI_UDMA_GetMemSize(void)
{
  return sizeof(RSI_UDMA_DATACONTEXT_T);
}

/*===================================================*/
/**
 * @fn          void RSI_UDMA_RegisterCallback(RSI_UDMA_HANDLE_T pHandle, void *pCB)
 * @brief		Registers an DMA controller callback for a queue
 * @param[in]   pHandle	 :  Pointer to driver context handle
 * @param[in]   pCB		 :  Pointer to callback function
 * @return 		none
 */
STATIC INLINE void RSI_UDMA_RegisterCallback(RSI_UDMA_HANDLE_T pHandle, udmaTransferCompleteCB pCB)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  pDrv->udmaCompCB = pCB;
}

/*===================================================*/
/**
 * @fn          void RSI_UDMA_UDMAEnable(RSI_UDMA_HANDLE_T pHandle)
 * @brief		This API is used to enable the UDMA interface 
 * @param[in]	pHandle	: Pointer to driver context handle
 * @return 		none 
 */
STATIC INLINE void RSI_UDMA_UDMAEnable(RSI_UDMA_HANDLE_T pHandle)
{
  RSI_UDMA_DATACONTEXT_T *pDrv        = (RSI_UDMA_DATACONTEXT_T *)pHandle;
  pDrv->base->DMA_CFG_b.MASTER_ENABLE = ENABLE;
}

/*===================================================*/
/**
 * @fn          void RSI_UDMA_UDMADisable(RSI_UDMA_HANDLE_T pHandle)
 * @brief		This API is used to disable the UDMA interface 
 * @param[in]	pHandle	: Pointer to driver context handle
 * @return 		none
 */

STATIC INLINE void RSI_UDMA_UDMADisable(RSI_UDMA_HANDLE_T pHandle)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;

  pDrv->base->DMA_CFG_b.MASTER_ENABLE = DISABLE;
}

/*===================================================*/
/**
 * @fn          uint8_t RSI_UDMA_ErrorStatusGet(const RSI_UDMA_T *pUDMA)
 * @brief		This API is used to get the error status/sets the signal low of UDMA.
 * @param[in]	pUDMA     : Pointer to the UDMA instance register area
 * @return 		Returns error status as below
 *              - 0 : Error is LOW
 *              - 1 : Error is HIGH
 */
STATIC INLINE uint8_t RSI_UDMA_ErrorStatusGet(const RSI_UDMA_T *pUDMA)
{
  return (pUDMA->ERR_CLR_b.ERR_CLR);
}
/*===================================================*/
/**
 * @fn          void RSI_UDMA_ErrorStatusClear(RSI_UDMA_HANDLE_T pHandle)
 * @brief		This API is used to clear the errors of UDMA 
 * @param[in]	pHandle	: Pointer to driver context handle
 * @return 		none
 */
STATIC INLINE void RSI_UDMA_ErrorStatusClear(RSI_UDMA_HANDLE_T pHandle)
{
  RSI_UDMA_DATACONTEXT_T *pDrv  = (RSI_UDMA_DATACONTEXT_T *)pHandle;
  pDrv->base->ERR_CLR_b.ERR_CLR = 0x1;
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_UDMA_ChannelEnable(RSI_UDMA_HANDLE_T pHandle,uint8_t dmaCh) 
 * @brief		  This API is used to enable the required channel of UDMA
 * @param[in]	  pHandle	: Pointer to driver context handle
 * @param[in]	  dmaCh	  : Channel number(1 to 32)
 * @return 		  RSI_OK if no errors occured, or an error code    
 */
STATIC INLINE rsi_error_t RSI_UDMA_ChannelEnable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;
  if (dmaCh <= CHNL_31) {
    // Enables required channel
    pDrv->base->CHNL_ENABLE_SET = SET_BIT(dmaCh);
    return RSI_OK;
  } else {
    return ERROR_UDMA_INVALIDCHNLNUM;
  }
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_UDMA_ChannelDisable(RSI_UDMA_HANDLE_T pHandle,uint8_t dmaCh)
 * @brief		  This API is used to disable the required channel of UDMA
 * @param[in]	  pHandle : Pointer to driver context handle
 * @param[in]	  dmaCh	  : Channel number(1 to 32)
 * @return 		  RSI_OK if no errors occured, or an error code    
 */
STATIC INLINE rsi_error_t RSI_UDMA_ChannelDisable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;
  if (dmaCh <= CHNL_31) {
    // Disables required channel
    pDrv->base->CHNL_ENABLE_CLR = SET_BIT(dmaCh);
    return RSI_OK;
  } else
    return ERROR_UDMA_INVALIDCHNLNUM;
}

/*===================================================*/
/**
 * @fn          rsi_error_t RSI_UDMA_ChannelIsEnabled(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
 * @brief		This API is used to check the required UDMA channel is enabled or not
 * @param[in]	pHandle	 : Pointer to driver context handle
 * @param[in]	dmaCh	 : Channel number(0 to 31)
 * @return 		If process is successful then return RSI_OK  else RSI_FAIL If requied channel is not enabled
 */
STATIC INLINE rsi_error_t RSI_UDMA_ChannelIsEnabled(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
  const RSI_UDMA_DATACONTEXT_T *pDrv = (const RSI_UDMA_DATACONTEXT_T *)pHandle;
  if (dmaCh <= CHNL_31) {
    if (((pDrv->base->CHNL_ENABLE_SET) & SET_BIT(dmaCh)) != 0) {
      return RSI_OK;
    } else {
      return RSI_FAIL;
    }
  } else {
    return ERROR_UDMA_INVALIDCHNLNUM;
  }
}

/*===================================================*/
/**
 * @fn          void *RSI_UDMA_GetControlBaseAddress(RSI_UDMA_HANDLE_T pHandle)
 * @brief		This API is used to get the UDMA control base address
 * @param[in]   pHandle	 : Pointer to driver context handle
 * @return 		Control structure base pointer
 */
STATIC INLINE void *RSI_UDMA_GetControlBaseAddress(RSI_UDMA_HANDLE_T pHandle)
{
  const RSI_UDMA_DATACONTEXT_T *pDrv = (const RSI_UDMA_DATACONTEXT_T *)pHandle;
  return ((void *)pDrv->base->CTRL_BASE_PTR);
}

/*===================================================*/
/**
 * @fn          void *RSI_UDMA_GetControlAlternateBase(RSI_UDMA_HANDLE_T pHandle)
 * @brief		This API is used to get the required UDMA channel alternate control base
 * @param[in]   pHandle	 : Pointer to driver context handle
 * @return 		Alternate control structure base pointer
 */
STATIC INLINE void *RSI_UDMA_GetControlAlternateBase(RSI_UDMA_HANDLE_T pHandle)
{
  const RSI_UDMA_DATACONTEXT_T *pDrv = (const RSI_UDMA_DATACONTEXT_T *)pHandle;
  return ((void *)pDrv->base->ALT_CTRL_BASE_PTR);
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_UDMA_ChannelSoftwareTrigger(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
 * @brief		  This API is used to to generate a software UDMA request on the corresponding UDMA channel 
 * @param[in]	  pHandle	:  Pointer to driver context handle
 * @param[in]	  dmaCh   : channel number (0 to 31).
 * @return        return ERROR_UDMA_INVALID_ARG if channel is  greater than 31 or channel is invalid , and on success return RSI_OK(0).
 */
STATIC INLINE rsi_error_t RSI_UDMA_ChannelSoftwareTrigger(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;
  if (dmaCh <= CHNL_31) {
    pDrv->base->CHNL_SW_REQUEST |= SET_BIT(dmaCh);
    return RSI_OK;
  } else
    return ERROR_UDMA_INVALIDCHNLNUM;
}

/*===================================================*/
/**
 * @fn             rsi_error_t RSI_UDMA_InterruptClear(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
 * @brief          This API is used to to generate a  UDMA interrupt clear. 
 * @param[in]	   pHandle :  Pointer to driver context handle
 * @param[in]	   dmaCh   :       channel number (0 to 31).
 * @return 	       return ERROR_UDMA_INVALID_ARG if channel is  greater than 31 or channel is invalid , 
 *                 and on success return RSI_OK(0). 
 */
STATIC INLINE rsi_error_t RSI_UDMA_InterruptClear(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
  RSI_UDMA_DATACONTEXT_T *pDrv = (RSI_UDMA_DATACONTEXT_T *)pHandle;
  if (dmaCh <= CHNL_31) {
    pDrv->base->UDMA_DONE_STATUS_REG = SET_BIT(dmaCh);
  } else {
    return ERROR_UDMA_INVALIDCHNLNUM;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            uint32_t RSI_UDMA_InterruptStatus(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
 * @brief		  Gets interrupt status of required channel.
 * @param[in]	  pHandle	:  Pointer to driver context handle
 * @param[in]	  dmaCh   : Channel number(0 to 31),Possible values are as below
                             - \b 0 : Channel disable
                             - \b 1 : Channel enable	
 * @return 		  Interrupt status of required channel 
 */
STATIC INLINE uint32_t RSI_UDMA_InterruptStatus(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
  uint32_t intr_stat                 = 0;
  const RSI_UDMA_DATACONTEXT_T *pDrv = (const RSI_UDMA_DATACONTEXT_T *)pHandle;

  intr_stat = pDrv->base->UDMA_DONE_STATUS_REG;
  return (intr_stat & SET_BIT(dmaCh));
}

/*===================================================*/
/**
 * @fn            uint8_t RSI_UDMA_GetControllerEnableStatus(RSI_UDMA_HANDLE_T pHandle)
 * @brief		  This API is used to get the enable status of controller
 * @param[in]	  pHandle : Pointer to driver context handle
 * @return 		  If 1 controller is enabled ,if 0 controller is disabled
 */
STATIC INLINE uint8_t RSI_UDMA_GetControllerEnableStatus(RSI_UDMA_HANDLE_T pHandle)
{
  const RSI_UDMA_DATACONTEXT_T *pDrv = (const RSI_UDMA_DATACONTEXT_T *)pHandle;
  return (pDrv->base->DMA_STATUS_b.MASTER_ENABLE);
}

/*===================================================*/
/**
 * @fn          uint32_t RSI_UDMA_GetControlState(RSI_UDMA_HANDLE_T pHandle)
 * @brief		This API is used to get the current state of the control state machine
 * @param[in]	pHandle	:  Pointer to driver context handle
 * @return 		returns state of the control state machine
									possible values are as below
									- 0000 = idle
									- 0001 = reading channel controller data
									- 0010 = reading source data end pointer
									- 0011 = reading destination data end pointer
									- 0100 = reading source data
									- 0101 = writing destination data
									- 0110 = waiting for DMA request to clear
									- 0111 = writing channel controller data
									- 1000 = stalled
									- 1001 = done
									- 1010 = peripheral scatter-gather transition
									- 1011-1111 = undefined.
 */
STATIC INLINE uint32_t RSI_UDMA_GetControlState(RSI_UDMA_HANDLE_T pHandle)
{
  const RSI_UDMA_DATACONTEXT_T *pDrv = (const RSI_UDMA_DATACONTEXT_T *)pHandle;
  return (pDrv->base->DMA_STATUS_b.STATE);
}

// FUNCTION PROTOTYPES
RSI_DRIVER_VERSION_M4 RSI_UDMA_GetVersion(void);

RSI_UDMA_CAPABILITIES_T RSI_UDMA_GetCapabilities(void);

RSI_UDMA_HANDLE_T udma_init(void *mem, const RSI_UDMA_INIT_T *pInit);

uint32_t udma_get_channel_transfer_mode(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

rsi_error_t udma_setup_channel_transfer(RSI_UDMA_HANDLE_T pHandle,
                                        const RSI_UDMA_CHA_CFG_T *pCfg,
                                        RSI_UDMA_CHA_CONFIG_DATA_T vsUdmaChaConfigData,
                                        void *pSrcAddr,
                                        volatile void *pDstAddr);

rsi_error_t udma_set_channel_scatter_gather_transfer(RSI_UDMA_HANDLE_T pHandle,
                                                     uint8_t dmaCh,
                                                     uint32_t taskCount,
                                                     void *pTaskList,
                                                     uint32_t transferType);

uint32_t udma_get_channel_transfer_length(RSI_UDMA_HANDLE_T pHandle,
                                          const RSI_UDMA_CHA_CFG_T *pCfg,
                                          RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData);

rsi_error_t udma_setup_channel(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

void udma_deInit(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

void udma_interrupt_handler(RSI_UDMA_HANDLE_T pHandle);

rsi_error_t udma_interrupt_enable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh);
rsi_error_t RSI_UDMA_ChannelControlsDisable(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);
void RSI_UDMA_SetSingleRequest(RSI_UDMA_HANDLE_T pHandle);
void RSI_UDMA_AckEnable(const void *pHandle, uint32_t peripheral);

#ifdef __cplusplus
}
#endif

#endif // RSI_UDMA_H
/** @} */
