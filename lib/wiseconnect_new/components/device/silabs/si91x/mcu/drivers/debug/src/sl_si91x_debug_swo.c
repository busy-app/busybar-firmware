/***************************************************************************/ /**
 * @file
 * @brief SI91x SWO
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_debug_swo.h"

#include "em_device.h"

#include "sl_si91x_debug_swo_config.h"
#include "sl_component_catalog.h"

#include "si91x_device.h"
#include "rsi_rom_egpio.h"
#include "rsi_rom_clks.h"
#include "sl_si91x_clock_manager.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
#include "rsi_debug.h"

/*******************************************************************************
****************************  LOCAL VARIABLES   ********************************
*******************************************************************************/

static sl_si91x_gpio_pin_config_t sl_mcu_clk_out_gpio_pin_config = { { SL_SI91X_GPIO_12_PORT, SL_SI91X_GPIO_12_PIN },
                                                                     GPIO_OUTPUT };
static sl_si91x_gpio_pin_config_t sl_mcu_trace_clk_in_gpio_pin_config = { { SL_SI91X_GPIO_15_PORT,
                                                                            SL_SI91X_GPIO_15_PIN },
                                                                          GPIO_INPUT };

/*******************************************************************************
****************************  DEFINES   ********************************
*******************************************************************************/

#define OUTPUT_VALUE     0
#define UNLOCK_ITM_MAGIC 0xC5ACCE55UL // Magic word to unlock ITM
#define MAX_ITM_CHANNEL  31
/*******************************************************************************
****************************  FUNCTIONS   ********************************
*******************************************************************************/

sl_status_t sl_si91x_debug_swo_init(void)
{
  sl_status_t status  = SL_STATUS_OK;
  uint32_t freq       = 0UL;
  uint16_t cyctap     = 0U;
  uint16_t postpreset = 0U;
  status = sl_si91x_gpio_driver_enable_pad_selection(PAD_SEL_MCU_CLK); // Enables Pad selection for MCU clock out
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_si91x_gpio_driver_enable_pad_receiver(GPIO_MCU_CLK_OUT); // Enables pad receiver for MCU Clock out
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_gpio_set_configuration(sl_mcu_clk_out_gpio_pin_config); // Configures GPIO 12 as out put
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_gpio_driver_set_pin_mode(&sl_mcu_clk_out_gpio_pin_config.port_pin,
                                       (sl_gpio_mode_t)SL_GPIO_MODE_8,
                                       OUTPUT_VALUE); //Sets pin mode as mcu clock out
  if (status != SL_STATUS_OK) {
    return status;
  }

  RSI_CLK_McuClkOutConfig(M4CLK, MCUCLKOUT_SOC_PLL_CLK, 0); // Configures soc pll as clock for MCU clock out

  status = sl_si91x_gpio_driver_enable_pad_receiver(GPIO_TRACE_CLK_IN); // Enables pad receiver for Trace Clock
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_si91x_gpio_driver_enable_pad_selection(PAD_SEL_TRACE_CLK_IN); // Enables Pad selection for Trace Clock
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_gpio_set_configuration(sl_mcu_trace_clk_in_gpio_pin_config); // Configures GPIO 11 as out put
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_gpio_driver_set_pin_mode(&sl_mcu_trace_clk_in_gpio_pin_config.port_pin,
                                       (sl_gpio_mode_t)SL_GPIO_MODE_6,
                                       OUTPUT_VALUE); //Sets pin mode as Trace clock In
  if (status != SL_STATUS_OK) {
    return status;
  }

  sl_si91x_clock_manager_m4_get_core_clk_src_freq(&freq);

  // Enable trace in core debug
  CoreDebug->DHCSR |= CoreDebug_DHCSR_C_DEBUGEN_Msk;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  //Tap the cyctap and postpreset based on the selected interval
#ifdef SL_SI91X_DEBUG_SWO_SAMPLE_INTERVAL
  if (SL_SI91X_DEBUG_SWO_SAMPLE_INTERVAL <= 960) {
    cyctap     = 0;
    postpreset = (SL_SI91X_DEBUG_SWO_SAMPLE_INTERVAL / 64);
  } else {
    cyctap     = 1;
    postpreset = (SL_SI91X_DEBUG_SWO_SAMPLE_INTERVAL / 1024);
  }
#else
  cyctap     = 1;
  postpreset = 0xF;
#endif
  // Enable PC and IRQ sampling output
  DWT->CTRL = ((4UL << DWT_CTRL_NUMCOMP_Pos)             // Number of comparators. Hardwired to 4.
               | (cyctap << DWT_CTRL_CYCTAP_Pos)         // Tap cycle counter at bit 10 (vs bit 6 if 0)
               | (0xFUL << DWT_CTRL_POSTINIT_Pos)        // Post-tap counter
               | (postpreset << DWT_CTRL_POSTPRESET_Pos) // Post-tap counter reload value
               | (1UL << DWT_CTRL_CYCCNTENA_Pos));       // Enable cycle counter
  // Set TPIU prescaler for the current debug clock frequency. ACPR value is div - 1.
  TPI->ACPR = (freq / SL_SI91X_DEBUG_SWO_FREQ) - 1UL;

  // Set protocol to NRZ
  TPI->SPPR = 2UL;

  // Disable continuous formatting
  TPI->FFCR = TPI_FFCR_TrigIn_Msk;

  // Unlock ITM and output data
  ITM->LAR = UNLOCK_ITM_MAGIC;
  // CMSIS bitfield naming is inconsistent - 16U maps to
  // ITM_TCR_TraceBusID_Pos (v7M) or ITM_TCR_TRACEBUSID_Pos (v8M)
  ITM->TCR = ((1UL << 16U) | (1UL << ITM_TCR_DWTENA_Pos) | (1UL << ITM_TCR_ITMENA_Pos));

  // Send data on the SWO channel. This avoids corrupting data
  // sent on the SWO channel shortly after initialization.
  ITM->TER |= (1UL << 8);
  ITM->PORT[8].u8 = 0xFF;
  ITM->TER &= ~(1UL << 8);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_debug_swo_enable_itm(uint32_t channel)
{
  if (channel > MAX_ITM_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  ITM->TER |= (1UL << channel);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_debug_swo_disable_itm(uint32_t channel)
{
  if (channel > MAX_ITM_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  ITM->TER &= ~(1UL << channel);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_debug_swo_write_u8(uint32_t channel, uint8_t byte)
{
  if (channel > MAX_ITM_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (ITM->TCR & ITM_TCR_ITMENA_Msk) {
    do {
      // Some versions of JLink (erroneously) disable SWO when debug connections
      // are closed. Re-enabling trace works around this.
      CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

      // Ensure ITM channel is enabled
      ITM->TER |= (1UL << channel);
    } while (ITM->PORT[channel].u32 == 0);

    ITM->PORT[channel].u8 = byte;

    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_INITIALIZED;
}

sl_status_t sl_si91x_debug_swo_write_u16(uint32_t channel, uint16_t half_word)
{
  if (channel > MAX_ITM_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (ITM->TCR & ITM_TCR_ITMENA_Msk) {
    do {
      // Some versions of JLink (erroneously) disable SWO when debug connections
      // are closed. Re-enabling trace works around this.
      CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

      // Ensure ITM channel is enabled
      ITM->TER |= (1UL << channel);
    } while (ITM->PORT[channel].u32 == 0);

    ITM->PORT[channel].u16 = half_word;

    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_INITIALIZED;
}

sl_status_t sl_si91x_debug_swo_write_u32(uint32_t channel, uint32_t word)
{
  if (channel > MAX_ITM_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (ITM->TCR & ITM_TCR_ITMENA_Msk) {
    do {
      // Some versions of JLink (erroneously) disable SWO when debug connections
      // are closed. Re-enabling trace works around this.
      CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

      // Ensure ITM channel is enabled
      ITM->TER |= (1UL << channel);
    } while (ITM->PORT[channel].u32 == 0);

    ITM->PORT[channel].u32 = word;

    return SL_STATUS_OK;
  }
  return SL_STATUS_INVALID_PARAMETER;
}
