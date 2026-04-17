/***************************************************************************//**
 * @file
 * @brief The MFM specific header file for the RAIL library.
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

#ifndef SL_RAIL_MFM_H
#define SL_RAIL_MFM_H

#include "sl_rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @addtogroup MFM Multi-Level Frequency Modulation
/// @ingroup Protocol_Specific
/// @brief MFM configuration routines
///
/// @note This feature is only supported on EFR32xG23 devices.
///
/// This feature can be used to directly control the TX interpolation filter
/// input to allow for a more flexible frequency modulation scheme than the
/// standard MODEM. When doing this, the MFM buffer is treated as an array
/// of 8-bit signed data used as normalized frequency deviation to the SYNTH
/// frequency to directly control the interpolation filter input.
/// No support for frame handling, coding, nor shaping is supported.
/// Only compatible with FSK modulations.
///
/// The functions in this group configure RAIL Multi-Level Frequency Modulation (MFM)
/// hardware acceleration features.
///
/// To configure MFM functionality, the application must first set up
/// a RAIL instance with \ref sl_rail_init() and other setup functions.
/// Before enabling MFM, a ping-pong buffer (called buffer0 and buffer1
/// below) must be configured via \ref sl_rail_set_mfm_ping_pong_fifo() and
/// populated with the initial buffer content.
/// MFM is enabled by setting \ref sl_rail_tx_data_source_t::SL_RAIL_TX_DATA_SOURCE_MFM_DATA using
/// \ref sl_rail_config_tx_data() and is activated when transmit is started by
/// \ref sl_rail_start_tx(). Once transmitting the data in the ping-pong buffers,
/// RAIL will manage them so it looks like a continuous transmission to the
/// receiver. Every time one of the ping-pong buffers has been transmitted,
/// \ref SL_RAIL_EVENT_MFM_TX_BUFFER_DONE is triggered so the application can
/// update the data in that buffer without the need to start/stop the
/// transmission. \ref SL_RAIL_EVENT_MFM_TX_BUFFER_DONE can be enable with \ref
/// sl_rail_config_events().
/// Use \ref sl_rail_stop_tx() to finish transmitting.
/// @code{.c}
/// #define MFM_RAW_BUF_WORDS 128
/// extern sl_rail_handle_t rail_handle;
/// uint8_t tx_count = 0;
/// uint32_t mfm_ping_pong_buffers[2][MFM_RAW_BUF_WORDS];
///
/// typedef struct mfm_config_app {
///   sl_rail_mfm_ping_pong_buffer_config_t buffer;
///   sl_rail_state_timing_t timings;
///   sl_rail_tx_data_config_t tx_data_config;
/// } mfm_config_app_t;
///
/// static mfm_config_app_t mfm_config = {
///   .buffer = {
///     .p_buffer_0 = (&mfm_ping_pong_buffers[0]),
///     .p_buffer_1 = (&mfm_ping_pong_buffers[1]),
///     .buffer_words = MFM_RAW_BUF_WORDS,
///   },
///   .timings = {
///     .idle_to_tx = 100,
///     .idle_to_rx = 0,
///     .rx_to_tx = 0,
///     .tx_to_rx = 0,
///     .rxsearch_timeout = 0,
///     .tx_to_rxsearch_timeout = 0
///   },
///   .tx_data_config = {
///      .tx_source = SL_RAIL_TX_DATA_SOURCE_MFM_DATA;
///      .tx_method = SL_RAIL_DATA_METHOD_PACKET_MODE,
///   },
/// };
///
/// // Main RAIL events handler callback
/// static void events_handler(sl_rail_handle_t rail_handle, sl_rail_events_t events)
/// {
///   // Increment TX counter
///   if (events & SL_RAIL_EVENT_MFM_BUF_DONE) {
///       tx_count++;
///       return;
///     }
///   }
/// }
///
/// void mfm_init(void)
/// {
///   // initialize MFM
///   uint32_t idx;
///   uint32_t *p_dst_0 = mfm_config.buffer.p_buffer_0;
///   uint32_t *p_dst_1 = mfm_config.buffer.p_buffer_1;
///   for (idx = 0; idx < (mfm_config.buffer.buffer_words / 4); idx++) {
///     p_dst_0[4 * idx + 0] = 0x755A3100;
///     p_dst_1[4 * idx + 0] = 0x755A3100;
///     p_dst_0[4 * idx + 1] = 0x315A757F;
///     p_dst_1[4 * idx + 1] = 0x315A757F;
///     p_dst_0[4 * idx + 2] = 0x8BA6CF00;
///     p_dst_1[4 * idx + 2] = 0x8BA6CF00;
///     p_dst_0[4 * idx + 3] = 0xCFA68B81;
///     p_dst_1[4 * idx + 3] = 0xCFA68B81;
///   }
///
///   sl_rail_status_t status;
///   status = sl_rail_set_mfm_ping_pong_fifo(rail_handle, &mfm_config.buffer);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   status = sl_rail_set_state_timing(rail_handle, &mfm_config.timings);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   mfm_config.tx_data_config.tx_source = SL_RAIL_TX_DATA_SOURCE_MFM_DATA;
///   status = sl_rail_config_tx_data(rail_handle, &mfm_config.tx_data_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   // start transmitting
///   status = sl_rail_start_tx(rail_handle, 0, 0, NULL);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
/// }
///
/// void mfm_deinit(void)
/// {
///   sl_rail_status_t status;
///   status = sl_rail_stop_tx(rail_handle, SL_RAIL_STOP_MODES_ALL);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   mfm_config.tx_data_config.tx_source = SL_RAIL_TX_DATA_SOURCE_PACKET_DATA;
///   status = sl_rail_config_tx_data(rail_handle, &mfm_config.tx_data_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
/// }
/// @endcode
///
/// @{

/**
 * @struct sl_rail_mfm_ping_pong_buffer_config_t
 * @brief A configuration structure for MFM Ping-pong buffer in RAIL.
 */
typedef struct sl_rail_mfm_ping_pong_buffer_config {
  /**
   * A pointer to buffer 0. Must be 32-bit aligned.
   */
  uint32_t *p_buffer_0;
  /**
   * A pointer to buffer 1. Must be 32-bit aligned.
   */
  uint32_t *p_buffer_1;
  /**
   * Size of each buffer in 32-bit words.
   */
  uint32_t buffer_words;
} sl_rail_mfm_ping_pong_buffer_config_t;

/**
 * Set MFM ping-pong buffer.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A non-NULL pointer to the MFM ping-pong buffer configuration structure.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_set_mfm_ping_pong_fifo(sl_rail_handle_t rail_handle,
                                                const sl_rail_mfm_ping_pong_buffer_config_t *p_config);

/** @} */ // end of MFM

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_MFM_H
