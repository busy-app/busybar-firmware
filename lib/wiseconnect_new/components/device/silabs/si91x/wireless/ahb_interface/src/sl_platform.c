/***************************************************************************/ /**
 * @file  sl_platform.c
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
#include "sli_siwx917_soc.h"
#include "sl_device.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_status.h"
#include "sl_si91x_core_utilities.h"
#include "sl_status.h"
#include "sl_constants.h"
#include <stdbool.h>
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif /* SL_COMPONENT_CATALOG_PRESENT */
#include "sl_board_configuration.h"
#include "rsi_rom_clks.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#include "sl_si91x_os.h"
#endif

sl_status_t sli_si91x_submit_rx_pkt(void);
void sl_board_enable_vcom(void);
sl_status_t sli_si91x_bootup_firmware(const uint8_t select_option, uint8_t image_number);

void sli_si91x_platform_init(void)
{
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
  /* Before NWP going to power save mode ,set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl,
  AON domain power supply controls from NWP to M4 */
  RSI_Set_Cntrls_To_M4();
#endif

  // Enable DWT and cycle counting
  CoreDebug->DEMCR |= 0x01000000;
  DWT->CTRL |= 0x1;

#if (SL_SI91X_TICKLESS_MODE == 0)
  SysTick_Config(SystemCoreClock / SL_OS_SYSTEM_TICK_RATE);
  // Set P2P Intr priority
  NVIC_SetPriority(SysTick_IRQn, SYSTICK_INTR_PRI);
#endif
  //On boot-up, verify the M4_wakeup_TA bit in the P2P status register and clearing the bit if it is set.
  if (P2P_STATUS_REG & M4_wakeup_TA) {
    P2P_STATUS_REG &= ~M4_wakeup_TA;
  }
}

void sl_board_enable_vcom(void)
{
  //empty function
}

sl_status_t sli_si91x_bootup_firmware(const uint8_t select_option, uint8_t image_number)
{
  uint8_t skip_bootload_sequence = 0;
  si91x_status_t retval          = RSI_ERROR_NONE;

  if (!(P2P_STATUS_REG & TA_is_active)) {
    P2P_STATUS_REG |= M4_wakeup_TA;
    skip_bootload_sequence = 1;
  }
  while (!(P2P_STATUS_REG & TA_is_active)) {
    //loop is waiting for the TA to become active
  }

  if (!skip_bootload_sequence) {
    do {
      retval = rsi_waitfor_boardready();
      if (retval == RSI_ERROR_NONE) {
        break;
      }
      if ((retval < 0) && (retval != RSI_ERROR_WAITING_FOR_BOARD_READY) && (retval != RSI_ERROR_IN_OS_OPERATION)) {
        return sli_convert_si91x_status_to_sl_status(retval);
      }
    } while ((retval == RSI_ERROR_WAITING_FOR_BOARD_READY) || (retval == RSI_ERROR_IN_OS_OPERATION));
    retval = rsi_select_option(select_option, image_number);
    VERIFY_STATUS_AND_RETURN(sli_convert_si91x_status_to_sl_status(retval));
  }

  // Update TX & RX DMA descriptor address
  rsi_update_tx_dma_desc(skip_bootload_sequence);
  rsi_update_rx_dma_desc();

#if SL_SI91X_FAST_FW_UP
  status = rsi_set_fast_fw_up();
  if (status != RSI_SUCCESS) {
    SL_PRINTF(SL_DEVICE_INIT_SET_FAST_FIRMWARE_UP_ERROR, COMMON, LOG_ERROR, "status: %4x", status);
    return status;
  }
#endif

  sli_m4_ta_interrupt_init();
  if (!(M4SS_P2P_INTR_SET_REG & RX_BUFFER_VALID)) {
    sl_status_t rx_status = sli_si91x_submit_rx_pkt();
    if (rx_status != SL_STATUS_OK) {
      return rx_status;
    }
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  osKernelInitialize();
#endif

  return SL_STATUS_OK;
}
