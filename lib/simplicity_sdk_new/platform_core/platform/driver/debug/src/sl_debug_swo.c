/***************************************************************************//**
 * @file
 * @brief SWO debug
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
#include "sl_debug_swo.h"

#include "em_device.h"

#if defined(__CORTEX_M) && (__CORTEX_M >= 3)

#if _SILICON_LABS_32B_SERIES <= 2
#include "em_cmu.h"
#endif

#include "sl_hal_gpio.h"
#include "sl_gpio.h"

#include "sl_clock_manager.h"
#include "sl_debug_swo_config.h"
#include "sl_component_catalog.h"

sl_status_t sl_debug_swo_init(void)
{
  sl_status_t status;
  uint32_t freq = 0UL;
  uint16_t cyctap = 0U;
  uint16_t postpreset = 0U;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

#if !defined(SL_DEBUG_SWO_ENABLE) || (SL_DEBUG_SWO_ENABLE == 1)
  // Setup GPIO
#if defined(GPIO_SWV_PORT)
  sl_gpio_t gpio_swv;
  gpio_swv.port = (sl_gpio_port_t)GPIO_SWV_PORT;
  gpio_swv.pin = GPIO_SWV_PIN;
  sl_gpio_set_pin_mode(&gpio_swv, SL_GPIO_MODE_PUSH_PULL, 1);
#endif
  sl_hal_gpio_enable_debug_swo(true);
#endif

#if (_SILICON_LABS_32B_SERIES == 2) && !defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#if defined(_CMU_TRACECLKCTRL_CLKSEL_MASK)
#if defined(_CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23)
#if defined(CMU_CLKEN0_HFRCOEM23)
  CMU->CLKEN0_SET = CMU_CLKEN0_HFRCOEM23;
#endif
  // Select HFRCOEM23 as source for TRACECLK
  CMU_CLOCK_SELECT_SET(TRACECLK, HFRCOEM23);
#elif defined(_CMU_TRACECLKCTRL_CLKSEL_SYSCLK)
  // Select SYSCLK as source for TRACECLK
  CMU_CLOCK_SELECT_SET(TRACECLK, SYSCLK);
#endif
#endif
#endif

  // Setup clock
  status = sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_TRACECLK, &freq);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Enable trace in core debug
  CoreDebug->DHCSR |= CoreDebug_DHCSR_C_DEBUGEN_Msk;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  //Tap the cyctap and postpreset based on the selected interval
#ifdef SL_DEBUG_SWO_SAMPLE_INTERVAL
  if (SL_DEBUG_SWO_SAMPLE_INTERVAL <= 960) {
    cyctap = 0;
    postpreset = (SL_DEBUG_SWO_SAMPLE_INTERVAL / 64);
  } else {
    cyctap = 1;
    postpreset = (SL_DEBUG_SWO_SAMPLE_INTERVAL / 1024);
  }
#else
  cyctap = 1;
  postpreset = 0xF;
#endif
  // Enable PC and IRQ sampling output
  DWT->CTRL = ((4UL << DWT_CTRL_NUMCOMP_Pos)        // Number of comparators. Hardwired to 4.
               | (SL_DEBUG_SWO_SAMPLE_IRQ << DWT_CTRL_EXCTRCENA_Pos)  // Interrupt events
               | (SL_DEBUG_SWO_SAMPLE_PC << DWT_CTRL_PCSAMPLENA_Pos)  // PC sample events
               | (cyctap << DWT_CTRL_CYCTAP_Pos)       // Tap cycle counter at bit 10 (vs bit 6 if 0)
               | (0xFUL << DWT_CTRL_POSTINIT_Pos)   // Post-tap counter
               | (postpreset << DWT_CTRL_POSTPRESET_Pos) // Post-tap counter reload value
               | (1UL << DWT_CTRL_CYCCNTENA_Pos));  // Enable cycle counter
  // Set TPIU prescaler for the current debug clock frequency. ACPR value is div - 1.
  TPI->ACPR = ((freq + (SL_DEBUG_SWO_FREQ / 2)) / SL_DEBUG_SWO_FREQ) - 1UL;

  // Set protocol to NRZ
  TPI->SPPR = 2UL;

  // Disable continuous formatting
  TPI->FFCR = TPI_FFCR_TrigIn_Msk;

  // Unlock ITM and output data
  ITM->LAR = 0xC5ACCE55UL;
  // CMSIS bitfield naming is inconsistent - 16U maps to
  // ITM_TCR_TraceBusID_Pos (v7M) or ITM_TCR_TRACEBUSID_Pos (v8M)
  ITM->TCR = ((1UL << 16U)
              | (1UL << ITM_TCR_DWTENA_Pos)
              | (1UL << ITM_TCR_ITMENA_Pos));

  // Send data on the SWO channel. This avoids corrupting data
  // sent on the SWO channel shortly after initialization.
  ITM->TER |= (1UL << 8);
  ITM->PORT[8].u8 = 0xFF;
  ITM->TER &= ~(1UL << 8);

  return SL_STATUS_OK;
}

sl_status_t sl_debug_swo_enable_itm(uint32_t channel)
{
  ITM->TER |= (1UL << channel);
  return SL_STATUS_OK;
}

sl_status_t sl_debug_swo_disable_itm(uint32_t channel)
{
  ITM->TER &= ~(1UL << channel);
  return SL_STATUS_OK;
}

sl_status_t sl_debug_swo_write_u8(uint32_t channel, uint8_t byte)
{
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

sl_status_t sl_debug_swo_write_u16(uint32_t channel, uint16_t half_word)
{
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

sl_status_t sl_debug_swo_write_u32(uint32_t channel, uint32_t word)
{
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

  return SL_STATUS_NOT_INITIALIZED;
}

#else // __CORTEX_M

sl_status_t sl_debug_swo_init(void)
{
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_debug_swo_enable_itm(uint32_t channel)
{
  (void) channel;
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_debug_swo_disable_itm(uint32_t channel)
{
  (void) channel;
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_debug_swo_write_u8(uint32_t channel, uint8_t byte)
{
  (void) channel;
  (void) byte;
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_debug_swo_write_u16(uint32_t channel, uint16_t half_word)
{
  (void) channel;
  (void) half_word;
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_debug_swo_write_u32(uint32_t channel, uint32_t word)
{
  (void) channel;
  (void) word;
  return SL_STATUS_NOT_SUPPORTED;
}

#endif // __CORTEX_M
