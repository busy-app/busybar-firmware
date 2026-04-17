/***************************************************************************//**
 * @file
 * @brief app_measurement.c
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
#include <string.h>
#include <stdlib.h>
#include "sl_core.h"
#include "sl_rail.h"
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
#include "sl_rail_util_init.h"
#endif
#include "sl_rail_ieee802154.h"
#include "sl_rail_ble.h"
#include "app_measurement.h"
#if defined(SL_CATALOG_GLIB_PRESENT)
#include "app_graphics.h"
#endif
#include "app_menu.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_common.h"
#include "sl_rail_util_compatible_pa.h"
#include "printf.h"
#include "sl_rail_sdk_fifo_size_config.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
#include "rail_config.h"
#endif

#ifdef  SL_CATALOG_RANGE_TEST_STD_COMPONENT_PRESENT
#include "app_measurement_standard.h"
#endif

#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
#include "app_bluetooth.h"
#include "gatt_db.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#if defined(SL_CATALOG_RAIL_PACKET_ASSISTANT_PRESENT)
#include "sl_rail_sdk_packet_assistant.h"
#endif

#include "cmsis_compiler.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Macro to print out variable name
#define GET_VARIABLE_NAME(NAME) #NAME

/// Macro to easily print out errors from rail_handle
#define PRINT_AND_CLEAR_FLAG(flag)  if (flag) { flag = 0; app_log_info(" %s\n", GET_VARIABLE_NAME(flag)); }

/// Time between send in microseconds
#define RANGE_TEST_SEND_TIME ((uint32_t) 100000)

/// strucure to keep the error flags in a small variable
typedef struct error_flags_t {
  struct tx_errors{
    uint8_t tx_aborted : 1;
    uint8_t tx_blocked : 1;
    uint8_t tx_underflow : 1;
    uint8_t tx_channel_busy : 1;
    uint8_t tx_scheduled_tx_missed : 1;
  }tx_errors;
  struct rx_errors{
    uint8_t rx_aborted : 1;
    uint8_t rx_frame_error : 1;
    uint8_t rx_fifo_overflow : 1;
    uint8_t rx_address_filtered : 1;
    uint8_t rx_scheduled_rx_missed : 1;
  }rx_errors;
}error_flags_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief Modify the currently configured fixed frame length in bytes.
 ******************************************************************************/
static void set_fixed_length(sl_rail_handle_t rail_handle, uint16_t length);

/*******************************************************************************
 * @brief Function to generate the payload of the packet to be sent.
 ******************************************************************************/
static void range_test_generate_payload(uint8_t *data, uint16_t data_length);

/*******************************************************************************
 * @brief Function get packet and send it on selected handler
 ******************************************************************************/
static void send_packet(uint16_t packet_number);

/*******************************************************************************
 * @brief This function inserts a number of bits into the moving average
 *         history.
 ******************************************************************************/
static void range_test_MA_set(uint32_t nr);

/*******************************************************************************
 * @brief This function clears the most recent bit in the moving average
 *         history. This indicates that last time we did not see any missing
 *         packets.
 ******************************************************************************/
static void range_test_MA_clear(void);

/*******************************************************************************
 * @brief Returns the moving average of missing packets based on the
 *         history data.
 ******************************************************************************/
static uint8_t range_test_MA_get(void);

/*******************************************************************************
 * @brief Clears the history of the moving average calculation.
 ******************************************************************************/
static void range_test_MA_clear_all(void);

/*******************************************************************************
 * @brief Function to count how many bits has the value of 1.
 ******************************************************************************/
static uint32_t range_test_count_bits(uint32_t u);

/*******************************************************************************
 * @brief Stops RX and TX and set custom rail handler to IDLE
 ******************************************************************************/
static void prepare_radio_config_packet(uint16_t packet_number, uint8_t *tx_buffer);

/*******************************************************************************
 * @brief Stops RX and TX and set custom rail handler to IDLE
 ******************************************************************************/
void set_custom_handler_to_idle(void);

/*******************************************************************************
 * @brief Prints out the TX side of the logs
 ******************************************************************************/
static inline void print_tx_logs(void);

/*******************************************************************************
 * @brief Prints out the RX side of the logs
 ******************************************************************************/
static inline void print_rx_logs(void);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// All setting regarding the range test
volatile range_test_settings_t range_test_settings;
/// All data regarding the range test
volatile range_test_measurement_t range_test_measurement;

/// Flag indication if a service packet is received
extern volatile bool service_received;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Flag indication if the device is waiting for a service packet
static volatile bool waiting_service_packet;

/// Variable that saves the original value of range_test_settings.current_phy
/// if the service phy is configured
static uint8_t range_test_settings_current_phy_tmp = 0;

/// Variable that saves the original value of range_test_settings.payload_length
/// if the service payload length is configured
static uint8_t range_test_settings_payload_length_tmp = 5;

/// Scheduling setting for TX part of the range test
static sl_rail_scheduled_tx_config_t schedule = {
  .mode = SL_RAIL_TIME_DELAY,
  .when = RANGE_TEST_SEND_TIME,
};

/// Flag for TX, shows if the sending was successful and happened
static volatile bool schedule_is_ready = false;

/// Flag for TX, shows if the sending wasn't successful and happened
static volatile bool send_failed = false;

/// Flag for RX, shows that CRC error happened
static volatile bool rx_crc_error_happened = false;

/// Counter for RX, shows that packet was received without error
static volatile uint8_t rx_packet_received = 0;

/// Flag for printing out log on UART set true to print out the current status
static volatile bool logging_needed = false;

/// Contains the status of RAIL Calibration
static volatile sl_rail_status_t calibration_status = 0;

/// Contains the last RAIL Rx/Tx error events
static volatile uint64_t current_rail_err = 0;

/// RAIL Rx packet handle
static volatile sl_rail_rx_packet_handle_t rx_packet_handle;

/// Variable to store received packet info
static sl_rail_rx_packet_info_t packet_info;

/// Variable to store received packet details
static sl_rail_rx_packet_details_t packet_details;

/// Number of the user defined phys
static uint8_t number_of_custom_phys = 0;

/// variable to hold all rail rx and tx errors from rail_handle
static error_flags_t error_flags = { 0 };

static uint8_t tx_buffer[128];
static uint8_t rx_buffer[128];

/// Scheduler setting for dmp case
#if defined(SL_CATALOG_KERNEL_PRESENT)
static sl_rail_scheduler_info_t scheduler_info = {
  .priority = 100,
  .slip_time = 100000,
  .transaction_time = 2500
};

#endif

// -----------------------------------------------------------------------------
//                          Weak Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK bool standard_phy_are_present(void)
{
  return 0;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK void handle_payload_length_for_standard(void)
{
  return;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK uint8_t current_phy_standard_value(void)
{
  return range_test_settings.current_phy;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK sl_rail_handle_t get_standard_rail_handler(void)
{
  return NULL;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK void get_rail_standard_config_data(uint32_t *base_frequency, uint32_t *channel_spacing)
{
  *base_frequency  = 0;
  *channel_spacing = 0;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK void get_rail_standard_channel_range(uint16_t *min, uint16_t *max, uint8_t phy_index)
{
  *min = 0;
  *max = 0;
  (void)phy_index;  // Unused parameter.
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK void get_rail_standard_payload_range(uint8_t *payload_min, uint8_t *payload_max)
{
  *payload_min = 0;
  *payload_max = 0;
}

/*******************************************************************************
 * Generate a string for the BLE mobile APP from standard phys
 * @param[in] phy_index: which phy name is needed
 * @param[in] buffer: where to write the characters
 * @param[in] length: the length of the whole written string
 ******************************************************************************/
SL_WEAK void std_phy_list_generation(uint8_t phy_index, uint8_t *buffer, uint8_t *length)
{
  (void)phy_index;
  (void)buffer;
  (void)length;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK void init_range_test_standard_phys(uint8_t* number_of_phys)
{
  (void)number_of_phys;  // Unused parameter.
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK void set_ieee_handler_to_idle(void)
{
  return;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK void set_ble_handler_to_idle(void)
{
  return;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK bool is_current_phy_ble(void)
{
  return false;
}

/*******************************************************************************
 * @brief This function is defined in app_measurement_standard.c
 *         If that file is not present then it is an only custom phy range test
 ******************************************************************************/
SL_WEAK range_test_packet_t* get_start_of_payload_for_standard(uint8_t* received_buffer)
{
  (void)received_buffer;  // Unused parameter.
  return NULL;
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
SL_WEAK uint16_t unpack_packet(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
#endif
  }
  *start_of_payload = rx_destination;
  return packet_information->packet_bytes;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
SL_WEAK void prepare_packet(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, out_data, length, true);
  app_assert(bytes_written_in_fifo == length,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             length);
}

SL_WEAK void prepare_ieee802154_data_frame(uint16_t packet_number, uint8_t *tx_buffer)
{
  (void)packet_number;  // Unused parameter.
  (void)tx_buffer;  // Unused parameter.
}

SL_WEAK void prepare_ble_advertising_channel_pdu(uint16_t packet_number, uint8_t *tx_buffer)
{
  (void)packet_number;  // Unused parameter.
  (void)tx_buffer;  // Unused parameter.
}

SL_WEAK uint8_t get_ieee802154_data_frame_length(void)
{
  return 0;
}

SL_WEAK bool is_phy_standard(uint8_t index)
{
  (void)index;
  return false;
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Get the minimum and maximum Power Amplifier setting in deci-dBm units
 * @param[out] min_power_deci_dbm: pointer to store minimum PA value in deci-dBm (can be NULL)
 * @param[out] max_power_deci_dbm: pointer to store maximum PA value in deci-dBm (can be NULL)
 ******************************************************************************/
void get_tx_power_deci_dbm_range(int16_t *min_power_deci_dbm, int16_t *max_power_deci_dbm)
{
  sl_rail_handle_t rail_handle;
  sl_rail_tx_power_t min_power_ddbm = SL_RAIL_TX_POWER_MIN;
  sl_rail_tx_power_t max_power_ddbm = SL_RAIL_TX_POWER_MIN;
  bool rail_init_ready = true;

#ifdef  SL_CATALOG_RANGE_TEST_STD_COMPONENT_PRESENT
  rail_init_ready = is_init_range_test_standard_ready();
#endif

  if (rail_init_ready) {
    rail_handle = get_current_rail_handler();
    sl_rail_util_pa_get_tx_power_limits(rail_handle,
                                        SL_RAIL_TX_PA_MODE_INVALID,
                                        &min_power_ddbm,
                                        &max_power_ddbm,
                                        NULL);
  }
  if (min_power_deci_dbm != NULL) {
    *min_power_deci_dbm = min_power_ddbm;
  }
  if (max_power_deci_dbm != NULL) {
    *max_power_deci_dbm = max_power_ddbm;
  }
}
/*******************************************************************************
 * Helper function to apply the new power level from the range test settings
 ******************************************************************************/
void update_tx_power(void)
{
  sl_rail_status_t power_status = SL_RAIL_STATUS_NO_ERROR;
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  power_status = sl_rail_set_tx_power_dbm(rail_handle, range_test_settings.tx_power);
  if (power_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_error("sl_rail_set_tx_power_dbm failed with %lu\n", power_status);
  }
}

/*******************************************************************************
 * Helper function to return the custom phy number
 * @return number_of_custom_phys
 ******************************************************************************/
uint8_t get_number_of_custom_phys(void)
{
  return number_of_custom_phys;
}

/*******************************************************************************
 * Helper function to differentiate custom and BLE or IEEE phys.
 * If only custom phys are in the project the return value will be false always.
 * @return if custom false otherwise true
 ******************************************************************************/
bool is_current_phy_standard(void)
{
  return standard_phy_are_present();
}

/*******************************************************************************
 * Get the rail handler which is selected in the phy selection
 * @return sl_rail_handle_t a pointer to the will be used handler
 ******************************************************************************/
sl_rail_handle_t get_current_rail_handler(void)
{
  if (is_current_phy_standard()) {
    return get_standard_rail_handler();
  } else {
    #if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
    return sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  #else
    return NULL;
    #endif
  }
}

/*******************************************************************************
 * Set power level to the defined max and init it if needed
 *
 * @param[in] init: id true it will reinit the config
 ******************************************************************************/
void set_power_level_to_max(bool init)
{
  sl_rail_handle_t rail_handle = get_current_rail_handler();

  #if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  if (!is_current_phy_standard()) {
    sl_rail_config_channels(rail_handle,
                            (const sl_rail_channel_config_t *)channelConfigs[range_test_settings.current_phy], NULL);
    uint16_t channel_first = sl_rail_get_first_channel(rail_handle,
                                                       (const sl_rail_channel_config_t *)channelConfigs[range_test_settings.current_phy]);
    sl_rail_prepare_channel(rail_handle, channel_first);

    sl_rail_tx_pa_mode_t new_pa_mode = sl_rail_get_pa_mode_from_channel_entry(rail_handle); //from configs
    (void) sl_rail_util_pa_post_init(rail_handle, new_pa_mode);

    uint16_t channel = range_test_settings.channel;
    if ((channel
         > channelConfigs[range_test_settings.current_phy]->configs[0U].channelNumberEnd)
        || (init)) {
      // Initialize the channel to the first one in channel config
      range_test_settings.channel =
        channelConfigs[range_test_settings.current_phy]->configs[0U].channelNumberStart;
    }
  }
  #endif

  int16_t  max_power_deci_dbm = 0;
  get_tx_power_deci_dbm_range(NULL, &max_power_deci_dbm);
  sl_rail_set_tx_power_dbm(rail_handle, max_power_deci_dbm);
}

/*******************************************************************************
 * Hides the configs from other parts of the application
 *
 * @param[out] base_frequency: The current frequency of the radio
 * @param[out] channel_spacing: The current channel spacing of the selected phy
 * @param[out] power: The current power of the radio
 ******************************************************************************/
void get_rail_config_data(uint32_t *base_frequency, uint32_t *channel_spacing, int16_t *power)
{
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  int16_t power_buffer = sl_rail_get_tx_power_dbm(rail_handle);

  *power = power_buffer;
  if (is_current_phy_standard()) {
    get_rail_standard_config_data(base_frequency, channel_spacing);
  } else {
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
    *base_frequency = channelConfigs[range_test_settings.current_phy]->configs[0].baseFrequency;
    *channel_spacing = channelConfigs[range_test_settings.current_phy]->configs[0U].channelSpacing;
#endif
  }
}

/*******************************************************************************
 * Hides the configs from other parts of the application
 *
 * @param[out] start: minimum number of channels in phy
 * @param[out] end: maximum number of channels in phy
 * @param[in] phy_index: index of the phy
 ******************************************************************************/
void get_rail_channel_range(uint16_t *start, uint16_t *end, uint8_t phy_index)
{
  if (is_phy_standard(phy_index)) {
    get_rail_standard_channel_range(start, end, phy_index);
  } else {
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
    *start = channelConfigs[phy_index]->configs[0U].channelNumberStart;
    *end = channelConfigs[phy_index]->configs[0U].channelNumberEnd;
#endif
  }
}

/*******************************************************************************
 * Hides the configs from other parts of the application
 *
 * @param[out] payload_min: minimum number of payload in phy
 * @param[out] payload_max: maximum number of payload in phy
 ******************************************************************************/
void get_rail_payload_range(uint8_t *payload_min, uint8_t *payload_max)
{
  if (is_current_phy_standard()) {
    get_rail_standard_payload_range(payload_min, payload_max);
  } else {
  #if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
    *payload_min = RANGETEST_PAYLOAD_LEN_MIN;
    *payload_max = RANGETEST_PAYLOAD_LEN_MAX;
  #endif
  }
}

/*******************************************************************************
 * Generate a string for the BLE mobile APP
 * @param[out] buffer: where to write the characters
 * @param[out] length: the length of the whole written string
 ******************************************************************************/
void phy_list_generation(uint8_t *buffer, uint8_t *length)
{
  for (uint8_t phy_index = 0; phy_index < number_of_phys; phy_index++) {
    if (number_of_custom_phys == 0 || number_of_custom_phys <= phy_index) {
      std_phy_list_generation(phy_index, buffer, length);
    } else {
      // *buffer length comes from app_bluetooth.c
      snprintf((char*)(&buffer[*length]), 255, "%u:custom_%u,", phy_index, phy_index);
      *length = safe_strlen((char*)(buffer));
    }
  }
}

/*******************************************************************************
 * Inits the phys for the range test, and counts them.
 ******************************************************************************/
void init_range_test_phys(void)
{
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  for (number_of_custom_phys = 0; number_of_custom_phys < 255; ++number_of_custom_phys) {
    if (channelConfigs[number_of_custom_phys] == NULL) {
      break;
    }
  }
#endif
  number_of_phys = number_of_custom_phys;
  init_range_test_standard_phys(&number_of_phys);
}

/*******************************************************************************
 * @brief Resets the internal status of the Range Test and sets up needed
 *         settings
 ******************************************************************************/
void range_test_init(void)
{
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  range_test_reset_values();
  set_fixed_length(rail_handle, range_test_settings.payload_length);
}

/*******************************************************************************
 * @brief Resets all measurement data
 ******************************************************************************/
void range_test_reset_values(void)
{
  range_test_measurement.packets_received_counter = 0U;
  range_test_measurement.first_received_packet_offset = 0U;
  range_test_measurement.packets_received_correctly = RANGETEST_PACKET_COUNT_INVALID;
  range_test_measurement.packets_sent = 0U;
  range_test_measurement.packets_with_crc_error = 0U;
  schedule_is_ready = true;
}

/*******************************************************************************
 * @brief TX part of the measurement, manages counting and logging/updating
 *
 * @return refresh_screen_is_needed: true if LCD needed to be updated
 ******************************************************************************/
bool send_measurement(void)
{
  bool refresh_screen_is_needed = false;
  uint16_t packets_sent = range_test_measurement.packets_sent;
  if (packets_sent < range_test_settings.packets_repeat_number) {
    if (schedule_is_ready || send_failed) {
      schedule_is_ready = false;
      if (!send_failed) {
        range_test_measurement.packets_sent++;
      } else {
        send_failed = false;
      }
      send_packet(range_test_measurement.packets_sent);
      refresh_screen_is_needed = true;

#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
      add_bluetooth_indication(gattdb_pktsSent);
#endif
    }
  } else {
    range_test_measurement.tx_is_running = false;
    refresh_screen_is_needed = true;
#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
    if (!is_bluetooth_connected()) {
      activate_bluetooth();
    }
    add_bluetooth_indication(gattdb_isRunning);
#endif
  }
  if (range_test_settings.usart_log_enable) {
    logging_needed = refresh_screen_is_needed;
  }
  return refresh_screen_is_needed;
}

/*******************************************************************************
 * @brief Set all handlers to IDLE state, useful as multiply handlers can
 *         occupy the radio same time and block each other at worst cases
 ******************************************************************************/
void set_all_radio_handlers_to_idle(void)
{
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  set_custom_handler_to_idle();
#endif
  set_ieee_handler_to_idle();
  set_ble_handler_to_idle();
}

/*******************************************************************************
 * @brief Prepare the selected handler to RX measurement
 ******************************************************************************/
void receive_setup_radio(void)
{
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  sl_rail_status_t status = 0;

  range_test_measurement.tx_is_running = false;

#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
  deactivate_bluetooth();
  scheduler_info = (sl_rail_scheduler_info_t){.priority = 200 };
  status = sl_rail_start_rx(rail_handle, range_test_settings.channel, &scheduler_info);
#else
  status = sl_rail_start_rx(rail_handle, range_test_settings.channel, NULL);
#endif
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_error("sl_rail_start_rx failed with code 0x%lx", status);
  }
  rx_crc_error_happened = false;
  rx_packet_received = 0;
#if defined(SL_CATALOG_GLIB_PRESENT)
  graphics_clear_rssi_buffer();
#endif
}

/*******************************************************************************
 * @brief Stops the RX measurement and sets the radio handler to IDLE
 ******************************************************************************/
void stop_receive_measurement(void)
{
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  sl_rail_idle(rail_handle, SL_RAIL_IDLE, true);
#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
  activate_bluetooth();
#endif
}

/*******************************************************************************
 * @brief RX measurement logic, here is the packet number check and etc.
 *
 * @return refresh_screen_is_needed: true if the LCD is needed to be updated
 ******************************************************************************/
bool receive_measurement(void)
{
  range_test_packet_t *rx_packet = NULL;
  // Status indicator of the RAIL API calls
  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;
  bool refresh_screen_is_needed = false;
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  static uint32_t last_received_packet_count = 0U;
  int8_t rssi_value = 0;

  if (rx_crc_error_happened) {
    rx_crc_error_happened = false;
    if (range_test_measurement.packets_with_crc_error < 0xFFFF) {
      range_test_measurement.packets_with_crc_error++;
      refresh_screen_is_needed = true;
    }

#if defined(SL_CATALOG_KERNEL_PRESENT)
    scheduler_info = (sl_rail_scheduler_info_t){.priority = 200 };
    rail_status = sl_rail_start_rx(rail_handle, range_test_settings.channel, &scheduler_info);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_error("sl_rail_start_rx failed with %lu", rail_status);
    }
    #else
    rail_status = sl_rail_start_rx(rail_handle, range_test_settings.channel, NULL);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_error("sl_rail_start_rx failed with %lu", rail_status);
    }
    #endif
  }

  if (rx_packet_received > 0) {
    CORE_ATOMIC_SECTION(
      rx_packet_received--;
      )

#if defined(SL_CATALOG_KERNEL_PRESENT)
    if (rx_packet_received > 0) {
      app_task_notify();
    }
#endif
    // read packet details
    rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle,
                                                  SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
    sl_rail_get_rx_packet_details(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_details);
    if (is_current_phy_standard()) {
      // copy the content of the packet
      rail_status = sl_rail_copy_rx_packet(rail_handle, rx_buffer, &packet_info);
      sl_rail_release_rx_packet(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE);
      rx_packet = get_start_of_payload_for_standard(rx_buffer);
    } else {
      uint8_t *start_of_packet = 0;
      (void)unpack_packet(rail_handle, rx_buffer, &packet_info, &start_of_packet);
      rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
      rx_packet = (range_test_packet_t*) start_of_packet;
    }

#if defined(SL_CATALOG_KERNEL_PRESENT)
    scheduler_info = (sl_rail_scheduler_info_t){.priority = 200 };
    sl_rail_start_rx(rail_handle, range_test_settings.channel, &scheduler_info);
#else
    sl_rail_start_rx(rail_handle, range_test_settings.channel, NULL);
#endif

    // Make sure the packet addressed to me
    if (rx_packet->destination_id != range_test_settings.source_id) {
      return refresh_screen_is_needed;
    }

    // Make sure the packet sent by the selected remote
    if (rx_packet->source_id != range_test_settings.destination_id) {
      return refresh_screen_is_needed;
    }

    if ((RANGETEST_PACKET_COUNT_INVALID == range_test_measurement.packets_received_correctly)
        || (rx_packet->packet_counter <= range_test_measurement.packets_received_counter)) {
      // First packet received OR
      // Received packet counter lower than already received counter.

#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
      deactivate_bluetooth();
#endif

      // Reset received counter
      range_test_measurement.packets_received_correctly = 0U;
      // Set counter offset
      range_test_measurement.first_received_packet_offset = rx_packet->packet_counter - 1U;

#if defined(SL_CATALOG_GLIB_PRESENT)
      // Clear RSSI Chart
      graphics_clear_rssi_buffer();
#endif

      // Clear Moving-Average history
      range_test_MA_clear_all();

      // Restart Moving-Average calculation
      last_received_packet_count = 0U;
    }

    if (range_test_measurement.packets_received_correctly < 0xFFFF) {
      range_test_measurement.packets_received_correctly++;
    }

    range_test_measurement.packets_received_counter = rx_packet->packet_counter - range_test_measurement.first_received_packet_offset;
    range_test_measurement.rssi_latch_value = packet_details.rssi_dbm;

    // Store RSSI value from the latch
    rssi_value = range_test_measurement.rssi_latch_value;
    // Limit stored RSSI values to the displayable range
#if defined(SL_CATALOG_GLIB_PRESENT)
    rssi_value = rssi_value_for_display(range_test_measurement.rssi_latch_value);
#endif
#if defined(SL_CATALOG_GLIB_PRESENT)
    // Store RSSI value in ring buffer
    graphics_add_to_rssi_buffer(rssi_value);
#else
    (void)rssi_value;
#endif

    // Calculate recently lost packets number based on newest counter
    if ((range_test_measurement.packets_received_counter - last_received_packet_count) > 1U) {
      // At least one packet lost
      range_test_MA_set(range_test_measurement.packets_received_counter - last_received_packet_count - 1U);
    }
    // Current packet is received
    range_test_MA_clear();
    last_received_packet_count = range_test_measurement.packets_received_counter;

    // Calculate Moving-Average Error Rate
    range_test_measurement.moving_average = (range_test_MA_get() * 100.0F)
                                            / range_test_settings.moving_average_window_size;

    uint16_t packets_received_counter = range_test_measurement.packets_received_counter;
    // Calculate Packet Error Rate
    range_test_measurement.PER = (packets_received_counter)   // Avoid zero division
                                 ? (((float) (packets_received_counter - range_test_measurement.packets_received_correctly)
                                     * 100.0F) / packets_received_counter)   // Calculate PER
                                 :
                                 0.0F;   // By default PER is 0.0%

    refresh_screen_is_needed = true;
  }

  if (range_test_settings.usart_log_enable) {
    logging_needed = refresh_screen_is_needed;
  }
  return refresh_screen_is_needed;
}

/*******************************************************************************
 * @brief Logging function which prints out the current received or sent packet,
 *        and statistics.
 ******************************************************************************/
void print_log(void)
{
  if (logging_needed) {
    logging_needed = false;
    if (RADIO_MODE_RX == range_test_settings.radio_mode) {
      print_rx_logs();
    } else if (RADIO_MODE_TX == range_test_settings.radio_mode) {
      if (range_test_measurement.tx_is_running) {
        print_tx_logs();
      }
    }
  }
}

/******************************************************************************
* RAIL callback, called if a RAIL event occurs
******************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  (void) rail_handle;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (range_test_settings.radio_mode == RADIO_MODE_TX) {
    if (events & SL_RAIL_EVENT_SCHEDULER_STATUS) {
      send_failed = true;
      sl_rail_yield_radio(rail_handle);
    }
  }
#endif
  // Handle Tx events
  if (events & SL_RAIL_EVENTS_TX_COMPLETION) {
    if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
      schedule_is_ready = true;
    } else if (events & SL_RAIL_EVENT_TX_ABORTED) {
      error_flags.tx_errors.tx_aborted = 1;
    } else if (events & SL_RAIL_EVENT_TX_BLOCKED) {
      error_flags.tx_errors.tx_blocked = 1;
    } else if (events & SL_RAIL_EVENT_TX_UNDERFLOW) {
      error_flags.tx_errors.tx_underflow = 1;
    } else if (events & SL_RAIL_EVENT_TX_CHANNEL_BUSY) {
      error_flags.tx_errors.tx_channel_busy = 1;
    } else if (events & SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED) {
      error_flags.tx_errors.tx_scheduled_tx_missed = 1;
      send_failed = true;
    }
#if defined(SL_CATALOG_KERNEL_PRESENT)
    sl_rail_yield_radio(rail_handle);
#endif
  }

  // Handle Rx events
  if (events & SL_RAIL_EVENTS_RX_COMPLETION) {
    if (waiting_service_packet) {
      if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
        service_received = true;
        sl_rail_hold_rx_packet(rail_handle);
      }
      waiting_service_packet = false;
    } else {
      if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
        CORE_ATOMIC_SECTION(
          rx_packet_received++;
          )
        sl_rail_hold_rx_packet(rail_handle);
      } else if (events & SL_RAIL_EVENT_RX_FRAME_ERROR) {
        rx_crc_error_happened = true;
        error_flags.rx_errors.rx_frame_error = 1;
      } else if (events & SL_RAIL_EVENT_RX_PACKET_ABORTED) {
        error_flags.rx_errors.rx_aborted = 1;
      } else if (events & SL_RAIL_EVENT_RX_FIFO_OVERFLOW) {
        error_flags.rx_errors.rx_fifo_overflow = 1;
      } else if (events & SL_RAIL_EVENT_RX_ADDRESS_FILTERED) {
        error_flags.rx_errors.rx_address_filtered = 1;
      } else if (events & SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED) {
        error_flags.rx_errors.rx_scheduled_rx_missed = 1;
      }
#if defined(SL_CATALOG_KERNEL_PRESENT)
      if (range_test_settings.radio_mode == RADIO_MODE_RX) {
        sl_rail_yield_radio(rail_handle);
      }
#endif
    }
  }

  // Perform all calibrations when needed
  if (events & SL_RAIL_EVENT_CAL_NEEDED) {
    calibration_status = sl_rail_calibrate(rail_handle, NULL, SL_RAIL_CAL_ALL_PENDING);
    if (calibration_status != SL_RAIL_STATUS_NO_ERROR) {
      current_rail_err = (events & SL_RAIL_EVENT_CAL_NEEDED);
    }
#if defined(SL_CATALOG_KERNEL_PRESENT)
    sl_rail_yield_radio(rail_handle);
#endif
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

/*******************************************************************************
 * @brief Prints out the errors that happened during sl_rail_handle_t on RX or TX
 ******************************************************************************/
void print_errors_from_rail_handler(void)
{
  PRINT_AND_CLEAR_FLAG(error_flags.tx_errors.tx_aborted)
  PRINT_AND_CLEAR_FLAG(error_flags.tx_errors.tx_blocked)
  PRINT_AND_CLEAR_FLAG(error_flags.tx_errors.tx_underflow)
  PRINT_AND_CLEAR_FLAG(error_flags.tx_errors.tx_channel_busy)
  PRINT_AND_CLEAR_FLAG(error_flags.tx_errors.tx_scheduled_tx_missed)
  PRINT_AND_CLEAR_FLAG(error_flags.rx_errors.rx_aborted)
  PRINT_AND_CLEAR_FLAG(error_flags.rx_errors.rx_frame_error)
  PRINT_AND_CLEAR_FLAG(error_flags.rx_errors.rx_fifo_overflow)
  PRINT_AND_CLEAR_FLAG(error_flags.rx_errors.rx_address_filtered)
  PRINT_AND_CLEAR_FLAG(error_flags.rx_errors.rx_scheduled_rx_missed)
}

/*******************************************************************************
 * @brief Function that returns the safe size of the string
 *
 * @param[in] src: pointer to the string
 *
 * @return Size of the string
 ******************************************************************************/
uint8_t safe_strlen(char *src)
{
  char dest[256];
  strncpy(dest, src, sizeof(dest)); // Truncation may happen
  dest[sizeof(dest) - 1] = 0;
  return strlen(dest); // Compliant: "dest" is guaranteed to be null-terminated
}

/*******************************************************************************
 * Sends the service packet with the configuration details on the service channel.
 ******************************************************************************/
void send_service_packet(void)
{
  sl_rail_status_t rail_status;
  service_data_t *tx_data;
  sl_rail_handle_t rail_handle;
  uint16_t tx_length = 0;
  uint16_t temp_channel = 0;

  range_test_settings_current_phy_tmp = current_phy_standard_value();
  range_test_settings.current_phy = range_test_settings.service_phy;
  menu_set_std_phy(false);

  rail_handle = get_current_rail_handler();
  memset(tx_buffer, 0, sizeof(tx_buffer));

#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  sl_rail_config_channels(rail_handle,
                          (const sl_rail_channel_config_t *)channelConfigs[range_test_settings.current_phy], NULL);
  set_power_level_to_max(false);
#endif
  range_test_settings_payload_length_tmp = range_test_settings.payload_length;
  range_test_settings.payload_length = sizeof(range_test_packet_t) + sizeof(service_data_t);
  set_fixed_length(rail_handle, range_test_settings.payload_length);

#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  prepare_radio_config_packet(0, tx_buffer);
  tx_length = range_test_settings.payload_length;
  tx_data = (service_data_t*)(&(tx_buffer[sizeof(range_test_packet_t)]));
#else
  if (!is_current_phy_ble()) {
    data_frame_format_t* data_frame;
    prepare_ieee802154_data_frame(0, tx_buffer);
    tx_length = get_ieee802154_data_frame_length();
    data_frame = (data_frame_format_t*) &tx_buffer[1];
    tx_data = (service_data_t*)(data_frame->remainder);
  } else {
    prepare_ble_advertising_channel_pdu(0, tx_buffer);
    advertising_nonconnectable_undirected_t* ble_tx_pdu = (advertising_nonconnectable_undirected_t*)tx_buffer;
    tx_data = (service_data_t*)(ble_tx_pdu->manufactSpec.remainder);
    tx_length = tx_buffer[1] + 2;
  }
#endif

  tx_data->channel = range_test_settings.channel;
  tx_data->current_phy = range_test_settings_current_phy_tmp;
  tx_data->packets_repeat_number = range_test_settings.packets_repeat_number;
  tx_data->payload_length = range_test_settings_payload_length_tmp;
  tx_data->tx_power = range_test_settings.tx_power;

  prepare_packet(rail_handle, tx_buffer, tx_length);
  temp_channel = range_test_settings.channel;
  rail_status = sl_rail_start_tx(rail_handle, range_test_settings.service_channel, SL_RAIL_TX_OPTIONS_DEFAULT, NULL);
  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_error("sl_rail_start_tx() error 0x%0lX\n", rail_status);
  }
  set_all_radio_handlers_to_idle();

  range_test_settings.current_phy = range_test_settings_current_phy_tmp;
  range_test_settings.channel = temp_channel;
  rail_handle = get_current_rail_handler();
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  sl_rail_config_channels(rail_handle,
                          (const sl_rail_channel_config_t *)channelConfigs[range_test_settings.current_phy], NULL);
  set_power_level_to_max(false);
#endif
  menu_set_std_phy(false);
  update_tx_power();
  range_test_settings.payload_length = range_test_settings_payload_length_tmp;
  set_fixed_length(rail_handle, range_test_settings.payload_length);
}

/*******************************************************************************
 * Starts the reception of the service packet on the service channel.
 ******************************************************************************/
void receive_service_packet(void)
{
  sl_rail_status_t rail_status;
  sl_rail_handle_t rail_handle;

  range_test_settings_current_phy_tmp = range_test_settings.current_phy;
  range_test_settings.current_phy = range_test_settings.service_phy;
  rail_handle = get_current_rail_handler();
  menu_set_std_phy(false);
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  sl_rail_config_channels(rail_handle,
                          (const sl_rail_channel_config_t *)channelConfigs[range_test_settings.current_phy], NULL);
  set_power_level_to_max(false);
#endif
  set_all_radio_handlers_to_idle();
  update_tx_power();
  range_test_settings_payload_length_tmp = range_test_settings.payload_length;
  range_test_settings.payload_length = sizeof(range_test_packet_t) + sizeof(service_data_t);
  set_fixed_length(rail_handle, range_test_settings.payload_length);

#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
  scheduler_info = (sl_rail_scheduler_info_t){.priority = 200 };
  rail_status = sl_rail_start_rx(rail_handle, range_test_settings.service_channel, &scheduler_info);
#else
  rail_status = sl_rail_start_rx(rail_handle, range_test_settings.service_channel, NULL);
#endif
  if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_error("sl_rail_start_rx() error 0x%0lX\n", rail_status);
  }
  waiting_service_packet = true;
}

/*******************************************************************************
 * Sets the RAIL PHY back to the test defined one.
 ******************************************************************************/
void undo_service_config(void)
{
  set_all_radio_handlers_to_idle();
  range_test_settings.current_phy = range_test_settings_current_phy_tmp;
  menu_set_std_phy(false);
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  sl_rail_config_channels(rail_handle,
                          (const sl_rail_channel_config_t *)channelConfigs[range_test_settings.current_phy], NULL);
  set_power_level_to_max(false);
#endif
#if defined(SL_CATALOG_RAIL_PACKET_ASSISTANT_PRESENT)
  update_assistant_pointers(range_test_settings.current_phy);
#endif

  update_tx_power();
  range_test_settings.payload_length = range_test_settings_payload_length_tmp;
}

/*******************************************************************************
 * Handles the service packet containing the test configuration.
 * @return Boolean indicates the validity of the received packet
 ******************************************************************************/
bool service_packet_received(void)
{
  if (range_test_settings.remote_config_enable) {
    range_test_packet_t *rx_packet;
    const service_data_t *rx_data;
    uint8_t *start_of_packet;
    sl_rail_handle_t rail_handle = get_current_rail_handler();

    // read packet details
    rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle,
                                                  SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);

    if (is_current_phy_standard()) {
      // copy the content of the packet
      sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_buffer, &packet_info);
      if (result != SL_RAIL_STATUS_NO_ERROR) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
        app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
#else
        // Avoid unused variable warning
        (void)result;
#endif
      }
      sl_rail_release_rx_packet(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE);
      rx_packet = get_start_of_payload_for_standard(rx_buffer);
      start_of_packet = (uint8_t*) rx_packet;
    } else {
      (void)unpack_packet(rail_handle, rx_buffer, &packet_info, &start_of_packet);
      sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
      rx_packet = (range_test_packet_t*) start_of_packet;
    }

    rx_data = (service_data_t*)(&(start_of_packet[sizeof(range_test_packet_t)]));
    // Make sure the packet addressed to me
    if ((rx_packet->destination_id == range_test_settings.source_id)
        && (rx_packet->source_id == range_test_settings.destination_id)) {
      range_test_settings.channel = rx_data->channel;
      range_test_settings.current_phy = rx_data->current_phy;
      range_test_settings_current_phy_tmp = range_test_settings.current_phy;
      range_test_settings.packets_repeat_number = rx_data->packets_repeat_number;
      range_test_settings.payload_length = rx_data->payload_length;
      range_test_settings_payload_length_tmp = rx_data->payload_length;
      range_test_settings.tx_power = rx_data->tx_power;
#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
      add_bluetooth_indication(gattdb_channel);
      add_bluetooth_indication(gattdb_phy);
      add_bluetooth_indication(gattdb_pktsReq);
      add_bluetooth_indication(gattdb_payload);
      add_bluetooth_indication(gattdb_txPower);
#endif
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief Stops RX and TX and set custom rail handler to IDLE
 ******************************************************************************/
void set_custom_handler_to_idle(void)
{
  #if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
  sl_rail_handle_t rail_handle_custom = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  sl_rail_idle(rail_handle_custom, SL_RAIL_IDLE, true);
  #endif
}

/*******************************************************************************
 * @brief Modify the currently configured fixed frame length in bytes.
 *
 * @param[in] rail_handle: sl_rail_handle_t that needs to be configured.
 *
 * @param[in] length: Fixed frame length in bytes
 ******************************************************************************/
static void set_fixed_length(sl_rail_handle_t rail_handle, uint16_t length)
{
  if (!is_current_phy_standard()) {
#ifdef  SL_CATALOG_RAIL_PACKET_ASSISTANT_PRESENT
    update_assistant_pointers(range_test_settings.current_phy);
    if (channelConfigs[range_test_settings.current_phy]->configs[0].stackInfo == NULL) {
      sl_rail_set_fixed_length(rail_handle, length);
    } else {
      if (channelConfigs[range_test_settings.current_phy]->configs[0].stackInfo[0] == 0x00 && channelConfigs[range_test_settings.current_phy]->configs[0].stackInfo[1] == 0x00) {
        sl_rail_set_fixed_length(rail_handle, length);
      } else if (channelConfigs[range_test_settings.current_phy]->configs[0].stackInfo[0] == CONNECT && channelConfigs[range_test_settings.current_phy]->configs[0].stackInfo[1] != 0x50) {
        sl_rail_set_fixed_length(rail_handle, length);
      } else {
        sl_rail_set_fixed_length(rail_handle, SL_RAIL_SET_FIXED_LENGTH_INVALID);
      }
    }
#else
    (void)rail_handle;
    (void)length;
#endif
  }

  handle_payload_length_for_standard();
}

/*******************************************************************************
 * @brief Stops RX and TX and set custom rail handler to IDLE
 *
 * @param[in] packet_number: the actual packet count to be put into the packet
 *
 * @param[in] tx_buffer: pointer to the buffer where the packet is put together
 ******************************************************************************/
static void prepare_radio_config_packet(uint16_t packet_number, uint8_t *tx_buffer)
{
  range_test_packet_t *tx_packet;
  tx_packet = (range_test_packet_t*)tx_buffer;

  tx_packet->packet_counter = packet_number;
  tx_packet->destination_id = range_test_settings.destination_id;
  tx_packet->source_id = range_test_settings.source_id;

  uint16_t packet_length = range_test_settings.payload_length;

  range_test_generate_payload(tx_buffer, packet_length);
}

/*******************************************************************************
 * @brief Function to generate the payload of the packet to be sent.
 *
 * @param[in] data: TX buffer to fill out with the generated payload.
 *
 * @param[in] data_length: how many filling is needed for packet size
 ******************************************************************************/
static void range_test_generate_payload(uint8_t *data, uint16_t data_length)
{
  for (uint8_t i = sizeof(range_test_packet_t); i < data_length; i++) {
    data[i] = (i % 2U) ? (0x55U) : (0xAAU);
  }
}

/*******************************************************************************
 * @brief Function get packet and send it on selected handler
 *
 * @param[in] packet_number: this number will be put into the packet
 ******************************************************************************/
static void send_packet(uint16_t packet_number)
{
  sl_rail_status_t rail_status;
  sl_rail_handle_t rail_handle = get_current_rail_handler();
  bool set_tx_failed = false;
  uint16_t tx_length = 0;
  memset(tx_buffer, 0, sizeof(tx_buffer));

  if (is_current_phy_standard()) {
    if (!is_current_phy_ble()) {
      prepare_ieee802154_data_frame(packet_number, tx_buffer);
      tx_length = get_ieee802154_data_frame_length();
    } else {
      prepare_ble_advertising_channel_pdu(packet_number, tx_buffer);
      tx_length = tx_buffer[1] + 2;
    }
  } else {
    prepare_radio_config_packet(packet_number, tx_buffer);
    tx_length = range_test_settings.payload_length;
  }

  prepare_packet(rail_handle, tx_buffer, tx_length);

  if (!set_tx_failed) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    scheduler_info = (sl_rail_scheduler_info_t){ .priority = 100,
                                                 .slip_time = 100000,
                                                 .transaction_time = 2500 };

    rail_status = sl_rail_start_scheduled_tx(rail_handle,
                                             range_test_settings.channel, SL_RAIL_TX_OPTIONS_DEFAULT,
                                             &schedule,
                                             &scheduler_info);
#else
    rail_status = sl_rail_start_scheduled_tx(rail_handle,
                                             range_test_settings.channel, SL_RAIL_TX_OPTIONS_DEFAULT, &schedule,
                                             NULL);
#endif
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_error("sl_rail_start_tx() error 0x%0lX\n", rail_status);
    }
  }
}

/*******************************************************************************
 * @brief This function clears the most recent bit in the moving average
 *         history. This indicates that last time we did not see any missing
 *         packets.
 ******************************************************************************/
static void range_test_MA_clear(void)
{
  // Buffering volatile value
  uint8_t  ma_finger = range_test_measurement.moving_average_current_point_value;

  range_test_measurement.moving_average_history[ma_finger >> 5U] &= ~(1U << (ma_finger % 32U));

  ma_finger++;
  if (ma_finger >= range_test_settings.moving_average_window_size) {
    ma_finger = 0U;
  }
  // Updating new value back to volatile
  range_test_measurement.moving_average_current_point_value = ma_finger;
}

/*******************************************************************************
 * @brief This function inserts a number of bits into the moving average
 *         history.
 *
 * @param[in] nr: The value to be inserted into the history.
 ******************************************************************************/
static void range_test_MA_set(uint32_t nr)
{
  uint8_t i;
  // Buffering volatile fields
  uint8_t  ma_finger = range_test_measurement.moving_average_current_point_value;

  if (nr >= range_test_settings.moving_average_window_size) {
    // Set all bits to 1's
    i = range_test_settings.moving_average_window_size;

    while (i >> 5U) {
      range_test_measurement.moving_average_history[(i >> 5U) - 1U] = 0xFFFFFFFFUL;
      i -= 32U;
    }
    return;
  }

  while (nr) {
    range_test_measurement.moving_average_history[ma_finger >> 5U] |= (1U << ma_finger % 32U);
    ma_finger++;
    if (ma_finger >= range_test_settings.moving_average_window_size) {
      ma_finger = 0U;
    }
    nr--;
  }
  // Update the buffered value back to the volatile field
  range_test_measurement.moving_average_current_point_value = ma_finger;
}

/*******************************************************************************
 * @brief Returns the moving average of missing packets based on the
 *         history data.
 *
 * @return The current moving average .
 ******************************************************************************/
static uint8_t range_test_MA_get(void)
{
  uint8_t return_value = 0U;

  for (uint8_t i = 0U; i < (range_test_settings.moving_average_window_size >> 5U); i++) {
    return_value += range_test_count_bits(range_test_measurement.moving_average_history[i]);
  }
  return return_value;
}

/*******************************************************************************
 * @brief Clears the history of the moving average calculation.
 ******************************************************************************/
static void range_test_MA_clear_all(void)
{
  range_test_measurement.moving_average_history[0U] = 0U;
  range_test_measurement.moving_average_history[1U] = 0U;
  range_test_measurement.moving_average_history[2U] = 0U;
  range_test_measurement.moving_average_history[3U] = 0U;
}

/*******************************************************************************
 * @brief Function to count how many bits has the value of 1.
 *
 * @param[in] u: Input value to count its '1' bits.
 *
 * @return Number of '1' bits in the input.
 ******************************************************************************/
static uint32_t range_test_count_bits(uint32_t u)
{
  uint32_t count = u
                   - ((u >> 1U) & 033333333333)
                   - ((u >> 2U) & 011111111111);

  return  (((count + (count >> 3U)) & 030707070707) % 63U);
}

/*******************************************************************************
 * @brief Prints out the TX side of the logs
 ******************************************************************************/
static inline void print_tx_logs(void)
{
  uint16_t packets_sent = range_test_measurement.packets_sent;
  uint16_t packets_repeat_number = range_test_settings.packets_repeat_number;
  uint8_t source_id = range_test_settings.source_id;
  uint8_t destination_id = range_test_settings.destination_id;

  app_log_info("Sent, Actual:%u, Max:%u, IdS:%u, IdR:%u\n",
               packets_sent,
               packets_repeat_number,
               source_id,
               destination_id);
}

/*******************************************************************************
 * @brief Prints out the RX side of the logs
 ******************************************************************************/
static inline void print_rx_logs(void)
{
#if defined(__IAR_SYSTEMS_ICC__)
  #pragma diag_suppress=Pa205
#endif
  if (RANGETEST_PACKET_COUNT_INVALID != range_test_measurement.packets_received_correctly ) {
    range_test_measurement_t range_test_measurement_buf = range_test_measurement;
    range_test_settings_t range_test_settings_buf = range_test_settings;
    app_log_error(
      "Rcvd, "          //6
      "OK:%u, "    //10
      "CRC:%u, "    //11
      "Sent:%u, "    //12
      "Payld:%u, "    //10
      "MASize:%u, "    //12
      "PER:%3.1f, "    //11
      "MA:%3.1f, "    //10
      "RSSI:% 3d, "    //12
      "IdS:%u, "    //8
      "IdR:%u"    //8
      "\n",    //1+1
      range_test_measurement_buf.packets_received_correctly,
      range_test_measurement_buf.packets_with_crc_error,
      range_test_measurement_buf.packets_received_counter,
      range_test_settings_buf.payload_length,
      range_test_settings_buf.moving_average_window_size,
      range_test_measurement_buf.PER,
      range_test_measurement_buf.moving_average,
      range_test_measurement_buf.rssi_latch_value,
      range_test_settings_buf.source_id,
      range_test_settings_buf.destination_id);
  }
#if defined(__IAR_SYSTEMS_ICC__)
  #pragma diag_default=Pa205
#endif
}
