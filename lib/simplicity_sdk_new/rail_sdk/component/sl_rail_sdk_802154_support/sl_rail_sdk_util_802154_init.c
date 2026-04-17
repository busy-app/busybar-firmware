/***************************************************************************//**
 * @file
 * @brief sl_rail_sdk_util_802154_init.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_rail.h"
#include "sli_rail_util_callbacks.h" // for internal-only callback signatures
#include "sl_rail_sdk_util_802154_init.h"
#include "sl_rail_sdk_util_802154_protocol.h"
#include "app_assert.h"
#include "app_log.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/** Macro to determine array size. */
#define COMMON_UTILS_COUNTOF(a) (sizeof(a) / sizeof((a)[0]))

#define SL_RAIL_BUILTIN_TX_FIFO_BYTES 512
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// Provide weak overridable internal RX FIFO and RX Packet Buffer for
// non-UC protocols to share and for RAIL 2.x backwards compatibility.
static sl_rail_packet_queue_entry_t builtin_rx_packet_queue[SL_RAIL_BUILTIN_RX_PACKET_QUEUE_ENTRIES];
static SL_RAIL_DECLARE_FIFO_BUFFER(builtin_rx_fifo, SL_RAIL_BUILTIN_RX_FIFO_BYTES);
__WEAK sl_rail_packet_queue_entry_t * const sl_rail_builtin_rx_packet_queue_ptr = builtin_rx_packet_queue;
__WEAK sl_rail_fifo_buffer_align_t * const sl_rail_builtin_rx_fifo_ptr = builtin_rx_fifo;
__WEAK const uint16_t sl_rail_builtin_rx_packet_queue_entries = COMMON_UTILS_COUNTOF(builtin_rx_packet_queue);
__WEAK const uint16_t sl_rail_builtin_rx_fifo_bytes = sizeof(builtin_rx_fifo);

// Provide weak overridable internal RX FIFO and RX Packet Buffer for
// non-UC protocols to share and for RAIL 2.x backwards compatibility.
static SL_RAIL_DECLARE_FIFO_BUFFER(sli_tx_fifo_buffer_inst0, SL_RAIL_BUILTIN_TX_FIFO_BYTES);

/// RAIL handle for RAIL context
static sl_rail_handle_t sl_rail_sdk_handle = SL_RAIL_EFR32_HANDLE;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sl_rail_handle_t sl_rail_sdk_util_get_handle(void)
{
  return sl_rail_sdk_handle;
}

void sl_rail_sdk_util_init(void)
{
  sl_rail_status_t status;
  /// RAIL configuration
  sl_rail_config_t rail_init_config;
  // Initialize the rail_init_config structure
  rail_init_config.events_callback = &sli_rail_util_on_event;
  rail_init_config.rx_packet_queue_entries = sl_rail_builtin_rx_packet_queue_entries;
  rail_init_config.rx_fifo_bytes = sl_rail_builtin_rx_fifo_bytes;
  rail_init_config.p_rx_packet_queue = sl_rail_builtin_rx_packet_queue_ptr;
  rail_init_config.p_rx_fifo_buffer = sl_rail_builtin_rx_fifo_ptr;
  rail_init_config.tx_fifo_bytes = SL_RAIL_BUILTIN_TX_FIFO_BYTES;
  rail_init_config.tx_fifo_init_bytes = 0U;
  rail_init_config.p_tx_fifo_buffer = sli_tx_fifo_buffer_inst0;

  // initializes the RAIL core
  status = sl_rail_init(&sl_rail_sdk_handle,
                        &rail_init_config,
                        &sli_rail_util_on_rf_ready
                        );
  app_assert((SL_RAIL_STATUS_NO_ERROR == status),
             "sl_rail_init failed, return value: %ld", status);
  app_assert((NULL != sl_rail_sdk_handle),
             "sl_rail_init failed, return value: %ld", status);

  sl_rail_tx_data_config_t tx_data_config = {
    .tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA,
    .tx_method = SL_RAIL_DATA_METHOD_PACKET_MODE,
  };

  sl_rail_rx_data_config_t rx_data_config = {
    .rx_source = SL_RAIL_RX_DATA_SOURCE_PACKET_DATA,
    .rx_method = SL_RAIL_DATA_METHOD_PACKET_MODE,
  };

  status = sl_rail_config_tx_data(sl_rail_sdk_handle, &tx_data_config);
  app_assert((SL_RAIL_STATUS_NO_ERROR == status),
             "sl_rail_config_tx_data failed, return value: %lu",
             status);

  status = sl_rail_config_rx_data(sl_rail_sdk_handle, &rx_data_config);
  app_assert((SL_RAIL_STATUS_NO_ERROR == status),
             "sl_rail_config_rx_data failed, return value: %lu",
             status);

  // configures the channels
  const sl_rail_channel_config_t *channel_config = NULL;
  (void) sl_rail_config_channels(sl_rail_sdk_handle,
                                 channel_config,
                                 (sl_rail_radio_config_changed_callback_t)(void *)
                                 &sli_rail_util_on_channel_config_change);
  if (channel_config != NULL) {
    // Establish first channel by default
    // Perhaps someday this will be parameterized in the init config
    uint16_t channel = sl_rail_get_first_channel(sl_rail_sdk_handle,
                                                 channel_config);
    if (channel != SL_RAIL_CHANNEL_INVALID) {
      status = sl_rail_prepare_channel(sl_rail_sdk_handle, channel);
      app_assert((SL_RAIL_STATUS_NO_ERROR == status),
                 "sl_rail_prepare_channel failed, return value: %lu",
                 status);
    }
  }

  // configures the IEEE 802.15.4 protocol based on the chosen protocol by user
  status = sl_rail_sdk_util_802154_protocol_config(sl_rail_sdk_handle,
                                                   SL_RAIL_SDK_UTIL_INIT_PROTOCOL_INSTANCE_DEFAULT);
  app_assert((SL_RAIL_STATUS_NO_ERROR == status),
             "sl_rail_util_protocol_config failed, return value: %lu",
             status);

  // configures RAIL calibration
  status = sl_rail_config_cal(sl_rail_sdk_handle,
                              0U
                              | (0
                                 ? SL_RAIL_CAL_TEMP : 0U)
                              | (0
                                 ? SL_RAIL_CAL_ONETIME : 0U));
  app_assert((SL_RAIL_STATUS_NO_ERROR == status),
             "sl_rail_config_cal failed, return value: %lu",
             status);

  // configures the using RAIL events
  status = sl_rail_config_events(sl_rail_sdk_handle,
                                 SL_RAIL_EVENTS_ALL,
                                 SL_RAIL_EVENT_RX_PACKET_RECEIVED
                                 | SL_RAIL_EVENT_TX_PACKET_SENT
                                 | SL_RAIL_EVENT_CAL_NEEDED
                                 | SL_RAIL_EVENT_TXACK_PACKET_SENT
                                 | SL_RAIL_EVENT_RX_PACKET_ABORTED
                                 | SL_RAIL_EVENT_RX_FRAME_ERROR
                                 | SL_RAIL_EVENT_RX_FIFO_OVERFLOW
                                 | SL_RAIL_EVENT_RX_ADDRESS_FILTERED
                                 | SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED
                                 | SL_RAIL_EVENT_TX_ABORTED
                                 | SL_RAIL_EVENT_TX_BLOCKED
                                 | SL_RAIL_EVENT_TX_UNDERFLOW
                                 | SL_RAIL_EVENT_TX_CHANNEL_BUSY
                                 | SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED
                                 | SL_RAIL_EVENT_TXACK_ABORTED
                                 | SL_RAIL_EVENT_TXACK_BLOCKED
                                 | SL_RAIL_EVENT_TXACK_UNDERFLOW
                                 );
  app_assert((SL_RAIL_STATUS_NO_ERROR == status),
             "sl_rail_config_events failed, return value: %lu",
             status);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
