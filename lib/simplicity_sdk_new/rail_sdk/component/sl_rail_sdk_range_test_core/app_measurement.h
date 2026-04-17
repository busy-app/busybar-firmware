/***************************************************************************//**
 * @file
 * @brief app_measurement.h
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

#ifndef APP_MEASUREMENT_H
#define APP_MEASUREMENT_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "sl_rail_types.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// Invalid counter value
#define RANGETEST_PACKET_COUNT_INVALID    ((uint16_t) 0xFFFF)

/// This enumeration contains the supported modes of the Range Test.
typedef enum {
  RADIO_MODE_RX,    ///> Receiver Mode. Device listens for packet.
  RADIO_MODE_TX,    ///> Transmitter Mode. Device sends packets.
} radio_modes_t;

/// Range Test status variables that contain settings for the remote
/// configuration.
#pragma pack(1)
typedef struct service_data_t{
  uint16_t       packets_repeat_number;         ///> Number of requested packets to send.
  uint16_t       channel;             ///> Selected channel for TX and RX.
  int16_t        tx_power;            ///> Radio transmit power in 0.1 dBm steps
  uint8_t        current_phy;         ///> Selected phy
  uint8_t        payload_length;      ///> Payload length of the packets.
} service_data_t;
#pragma pack()

/// Range Test status variables that contain settings and data that is
/// used during the Range Test execution.
typedef struct range_test_settings_t{
  radio_modes_t  radio_mode;          ///> TX or RX operation.
  uint8_t        current_phy;         ///> Selected phy
  int16_t        tx_power;            ///> Radio transmit power in 0.1 dBm steps
  uint16_t       channel;             ///> Selected channel for TX and RX.
  uint8_t        payload_length;      ///> Payload length of the packets.
  uint16_t       packets_repeat_number;         ///> Number of requested packets to send.
  uint8_t        destination_id;      ///> ID of the other device to send or receive.
  uint8_t        source_id;           ///> ID of this device.
  uint8_t        moving_average_window_size;    ///> Moving Average window size (no. of values).
  bool           usart_log_enable;    ///> True if UART logging is enabled in menu.
  bool           remote_config_enable;          ///> True if remote config is enabled in menu.
  uint16_t       service_channel;     ///> Selected service channel.
  uint8_t        service_phy;         ///> Selected service phy.
  uint8_t        number_of_retransmissions;    ///> Number of times of retransmissions tries
} range_test_settings_t;

typedef struct range_test_measurement_t{
  float    PER;                  ///> Current Packet Error Rate.
  float    moving_average;       ///> Current Moving Average.
  uint32_t moving_average_history[4U];   ///> Array that stores history to calculate MA.
  uint16_t packets_sent;         ///> Number of sent packets.
  uint16_t packets_received_counter;         ///> Counter in received packet.
  uint16_t packets_received_correctly;       ///> Number of CRC OK packets.
  uint16_t first_received_packet_offset;     ///> Counter offset in first received packet.
  uint16_t packets_with_crc_error;           ///> Number of CRC error packets.
  int8_t   rssi_latch_value;     ///> RSSI value logged for the last RX packet.
  uint8_t  moving_average_current_point_value;        ///> Points to current value in the MA window.
  bool     tx_is_running;        ///> True if the Range Test is running.
}range_test_measurement_t;

/// Packet structure.
#pragma pack(1)
typedef struct range_test_packet_t{
  uint16_t packet_counter;    ///> Value showing the number of this packet.
  uint8_t  destination_id;    ///> Destination device ID this packet was sent to.
  uint8_t  source_id;         ///> Device ID which shows which device sent this packet.
  uint8_t  repeat;            ///> Unused.
} range_test_packet_t;
#pragma pack()

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// All setting regarding the range test
extern volatile range_test_settings_t range_test_settings;
/// All data regarding the range test
extern volatile range_test_measurement_t range_test_measurement;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Helper function to differentiate custom and BLE or IEEE phys.
 * If only custom phys are in the project the retrun value will be false always.
 *
 * @param[in] index: index of the phy
 * @return if custom false otherwise true
 ******************************************************************************/
bool is_phy_standard(uint8_t index);

/*******************************************************************************
 * Helper function to differentiate custom and BLE or IEEE phys.
 * If only custom phys are in the project the retrun value will be false always.
 * @return if custom false otherwise true
 ******************************************************************************/
bool is_current_phy_standard(void);

/*******************************************************************************
 * Helper function to calculate the correct number of the BLE & IEEE phys.
 * @return uint8_t value where the ieee phy is the 0 number
 ******************************************************************************/
uint8_t current_phy_standard_value(void);

/*******************************************************************************
 * @brief Helper function to return the offsetted value for comparison
 *
 * @param[in] phy_index: index of the phy
 *
 * @return uint8_t: the phy number of the current std minus the custom phys
 ******************************************************************************/
uint8_t phy_standard_value(uint8_t phy_index);

/*******************************************************************************
 * @brief The function handles payload length overflow of BLE and IEEE802154
 ******************************************************************************/
void handle_payload_length_for_standard(void);

/*******************************************************************************
 * Get the rail handler which is selected in the phy selection
 * @return sl_rail_handle_t a pointer to the will be used handler
 ******************************************************************************/
sl_rail_handle_t get_current_rail_handler(void);

/*******************************************************************************
 * Set power level to the defined max and init it if needed
 *
 * @param[in] init: id true it will reinit the config
 ******************************************************************************/
void set_power_level_to_max(bool init);

/*******************************************************************************
 * Hides the configs from other parts of the application
 *
 * @param[out] base_frequency: The current frequency of the radio
 * @param[out] channel_spacing: The current channel spacing of the selected phy
 * @param[out] power: The current power of the radio
 ******************************************************************************/
void get_rail_config_data(uint32_t *base_frequency, uint32_t* channel_spacing, int16_t *power);

/*******************************************************************************
 * Hides the configs from other parts of the application
 *
 * @param[out] start: minimum number of channels in phy
 * @param[out] end: maximum number of channels in phy
 * @param[out] phy_index: index of the phy
 ******************************************************************************/
void get_rail_channel_range(uint16_t *start, uint16_t *end, uint8_t phy_index);

/*******************************************************************************
 * Hides the configs from other parts of the application
 *
 * @param[out] payload_min: minimum number of payload in phy
 * @param[out] payload_max: maximum number of payload in phy
 ******************************************************************************/
void get_rail_payload_range(uint8_t *payload_min, uint8_t *payload_max);

/*******************************************************************************
 * Generate a string for the BLE mobile APP
 * @param[out] buffer: where to write the characters
 * @param[out] length: the length of the whole written string
 ******************************************************************************/
void phy_list_generation(uint8_t *buffer, uint8_t *length);

/*******************************************************************************
 * Inits the phys for the range test, and counts them.
 ******************************************************************************/
void init_range_test_phys(void);

/*******************************************************************************
 * @brief Resets the internal status of the Range Test and sets up needed
 *         settings
 ******************************************************************************/
void range_test_init(void);

/*******************************************************************************
 * @brief Resets all measurement data
 ******************************************************************************/
void range_test_reset_values(void);

/*******************************************************************************
 * @brief TX part of the measurement, manages counting and logging/updating
 *
 * @return refresh_screen_is_needed: true if LCD needed to be updated
 ******************************************************************************/
bool send_measurement(void);

/*******************************************************************************
 * @brief Set all handlers to IDLE state, useful as multiply handlers can
 *         occupy the radio same time and block each other at worst cases
 ******************************************************************************/
void set_all_radio_handlers_to_idle(void);

/*******************************************************************************
 * @brief Prepare the selected handler to RX measurement
 ******************************************************************************/
void receive_setup_radio(void);

/*******************************************************************************
 * @brief Stops the RX measurement and sets the radio handler to IDLE
 ******************************************************************************/
void stop_receive_measurement(void);

/*******************************************************************************
 * @brief Measurement RX part logic, here is the packet number check and etc.
 *
 * @return refresh_screen_is_needed: true if the LCD is needed to be updated
 ******************************************************************************/
bool receive_measurement(void);

/*******************************************************************************
 * @brief Logging function which prints out the current received or sent packet,
 *        and statistics.
 ******************************************************************************/
void print_log(void);

/*******************************************************************************
 * @brief Prints out the errors that happened during rail_handle on RX or TX
 ******************************************************************************/
void print_errors_from_rail_handler(void);

/*******************************************************************************
 * Helper function to return the custom phy number
 * @return number_of_custom_phys
 ******************************************************************************/
uint8_t get_number_of_custom_phys(void);

/******************************************************************************
* RAIL callback, called if a RAIL event occurs
******************************************************************************/
void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events);

/*******************************************************************************
 * Helper function to apply the new power level from the range test settings
 ******************************************************************************/
void update_tx_power(void);

/*******************************************************************************
 * Get the minimum and maximum Power Amplifier setting in deci-dBm units
 * @param[out] min_power_deci_dbm: pointer to store minimum PA value in deci-dBm (can be NULL)
 * @param[out] max_power_deci_dbm: pointer to store maximum PA value in deci-dBm (can be NULL)
 ******************************************************************************/
void get_tx_power_deci_dbm_range(int16_t *min_power_deci_dbm, int16_t *max_power_deci_dbm);

/*******************************************************************************
 * @brief Function that returns the safe size of the string
 *
 * @param[in] src: pointer to the string
 *
 * @return Size of the string
 ******************************************************************************/
uint8_t safe_strlen(char *src);

/*******************************************************************************
 * Sends the service packet with the configuration details on the service channel.
 ******************************************************************************/
void send_service_packet(void);

/*******************************************************************************
 * Starts the reception of the service packet on the service channel.
 ******************************************************************************/
void receive_service_packet(void);

/*******************************************************************************
 * Sets the RAIL PHY back to the test defined one.
 ******************************************************************************/
void undo_service_config(void);

/*******************************************************************************
 * Handles the service packet containing the test configuration.
 * @return Boolean indicates the validity of the received packet
 ******************************************************************************/
bool service_packet_received(void);

#endif // APP_MEASUREMENT_H
