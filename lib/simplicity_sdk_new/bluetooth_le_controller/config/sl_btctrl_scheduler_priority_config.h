/***************************************************************************//**
 * @file
 * @brief Bluetooth Scheduler Priority configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_BTCTRL_SCHEDULER_PRIORITY_CONFIG_H
#define SL_BTCTRL_SCHEDULER_PRIORITY_CONFIG_H
#include "sl_btctrl_linklayer_defs.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Bluetooth Low Energy Scheduler Priorities

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MIN> Scan - Min
// <0-255:1>
// <i> Default: 191
// <i> Define the lowest permitted BLE scan priority. [0-255]
// <i> A lower value indicates a higher priority.
// <i> Scanning is typically considered to be a low priority
// <i> operation, and the default values are set to reflect that.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MIN
#define SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MIN        191
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MIN

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MAX> Scan - Max
// <0-255:1>
// <i> Default: 143
// <i> Define the highest permitted BLE scan priority. [0-255]
// <i> A lower value indicates a higher priority.
// <i> Scanning is typically considered to be a low priority
// <i> operation, and the default values are set to reflect that.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MAX
#define SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MAX        143
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MAX

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_STEP> Scan - Step
// <1-255:1>
// <i> Default: 4
// <i> Define the step size of an increment or decrement of the scan priority.
// <i> This defines how quickly the scanner adjusts the task priority
// <i> in the event of a scheduling conflict.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_STEP
#define SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_STEP        4
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_STEP

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MIN> Advertisement - Min
// <0-255:1>
// <i> Default: 175
// <i> Define the lowest permitted BLE advertisement priority. [0-255]
// <i> A lower value indicates a higher priority.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MIN
#define SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MIN        175
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MIN

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MAX> Advertisement - Max
// <0-255:1>
// <i> Default: 127
// <i> Define the highest permitted BLE advertisement priority. [0-255]
// <i> A lower value indicates a higher priority.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MAX
#define SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MAX        127
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MAX

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_STEP> Advertisement - Step
// <1-255:1>
// <i> Default: 4
// <i> Define the step size of an increment or decrement of the advertisement priority.
// <i> This defines how quickly the advertiser adjusts the task priority
// <i> in the event of a scheduling conflict.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_STEP
#define SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_STEP        4
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_STEP

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MIN> Connection - Min
// <0-255:1>
// <i> Default: 135
// <i> Define the lowest permitted BLE connection priority. [0-255]
// <i> A lower value indicates a higher priority.
// <i> The minimum connection priority is by default above the priority of scanning
// <i> and advertising. Connection priority is calculated by how close to the
// <i> supervision timeout the connection is. In good link conditions, the
// <i> connection priority is close to the minimum specified value.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MIN
#define SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MIN        135
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MIN

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MAX> Connection - Max
// <0-255:1>
// <i> Default: 0
// <i> Define the highest permitted BLE connection priority. [0-255]
// <i> A lower value indicates a higher priority.
// <i> The default is equal to the maximum possible value. If the connection
// <i> priority has risen to the maximum value then this is likely the last connection
// <i> event before the supervision timeout.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MAX
#define SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MAX         0
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MAX

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MIN> Connection Initiation - Min
// <0-255:1>
// <i> Default: 55
// <i> Define the lowest permitted BLE connection initiation priority. [0-255]
// <i> A lower value indicates a higher priority.
// <i> The scanning priority during connection establishment.
// <i> The default value gives a relatively high minimum priority to connection
// <i> establishment.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MIN
#define SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MIN         55
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MIN

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MAX> Connection Initiation - Max
// <0-255:1>
// <i> Default: 15
// <i> Define the highest permitted BLE connection initiation priority. [0-255]
// <i> A lower value indicates a higher priority.
// <i> This default value prevents the initiation task from consuming too much
// <i> high priority connection task time.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MAX
#define SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MAX         15
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MAX

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MIN> Rail Mapping Window Min
// <0-255:1>
// <i> Default: 16
// <i> Define the minimum value of RAIL mapping window.
// <i> This parameter specifies how task priorities are passed to radio
// <i> coexistence and multiprotocol schedulers.
// <i> Internal task priorities in Bluetooth should always use maximum range of 0-255,
// <i> and use this mapping window to adapt to other protocols.
// <i> Internal task priorities are linear scaled to the window before passing
// <i> to RAIL.
// <i> If coexistence or multiprotocol configuration is not used, then these
// <i> parameters have no effect.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MIN
#define SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MIN 16
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MIN

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MAX> Rail Mapping Window Range
// <0-255:1>
// <i> Default: 32
// <i> Define the maximum value of the RAIL mapping window.
// <i> See SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MIN for a description of
// <i> this parameter.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MAX
#define SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MAX 32
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MAX

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MIN> PAwR Tx - Min
// <0-255:1>
// <i> Default: 15
// <i> Define the lowest permitted BLE PAwR transmission priority. [0-255]
// <i> A lower value indicates a higher priority.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MIN
#define SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MIN        15
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MIN

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MAX> PAwR Tx - Max
// <0-255:1>
// <i> Default: 5
// <i> Define the highest permitted BLE PAwR transmission priority. [0-255]
// <i> A lower value indicates a higher priority.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MAX
#define SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MAX        5
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MAX

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MIN> PAwR Rx - Min
// <0-255:1>
// <i> Default: 20
// <i> Define the lowest permitted BLE PAwR reception priority. [0-255]
// <i> A lower value indicates a higher priority.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MIN
#define SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MIN        20
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MIN

// <o SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MAX> PAwR Rx - Max
// <0-255:1>
// <i> Default: 10
// <i> Define the highest permitted BLE PAwR reception priority. [0-255]
// <i> A lower value indicates a higher priority.
#ifndef SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MAX
#define SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MAX        10
#endif // SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MAX

// </h>

// <<< end of configuration section >>>

//Default priority configuration
#define SL_BTCTRL_SCHEDULER_PRIORITIES {                                      \
    .scan_min = SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MIN,                      \
    .scan_max = SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_MAX,                      \
    .adv_min = SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MIN,                        \
    .adv_max = SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_MAX,                        \
    .conn_min = SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MIN,                      \
    .conn_max = SL_BT_CONTROLLER_SCHEDULER_PRI_CONN_MAX,                      \
    .init_min = SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MIN,                      \
    .init_max = SL_BT_CONTROLLER_SCHEDULER_PRI_INIT_MAX,                      \
    .rail_mapping_offset = SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MIN,    \
    .rail_mapping_range = (SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MAX     \
                           - SL_BT_CONTROLLER_SCHEDULER_PRI_RAIL_WINDOW_MIN), \
    0,                                                                        \
    .adv_step = SL_BT_CONTROLLER_SCHEDULER_PRI_ADV_STEP,                      \
    .scan_step = SL_BT_CONTROLLER_SCHEDULER_PRI_SCAN_STEP,                    \
    .pawr_tx_min = SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MIN,                \
    .pawr_tx_max = SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_TX_MAX,                \
    .pawr_rx_min = SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MIN,                \
    .pawr_rx_max = SL_BT_CONTROLLER_SCHEDULER_PRI_PAWR_RX_MAX                 \
}

#endif // SL_BTCTRL_SCHEDULER_PRIORITY_CONFIG_H
