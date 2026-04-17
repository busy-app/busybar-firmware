/******************************************************************************
* @file  rsi_udma_wrapper.c
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
// Include Files

#include "rsi_ccp_user_config.h"
#if !defined(A11_ROM) || !defined(UDMA_ROMDRIVER_PRESENT)
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_rom_udma.h"
#include "UDMA.h"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER

#ifdef __cplusplus
extern "C" {
#endif
/** @addtogroup SOC18
* @{
*/
/*==============================================*/
/**
 * @fn          RSI_UDMA_HANDLE_T uDMAx_Initialize(const UDMA_RESOURCES *udma,
 *                                  RSI_UDMA_DESC_T *UDMA_Table,
 *                                  RSI_UDMA_HANDLE_T udmaHandle,
 *                                  uint32_t *mem)
 * @brief       This API is used to Initialize UDMA peripheral
 * @param[in]   udma       : Pointer to UDMA resources
 * @param[in]   UDMA_Table : Pointer to UDMA descriptor
 * @param[in]   udmaHandle : Pointer to UDMA Handle
 * @param[in]   mem        : Pointer to memory buffer
 * @return      Zero - if success
 *              Non zero - if fails
 */
RSI_UDMA_HANDLE_T uDMAx_Initialize(const UDMA_RESOURCES *udma,
                                   RSI_UDMA_DESC_T *UDMA_Table,
                                   RSI_UDMA_HANDLE_T udmaHandle,
                                   uint32_t *mem)
{
  RSI_UDMA_INIT_T udmaInit0 = { 0 };
  RSI_UDMA_INIT_T udmaInit1 = { 0 };

  if (udma->reg == UDMA0) {
#if defined(SLI_SI917)
    RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_EFUSE_PERI);
#else
    RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_PERI1);
#endif
    RSI_CLK_PeripheralClkEnable(M4CLK, UDMA_CLK, ENABLE_STATIC_CLK);
  }
  if (udma->reg == UDMA1) {
    RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_UDMA);
    RSI_ULPSS_PeripheralEnable(ULPCLK, ULP_UDMA_CLK, ENABLE_STATIC_CLK);
  }
  if (udma->reg == UDMA0) {
    udmaInit0.base      = (uint32_t)UDMA0;
    udmaInit0.sramBase  = (uint32_t)UDMA_Table;
    udmaInit0.pUserData = NULL;
    udmaHandle          = RSI_UDMA_Init(mem, &udmaInit0); //doubt in mem
    if (udmaHandle == NULL) {
      return 0;
    }
  }
  if (udma->reg == UDMA1) {
    udmaInit1.base = (uint32_t)UDMA1;

    if (M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS) {
      if (((uint32_t)UDMA_Table >= ULP_SRAM_START_ADDR) && ((uint32_t)UDMA_Table <= ULP_SRAM_END_ADDR)) {
        udmaInit1.sramBase = (uint32_t)UDMA_Table;
      } else {
        return 0;
      }
    } else {
      udmaInit1.sramBase = (uint32_t)UDMA_Table;
    }
    udmaInit1.pUserData = NULL;
    udmaHandle          = RSI_UDMA_Init(mem, &udmaInit1);
    if (udmaHandle == NULL) {
      return 0;
    }
  }
  // Clear all DMA interrupt flags
  NVIC_ClearPendingIRQ(udma->udma_irq_num);
  NVIC_EnableIRQ(udma->udma_irq_num);

  return udmaHandle;
}

/*==============================================*/
/**
 * @fn          int32_t uDMAx_Uninitialize(const UDMA_RESOURCES *udma)
 * @brief       This API is used to De-initialize UDMA peripheral
 * @param[in]   udma : Pointer to UDMA resources
 * @return      Zero - if success
 *              Non zero - if fails
 */
int32_t uDMAx_Uninitialize(const UDMA_RESOURCES *udma)
{
  // Disable DMA clock,Disable and Clear DMA IRQ
  if (udma->reg == UDMA0) {
    RSI_CLK_PeripheralClkDisable(M4CLK, UDMA_CLK);
  }
  if (udma->reg == UDMA1) {
    RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_UDMA);
    RSI_ULPSS_PeripheralDisable(ULPCLK, ULP_UDMA_CLK);
  }
  NVIC_DisableIRQ(udma->udma_irq_num);
  NVIC_ClearPendingIRQ(udma->udma_irq_num);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          int32_t uDMAx_ChannelConfigure(const UDMA_RESOURCES *udma,
 *                              uint8_t ch,
 *                              uint32_t src_addr,
 *                              uint32_t dest_addr,
 *                              uint32_t size,
 *                              RSI_UDMA_CHA_CONFIG_DATA_T control,
 *                              const RSI_UDMA_CHA_CFG_T *config,
 *                              UDMA_SignalEvent_t cb_event,
 *                              UDMA_Channel_Info *chnl_info,
 *                              RSI_UDMA_HANDLE_T udmaHandle)
 * @brief       This API is used to Configure UDMA channel for next transfer
 * @param[in]   udma      : Pointer to UDMA resources
 * @param[in]   ch        : Channel number (0..7)
 * @param[in]   src_addr  : Source address
 * @param[in]   dest_addr : Destination address
 * @param[in]   size      : Amount of data to transfer
 * @param[in]   control   : Channel control
 * @param[in]   config    : Channel configuration
 * @param[in]   cb_event  : Channel callback pointer
 * @param[in]   chnl_info : Pointer to UDMA channel info
 * @param[in]   udmaHandle: udma Handler
 * @return      Zero - if success
 *              Non zero - if fails
 */
int32_t uDMAx_ChannelConfigure(const UDMA_RESOURCES *udma,
                               uint8_t ch,
                               uint32_t src_addr,
                               uint32_t dest_addr,
                               uint32_t size,
                               RSI_UDMA_CHA_CONFIG_DATA_T control,
                               const RSI_UDMA_CHA_CFG_T *config,
                               UDMA_SignalEvent_t cb_event,
                               UDMA_Channel_Info *chnl_info,
                               RSI_UDMA_HANDLE_T udmaHandle)
{
  // Added below macro to avoid ULP memory validations when using SL_DMA with I2C and SSI in ULP mode,
  // as I2C and SSI data register does not belong to ULP memory section
#if !(defined SL_SI91X_I2C_DMA) && !(defined SL_SI91X_SSI_DMA) && !(defined DAC_FIFO_MODE_EN)
  if (M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS) { //PS2 state
    if ((control.dstInc == UDMA_DST_INC_NONE)
        && (!((src_addr >= ULP_SRAM_START_ADDR) && (src_addr <= ULP_SRAM_END_ADDR)))) {
      return ERROR_UDMA_SRC_ADDR;
    }
    if ((control.srcInc == UDMA_SRC_INC_NONE)
        && (!((dest_addr >= ULP_SRAM_START_ADDR) && (dest_addr <= ULP_SRAM_END_ADDR)))) {
      return ERROR_UDMA_DST_ADDR;
    }
  }
#endif
  if ((udma->reg == UDMA0) || (udma->reg == UDMA1)) {
    chnl_info[ch].cb_event = cb_event;
  }
  if ((udma->reg == UDMA0) || (udma->reg == UDMA1)) {
    // Clear DMA interrupts
    RSI_UDMA_InterruptClear(udmaHandle, ch);
    RSI_UDMA_ErrorStatusClear(udmaHandle);
    RSI_UDMA_SetupChannel(udmaHandle, config);

    RSI_UDMA_SetupChannelTransfer(udmaHandle, config, control, (void *)src_addr, (void *)dest_addr);
    // Save channel information
    chnl_info[ch].SrcAddr  = src_addr;
    chnl_info[ch].DestAddr = dest_addr;
    chnl_info[ch].Size     = size;
    chnl_info[ch].Cnt      = control.totalNumOfDMATrans + 1;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          int32_t uDMAx_ChannelEnable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
 * @brief       This API is used to Enable UDMA channel
 * @param[in]   ch    : Channel number (0..7)
 * @param[in]   udma  : Pointer to UDMA resources
 * @param[in]   udmaHandle : udma Handler
 * @return      Zero - if success
 *              Non zero - if fails
 */
int32_t uDMAx_ChannelEnable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
{
  UNUSED_PARAMETER(udma);
  RSI_UDMA_ChannelEnable(udmaHandle, ch);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          int32_t uDMAx_DMAEnable(const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
 * @brief       This API is used to Enable UDMA DMA
 * @param[in]   udma      : Pointer to UDMA resources
 * @param[in]   udmaHandle: udma Handler
 * @return      Zero - if success
 *              Non zero - if fails
 */
int32_t uDMAx_DMAEnable(const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
{
  UNUSED_PARAMETER(udma);
  RSI_UDMA_UDMAEnable(udmaHandle);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          int32_t uDMAx_ChannelDisable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
 * @brief       This API is used to Disable UDMA channel
 * @param[in]   ch : Channel number (0..7)
 * @param[in]   udma : Pointer to UDMA resources
 * @param[in]   udmaHandle : udma Handler
 * @return      Zero - if success
 *              Non zero - if fails
 */
int32_t uDMAx_ChannelDisable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
{
  UNUSED_PARAMETER(udma);
  RSI_UDMA_ChannelDisable(udmaHandle, ch);
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          uint32_t uDMAx_ChannelGetCount(uint8_t ch,
 *                              RSI_UDMA_CHA_CONFIG_DATA_T control,
 *                              RSI_UDMA_CHA_CFG_T config,
 *                              const UDMA_RESOURCES *udma,
 *                              RSI_UDMA_HANDLE_T udmaHandle)                                       
 * @brief       This API Gets the current transfer size for a UDMA channel control structure
 * @param[in]	ch	    : Channel number
 * @param[in]	control : structure variable of type RSI_UDMA_CHA_CONFIG_DATA
 * @param[in]   config  : structure variable of type RSI_UDMA_CHA_CFG_T
 * @param[in]   udma    : Pointer to UDMA resources
 * @param[in]   udmaHandle : UDMA Handle
 * @return      RSI_OK - if success
 */
uint32_t uDMAx_ChannelGetCount(uint8_t ch,
                               RSI_UDMA_CHA_CONFIG_DATA_T control,
                               RSI_UDMA_CHA_CFG_T config,
                               const UDMA_RESOURCES *udma,
                               RSI_UDMA_HANDLE_T udmaHandle)
{
  UNUSED_PARAMETER(ch);
  UNUSED_PARAMETER(udma);
  return (RSI_UDMA_GetChannelTransferLength(udmaHandle, &config, control));
}

/** @} */

#ifdef __cplusplus
}
#endif

#else
typedef int dummy; // To remove empty translation unit warning.
#endif //  A11_ROM ||  UDMA_ROMDRIVER_PRESENT
