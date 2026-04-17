/***************************************************************************//**
 * @file
 * @brief app_measurement_standard.c
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
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "sl_component_catalog.h"
#include "app_menu.h"
#include "app_measurement.h"
#include "app_measurement_standard.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_ble.h"
#include "sl_rail_sdk_fifo_size_config.h"
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
#include "sl_rail_util_init.h"
#endif
#include "sl_rail_chip_specific.h"
#include "app_assert.h"
#include "app_log.h"
#include "sl_rail_util_compatible_pa.h"
#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
#include "sl_rail_util_ant_div.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void range_test_generate_remainder(uint8_t *remainder);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
sl_rail_handle_t emPhyRailHandle;
#endif

/** Macro to determine array size. */
#define COMMON_UTILS_COUNTOF(a) (sizeof(a) / sizeof((a)[0]))

// Provide weak overridable internal RX FIFO and RX Packet Buffer for
// non-UC protocols to share and for RAIL 2.x backwards compatibility.
static sl_rail_packet_queue_entry_t builtin_rx_packet_queue[SL_RAIL_BUILTIN_RX_PACKET_QUEUE_ENTRIES];
static SL_RAIL_DECLARE_FIFO_BUFFER(builtin_rx_fifo, SL_RAIL_BUILTIN_RX_FIFO_BYTES);
__WEAK sl_rail_packet_queue_entry_t * const sl_rail_builtin_rx_packet_queue_ptr = builtin_rx_packet_queue;
__WEAK sl_rail_fifo_buffer_align_t * const sl_rail_builtin_rx_fifo_ptr = builtin_rx_fifo;
__WEAK const uint16_t sl_rail_builtin_rx_packet_queue_entries = COMMON_UTILS_COUNTOF(builtin_rx_packet_queue);
__WEAK const uint16_t sl_rail_builtin_rx_fifo_bytes = sizeof(builtin_rx_fifo);

// Provide weak overridable internal TX FIFO and TX Packet Buffer for
// non-UC protocols to share and for RAIL 2.x backwards compatibility.
static SL_RAIL_DECLARE_FIFO_BUFFER(sli_tx_fifo_buffer, SL_RAIL_SDK_TX_FIFO_SIZE);

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Config for IEEE protocol
static const sl_rail_ieee802154_config_t rail_ieee802154_config = {
  .p_addresses = NULL,
  .ack_config = {
    .enable = false,          // Turn on auto ACK for IEEE 802.15.4.
    .ack_timeout_us = 672,        // 54-12 symbols * 16 us/symbol = 672 us.
    .rx_transitions = {
      .success = SL_RAIL_RF_STATE_RX,        // Go to TX to send the ACK.
      .error = SL_RAIL_RF_STATE_RX,          // For an always-on device stay in RX.
    },
    .tx_transitions = {
      .success = SL_RAIL_RF_STATE_RX,        // Go to RX for receiving the ACK.
      .error = SL_RAIL_RF_STATE_RX,          // For an always-on device stay in RX.
    },
  },
  .timings = {
    .idle_to_rx = 100,
    .idle_to_tx = 100,
    .rx_to_tx = 192,          // 12 symbols * 16 us/symbol = 192 us
    .tx_to_rx = 192,          // 12 symbols * 16 us/symbol = 192 us
    .rxsearch_timeout = 0,       // Not used
    .tx_to_rxsearch_timeout = 0,       // Not used
  },
  .frames_mask = SL_RAIL_IEEE802154_ACCEPT_STANDARD_FRAMES,
  .promiscuous_mode = true,        // Enable format and address filtering.
  .is_pan_coordinator = false,
};

/// Rail handlers for ble and ieee protocols
sl_rail_handle_t rail_handles[PROT_NO_OF_ELEMENTS] = { SL_RAIL_EFR32_HANDLE, SL_RAIL_EFR32_HANDLE };

/// Collection of all available standard phys
static range_test_std_phys_t range_test_std_phys[NUM_OF_PREDEFINED_PHYS] = {
  {
    .phy = IEEE802154_250KBPS,
    .protocol = PROT_IEEE802154,
#if SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 && !defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8)
    .is_supported = true
#else
    .is_supported = false
#endif
  },
  {
    .phy = IEEE802154_250KBPS_ANTDIV,
    .protocol = PROT_IEEE802154,
#if SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 && defined(SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT)
    .is_supported = true
#else
    .is_supported = false
#endif
  },
  {
    .phy = BLE_125KBPS,
    .protocol = PROT_BLE,
#if ((SL_RAIL_BLE_SUPPORTS_CODED_PHY == 1) && (SL_RAIL_SUPPORTS_PROTOCOL_BLE == 1))
    .is_supported = true
#else
    .is_supported = false
#endif
  },
  {
    .phy = BLE_500KBPS,
    .protocol = PROT_BLE,
#if ((SL_RAIL_BLE_SUPPORTS_CODED_PHY == 1) && (SL_RAIL_SUPPORTS_PROTOCOL_BLE == 1))
    .is_supported = true
#else
    .is_supported = false
#endif
  },
  {
    .phy = BLE_1MBPS,
    .protocol = PROT_BLE,
#if ((SL_RAIL_BLE_SUPPORTS_1_MBPS == 1) && (SL_RAIL_SUPPORTS_PROTOCOL_BLE == 1))
    .is_supported = true
#else
    .is_supported = false
#endif
  },
  {
    .phy = BLE_2MBPS,
    .protocol = PROT_BLE,
#if ((SL_RAIL_BLE_SUPPORTS_2_MBPS == 1) && (SL_RAIL_SUPPORTS_PROTOCOL_BLE == 1))
    .is_supported = true
#else
    .is_supported = false
#endif
  }
};

/// Rail configurations for available non custom phys
static sl_rail_config_t rail_standard_configs[PROT_NO_OF_ELEMENTS] = {
  {
    .events_callback = &sl_rail_util_on_event,
  },
  {
    .events_callback = &sl_rail_util_on_event,
  }
};

/// Flag to follow init part of the component
static bool init_done = false;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief Returns the currently used standard phy, and its data
 *
 * @return range_test_std_phys_t
 ******************************************************************************/
range_test_std_phys_t get_current_std_phy(void)
{
  return range_test_std_phys[current_phy_standard_value()];
}

/*******************************************************************************
 * @brief Checks if the phy or protocol is supported by the device
 *
 * @return is_supported: returns true if yes
 ******************************************************************************/
bool current_std_phy_is_supported(void)
{
  return range_test_std_phys[current_phy_standard_value()].is_supported;
}

/*******************************************************************************
 * @brief Checks if the phy or protocol is supported by the device
 *
 * @param[in] index: Index of the phy
 *
 * @return is_supported: returns true if yes
 ******************************************************************************/
bool std_phy_is_supported(uint8_t index)
{
  return range_test_std_phys[index - get_number_of_custom_phys()].is_supported;
}

/*******************************************************************************
 * @brief Return the standard handler which is used currently
 *
 * @return sl_rail_handle_t: pointer for actual rail standard handler
 ******************************************************************************/
sl_rail_handle_t get_standard_rail_handler(void)
{
  return rail_handles[range_test_std_phys[current_phy_standard_value()].protocol];
}

/*******************************************************************************
 * @brief Return True if the Standards init function run successfully
 *
 * @return bool: True if the Standards init function run successfully
 ******************************************************************************/
bool is_init_range_test_standard_ready(void)
{
  return init_done;
}

/*******************************************************************************
 * @brief Init block for the standard phys
 ******************************************************************************/
void init_range_test_standard_phys(uint8_t* number_of_phys)
{
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

#if defined(SL_CATALOG_RANGE_TEST_DMP_COMPONENT_PRESENT)
  // Added 3rd buffer as only 2 buffer is supported by default
  status = sl_rail_add_state_buffer_3(RAIL_EFR32_HANDLE);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_error("sl_rail_add_state_buffer_3 error, 3rd state buffer was previously added or this isn't the RAIL multiprotocol library. Error code: %lX", status);
  }
#endif

  *number_of_phys = *number_of_phys + NUM_OF_PREDEFINED_PHYS;
  for (int i = 0; i < PROT_NO_OF_ELEMENTS; i++) {
    // Initialize the rail_init_config structure
    rail_standard_configs[i].events_callback = &sl_rail_util_on_event;
    rail_standard_configs[i].rx_packet_queue_entries = sl_rail_builtin_rx_packet_queue_entries;
    rail_standard_configs[i].rx_fifo_bytes = sl_rail_builtin_rx_fifo_bytes;
    rail_standard_configs[i].p_rx_packet_queue = sl_rail_builtin_rx_packet_queue_ptr;
    rail_standard_configs[i].p_rx_fifo_buffer = sl_rail_builtin_rx_fifo_ptr;
    rail_standard_configs[i].tx_fifo_bytes = SL_RAIL_SDK_TX_FIFO_SIZE;
    rail_standard_configs[i].tx_fifo_init_bytes = 0U;
    rail_standard_configs[i].p_tx_fifo_buffer = sli_tx_fifo_buffer;

    status =  sl_rail_init(&rail_handles[i], &rail_standard_configs[i], NULL);
    app_assert(rail_handles[i] != NULL, "Init failed on %s with error code %ld", i == 0 ? "BLE" : "IEEE", status);

    while (!sl_rail_is_initialized(rail_handles[i])) ;

    status =
      sl_rail_config_events(rail_handles[i],
                            SL_RAIL_EVENTS_ALL,
                            (SL_RAIL_EVENTS_RX_COMPLETION
                             | SL_RAIL_EVENTS_TX_COMPLETION
                             | SL_RAIL_EVENT_SCHEDULER_STATUS
                             | SL_RAIL_EVENT_CONFIG_SCHEDULED
                             | SL_RAIL_EVENT_CONFIG_UNSCHEDULED
                             | SL_RAIL_EVENT_CAL_NEEDED));

    if (status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_error("sl_rail_config_events failed");
    }

    if (PROT_IEEE802154 == i) {
      // Configure RAIL instance to run in IEEE 802.15.4 mode
      status = sl_rail_ieee802154_init(rail_handles[i], &rail_ieee802154_config);
#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
      emPhyRailHandle = rail_handles[i];
      sl_rail_util_ant_div_init();
      sl_rail_util_ant_div_set_rx_antenna_mode(SL_RAIL_UTIL_ANTENNA_MODE_DISABLED);
      sl_rail_util_ant_div_update_antenna_config();
#endif
      if (status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_error("sl_rail_ieee802154_init failed");
      }
      // Configures channels and more.
      // Note: As discussed in RAIL_LIB-2862, this API has to be called before
      //       TX power is configured.
      status = sl_rail_ieee802154_config_2p4_ghz_radio(rail_handles[i]);
      if (status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_error("sl_rail_ieee802154_config_2p4_ghz_radio failed");
      }
      if (status == SL_RAIL_STATUS_NO_ERROR) {
        range_test_std_phys[IEEE802154_250KBPS].is_supported = true;
      } else {
        range_test_std_phys[IEEE802154_250KBPS].is_supported = false;
      }
      sl_rail_idle(rail_handles[i], SL_RAIL_IDLE, true);
    } else if (PROT_BLE == i) {
#if SL_RAIL_SUPPORTS_PROTOCOL_BLE == 1
      // Configure RAIL instance to run in BLE mode
      sl_rail_ble_init(rail_handles[PROT_BLE]);

      sl_rail_idle(rail_handles[PROT_BLE], SL_RAIL_IDLE, true);
      // To get the default to switch from
      status = sl_rail_ble_config_phy_1_mbps(rail_handles[PROT_BLE]);
      if (status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_error("sl_rail_ble_config_phy_1_mbps failed with %lu\n", status);
      }

      sl_rail_idle(rail_handles[i], SL_RAIL_IDLE, true);
      // Configures us for the first advertising channel (Physical: 0, Logical: 37).
      // The CRC init value and Access Address come from the BLE specification.
      // Note: As discussed in RAIL_LIB-2862, this API has to be called before
      //       TX power is configured.
      sl_rail_ble_state_t ble_params = {
        .crc_init = BLE_CRC_INIT,
        .access_address = BLE_ACCESS_ADDRESS,
        .logical_channel = BLE_LOGICAL_CH,
        .disable_whitening = DISABLE_WHITENING,
      };
      status = sl_rail_ble_config_channel_radio_params(rail_handles[PROT_BLE],
                                                       &ble_params);
      if (status != SL_RAIL_STATUS_NO_ERROR) {
        app_log_error("RAIL_BLE_ConfigChannelRadioParams failed");
      }
      sl_rail_idle(rail_handles[PROT_BLE], SL_RAIL_IDLE, true);
#endif
    }

    uint16_t channel_first = sl_rail_get_first_channel(rail_handles[i], NULL);
    sl_rail_prepare_channel(rail_handles[i], channel_first);

    sl_rail_tx_pa_mode_t new_pa_mode = sl_rail_get_pa_mode_from_channel_entry(rail_handles[i]); //from configs
    (void) sl_rail_util_pa_post_init(rail_handles[i], new_pa_mode);

    sl_rail_tx_power_t max_power_ddbm = SL_RAIL_TX_POWER_MIN;
    sl_rail_tx_power_t min_power_ddbm = SL_RAIL_TX_POWER_MIN;
    sl_rail_tx_power_t step_ddbm = SL_RAIL_TX_POWER_MIN;

    status = sl_rail_util_pa_get_tx_power_limits(rail_handles[i],
                                                 SL_RAIL_TX_PA_MODE_INVALID,
                                                 &min_power_ddbm,
                                                 &max_power_ddbm,
                                                 &step_ddbm);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      status = sl_rail_set_tx_power_dbm(rail_handles[i], 100);
    } else {
      status = sl_rail_set_tx_power_dbm(rail_handles[i], max_power_ddbm);
    }
    app_assert(status == SL_RAIL_STATUS_NO_ERROR, "sl_rail_set_tx_power failed");
  }

  init_done = true;
}

/*******************************************************************************
 * @brief Change the protocol on the BLE handler
 *
 * @return is_supported: true if the radio can use this protocol
 ******************************************************************************/
bool ble_protocol_change(void)
{
  bool is_supported = false;
// Idle
  sl_rail_idle(rail_handles[PROT_BLE], SL_RAIL_IDLE, true);
#if SL_RAIL_SUPPORTS_PROTOCOL_BLE == 1
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  switch (current_phy_standard_value()) {
#if SL_RAIL_BLE_SUPPORTS_CODED_PHY
    case BLE_125KBPS:
      if (range_test_std_phys[current_phy_standard_value()].is_supported) {
        status = sl_rail_ble_config_phy_coded(rail_handles[PROT_BLE],
                                              SL_RAIL_BLE_CODING_125_KBPS);
        if (status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_error("SL_RAIL_BLE_CODING_125_KBPS failed with %lu\n", status);
        }
      }
      break;
    case BLE_500KBPS:
      if (range_test_std_phys[current_phy_standard_value()].is_supported) {
        status = sl_rail_ble_config_phy_coded(rail_handles[PROT_BLE],
                                              SL_RAIL_BLE_CODING_500_KBPS);
        if (status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_error("SL_RAIL_BLE_CODING_500_KBPS failed with %lu\n", status);
        }
      }
      break;
#endif
#if SL_RAIL_BLE_SUPPORTS_1_MBPS
    case BLE_1MBPS:
      if (range_test_std_phys[current_phy_standard_value()].is_supported) {
        status = sl_rail_ble_config_phy_1_mbps(rail_handles[PROT_BLE]);
        if (status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_error("sl_rail_ble_config_phy_1_mbps failed with %lu\n", status);
        }
      }
      break;
#endif
#if SL_RAIL_BLE_SUPPORTS_2_MBPS
    case BLE_2MBPS:
      if (range_test_std_phys[current_phy_standard_value()].is_supported) {
        status = sl_rail_ble_config_phy_2_mbps(rail_handles[PROT_BLE]);
        if (status != SL_RAIL_STATUS_NO_ERROR) {
          app_log_error("sl_rail_ble_config_phy_2_mbps failed with %lu\n", status);
        }
      }
      break;
#endif
    default:
      break;
  }
  if (range_test_std_phys[current_phy_standard_value()].is_supported) {
    if (status == SL_RAIL_STATUS_NO_ERROR) {
      is_supported = true;
    } else {
      is_supported = false;
      range_test_std_phys[current_phy_standard_value()].is_supported =
        false;
    }
  }
  sl_rail_ble_state_t ble_params = {
    .crc_init = BLE_CRC_INIT,
    .access_address = BLE_ACCESS_ADDRESS,
    .logical_channel = BLE_LOGICAL_CH,
    .disable_whitening = DISABLE_WHITENING,
  };
  status = sl_rail_ble_config_channel_radio_params(rail_handles[PROT_BLE], &ble_params);

  if (status != SL_RAIL_STATUS_NO_ERROR) {
    app_log_error("RAIL_BLE_ConfigChannelRadioParams failed");
  }
#else
  is_supported = false;
#endif
  return is_supported;
}

/*******************************************************************************
 * @brief Set IEEE handler to IDLE state, useful as multiply handlers can
 *        occupy the radio same time and block each other at worst cases
 ******************************************************************************/
void set_ieee_handler_to_idle(void)
{
  sl_rail_idle(rail_handles[PROT_IEEE802154], SL_RAIL_IDLE, true);
}

/*******************************************************************************
 * @brief Checks if the current phy is BLE or IEEE
 *
 * @return bool: true if current phy is a BLE protocol phy
 ******************************************************************************/
bool is_current_phy_ble(void)
{
  if (current_phy_standard_value() > IEEE802154_250KBPS_ANTDIV) {
    return true;
  } else {
    return false;
  }
}

/*******************************************************************************
 * @brief Set BLE handler to IDLE state, useful as multiply handlers can
 *        occupy the radio same time and block each other at worst cases
 ******************************************************************************/
void set_ble_handler_to_idle(void)
{
  sl_rail_idle(rail_handles[PROT_BLE], SL_RAIL_IDLE, true);
}

/*******************************************************************************
 * @brief The function resets payload length to the minimum value
 *        It prevents failures if the previously set payload length
 *        (of the previous PHY) is bigger then the maximum payload length
 *        of the actually set
 ******************************************************************************/
void reset_payload_length_for_standard(void)
{
  range_test_settings.payload_length = PAYLOAD_LEN_MIN;
}

/*******************************************************************************
 * @brief The function handles payload length overflow of BLE and IEEE802154
 ******************************************************************************/
void handle_payload_length_for_standard(void)
{
  if (current_phy_standard_value() == IEEE802154_250KBPS || current_phy_standard_value() == IEEE802154_250KBPS_ANTDIV) {
    if (range_test_settings.payload_length > IEEE802154_PAYLOAD_LEN_MAX) {
      reset_payload_length_for_standard();
    }
  } else {
    // BLE
    if (range_test_settings.payload_length > BLE_PAYLOAD_LEN_MAX) {
      reset_payload_length_for_standard();
    }
  }
}

/*******************************************************************************
 * @brief Print function for menu, also updates the channel settings
 *
 * @param[out] print_buffer: where the phy name should be written
 *
 * @param[in] phy_index: index of the PHY
 ******************************************************************************/
void print_standard_name(char *print_buffer, uint8_t phy_index)
{
  // Length for print_buffer comes from app_menu.c
  if (phy_standard_value(phy_index) == IEEE802154_250KBPS) {
#if SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154
    snprintf(print_buffer, 15, "IEEE 802.15.4");
#endif
  } else if (phy_standard_value(phy_index) == IEEE802154_250KBPS_ANTDIV) {
    snprintf(print_buffer, 16, "IEEE 802.ANTDIV");
  } else {
    switch (phy_standard_value(phy_index)) {
#if SL_RAIL_BLE_SUPPORTS_CODED_PHY
      case BLE_125KBPS:
        snprintf(print_buffer, 15, "BLE 125kbps");
        break;
      case BLE_500KBPS:
        snprintf(print_buffer, 15, "BLE 500kbps");
        break;
#endif
#if SL_RAIL_BLE_SUPPORTS_1_MBPS
      case BLE_1MBPS:
        snprintf(print_buffer, 15, "BLE 1Mbps");
        break;
#endif
#if SL_RAIL_BLE_SUPPORTS_2_MBPS
      case BLE_2MBPS:
        snprintf(print_buffer, 15, "BLE 2Mbps");
        break;
#endif
      default:
        break;
    }
  }
}

/*******************************************************************************
 * @brief Set the correct value for the standard phy channels
 ******************************************************************************/
void set_standard_phy_channel(void)
{
  if (current_phy_standard_value() == IEEE802154_250KBPS || current_phy_standard_value() == IEEE802154_250KBPS_ANTDIV) {
    if (range_test_settings.channel > IEEE802154_END_CHANNEL || range_test_settings.channel < IEEE802154_START_CHANNEL) {
      range_test_settings.channel = IEEE802154_START_CHANNEL;
    }
  } else {
    if (range_test_settings.channel > BLE_PHYSICAL_END_CH) {
      range_test_settings.channel = BLE_PHYSICAL_START_CH;
    }
  }
}

/*******************************************************************************
 * @brief Points to the start of the payload in case of
 *        standard phy communication
 *
 * @param[in] received_buffer: where the radio received the packet
 *
 * @return payload: The start of the payload address
 ******************************************************************************/
range_test_packet_t* get_start_of_payload_for_standard(uint8_t* received_buffer)
{
  range_test_packet_t* payload = NULL;
  uint8_t current_std_phy = current_phy_standard_value();
  if (current_std_phy == IEEE802154_250KBPS || current_std_phy == IEEE802154_250KBPS_ANTDIV) {
    // 1st byte is the length field (PHR)
    data_frame_format_t* data_frame = (data_frame_format_t *) &received_buffer[1];
    payload = &data_frame->payload;
  } else if (current_std_phy == BLE_125KBPS
             || current_std_phy == BLE_500KBPS
             || current_std_phy == BLE_1MBPS
             || current_std_phy == BLE_2MBPS) {
    payload = (range_test_packet_t*)&received_buffer[16];
  } else {
    // Can not happen
    app_log_info("Unknown PHY \n");
  }
  return payload;
}

uint8_t get_ieee802154_data_frame_length(void)
{
  return sizeof(mhr_t) + range_test_settings.payload_length + IEEE802154_PHR_LENGTH;
}

/*******************************************************************************
* @brief The function prepares a std. IEEE 802.15.4 Data frame format
* Data frame format
* - MHR:
*   - Frame Control (2 bytes)
*   - Sequence number (0/1)
*   - Addressing fields (variable)
*   - Auxiliary Security Header (variable)
*   - Header IEs (variable)
* - MAC Payload:
*   - Payload IEs (variable)
*   - Data Payload (variable)
* - MFR (2/4)
*
* @param[in] packet_number: payload packet counter
* @param[out] tx_buffer: this buffer contains the prepared std packet
*******************************************************************************/
void prepare_ieee802154_data_frame(uint16_t packet_number, uint8_t *tx_buffer)
{
  uint8_t i = 0;
  data_frame_format_t* data_frame;

  // PHR - Length field (byte0) is not included but the CRC (2 bytes)
  tx_buffer[i++] = sizeof(mhr_t) + range_test_settings.payload_length + IEEE802154_CRC_LENGTH;
  // Payload length is counted from txBuffer[1] (byte0: Length field)
  data_frame = (data_frame_format_t*) &tx_buffer[i];
  data_frame->mhr.frame_control = FRAME_CONTROL;
  data_frame->mhr.sequence_number = packet_number;
  data_frame->mhr.destination_pan_id = 0xFFFF;
  data_frame->mhr.source_address = 0x0000;
  data_frame->mhr.destination_address = 0xFFFF;
  data_frame->payload.source_id = range_test_settings.source_id;
  data_frame->payload.destination_id = range_test_settings.destination_id;
  data_frame->payload.packet_counter = packet_number;
  data_frame->payload.repeat = 0x00;
  if (range_test_settings.payload_length > PAYLOAD_LEN_MIN) {
    range_test_generate_remainder(&data_frame->remainder[0]);
  }
}

/*******************************************************************************
* @brief The function prepares the following BLE PDU:
*         - ADV_NONCONN_IND
*         - LL advertiser address
*         - AD Structure: Flags
*         - AD Structure: Manufacturer specific
*           - Company ID
*           - Structure type; used for backward compatibility
*           - rangeTestPacket_t
*           - 0x55, 0xAA, 0x55, 0xAA... (only if payload length is more than sizeof(rangeTestPacket_t))
*
* @param[in] packet_number: payload packet counter
* @param[out] tx_buffer: this buffer contains the prepared BLE PDU packet
*******************************************************************************/
void prepare_ble_advertising_channel_pdu(uint16_t packet_number, uint8_t *tx_buffer)
{
  advertising_nonconnectable_undirected_t* ble_tx_pdu = (advertising_nonconnectable_undirected_t*)tx_buffer;

  // BLE advertisement header
  ble_tx_pdu->header.type = BLE_HEADER_LSB;   //ADV_NONCONN_IND
  //LL header, LL advertiser's address, LL advertisement data
  ble_tx_pdu->header.length = sizeof(ble_tx_pdu->header.type)
                              + sizeof(ble_tx_pdu->advAddr)
                              + sizeof(ble_tx_pdu->flags)
                              + sizeof(ble_tx_pdu->manufactSpec.length)
                              + sizeof(ble_tx_pdu->manufactSpec.advertising_type)
                              + sizeof(ble_tx_pdu->manufactSpec.company_id)
                              + sizeof(ble_tx_pdu->manufactSpec.version)
                              + range_test_settings.payload_length;

  // LL advertiser's address
  ble_tx_pdu->advAddr[0] = 0xC1;
  ble_tx_pdu->advAddr[1] = 0x29;
  ble_tx_pdu->advAddr[2] = 0xD8;
  ble_tx_pdu->advAddr[3] = 0x57;
  ble_tx_pdu->advAddr[4] = 0x0B;
  ble_tx_pdu->advAddr[5] = 0x00;
  // AD Structure: Flags
  ble_tx_pdu->flags.length = sizeof(ble_tx_pdu->flags.advertising_type) + sizeof(ble_tx_pdu->flags.flags);   // Length of field: Type + Flags
  ble_tx_pdu->flags.advertising_type = ADSTRUCT_TYPE_FLAG;   // AD type: Flags
  ble_tx_pdu->flags.flags = DISABLE_BR_EDR | LE_GENERAL_DISCOVERABLE_MODE;   // Flags: BR/EDR is disabled, LE General Discoverable Mode
  // AD Structure: Manufacturer specific
  ble_tx_pdu->manufactSpec.length = sizeof(ble_tx_pdu->manufactSpec.advertising_type)
                                    + sizeof(ble_tx_pdu->manufactSpec.company_id)
                                    + sizeof(ble_tx_pdu->manufactSpec.version)
                                    + range_test_settings.payload_length;
  ble_tx_pdu->manufactSpec.advertising_type = ADSTRUCT_TYPE_MANUFACTURER_SPECIFIC;   // AD type: Manufacturer Specific Data
  ble_tx_pdu->manufactSpec.company_id = COMPANY_ID;
  ble_tx_pdu->manufactSpec.version = 0x01;
  // RangeTest payload

  ble_tx_pdu->manufactSpec.payload.packet_counter = packet_number;
  ble_tx_pdu->manufactSpec.payload.destination_id = range_test_settings.destination_id;
  ble_tx_pdu->manufactSpec.payload.source_id = range_test_settings.source_id;
  ble_tx_pdu->manufactSpec.payload.repeat = 0xFF;
  if (range_test_settings.payload_length > PAYLOAD_LEN_MIN) {
    range_test_generate_remainder(ble_tx_pdu->manufactSpec.remainder);
  }
}

/*******************************************************************************
 * @brief This function selects (cycles) which PHY to use for TX/RX.
 *
 * @param[in] init: needs to be set to init value
 ******************************************************************************/
void menu_set_std_phy(bool init)
{
  if (is_current_phy_standard()) {
    if (range_test_std_phys[current_phy_standard_value()].is_supported
        && current_phy_standard_value() == IEEE802154_250KBPS) {
#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
      sl_rail_util_ant_div_set_rx_antenna_mode(SL_RAIL_UTIL_ANTENNA_MODE_DISABLED);
      sl_rail_util_ant_div_update_antenna_config();
#endif
      sl_rail_ieee802154_config_2p4_ghz_radio(rail_handles[PROT_IEEE802154]);
    } else if (range_test_std_phys[current_phy_standard_value()].is_supported
               && current_phy_standard_value() == IEEE802154_250KBPS_ANTDIV) {
#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
      sl_rail_util_ant_div_set_rx_antenna_mode(SL_RAIL_UTIL_ANTENNA_MODE_DIVERSITY);
      sl_rail_util_ant_div_update_antenna_config();
      sl_rail_ieee802154_config_2p4_ghz_radio_ant_div(rail_handles[PROT_IEEE802154]);
#else
      range_test_settings.current_phy++;
      menu_set_std_phy(false);
#endif
    } else {
      if (init) {
        range_test_settings.current_phy = 0;
      }
      while (true) {
        if (!ble_protocol_change()) {
          range_test_settings.current_phy++;
          if (range_test_settings.current_phy >= get_number_of_phys()) {
            range_test_settings.current_phy = 0;
            break;
          }
          if (range_test_std_phys[current_phy_standard_value()].is_supported) {
            break;
          }
        } else {
          break;
        }
      }
    }
  }
}

/*******************************************************************************
 * @brief Helper function to check if BLE or IEEE is enabled for range test
 ******************************************************************************/
bool standard_phy_are_present(void)
{
  if ((get_number_of_phys() - NUM_OF_PREDEFINED_PHYS) > 0) {
    if (range_test_settings.current_phy < get_number_of_custom_phys()) {
      return false;
    } else {
      return true;
    }
  } else {
    return true;
  }
}

/*******************************************************************************
 * @brief Helper function to check if phy of index is standard
 *
 * @param[in] index: index of the phy
 ******************************************************************************/
bool is_phy_standard(uint8_t index)
{
  if ((get_number_of_phys() - NUM_OF_PREDEFINED_PHYS) > 0) {
    if (index < get_number_of_custom_phys()) {
      return false;
    } else {
      return true;
    }
  } else {
    return true;
  }
}

/*******************************************************************************
 * @brief Helper function to return the offsetted value for comparison
 *
 * @return uint8_t: the phy number of the current std minus the custom phys
 ******************************************************************************/
uint8_t current_phy_standard_value(void)
{
  return phy_standard_value(range_test_settings.current_phy);
}

/*******************************************************************************
 * @brief Helper function to return the offsetted value for comparison
 *
 * @param[in] phy_index: index of the phy
 *
 * @return uint8_t: the phy number of the current std minus the custom phys
 ******************************************************************************/
uint8_t phy_standard_value(uint8_t phy_index)
{
  return (phy_index - get_number_of_custom_phys());
}

/*******************************************************************************
 * @brief Hides the configs from other parts of the application
 *
 * @param[out] base_frequency: The current frequency of the radio
 * @param[out] channel_spacing: The current channel spacing of the selected phy
 ******************************************************************************/
void get_rail_standard_config_data(uint32_t *base_frequency, uint32_t *channel_spacing)
{
  *base_frequency  = _2450_MHZ;
  *channel_spacing = NO_CHANNEL_SPACING;
}

/*******************************************************************************
 * @brief Hides the configs from other parts of the application
 *
 * @param[out] start: minimum number of channels in phy
 * @param[out] end: maximum number of channels in phy
 ******************************************************************************/
void get_rail_standard_channel_range(uint16_t *min, uint16_t *max, uint8_t phy_index)
{
  if (phy_standard_value(phy_index) == IEEE802154_250KBPS || phy_standard_value(phy_index) == IEEE802154_250KBPS_ANTDIV) {
    *min = IEEE802154_START_CHANNEL;
    *max = IEEE802154_END_CHANNEL;
  } else {
    *min = BLE_PHYSICAL_START_CH;
    *max = BLE_PHYSICAL_END_CH;
  }
}

/*******************************************************************************
 * @brief Generate a string for the BLE mobile APP from standard phys
 *
 * @param[in] phy_index: which phy name is needed
 * @param[out] buffer: where to write the characters
 * @param[out] length: the length of the whole written string
 ******************************************************************************/
void std_phy_list_generation(uint8_t phy_index, uint8_t *buffer, uint8_t *length)
{
  // *buffer length (255) comes from app_bluetooth.c
  uint8_t std_phy_index = phy_index - get_number_of_custom_phys();
  if (range_test_std_phys[std_phy_index].is_supported) {
    switch (std_phy_index) {
#if SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154
      case IEEE802154_250KBPS:
        snprintf((char*)(&buffer[*length]), 255, "%u:IEEE 802.15.4,", phy_index);
        break;
      case IEEE802154_250KBPS_ANTDIV:
        snprintf((char*)(&buffer[*length]), 255, "%u:IEEE 802.15.4 ANTDIV,", phy_index);
        break;
#endif
#if SL_RAIL_BLE_SUPPORTS_CODED_PHY
      case BLE_125KBPS:
        snprintf((char*)(&buffer[*length]), 255, "%u:BLE 125kbps,", phy_index);
        break;
      case BLE_500KBPS:
        snprintf((char*)(&buffer[*length]), 255, "%u:BLE 500kbps,", phy_index);
        break;
#endif
#if SL_RAIL_BLE_SUPPORTS_1_MBPS
      case BLE_1MBPS:
        snprintf((char*)(&buffer[*length]), 255, "%u:BLE 1Mbps,", phy_index);
        break;
#endif
#if SL_RAIL_BLE_SUPPORTS_2_MBPS
      case BLE_2MBPS:
        snprintf((char*)(&buffer[*length]), 255, "%u:BLE 2Mbps,", phy_index);
        break;
#endif
    }
    *length = safe_strlen((char*)(buffer));
  }
}

/*******************************************************************************
 * @brief Hides the configs from other parts of the application
 *
 * @param[out] payload_min: minimum number of channels in phy
 * @param[out] payload_max: maximum number of channels in phy
 ******************************************************************************/
void get_rail_standard_payload_range(uint8_t *payload_min, uint8_t *payload_max)
{
  if (current_phy_standard_value() == IEEE802154_250KBPS || current_phy_standard_value() == IEEE802154_250KBPS_ANTDIV) {
    *payload_min = PAYLOAD_LEN_MIN;
    *payload_max = IEEE802154_PAYLOAD_LEN_MAX;
  } else {
    *payload_min = PAYLOAD_LEN_MIN;
    *payload_max = BLE_PAYLOAD_LEN_MAX;
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief Function to fill remainder of the packet to be sent with 0x55, 0xAA
 *
 * @param[out] remainder: TX buffer to fill out with the generated payload.
 ******************************************************************************/
static void range_test_generate_remainder(uint8_t *remainder)
{
  uint8_t remainder_length = range_test_settings.payload_length - PAYLOAD_LEN_MIN;
  for (int i = 0; i < remainder_length; i++) {
    remainder[i] = (i % 2U) ? (0x55U) : (0xAAU);
  }
}
