/***************************************************************************//**
 * @file
 * @brief This file implements a simple API for configuring FEM control signals
 * via PRS.
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

#include <inttypes.h>

#include "sl_fem_util_config.h"
#include "sl_fem_util.h"
#include "sl_rail.h"

#define SL_FEM_UTIL_GLOBAL_ENABLE (SL_FEM_UTIL_RX_ENABLE               \
                                   || SL_FEM_UTIL_TX_ENABLE            \
                                   || SL_FEM_UTIL_BYPASS_ENABLE        \
                                   || SL_FEM_UTIL_TX_HIGH_POWER_ENABLE \
                                   || SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE)

#if SL_FEM_UTIL_GLOBAL_ENABLE == 1
  #include "em_device.h"
  #include "sl_gpio.h"
  #include "sl_hal_prs.h"
#if defined(_SILICON_LABS_32B_SERIES_2)
  #include "em_cmu.h"
#elif defined(_SILICON_LABS_32B_SERIES_3)
  #include "sl_clock_manager.h"
#endif // _SILICON_LABS_32B_SERIES_2

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_CATALOG_APP_ASSERT_PRESENT
#include "app_assert.h"
#define APP_ASSERT(expr, ...) app_assert(expr, __VA_ARGS__)
#else
#define APP_ASSERT(expr, ...) \
  do {                        \
    if (!(expr)) {            \
      while (1)               \
        ;                     \
    }                         \
  } while (0)
#endif // SL_CATALOG_APP_ASSERT_PRESENT

#if SL_FEM_UTIL_RX_ENABLE == 1
  #if (!defined(SL_FEM_UTIL_RX_CHANNEL) \
  || !defined(SL_FEM_UTIL_RX_PORT)      \
  || !defined(SL_FEM_UTIL_RX_PIN))
      #error "SL_FEM_UTIL_RX_CHANNEL/PORT/PIN must be defined."
  #endif
#endif // SL_FEM_UTIL_RX_ENABLE == 1

#if SL_FEM_UTIL_TX_ENABLE == 1
// if no separate CTX pin is defined, CRX is a combined RX-TX pin
  #if !defined(SL_FEM_UTIL_TX_CHANNEL) && (SL_FEM_UTIL_RX_ENABLE == 1)
    #define SL_FEM_UTIL_TX_PORT SL_FEM_UTIL_RX_PORT
    #define SL_FEM_UTIL_TX_PIN SL_FEM_UTIL_RX_PIN
    #define SL_FEM_UTIL_TX_CHANNEL SL_FEM_UTIL_RX_CHANNEL
  #elif !defined(SL_FEM_UTIL_TX_CHANNEL)
    #error "SL_FEM_UTIL_TX_CHANNEL must be defined."
  #endif
#endif // SL_FEM_UTIL_TX_ENABLE == 1

  #if SL_FEM_UTIL_TX_ENABLE == 1 && SL_FEM_UTIL_RX_ENABLE == 1
    #if SL_FEM_UTIL_RX_CHANNEL == SL_FEM_UTIL_TX_CHANNEL
      #error The RX and TX PRS channels cannot be equal.
    #endif
  #endif

  #if (SL_FEM_UTIL_RX_ENABLE == 1) && defined(SL_FEM_UTIL_SLEEP_CHANNEL)
    #if (SL_FEM_UTIL_RX_CHANNEL + 1) != SL_FEM_UTIL_SLEEP_CHANNEL
      #error "SL_FEM_UTIL_SLEEP_CHANNEL must immediately follow SL_FEM_UTIL_RX_CHANNEL"
    #endif
  #endif // SL_FEM_UTIL_RX_ENABLE

  #ifndef PRS_CHAN_COUNT
    #define PRS_CHAN_COUNT SL_HAL_PRS_ASYNC_CHAN_COUNT
  #endif

  #if defined(SL_FEM_UTIL_SLEEP_CHANNEL) && (SL_FEM_UTIL_SLEEP_CHANNEL >= PRS_CHAN_COUNT)
    #error "SL_FEM_UTIL_SLEEP_CHANNEL number higher than number of PRS channels"
  #endif

  #if SL_FEM_UTIL_TX_ENABLE
    #if SL_FEM_UTIL_TX_CHANNEL >= PRS_CHAN_COUNT
      #error "SL_FEM_UTIL_TX_CHANNEL number higher than number of PRS channels"
    #endif
  #endif

#if SL_FEM_UTIL_BYPASS_ENABLE == 1
  #if (!defined(SL_FEM_UTIL_BYPASS_PORT) || !defined(SL_FEM_UTIL_BYPASS_PIN))
      #error "SL_FEM_UTIL_BYPASS_PORT/PIN must be defined."
  #endif
#endif // SL_FEM_UTIL_BYPASS_ENABLE == 1

#if SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE == 1
  #if !defined(SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_CHANNEL)
      #error "SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_CHANNEL must be defined."
  #endif
  #if ((SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE != SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_NO_COMBINE) \
  && !defined(SL_FEM_UTIL_BYPASS_PRS_COMBINE_CHANNEL))
    #error "SL_FEM_UTIL_BYPASS_PRS_COMBINE_CHANNEL must be defined."
  #endif
#endif // SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE == 1
#endif // SL_FEM_UTIL_GLOBAL_ENABLE

void sl_fem_util_init(void)
{
#if SL_FEM_UTIL_GLOBAL_ENABLE == 1
  // Turn on the GPIO clock so that we can turn on GPIOs
#if defined(_SILICON_LABS_32B_SERIES_2)
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_PRS, true);
#elif defined(_SILICON_LABS_32B_SERIES_3)
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
#endif // _SILICON_LABS_32B_SERIES_2
#if SL_FEM_UTIL_TX_ENABLE == 1
  //Enable the output of TX based on a specific port and pin
  //Configure the tx gpio to be an output (FEM pin CTX)
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_FEM_UTIL_TX_PORT, SL_FEM_UTIL_TX_PIN }, SL_GPIO_MODE_PUSH_PULL, false);
  //Setup the PRS to output TX on the channel and location chosen.
  sl_hal_prs_async_connect_channel_producer(SL_FEM_UTIL_TX_CHANNEL,
#ifdef PRS_RACL_PAEN
                                            PRS_RACL_PAEN);
#elif defined(PRS_RAC_PAEN)
                                            PRS_RAC_PAEN);
#else
#error "No PRS setting defined for Source=RAC, Signal=PAEN"
#endif //PRS_RACL_PAEN
  sl_hal_prs_pin_output(SL_FEM_UTIL_TX_CHANNEL, SL_HAL_PRS_TYPE_ASYNC, SL_FEM_UTIL_TX_PORT, SL_FEM_UTIL_TX_PIN);
#endif //SL_FEM_UTIL_TX_ENABLE == 1

#if SL_FEM_UTIL_RX_ENABLE == 1
  //Enable the output of RX based on a specific port and pin
  //Configure the rx gpio to be an output (FEM pin CRX)
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_FEM_UTIL_RX_PORT, SL_FEM_UTIL_RX_PIN }, SL_GPIO_MODE_PUSH_PULL, false);
  sl_hal_prs_async_connect_channel_producer(SL_FEM_UTIL_RX_CHANNEL,
#ifdef PRS_RACL_LNAEN
                                            PRS_RACL_LNAEN);
#elif defined(PRS_RAC_LNAEN)
                                            PRS_RAC_LNAEN);
#else
#error "No PRS setting defined for Source=RAC, Signal=LNAEN"
#endif //PRS_RACL_LNAEN
  sl_hal_prs_pin_output(SL_FEM_UTIL_RX_CHANNEL, SL_HAL_PRS_TYPE_ASYNC, SL_FEM_UTIL_RX_PORT, SL_FEM_UTIL_RX_PIN);
#endif //SL_FEM_UTIL_RX_ENABLE == 1

#if defined(SL_FEM_UTIL_SLEEP_CHANNEL)
// initialize sleep as output (FEM pin CSD)
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_FEM_UTIL_SLEEP_PORT, SL_FEM_UTIL_SLEEP_PIN }, SL_GPIO_MODE_PUSH_PULL, false);
// set up the CSD to be active whenever the PA or LNA are enabled
// its signal is PA enable ORed with the RX channel's signal (LNA enable)
  sl_hal_prs_async_connect_channel_producer(SL_FEM_UTIL_SLEEP_CHANNEL,
#ifdef PRS_RACL_PAEN
                                            PRS_RACL_PAEN);
#elif defined(PRS_RAC_PAEN)
                                            PRS_RAC_PAEN);
#else
#error "No PRS setting defined for Source=RAC, Signal=PAEN"
#endif //PRS_RACL_PAEN
#if SL_FEM_UTIL_RX_ENABLE == 1
  sl_hal_prs_async_combine_signals(SL_FEM_UTIL_SLEEP_CHANNEL,
                                   SL_FEM_UTIL_RX_CHANNEL,
                                   SL_HAL_PRS_LOGIC_A_OR_B);
#endif
// Configure CSD PRS output to selected channel
  sl_hal_prs_pin_output(SL_FEM_UTIL_SLEEP_CHANNEL, SL_HAL_PRS_TYPE_ASYNC, SL_FEM_UTIL_SLEEP_PORT, SL_FEM_UTIL_SLEEP_PIN);
#endif // SL_FEM_UTIL_SLEEP_CHANNEL

// if fem has a bypass pin (FEM pin CPS)
#ifdef SL_FEM_UTIL_BYPASS_PORT
  // set up bypass pin
  #if SL_FEM_UTIL_BYPASS_ENABLE
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_FEM_UTIL_BYPASS_PORT, SL_FEM_UTIL_BYPASS_PIN }, SL_GPIO_MODE_PUSH_PULL, true);
  #else //!SL_FEM_UTIL_BYPASS_ENABLE
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_FEM_UTIL_BYPASS_PORT, SL_FEM_UTIL_BYPASS_PIN }, SL_GPIO_MODE_PUSH_PULL, false);
  #endif //SL_FEM_UTIL_BYPASS_ENABLE

  #if SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE && SL_RAIL_SUPPORTS_PRS_LNA_BYPASS
    #if SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE == SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_NO_COMBINE
  sl_hal_prs_pin_output(SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_CHANNEL, SL_HAL_PRS_TYPE_ASYNC, SL_FEM_UTIL_BYPASS_PORT, SL_FEM_UTIL_BYPASS_PIN);
    #else //SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE != SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_NO_COMBINE
  sl_hal_prs_pin_output(SL_FEM_UTIL_BYPASS_PRS_COMBINE_CHANNEL, SL_HAL_PRS_TYPE_ASYNC, SL_FEM_UTIL_BYPASS_PORT, SL_FEM_UTIL_BYPASS_PIN);
    #endif //SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE == SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_NO_COMBINE
  #endif //SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE && SL_RAIL_SUPPORTS_PRS_LNA_BYPASS
#endif //SL_FEM_UTIL_BYPASS_PORT

#if SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE && SL_RAIL_SUPPORTS_PRS_LNA_BYPASS
  sl_rail_prs_lna_bypass_config_t PrsLnaBypassConfig = {
    .timeout_us = SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_TIMEOUT_US,
    .threshold = SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_THRESHOLD,
    .delta_rssi_db = SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_DELTA_RSSI_DBM,
    .prs_channel = SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_CHANNEL,
    .polarity = SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_POLARITY
  };
  sl_rail_status_t status = sl_rail_enable_prs_lna_bypass(SL_RAIL_EFR32_HANDLE,
                                                          true,
                                                          &PrsLnaBypassConfig);
  APP_ASSERT((SL_RAIL_STATUS_NO_ERROR == status),
             "sl_rail_enable_prs_lna_bypass failed, return value: %" PRIu32,
             status);
  // To perform logical operation on SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_CHANNEL,
  // any call to PRS_Combine() must be done after sl_rail_enable_prs_lna_bypass().
  #if SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE == SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_OR_PAEN_COMBINE
  sl_hal_prs_async_connect_channel_producer(SL_FEM_UTIL_BYPASS_PRS_COMBINE_CHANNEL,
    #ifdef PRS_RACL_PAEN
                                            PRS_RACL_PAEN);
    #elif defined(PRS_RAC_PAEN)
                                            PRS_RAC_PAEN);
    #else
    #error "No PRS setting defined for Source=RAC, Signal=PAEN"
    #endif //PRS_RACL_PAEN
  sl_hal_prs_async_combine_signals(SL_FEM_UTIL_BYPASS_PRS_COMBINE_CHANNEL, SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_CHANNEL, SL_HAL_PRS_LOGIC_A_OR_B);
  #elif SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE == SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_XOR_LNAEN_COMBINE
  sl_hal_prs_async_connect_channel_producer(SL_FEM_UTIL_BYPASS_PRS_COMBINE_CHANNEL,
    #ifdef PRS_RACL_LNAEN
                                            PRS_RACL_LNAEN);
    #elif defined(PRS_RAC_LNAEN)
                                            PRS_RAC_LNAEN);
    #else
    #error "No PRS setting defined for Source=RAC, Signal=LNAEN"
    #endif //PRS_RACL_LNAEN
  sl_hal_prs_async_combine_signals(SL_FEM_UTIL_BYPASS_PRS_COMBINE_CHANNEL, SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_CHANNEL, SL_HAL_PRS_LOGIC_A_XOR_B);
  #endif //SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE == SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_OR_PAEN_COMBINE
#endif //SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE && SL_RAIL_SUPPORTS_PRS_LNA_BYPASS

// if fem has a tx power pin (FEM pin CHL)
#ifdef SL_FEM_UTIL_TX_HIGH_POWER_PORT
  // set up tx power pin
  #if SL_FEM_UTIL_TX_HIGH_POWER_ENABLE
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_FEM_UTIL_TX_HIGH_POWER_PORT, SL_FEM_UTIL_TX_HIGH_POWER_PIN }, SL_GPIO_MODE_PUSH_PULL, true);
  #else
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_FEM_UTIL_TX_HIGH_POWER_PORT, SL_FEM_UTIL_TX_HIGH_POWER_PIN }, SL_GPIO_MODE_PUSH_PULL, false);
  #endif
#endif
#endif // SL_FEM_UTIL_GLOBAL_ENABLE
}

void sl_fem_util_wakeup(void)
{
#if SL_FEM_UTIL_GLOBAL_ENABLE == 1
// if fem has a bypass pin (FEM pin CPS) and automatic bypass through PRS is not used
#if defined (SL_FEM_UTIL_BYPASS_PORT)
  #if SL_FEM_UTIL_BYPASS_ENABLE
    #if (!SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE)
  sl_gpio_set_pin(&(sl_gpio_t){SL_FEM_UTIL_BYPASS_PORT, SL_FEM_UTIL_BYPASS_PIN });
    #endif
  #else
  sl_gpio_clear_pin(&(sl_gpio_t){SL_FEM_UTIL_BYPASS_PORT, SL_FEM_UTIL_BYPASS_PIN });
  #endif
#endif

// if fem has a tx power pin (FEM pin CHL)
#ifdef SL_FEM_UTIL_TX_HIGH_POWER_PORT
  #if SL_FEM_UTIL_TX_HIGH_POWER_ENABLE
  sl_gpio_set_pin(&(sl_gpio_t){SL_FEM_UTIL_TX_HIGH_POWER_PORT, SL_FEM_UTIL_TX_HIGH_POWER_PIN });
  #else
  sl_gpio_clear_pin(&(sl_gpio_t){SL_FEM_UTIL_TX_HIGH_POWER_PORT, SL_FEM_UTIL_TX_HIGH_POWER_PIN });
  #endif
#endif
#endif // SL_FEM_UTIL_GLOBAL_ENABLE
}

void sl_fem_util_shutdown(void)
{
#if SL_FEM_UTIL_GLOBAL_ENABLE == 1
// if fem has a bypass pin (FEM pin CPS)
#ifdef SL_FEM_UTIL_BYPASS_PORT
  sl_gpio_clear_pin(&(sl_gpio_t){SL_FEM_UTIL_BYPASS_PORT, SL_FEM_UTIL_BYPASS_PIN });
#endif

// if fem has a tx power pin (FEM pin CHL)
#ifdef SL_FEM_UTIL_TX_HIGH_POWER_PORT
  sl_gpio_clear_pin(&(sl_gpio_t){SL_FEM_UTIL_TX_HIGH_POWER_PORT, SL_FEM_UTIL_TX_HIGH_POWER_PIN });
#endif
#endif // SL_FEM_UTIL_GLOBAL_ENABLE
}

#if SL_FEM_UTIL_RUNTIME_PHY_SELECT
#if SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE
static volatile bool fem_util_phy_enabled = true;
#else //!SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE
static volatile bool fem_util_phy_enabled = false;
#endif //SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE
#elif SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE
#define fem_util_phy_enabled (true)
#else //!SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE
#define fem_util_phy_enabled (false)
#endif //SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE

bool sl_fem_util_get_phy_select(void)
{
  return fem_util_phy_enabled;
}

void sl_fem_util_enable_phy(bool enable)
{
#if SL_FEM_UTIL_RUNTIME_PHY_SELECT
  fem_util_phy_enabled = enable;
#else //!SL_FEM_UTIL_RUNTIME_PHY_SELECT
  (void)enable;
#endif //SL_FEM_UTIL_RUNTIME_PHY_SELECT
}
