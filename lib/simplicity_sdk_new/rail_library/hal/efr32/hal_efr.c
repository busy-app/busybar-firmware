/***************************************************************************//**
 * @file
 * @brief This file contains EFR32 specific HAL code to handle chip startup.
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "em_device.h"

#include "sl_gpio.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#if defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#include "sl_clock_manager.h"
#else
#include "em_cmu.h"
#endif

#ifdef _SILICON_LABS_32B_SERIES_2
#include "em_emu.h"
#include "em_prs.h"
#else
#include "sl_hal_emu.h"
#include "sl_hal_prs.h"
#endif

#include "rail.h"
#include "hal_common.h"

#if defined(_SILICON_LABS_32B_SERIES_2)
#define sl_hal_prs_pin_output PRS_PinOutput
#define SL_HAL_PRS_TYPE_ASYNC prsTypeAsync
#endif
// Create defines for the different PRS signal sources as they vary per chip
#define _PRS_CH_CTRL_SOURCESEL_FRC     0x00000055UL

/**
 * Define the signals that are supported for debug in RAILtest. These are chip
 * specific because on some chips these are supported by the PRS while on others
 * the debugging must come from the library directly.
 */
static const debugSignal_t debugSignals[] =
{
  {
    .name = "RXACTIVE",
    .isPrs = true,
    .loc = {
      .prs = {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACRX,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RAC,
#else
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLRX,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
#endif
      }
    }
  },
  {
    .name = "TXACTIVE",
    .isPrs = true,
    .loc = {
      .prs = {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACTX,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RAC,
#else
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLTX,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
#endif
      }
    }
  },
  {
    .name = "LNAEN",
    .isPrs = true,
    .loc = {
      .prs = {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLNAEN,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RAC,
#else
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLLNAEN,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
#endif
      }
    }
  },
  {
    .name = "PAEN",
    .isPrs = true,
    .loc = {
      .prs = {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACPAEN,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RAC,
#else
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLPAEN,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
#endif
      }
    }
  },
  {
    .name = "RACACTIVE",
    .isPrs = true,
    .loc = {
      .prs = {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACACTIVE,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RAC,
#else
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLACTIVE,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
#endif
      }
    }
  },
  {
    .name = "PTIDATA",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_FRCDOUT,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_FRC
      }
    }
  },
  {
    .name = "FRAMEDETECT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMLFRAMEDET,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEML,
      }
    }
  },
  {
    .name = "PREAMBLEDETECT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMPREDET,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEM,
      }
    }
  },
  {
    .name = "TIMINGDETECT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMHTIMDET,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEMH,
      }
    }
  },
  {
    .name = "FRAMESENT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMFRAMESENT,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEM,
      }
    }
  },
  {
    .name = "SYNCSENT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMHSYNCSENT,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEMH,
      }
    }
  },
  {
    .name = "EOF",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMHEOF,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEMH,
      }
    }
  },
  {
    .name = "DOUT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMLDOUT,
#if SLI_HAL_RAIL_NEEDS_DOUT_DCLK_WORKAROUND
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_DOUT_DCLK_WORKAROUND,
#else
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEML,
#endif
      }
    }
  },
  {
    .name = "DCLK",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_MODEMLDCLK,
#if SLI_HAL_RAIL_NEEDS_DOUT_DCLK_WORKAROUND
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_DOUT_DCLK_WORKAROUND,
#else
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_MODEML,
#endif
      }
    }
  },
#ifndef _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC0
#define _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC0 (0x00000001UL)
#endif
  {
    .name = "CC0",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC0,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_PROTIMERL,
      }
    }
  },
#ifndef _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC1
#define _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC1 (0x00000002UL)
#endif
  {
    .name = "CC1",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC1,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_PROTIMERL,
      }
    }
  },
#ifndef _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC3
#define _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC3 (0x00000002UL)
#endif
  {
    .name = "CC3",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_PROTIMERLCC3,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_PROTIMERL,
      }
    }
  },
#if RAIL_SUPPORTS_OFDM_PA
// For OFDM debug signals, loc.prs.source is always
// _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL. loc.prs.signals fields are not
// Asynchronous signals (_PRS_CH_CTRL_SIGSEL_x) defined in
// efr32xg25_prs_signals.h:
// _PRS_ASYNC_CH_CTRL_SIGSEL_SMCTRLLSOFTM0
// _PRS_ASYNC_CH_CTRL_SIGSEL_SMCTRLLSOFTM1
// _PRS_ASYNC_CH_CTRL_SIGSEL_SMCTRLLSOFTM2
// _PRS_ASYNC_CH_CTRL_SIGSEL_SMCTRLLSOFTM3
// Asynchronous signals are selected during setDebugSignal() execution because
// of ressource allocation issue. The _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL
// souce can only provide 4 signals at the same time but there are 5 possible
// signals.
// Instead, loc.prs.signals contains the value that must be set in
// PRSMUXLSB register to route a given OFDM PRS signal to the
// _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL source.

#define OFDMFRAMEDETECT_PRSMUXLSB       1
#define OFDMEOF_PRSMUXLSB               2
#define OFDMFRAMESENT_PRSMUXLSB         11
#define OFDMSYNCSENT_PRSMUXLSB          12
#define OFDMSYMBOLCLK_PRSMUXLSB         13
  {
    .name = "OFDMFRAMEDETECT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = OFDMFRAMEDETECT_PRSMUXLSB,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL,
      }
    }
  },
  {
    .name = "OFDMEOF",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = OFDMEOF_PRSMUXLSB,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL,
      }
    }
  },
  {
    .name = "OFDMFRAMESENT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = OFDMFRAMESENT_PRSMUXLSB,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL,
      }
    }
  },
  {
    .name = "OFDMSYNCSENT",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = OFDMSYNCSENT_PRSMUXLSB,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL,
      }
    }
  },
  {
    .name = "OFDMSYMBOLCLK",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = OFDMSYMBOLCLK_PRSMUXLSB,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_SMCTRLL,
      }
    }
  },
#endif//RAIL_SUPPORTS_OFDM_PA
#ifdef _SILICON_LABS_32B_SERIES_3
  // Requires custom firmware with debug channels enabled from RAC
  {
    .name = "RAC_SWDBG0",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLCTIOUT0,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
      }
    }
  },
  {
    .name = "RAC_SWDBG1",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLCTIOUT1,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
      }
    }
  },
  {
    .name = "RAC_SWDBG2",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACLCTIOUT2,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RACL,
      }
    }
  },
  {
    .name = "RAC_SWDBG3",
    .isPrs = true,
    .loc = {
      .prs = {
        .signal = _PRS_ASYNC_CH_CTRL_SIGSEL_RACCTIOUT3,
        .source = _PRS_ASYNC_CH_CTRL_SOURCESEL_RAC,
      }
    }
  },
#endif
};

const debugSignal_t* halGetDebugSignals(uint32_t *size)
{
  if (size != NULL) {
    *size = sizeof(debugSignals) / sizeof(debugSignal_t);
  }
  return debugSignals;
}

void halDisablePrs(uint8_t channel)
{
  GPIO->PRSROUTE[0].ROUTEEN &= ~(0x1 << (channel + _GPIO_PRS_ROUTEEN_ASYNCH0PEN_SHIFT));
#if defined(_SILICON_LABS_32B_SERIES_2)
  PRS_SourceAsyncSignalSet(channel,
                           PRS_ASYNC_CH_CTRL_SOURCESEL_DEFAULT,
                           PRS_ASYNC_CH_CTRL_SIGSEL_DEFAULT);
#else
  sl_hal_prs_async_connect_channel_producer(channel,
                                            (sl_hal_prs_sync_producer_signal_t)(PRS_ASYNC_CH_CTRL_SOURCESEL_DEFAULT
                                                                                | PRS_ASYNC_CH_CTRL_SIGSEL_DEFAULT));
#endif
}

void halEnablePrs(uint8_t channel,
                  uint8_t loc,
                  sl_gpio_t portPin,
                  uint8_t source,
                  uint8_t signal)
{
#ifndef SL_CATALOG_CLOCK_MANAGER_PRESENT
  (void)loc;
  // Make sure the PRS is on and clocked
  CMU_ClockEnable(cmuClock_PRS, true);

  PRS_SourceAsyncSignalSet(channel,
                           ( ( uint32_t ) source << _PRS_ASYNC_CH_CTRL_SOURCESEL_SHIFT),
                           ( ( uint32_t ) signal << _PRS_ASYNC_CH_CTRL_SIGSEL_SHIFT) );
#else
  (void)loc;
  // Make sure the PRS is on and clocked
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);

  sl_hal_prs_async_connect_channel_producer(channel,
                                            (sl_hal_prs_sync_producer_signal_t)(((uint32_t)source << _PRS_ASYNC_CH_CTRL_SOURCESEL_SHIFT)
                                                                                | ((uint32_t)signal << _PRS_ASYNC_CH_CTRL_SIGSEL_SHIFT)));
#endif //SL_CATALOG_CLOCK_MANAGER_PRESENT
  sl_hal_prs_pin_output(channel, SL_HAL_PRS_TYPE_ASYNC, portPin.port, portPin.pin);
  // Configure this GPIO as an output low to finish enabling this signal
  sl_gpio_set_pin_mode(&portPin, SL_GPIO_MODE_PUSH_PULL, 0);
}

bool halIsPrsChannelFree(uint8_t channel)
{
#ifndef SL_CATALOG_CLOCK_MANAGER_PRESENT
  // Make sure the PRS is on and clocked
  CMU_ClockEnable(cmuClock_PRS, true);
#else
  // Make sure the PRS is on and clocked
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
#endif //#ifndef SL_CATALOG_CLOCK_MANAGER_PRESENT
  return ((PRS->ASYNC_CH[channel].CTRL
           & (_PRS_ASYNC_CH_CTRL_SOURCESEL_MASK | _PRS_ASYNC_CH_CTRL_SIGSEL_MASK))
          == (PRS_ASYNC_CH_CTRL_SOURCESEL_DEFAULT | PRS_ASYNC_CH_CTRL_SIGSEL_DEFAULT));
}
