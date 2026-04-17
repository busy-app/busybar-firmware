/***************************************************************************//**
 * @file sl_wisun_rf_test.c
 * @brief Wi-SUN RF test API
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "sl_rail.h"
#include "sl_status.h"
#include "sl_wisun_api.h"
#include "sl_wisun_rf_test.h"
#include "sli_wisun_internal_api.h"
#include "sl_wisun_rf_test_tools.h"
#include "sl_wisun_trace_api.h"

#define SLI_WISUN_ERROR_CHECK_SET_STATUS(__result, __value)\
do {\
  if (!(__result)){\
    status = __value;\
    goto error_handler;\
  }\
} while(0)

#define SLI_WISUN_ERROR_SET_STATUS(__value)\
do {\
  status = __value;\
  goto error_handler;\
} while(0)

static uint8_t rf_test_running = RF_TEST_OFF;
static sl_rail_tx_power_t stack_tx_power;
static int8_t test_tx_power;
static uint8_t rf_test_phy_mode_id;
static uint8_t rf_test_reg_domain;
static uint16_t rf_test_physical_channel_offset;
static uint16_t rf_test_channel_start;
static uint16_t rf_test_channel_end;
static uint32_t rf_test_rx_packet_count;
static uint16_t rf_test_tx_remaining_count;

static sl_rail_fifo_buffer_align_t rf_test_tx_fifo[SL_RAIL_MAXIMUM_FIFO_BYTES / sizeof(sl_rail_fifo_buffer_align_t)];

static sl_rail_scheduler_info_t rf_scheduler_info =
{
  .priority = RF_PRIORITY_BACKGROUND,
  .slip_time = 0,
  .transaction_time = 0
};

static uint8_t rf_test_crc_length = 2;

static sl_rail_cal_values_t rf_phy_cal_values = SL_RAIL_CAL_VALUES_UNINIT;

static sl_status_t start_rf_test(uint16_t channel, sl_rail_stream_mode_t mode);
static sl_status_t stop_rf_test(uint8_t mode);

sl_status_t sl_wisun_start_stream(uint16_t channel)
{
  return start_rf_test(channel, SL_RAIL_STREAM_PN9_STREAM);
}

sl_status_t sl_wisun_stop_stream()
{
  return stop_rf_test(RF_TEST_STREAM);
}

sl_status_t sl_wisun_start_tone(uint16_t channel)
{
  return start_rf_test(channel, SL_RAIL_STREAM_CARRIER_WAVE);
}

sl_status_t sl_wisun_stop_tone()
{
  return stop_rf_test(RF_TEST_TONE);
}

bool sl_wisun_is_running_rf_test()
{
  return (rf_test_running != RF_TEST_OFF);
}

sl_status_t sl_wisun_set_test_tx_power(int8_t tx_power)
{
  test_tx_power = tx_power;
  return SL_STATUS_OK;
}

static sl_status_t check_rf_test(bool check_phy_set)
{
  sl_rail_handle_t rail_handle;
  sl_status_t status;
  sl_rail_status_t rail_status;
  sl_wisun_join_state_t join_state;

  // check if sl_wisun_rf_test_set_phy_config was called
  if (check_phy_set && (rf_test_phy_mode_id == 0 || rf_test_reg_domain == 0)) {
    SLI_WISUN_ERROR_SET_STATUS(SL_STATUS_NOT_READY);
  }

  status = sli_wisun_get_rail_handle(&rail_handle);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == status, SL_STATUS_NOT_READY);

  SLI_WISUN_ERROR_CHECK_SET_STATUS(rf_test_running == RF_TEST_OFF, SL_STATUS_BUSY);

  // RF tests should not be run at the same time as the Wi-SUN stack
  status = sl_wisun_get_join_state(&join_state);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_WISUN_JOIN_STATE_DISCONNECTED == join_state, SL_STATUS_NETWORK_UP);

  rail_status = sl_rail_config_events(rail_handle, MAC_TASK_RAIL_EVENTS | SL_RAIL_EVENT_CAL_NEEDED, MAC_TASK_RAIL_EVENTS | SL_RAIL_EVENT_CAL_NEEDED);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  status = SL_STATUS_OK;
error_handler:
    return status;
}

sl_status_t sl_wisun_rf_test_set_phy_config(sl_wisun_phy_config_t *phy_config)
{
  sl_status_t status;
  sl_rail_channel_config_entry_t chan_config;
  uint8_t phy_mode_id = 0;
  uint8_t reg_domain = 0;

  if (phy_config == NULL) {
    SLI_WISUN_ERROR_SET_STATUS(SL_STATUS_INVALID_PARAMETER);
  }

  status = check_rf_test(false);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == status, status);

  status = rf_test_phy_config_to_chan_config(phy_config, &chan_config, &phy_mode_id, &reg_domain, &rf_test_physical_channel_offset, &rf_test_channel_start, &rf_test_channel_end);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == status, status);
  if (status != SL_STATUS_OK || chan_config.p_stack_info == NULL) {
    SLI_WISUN_ERROR_SET_STATUS(status);
  }
  rf_test_phy_mode_id = phy_mode_id;
  rf_test_reg_domain = reg_domain;

  status = SL_STATUS_OK;
error_handler:
  return status;
}

static void rf_test_tx_sent_callback(void)
{
  if (rf_test_running != RF_TEST_TX_ACTIVE) {
    sl_wisun_trace_error("rf_test: unexpected TX");
    stop_rf_test(RF_TEST_TX_ACTIVE);
    return;
  }

  if (rf_test_tx_remaining_count > 0) {
    rf_test_tx_remaining_count--;
  }

  if (rf_test_tx_remaining_count == 0) {
    stop_rf_test(RF_TEST_TX_ACTIVE);
  }
}

static void rf_test_tx_failure_callback(void)
{
  sl_wisun_trace_warn("rf_test: TX failure");
  if (rf_test_running != RF_TEST_TX_ACTIVE) {
    sl_wisun_trace_error("rf_test: unexpected TX");
    stop_rf_test(RF_TEST_TX_ACTIVE);
    return;
  }

  if (rf_test_tx_remaining_count > 0) {
    rf_test_tx_remaining_count--;
  }

  if (rf_test_tx_remaining_count == 0) {
    stop_rf_test(RF_TEST_TX_ACTIVE);
  }
}

static void rf_test_rx_received_callback(int8_t rssi)
{
  rf_test_rx_packet_count++;
  printf("RF test RX rssi=%d dBm, packets since start_rx: %lu\r\n",
         rssi,
         (unsigned long)rf_test_rx_packet_count);
}

void sl_wisun_rf_test_event_callback(uint64_t events, int8_t rssi)
{
  if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
    rf_test_tx_sent_callback();
  }
  if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
    rf_test_rx_received_callback(rssi);
  }
  if (events & SL_RAIL_EVENT_TX_UNDERFLOW) {
    rf_test_tx_failure_callback();
  }
}

static void rf_test_rx_timer_callback(sl_rail_handle_t rail_handle)
{
  (void)rail_handle;
  sl_wisun_rf_test_rx_stop();
}

sl_status_t sl_wisun_rf_test_start_tx(uint16_t channel,
                                      uint16_t count,
                                      uint16_t data_length,
                                      uint8_t *data,
                                      uint32_t interval_ms,
                                      bool cca_enabled)
{
  sl_rail_status_t rail_status;
  sl_status_t status;
  sl_rail_tx_options_t options = SL_RAIL_TX_OPTIONS_DEFAULT | SL_RAIL_TX_OPTION_RESEND;
  sl_rail_handle_t rail_handle;
  sl_rail_csma_config_t csma_config = SL_RAIL_CSMA_CONFIG_SINGLE_CCA;
  uint16_t fifo_size_bytes = 0;
  uint16_t init_bytes = 0;
  uint8_t phr_length = 0;
  bool use_phr = true;

  sl_wisun_set_event_filter(&sl_wisun_broadcast_mac,
                            SL_WISUN_LOGGER_EVENT_TYPE_RF_TEST);

  status = check_rf_test(true);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == status, status);

  if (count == 0 || data_length == 0 || interval_ms == 0 || data_length > MAX_PACKET_LENGTH) {
    SLI_WISUN_ERROR_SET_STATUS(SL_STATUS_INVALID_PARAMETER);
  }

  rail_status = sli_wisun_get_rail_handle(&rail_handle);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == rail_status, SL_STATUS_FAIL);

  // Backup stack Tx Power
  stack_tx_power = sl_rail_get_tx_power_dbm(rail_handle);

  rail_status = sl_rail_set_tx_power_dbm(rail_handle, 10*test_tx_power);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  // Add the physical channel offset to the channel
  channel = rf_test_physical_channel_offset + channel;

  rail_status = sl_rail_is_valid_channel(rail_handle, channel);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_INVALID_PARAMETER);
  if (channel < rf_test_channel_start || channel > rf_test_channel_end) {
    sl_wisun_trace_error("rf_test: channel %u is out of range", channel);
    SLI_WISUN_ERROR_SET_STATUS(SL_STATUS_INVALID_PARAMETER);
  }

  sl_rail_calibrate(rail_handle, &rf_phy_cal_values, SL_RAIL_CAL_ALL_PENDING);

  rf_test_running = RF_TEST_TX_ACTIVE;
  rf_test_tx_remaining_count = count;

  if (!rf_test_prepare_tx_buffer(rf_test_phy_mode_id,
                                 data_length,
                                 data,
                                 use_phr,
                                 rf_test_crc_length,
                                 (uint8_t *)rf_test_tx_fifo,
                                 SL_RAIL_MAXIMUM_FIFO_BYTES,
                                 &fifo_size_bytes,
                                 &phr_length,
                                 &init_bytes)) {
    SLI_WISUN_ERROR_SET_STATUS(SL_STATUS_INVALID_PARAMETER);
  }

  rf_scheduler_info.priority = RF_PRIORITY_PROTECTED;
  sl_rail_state_transitions_t tx_transitions = {
    .success = SL_RAIL_RF_STATE_IDLE,
    .error = SL_RAIL_RF_STATE_IDLE
  };
  rail_status = sl_rail_set_tx_transitions(rail_handle, &tx_transitions);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  sl_rail_reset_fifo(rail_handle, true, false);

  if (count > 1) {
    sl_rail_tx_repeat_config_t repeat_config = {
      .iterations = (uint16_t)(count - 1),
      .repeat_options = 0,
      .delay_or_hop = {
        .delay_us = (sl_rail_transition_time_t)(interval_ms * 1000)
      }
    };
    rail_status = sl_rail_set_next_tx_repeat(rail_handle, &repeat_config);
    SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);
  }
  rail_status = sl_rail_set_tx_fifo(rail_handle,
                                    rf_test_tx_fifo,
                                    fifo_size_bytes,
                                    init_bytes,
                                    0);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  if (cca_enabled) {
    rail_status = sl_rail_start_cca_csma_tx(rail_handle,
                                            channel,
                                            options,
                                            &csma_config,
                                            &rf_scheduler_info);
  } else {
    rail_status = sl_rail_start_tx(rail_handle, channel, options, &rf_scheduler_info);
  }
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  status = SL_STATUS_OK;
error_handler:
  if (status != SL_STATUS_OK) {
    stop_rf_test(RF_TEST_TX_ACTIVE);
  }
  return status;
}

sl_status_t sl_wisun_rf_test_start_rx(uint16_t channel, uint32_t duration)
{
  sl_status_t status;
  sl_rail_status_t rail_status;
  sl_rail_handle_t rail_handle;

  sl_wisun_set_event_filter(&sl_wisun_broadcast_mac,
                            SL_WISUN_LOGGER_EVENT_TYPE_RF_TEST);

  rf_test_rx_packet_count = 0;

  status = check_rf_test(true);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == status, status);

  rail_status = sli_wisun_get_rail_handle(&rail_handle);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == rail_status, SL_STATUS_FAIL);

  // Add the physical channel offset to the channel
  channel = rf_test_physical_channel_offset + channel;

  rail_status = sl_rail_is_valid_channel(rail_handle, channel);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_INVALID_PARAMETER);
  if (channel < rf_test_channel_start || channel > rf_test_channel_end) {
    sl_wisun_trace_error("rf_test: channel %u is out of range", channel);
    SLI_WISUN_ERROR_SET_STATUS(SL_STATUS_INVALID_PARAMETER);
  }
  rf_test_running = RF_TEST_RX_ACTIVE;
  rf_scheduler_info.priority = RF_PRIORITY_PROTECTED;
  //tx power will be set during the stop proceedure
  stack_tx_power = sl_rail_get_tx_power_dbm(rail_handle);

  rail_status = sl_rail_start_rx(rail_handle, channel, &rf_scheduler_info);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  if (duration > 0) {
    rail_status = sl_rail_set_timer(rail_handle,
                                    (sl_rail_time_t)duration * 1000,
                                    SL_RAIL_TIME_DELAY,
                                    rf_test_rx_timer_callback);
    SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);
  }

  status = SL_STATUS_OK;
error_handler:
  if (status != SL_STATUS_OK) {
    stop_rf_test(RF_TEST_RX_ACTIVE);
  }
  return status;
}

sl_status_t sl_wisun_rf_test_rx_stop()
{
  return stop_rf_test(RF_TEST_RX_ACTIVE);
}

static sl_status_t start_rf_test(uint16_t channel, sl_rail_stream_mode_t mode)
{
  sl_rail_status_t rail_status;
  sl_status_t status;
  sl_rail_handle_t rail_handle;

  status = check_rf_test(true);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == status, status);

  status = sli_wisun_get_rail_handle(&rail_handle);
  // Add the physical channel offset to the channel
  channel = rf_test_physical_channel_offset + channel;

  rail_status = sl_rail_is_valid_channel(rail_handle, channel);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_INVALID_PARAMETER);
  if (channel < rf_test_channel_start || channel > rf_test_channel_end) {
    sl_wisun_trace_error("rf_test: channel %u is out of range", channel);
    SLI_WISUN_ERROR_SET_STATUS(SL_STATUS_INVALID_PARAMETER);
  }

  // Backup stack Tx Power
  stack_tx_power = sl_rail_get_tx_power_dbm(rail_handle);

  rail_status = sl_rail_set_tx_power_dbm(rail_handle, 10*test_tx_power);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  rail_status = sl_rail_start_tx_stream(rail_handle, channel, mode, SL_RAIL_TX_OPTIONS_DEFAULT);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  switch (mode) {
    case SL_RAIL_STREAM_PN9_STREAM:
      rf_test_running = RF_TEST_STREAM;
      break;
    case SL_RAIL_STREAM_CARRIER_WAVE:
      rf_test_running = RF_TEST_TONE;
      break;
  }

  status = SL_STATUS_OK;
error_handler:
  return status;
}

static sl_status_t stop_rf_test(uint8_t mode)
{
  sl_rail_status_t rail_status;
  sl_status_t status;
  sl_rail_handle_t rail_handle;

  status = sli_wisun_get_rail_handle(&rail_handle);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_STATUS_OK == status, SL_STATUS_NOT_READY);

  if (mode == RF_TEST_STREAM || mode == RF_TEST_TONE) {
    rail_status = sl_rail_stop_tx_stream(rail_handle);
    SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);
  }
  if (mode == RF_TEST_RX_ACTIVE) {
    sl_rail_cancel_timer(rail_handle);
    rail_status = sl_rail_idle(rail_handle, SL_RAIL_IDLE_ABORT, true);
    SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);
  }

  rail_status = sl_rail_set_tx_power_dbm(rail_handle, stack_tx_power);
  SLI_WISUN_ERROR_CHECK_SET_STATUS(SL_RAIL_STATUS_NO_ERROR == rail_status, SL_STATUS_FAIL);

  rf_test_running = RF_TEST_OFF;

  status = SL_STATUS_OK;
error_handler:
  return status;
}
