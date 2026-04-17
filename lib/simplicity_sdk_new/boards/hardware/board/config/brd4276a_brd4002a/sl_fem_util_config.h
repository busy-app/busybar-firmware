/***************************************************************************//**
 * @file
 * @brief Front End Module configuration file.
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

#ifndef SL_FEM_UTIL_CONFIG_H
#define SL_FEM_UTIL_CONFIG_H

#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_NO_COMBINE          0U
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_OR_PAEN_COMBINE     1U
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_XOR_LNAEN_COMBINE   2U

// <<< Use Configuration Wizard in Context Menu >>>
// <h> FEM Configuration
// <q SL_FEM_UTIL_RX_ENABLE> Enable RX Mode
// <i> Default: 0
#define SL_FEM_UTIL_RX_ENABLE              1
// <q SL_FEM_UTIL_TX_ENABLE> Enable TX Mode
// <i> Default: 0
#define SL_FEM_UTIL_TX_ENABLE              1
// <q SL_FEM_UTIL_TX_HIGH_POWER_ENABLE> Enable TX High Power Mode
// <i> Default: 0
#define SL_FEM_UTIL_TX_HIGH_POWER_ENABLE   0
// <e SL_FEM_UTIL_BYPASS_ENABLE> Enable Bypass Mode
// <i> Default: 0
#define SL_FEM_UTIL_BYPASS_ENABLE          0
// <e SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE> Enable Automatic PRS LNA Bypass Mode
// <i> Default: 0
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_ENABLE              0
// <o SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_THRESHOLD> Automatic PRS LNA Bypass Threshold
// <1-31:1>
// <i> Default: 12
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_THRESHOLD           12
// <o SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_DELTA_RSSI_DBM> Automatic PRS LNA Bypass Delta RSSI (dB)
// <1-255:1>
// <i> Default: 15
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_DELTA_RSSI_DBM      15
// <o SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_POLARITY> Automatic PRS LNA Bypass Polarity
// <0-1:1>
// <i> Default: 1
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_POLARITY            1
// <o SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_TIMEOUT_US> Automatic PRS LNA Bypass timeout (Us)
// <1-65535:1>
// <i> Default: 4000
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_TIMEOUT_US          4000
// <o SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE> Automatic PRS LNA Bypass combine demo
// <SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_NO_COMBINE=> None
// <SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_OR_PAEN_COMBINE=> Ored with PA enable
// <SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_XOR_LNAEN_COMBINE=> Xored with LNA enable
// <i> Default: none
#define SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_COMBINE           SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS_NO_COMBINE
// </e>
// </e>
// </h>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <prs gpio=true readonly=true> SL_FEM_UTIL_RX
// $[PRS_SL_FEM_UTIL_RX]
#define SL_FEM_UTIL_RX_CHANNEL                   8

// PRS ASYNCH8 on PC07
#define SL_FEM_UTIL_RX_PORT                      SL_GPIO_PORT_C
#define SL_FEM_UTIL_RX_PIN                       7

// [PRS_SL_FEM_UTIL_RX]$

// <prs gpio=true optional=true readonly=true> SL_FEM_UTIL_TX
// $[PRS_SL_FEM_UTIL_TX]
#define SL_FEM_UTIL_TX_CHANNEL                   10

// PRS ASYNCH10 on PC05
#define SL_FEM_UTIL_TX_PORT                      SL_GPIO_PORT_C
#define SL_FEM_UTIL_TX_PIN                       5

// [PRS_SL_FEM_UTIL_TX]$

// <prs gpio=true readonly=true> SL_FEM_UTIL_SLEEP
// $[PRS_SL_FEM_UTIL_SLEEP]
#define SL_FEM_UTIL_SLEEP_CHANNEL                9

// PRS ASYNCH9 on PC04
#define SL_FEM_UTIL_SLEEP_PORT                   SL_GPIO_PORT_C
#define SL_FEM_UTIL_SLEEP_PIN                    4

// [PRS_SL_FEM_UTIL_SLEEP]$

// <gpio optional=true> SL_FEM_UTIL_BYPASS
// $[GPIO_SL_FEM_UTIL_BYPASS]

// [GPIO_SL_FEM_UTIL_BYPASS]$

// <gpio optional=true> SL_FEM_UTIL_TX_HIGH_POWER
// $[GPIO_SL_FEM_UTIL_TX_HIGH_POWER]

// [GPIO_SL_FEM_UTIL_TX_HIGH_POWER]$

// <prs optional=true> SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS
// $[PRS_SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS]

// [PRS_SL_FEM_UTIL_AUTO_PRS_LNA_BYPASS]$

// <prs optional=true> SL_FEM_UTIL_BYPASS_PRS_COMBINE
// $[PRS_SL_FEM_UTIL_BYPASS_PRS_COMBINE]

// [PRS_SL_FEM_UTIL_BYPASS_PRS_COMBINE]$
// <<< sl:end pin_tool >>>
// <<< sl:end pin_tool >>>

#define SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE   0
#define SL_FEM_UTIL_RUNTIME_PHY_SELECT     0

#endif // SL_FEM_UTIL_CONFIG_H
