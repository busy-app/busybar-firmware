/***************************************************************************//**
 * @file
 * @brief sli_rail_util_callbacks.h
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_RAIL_SDK_UTIL_BLE_INIT_H
#define SL_RAIL_SDK_UTIL_BLE_INIT_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_rail.h"
#include "sl_rail_sdk_util_ble_init_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**
 * An event mask, available to the application, specifying the radio events
 * setup within the init code.
 *
 * @note: Because the value of this define is evaluated based on values in the
 * \ref sl_rail_events_t enum, this define will only have a valid value during
 * run-time.
 */
#define SL_RAIL_UTIL_INIT_EVENT_BLE_MASK (SL_RAIL_EVENTS_NONE                                                       \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RSSI_AVERAGE_DONE_BLE_ENABLE                   \
                                             ? SL_RAIL_EVENT_RSSI_AVERAGE_DONE : SL_RAIL_EVENTS_NONE)               \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_ACK_TIMEOUT_BLE_ENABLE                      \
                                             ? SL_RAIL_EVENT_RX_ACK_TIMEOUT : SL_RAIL_EVENTS_NONE)                  \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_FIFO_ALMOST_FULL_BLE_ENABLE                 \
                                             ? SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL : SL_RAIL_EVENTS_NONE)             \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_PACKET_RECEIVED_BLE_ENABLE                  \
                                             ? SL_RAIL_EVENT_RX_PACKET_RECEIVED : SL_RAIL_EVENTS_NONE)              \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_PREAMBLE_LOST_BLE_ENABLE                    \
                                             ? SL_RAIL_EVENT_RX_PREAMBLE_LOST : SL_RAIL_EVENTS_NONE)                \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_PREAMBLE_DETECT_BLE_ENABLE                  \
                                             ? SL_RAIL_EVENT_RX_PREAMBLE_DETECT : SL_RAIL_EVENTS_NONE)              \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_FRAME_ERROR_BLE_ENABLE                      \
                                             ? SL_RAIL_EVENT_RX_FRAME_ERROR : SL_RAIL_EVENTS_NONE)                  \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_FIFO_FULL_BLE_ENABLE                        \
                                             ? SL_RAIL_EVENT_RX_FIFO_FULL : SL_RAIL_EVENTS_NONE)                    \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_FIFO_OVERFLOW_BLE_ENABLE                    \
                                             ? SL_RAIL_EVENT_RX_FIFO_OVERFLOW : SL_RAIL_EVENTS_NONE)                \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_ADDRESS_FILTERED_BLE_ENABLE                 \
                                             ? SL_RAIL_EVENT_RX_ADDRESS_FILTERED : SL_RAIL_EVENTS_NONE)             \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_TIMEOUT_BLE_ENABLE                          \
                                             ? SL_RAIL_EVENT_RX_TIMEOUT : SL_RAIL_EVENTS_NONE)                      \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_TX_SCHEDULED_RX_TX_STARTED_BLE_ENABLE       \
                                             ? SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED : SL_RAIL_EVENTS_NONE)         \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_SCHEDULED_RX_END_BLE_ENABLE                 \
                                             ? SL_RAIL_EVENT_RX_SCHEDULED_RX_END : SL_RAIL_EVENTS_NONE)             \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_SCHEDULED_RX_MISSED_BLE_ENABLE              \
                                             ? SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED : SL_RAIL_EVENTS_NONE)          \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_PACKET_ABORTED_BLE_ENABLE                   \
                                             ? SL_RAIL_EVENT_RX_PACKET_ABORTED : SL_RAIL_EVENTS_NONE)               \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_FILTER_PASSED_BLE_ENABLE                    \
                                             ? SL_RAIL_EVENT_RX_FILTER_PASSED : SL_RAIL_EVENTS_NONE)                \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_TIMING_LOST_BLE_ENABLE                      \
                                             ? SL_RAIL_EVENT_RX_TIMING_LOST : SL_RAIL_EVENTS_NONE)                  \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_TIMING_DETECT_BLE_ENABLE                    \
                                             ? SL_RAIL_EVENT_RX_TIMING_DETECT : SL_RAIL_EVENTS_NONE)                \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_CHANNEL_HOPPING_COMPLETE_BLE_ENABLE         \
                                             ? SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE : SL_RAIL_EVENTS_NONE)     \
                                          | (SL_RAIL_UTIL_INIT_EVENT_RX_DUTY_CYCLE_RX_END_BLE_ENABLE                \
                                             ? SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END : SL_RAIL_EVENTS_NONE)            \
                                          | (SL_RAIL_UTIL_INIT_EVENT_IEEE802154_DATA_REQUEST_COMMAND_BLE_ENABLE     \
                                             ? SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND : SL_RAIL_EVENTS_NONE) \
                                          | (SL_RAIL_UTIL_INIT_EVENT_ZWAVE_BEAM_BLE_ENABLE                          \
                                             ? SL_RAIL_EVENT_ZWAVE_BEAM : SL_RAIL_EVENTS_NONE)                      \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_FIFO_ALMOST_EMPTY_BLE_ENABLE                \
                                             ? SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY : SL_RAIL_EVENTS_NONE)            \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_PACKET_SENT_BLE_ENABLE                      \
                                             ? SL_RAIL_EVENT_TX_PACKET_SENT : SL_RAIL_EVENTS_NONE)                  \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TXACK_PACKET_SENT_BLE_ENABLE                   \
                                             ? SL_RAIL_EVENT_TXACK_PACKET_SENT : SL_RAIL_EVENTS_NONE)               \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_ABORTED_BLE_ENABLE                          \
                                             ? SL_RAIL_EVENT_TX_ABORTED : SL_RAIL_EVENTS_NONE)                      \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TXACK_ABORTED_BLE_ENABLE                       \
                                             ? SL_RAIL_EVENT_TXACK_ABORTED : SL_RAIL_EVENTS_NONE)                   \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_BLOCKED_BLE_ENABLE                          \
                                             ? SL_RAIL_EVENT_TX_BLOCKED : SL_RAIL_EVENTS_NONE)                      \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TXACK_BLOCKED_BLE_ENABLE                       \
                                             ? SL_RAIL_EVENT_TXACK_BLOCKED : SL_RAIL_EVENTS_NONE)                   \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_UNDERFLOW_BLE_ENABLE                        \
                                             ? SL_RAIL_EVENT_TX_UNDERFLOW : SL_RAIL_EVENTS_NONE)                    \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TXACK_UNDERFLOW_BLE_ENABLE                     \
                                             ? SL_RAIL_EVENT_TXACK_UNDERFLOW : SL_RAIL_EVENTS_NONE)                 \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_CHANNEL_CLEAR_BLE_ENABLE                    \
                                             ? SL_RAIL_EVENT_TX_CHANNEL_CLEAR  : SL_RAIL_EVENTS_NONE)               \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_CHANNEL_BUSY_BLE_ENABLE                     \
                                             ? SL_RAIL_EVENT_TX_CHANNEL_BUSY : SL_RAIL_EVENTS_NONE)                 \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_CCA_RETRY_BLE_ENABLE                        \
                                             ? SL_RAIL_EVENT_TX_CCA_RETRY : SL_RAIL_EVENTS_NONE)                    \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_START_CCA_BLE_ENABLE                        \
                                             ? SL_RAIL_EVENT_TX_START_CCA : SL_RAIL_EVENTS_NONE)                    \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_STARTED_BLE_ENABLE                          \
                                             ? SL_RAIL_EVENT_TX_STARTED : SL_RAIL_EVENTS_NONE)                      \
                                          | (SL_RAIL_UTIL_INIT_EVENT_TX_SCHEDULED_TX_MISSED_BLE_ENABLE              \
                                             ? SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED : SL_RAIL_EVENTS_NONE)          \
                                          | (SL_RAIL_UTIL_INIT_EVENT_CONFIG_UNSCHEDULED_BLE_ENABLE                  \
                                             ? SL_RAIL_EVENT_CONFIG_UNSCHEDULED : SL_RAIL_EVENTS_NONE)              \
                                          | (SL_RAIL_UTIL_INIT_EVENT_CONFIG_SCHEDULED_BLE_ENABLE                    \
                                             ? SL_RAIL_EVENT_CONFIG_SCHEDULED : SL_RAIL_EVENTS_NONE)                \
                                          | (SL_RAIL_UTIL_INIT_EVENT_SCHEDULER_STATUS_BLE_ENABLE                    \
                                             ? SL_RAIL_EVENT_SCHEDULER_STATUS : SL_RAIL_EVENTS_NONE)                \
                                          | (SL_RAIL_UTIL_INIT_EVENT_CAL_NEEDED_BLE_ENABLE                          \
                                             ? SL_RAIL_EVENT_CAL_NEEDED : SL_RAIL_EVENTS_NONE))

/**
 * An inverted event mask, available to the application, specifying the radio
 * events setup within the init code.
 *
 * @note: Because the value of this define is evaluated based on values in the
 * \ref sl_rail_events_t enum, this define will only have a valid value during
 * run-time.
 */
#define SL_RAIL_UTIL_INIT_EVENT_BLE_INVERSE_MASK \
  (~SL_RAIL_UTIL_INIT_EVENT_BLE_MASK)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * Initialize the RAIL Init Utility.
 *
 * @note: This function should be called during application initialization.
 */
void sl_rail_sdk_util_init(void);

/**
 * Get the RAIL handle created during initialization.
 * @param[in] handle The enum name of the desired RAIL handle.
 *
 * @return A valid RAIL handle. If the RAIL handle hasn't been set up, the
 * invalid value of \ref SL_RAIL_EFR32_HANDLE will be returned.
 */
sl_rail_handle_t sl_rail_sdk_util_get_handle(void);

/**
 * A callback available to the application, called on RAIL asserts.
 *
 * @param[in] rail_handle The RAIL handle associated with the assert.
 * @param[in] error_code The assertion error code.
 */
void sl_rail_util_on_assert_failed(sl_rail_handle_t rail_handle,
                                   sl_rail_assert_error_codes_t error_code);

/**
 * A callback available to the application, called on RAIL init completion.
 *
 * @param[in] rail_handle The RAIL handle associated with the RAIL init
 * completion notification.
 */
void sl_rail_util_on_rf_ready(sl_rail_handle_t rail_handle);

/**
 * A callback available to the application, called on a channel configuration
 * change.
 *
 * @param[in] rail_handle The RAIL handle associated with the channel config
 * change notification.
 * @param[in] entry The channel configuration being changed to.
 */
void sl_rail_util_on_channel_config_change(sl_rail_handle_t rail_handle,
                                           const sl_rail_channel_config_entry_t *entry);

/**
 * A callback available to the application, called on registered RAIL events.
 *
 * @param[in] rail_handle The RAIL handle associated with the RAIL event
 * notification.
 * @param[in] events The RAIL events having occurred.
 */
void sl_rail_util_on_event(sl_rail_handle_t rail_handle,
                           sl_rail_events_t events);

#endif // SL_RAIL_SDK_UTIL_BLE_INIT_H
