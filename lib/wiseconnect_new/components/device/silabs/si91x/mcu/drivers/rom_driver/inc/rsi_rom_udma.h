/******************************************************************************
* @file  rsi_rom_udma.h
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

#ifndef __RSI_ROM_UDMA_H__
#define __RSI_ROM_UDMA_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_UDMA_DRIVER
 *  @{
 *
 */
#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn         STATIC INLINE rsi_error_t RSI_UDMA_SetupChannelTransfer( RSI_UDMA_HANDLE_T pHandle, RSI_UDMA_CHA_CFG_T *pCfg,
																			                RSI_UDMA_CHA_CONFIG_DATA_T vsUdmaChaConfigData,
                                                      void *pSrcAddr, void *pDstAddr )
 * @brief		   This API is used to control parameters for a UDMA channel control structure.
 * @param[in]	 pHandle	            : Pointer to driver context handle
 * @param[in]	 pCfg	                : Pointer to channel configuration structure \ref RSI_UDMA_CHA_CFG_T
 * @param[in]  vsUdmaChaConfigData  :structure variable of type RSI_UDMA_CHA_CONFIG_DATA ,structure has below members
	                                 to configure and possible values
                                   -  transferType : The operating mode of the DMA cycle
	                                                          - \ref UDMA_MODE_STOP
	                                                          - \ref UDMA_MODE_BASIC
	                                                          - \ref UDMA_MODE_AUTO
	                                                          - \ref UDMA_MODE_PINGPONG
	                                                          - \ref UDMA_MODE_MEM_SCATTER_GATHER
	                                                          - \ref UDMA_MODE_PER_SCATTER_GATHER
                                   -  nextBurst : Controls if the chnl_useburst_set[0] bit is set to a 1
	                                                    when the controller is performing a peripheral scatter-gather
	                                                    and is completing a DMA cycle that uses the alternate data
																											structure
	                                                          -  0 : the controller does not change the value of the
	                                                                   chnl_useburst_set[0]bit
	                                                          -  1 : the controller sets the chnl_useburst_set [0] bit
	                                                                   to a 1
                                   -  totalNumOfDMATrans : Represent the total number of DMA transfers that
	                                                             the DMA cycle contains
	                                                             \n possible values are 0x00 to 0x1FF for
	                                                                1 to 1024 transfers
                                   -  rPower : The arbitration size determines how many items are
	                                                 transferred before the uDMA controller re-arbitrates for
	                                                 the bus.  Choose the arbitration size from one of
	                                                          - \ref ARBSIZE_1
	                                                          - \ref ARBSIZE_2
	                                                          - \ref ARBSIZE_4
	                                                          - \ref ARBSIZE_16
	                                                          - \ref ARBSIZE_64
	                                                          - \ref ARBSIZE_128
	                                                          - \ref ARBSIZE_256
	                                                          - \ref ARBSIZE_512
	                                                          - \ref ARBSIZE_1024
	                                                          \n to select the arbitration size from 1 to 1024 items,
	                                                          in powers of 2
                                   -  srcProtCtrl : Set the bits to control the state of HPROT[3:1]
	                                                      when the controller reads the source data.
                                   -  dstProtCtrl : Set the bits to control the state of HPROT[3:1] when
	                                                      the controller writes the destination data.
                                   -  srcSize : Choose the source data size from one of
              	                                            - \ref SRC_SIZE_8
	                                                          - \ref SRC_SIZE_16
	                                                          - \ref SRC_SIZE_32
	                                                          \n to select a data size of 8, 16, or 32 bits
                                   -  dstSize : Choose the destination data size from one of
              	                                            - \ref DST_SIZE_8
	                                                          - \ref DST_SIZE_16
	                                                          - \ref DST_SIZE_32
	                                                          \n to select a data size of 8, 16, or 32 bits
                                   -  srcInc : Choose the source address increment from one of
              	                                            - \ref SRC_INC_8
	                                                          - \ref SRC_INC_16
	                                                          - \ref SRC_INC_32
	                                                          - \ref SRC_INC_NONE
	                                                          \n to select a data size of 8, 16, or 32 bits
 	                                                          half-words, 32-bit words, or to select non-incrementing.
                                   -  dstInc : Choose the destination address increment from one of
              	                                            - \ref DST_INC_8
	                                                          - \ref DST_INC_8
	                                                          - \ref DST_INC_8
	                                                          - \ref DST_INC_NONE
	                                                          \n to select a data size of 8, 16, or 32 bits
 	                                                          half-words, 32-bit words, or to select non-incrementing.
 * @param[in]  pSrcAddr : Pointer to source address
 * @param[in]  pDstAddr : Pointer to destination address
 * @return 		  \ref RSI_OK if no errors occurred, or an error code
 */
STATIC INLINE rsi_error_t RSI_UDMA_SetupChannelTransfer(RSI_UDMA_HANDLE_T pHandle,
                                                        const RSI_UDMA_CHA_CFG_T *pCfg,
                                                        RSI_UDMA_CHA_CONFIG_DATA_T vsUdmaChaConfigData,
                                                        void *pSrcAddr,
                                                        void *pDstAddr)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  return ROMAPI_UDMA_API->udma_setup_channel_transfer(pHandle, pCfg, vsUdmaChaConfigData, pSrcAddr, pDstAddr);
#else
  return udma_setup_channel_transfer(pHandle, pCfg, vsUdmaChaConfigData, pSrcAddr, pDstAddr);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_UDMA_SetChannelScatterGatherTransfer(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh,
                                                                 uint8_t taskCount,
                                                                 void *pTaskList, uint32_t transferType)
 * @brief		    This API is used to configures a UDMA channel for scatter-gather mode
 * @param[in]	  pHandle	     :  Pointer to driver context handle
 * @param[in]	  dmaCh	       :  Channel number(0 to 31)
 * @param[in]	  taskCount    :  Number of scatter-gather tasks to execute
 * @param[in]	  pTaskList    :  pointer to the beginning of the scatter-gather task list
 * @param[in]	  transferType :  Transfer type
 * @return 		  return \ref ERROR_UDMA_INVALID_ARG if channel is  greater than 31, and on success return \ref RSI_OK
 */
STATIC INLINE rsi_error_t RSI_UDMA_SetChannelScatterGatherTransfer(RSI_UDMA_HANDLE_T pHandle,
                                                                   uint8_t dmaCh,
                                                                   uint8_t taskCount,
                                                                   void *pTaskList,
                                                                   uint32_t transferType)

{
#if defined(UDMA_ROMDRIVER_PRESENT)
  return ROMAPI_UDMA_API->udma_set_channel_scatter_gather_transfer(pHandle, dmaCh, taskCount, pTaskList, transferType);

#else
  return udma_set_channel_scatter_gather_transfer(pHandle, dmaCh, taskCount, pTaskList, transferType);
#endif
}

/**
 * @fn          STATIC INLINE uint32_t  RSI_UDMA_GetChannelTransferLength(RSI_UDMA_HANDLE_T pHandle, RSI_UDMA_CHA_CFG_T *pCfg,
																					                 RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData)

 * @brief		    Gets the current transfer size for a UDMA channel control structure
 * @param[in]	  pHandle	:  Pointer to driver context handle
 * @param[in]	  pCfg	  :  Pointer to channel configuration structure \ref RSI_UDMA_CHA_CFG_T
 * @param[in]   vsUDMAChaConfigData  :   structure variable of type \ref RSI_UDMA_CHA_CONFIG_DATA
 * @return 		  Number of items remaining to transfer and all transmission is complete then return RSI_OK(0)
 * @note        This function is used to get the UDMA transfer size for a channel.
								The transfer size is the number of items to transfer, where the size of an item
								might be 8, 16, or 32 bits.  If a partial transfer has already occurred,
								then the number of remaining items is returned.  If the transfer is
								complete, then 0 is returned.
 */
STATIC INLINE uint32_t RSI_UDMA_GetChannelTransferLength(RSI_UDMA_HANDLE_T pHandle,
                                                         const RSI_UDMA_CHA_CFG_T *pCfg,
                                                         RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  return ROMAPI_UDMA_API->udma_get_channel_transfer_length(pHandle, pCfg, vsUDMAChaConfigData);
#else
  return udma_get_channel_transfer_length(pHandle, pCfg, vsUDMAChaConfigData);
#endif
}
/**

 * @fn          STATIC INLINE uint32_t  RSI_UDMA_GetChannelTransferMode(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
 * @brief		    Gets the transfer mode for a UDMA channel control structure.
 * @param[in]	   pHandle	:  Pointer to driver context handle
 * @param[in]   pCfg	  : Pointer to DMA channel configuration structure \RSI_UDMA_CHA_CFG_T required parameter below
                          - \ref channelPrioHigh : channel priority level.
                          - \ref altStruct       : alternate structure if 1 then enable alternate structure
                                                   if 0 then primary structure
                          - \ref burstReq        : Burst request
                          - \ref reqMask         : mask the specific channel.
                          - \ref periphReq       : peripheral request like i2c ,i2s etc
                          - \ref periAck         : peripheral ack like i2c ,i2s etc
                          - \ref dmaCh           :dma channel number(0-31)
 * @return 		  Returns the transfer mode of the specified channel and control structure,possible value is below
                          - \ref UDMA_MODE_STOP
                          - \ref UDMA_MODE_BASIC
                          - \ref UDMA_MODE_AUTO
                          - \ref UDMA_MODE_PINGPONG
                          - \ref UDMA_MODE_MEM_SCATTER_GATHER
                          - \ref UDMA_MODE_PER_SCATTER_GATHER
 */

STATIC INLINE uint32_t RSI_UDMA_GetChannelTransferMode(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  return ROMAPI_UDMA_API->udma_get_channel_transfer_mode(pHandle, pCfg);
#else
  return udma_get_channel_transfer_mode(pHandle, pCfg);
#endif
}
/**
 * @fn          RSI_UDMA_HANDLE_T RSI_UDMA_Init(void *mem, const RSI_UDMA_INIT_T *pInit)

 * @brief		    This API is used to initialize driver context parameters
 * @param[in]   pInit	  : Pointer to DMA controller init data
 * @param[in]   mem		  : Pointer to memory area used to driver context
 * @return 		  NULL on error, or a pointer to the device context handle
 */
STATIC INLINE RSI_UDMA_HANDLE_T RSI_UDMA_Init(void *mem, const RSI_UDMA_INIT_T *pInit)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  return ROMAPI_UDMA_API->udma_init(mem, pInit);
#else
  return udma_init(mem, pInit);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_UDMA_SetupChannel(RSI_UDMA_HANDLE_T pHandle, RSI_UDMA_CHA_CFG_T *pCfg)

 * @brief		    This API is used to configures required parameters for a channel
 * @param[in]   pHandle	: Pointer to driver context handle
 * @param[in]   pCfg	  : Pointer to DMA channel configuration structure \RSI_UDMA_CHA_CFG_T required parameter below
                          - \ref channelPrioHigh : channel priority level.
                          - \ref altStruct       : alternate structure if 1 then enable alternate structure
                                                   if 0 then primary structure
                          - \ref burstReq        : Burst request
                          - \ref reqMask         : mask the specific channel.
                          - \ref periphReq       : peripheral request like i2c ,i2s etc
                          - \ref periAck         : peripheral ack like i2c ,i2s etc
                          - \ref dmaCh           :dma channel number(0-31)
 * @return 		  - \ref  RSI_OK if no errors occurred, or an error code
 */
STATIC INLINE rsi_error_t RSI_UDMA_SetupChannel(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  return ROMAPI_UDMA_API->udma_setup_channel(pHandle, pCfg);
#else
  return udma_setup_channel(pHandle, pCfg);
#endif
}
/**
 * @fn          STATIC INLINE void  RSI_UDMA_DeInit(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
 * @brief		    This API is used to Uninitialized driver context parameters
 * @param[in]   pHandle	: Pointer to driver context handle
 * @param[in]   pCfg	  : Pointer to DMA channel configuration structure
 * @return 		  none
 */
STATIC INLINE void RSI_UDMA_DeInit(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  ROMAPI_UDMA_API->udma_deInit(pHandle, pCfg);
#else
  udma_deInit(pHandle, pCfg);
#endif
}
/**
 * @fn          STATIC INLINE void  RSI_UDMA_Interrupthandler(RSI_UDMA_HANDLE_T pHandle)
 * @brief		    This API is used to handle all interrupt and error flags in interrupt context
 * @param[in]   pHandle	:  Pointer to driver context handle
 * @return 		  none
 * @note(s)     This function should be called from the DMA interrupt handler
 */

STATIC INLINE void RSI_UDMA_Interrupthandler(RSI_UDMA_HANDLE_T pHandle)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  ROMAPI_UDMA_API->udma_interrupt_handler(pHandle);
#else
  udma_interrupt_handler(pHandle);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_UDMA_InterruptEnable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
 * @brief		    This API is used to enable the done interrupt to processor
 * @param[in]	  pHandle	 : Pointer to driver context handle
 * @param[in]	  dmaCh    :  Channel number(0 to 31)
 * @return 		  return \ref ERROR_UDMA_INVALID_ARG if channel is  greater than 31, and on success return \ref RSI_OK
 */
STATIC INLINE rsi_error_t RSI_UDMA_InterruptEnable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh)
{
#if defined(UDMA_ROMDRIVER_PRESENT)
  return ROMAPI_UDMA_API->udma_interrupt_enable(pHandle, dmaCh);
#else
  return udma_interrupt_enable(pHandle, dmaCh);
#endif
}
/* @} end of  RSI_UDMA_DRIVERS */
/**************************************************************************************
								              UDMA ROM FUNCTION PROTOTYPES											  
		 **************************************************************************************/
STATIC INLINE rsi_error_t RSI_UDMA_SetupChannelTransfer(RSI_UDMA_HANDLE_T pHandle,
                                                        const RSI_UDMA_CHA_CFG_T *pCfg,
                                                        RSI_UDMA_CHA_CONFIG_DATA_T vsUdmaChaConfigData,
                                                        void *pSrcAddr,
                                                        void *pDstAddr);

STATIC INLINE rsi_error_t RSI_UDMA_SetChannelScatterGatherTransfer(RSI_UDMA_HANDLE_T pHandle,
                                                                   uint8_t dmaCh,
                                                                   uint8_t taskCount,
                                                                   void *pTaskList,
                                                                   uint32_t transferType);

STATIC INLINE uint32_t RSI_UDMA_GetChannelTransferLength(RSI_UDMA_HANDLE_T pHandle,
                                                         const RSI_UDMA_CHA_CFG_T *pCfg,
                                                         RSI_UDMA_CHA_CONFIG_DATA_T vsUDMAChaConfigData);

STATIC INLINE uint32_t RSI_UDMA_GetChannelTransferMode(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

STATIC INLINE RSI_UDMA_HANDLE_T RSI_UDMA_Init(void *mem, const RSI_UDMA_INIT_T *pInit);

STATIC INLINE rsi_error_t RSI_UDMA_SetupChannel(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

static void RSI_UDMA_DeInit(RSI_UDMA_HANDLE_T pHandle, const RSI_UDMA_CHA_CFG_T *pCfg);

STATIC INLINE void RSI_UDMA_Interrupthandler(RSI_UDMA_HANDLE_T pHandle);

STATIC INLINE rsi_error_t RSI_UDMA_InterruptEnable(RSI_UDMA_HANDLE_T pHandle, uint8_t dmaCh);

#ifdef __cplusplus
}
#endif

#endif /* __RSI_ROM_UDMA_H__*/
