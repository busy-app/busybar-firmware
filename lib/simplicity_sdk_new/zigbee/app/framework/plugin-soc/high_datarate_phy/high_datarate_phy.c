/***************************************************************************//**
 * @file high_datarate_phy.c
 * @brief High Bandwidth Phy implementation using rail_util_ieee802154_high_speed_phy
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "high_datarate_phy.h"
#include "sl_zigbee_debug_print.h"
#include "zigbee_app_framework_event.h"
#include "sl_zigbee_multi_network.h"
#include "high_datarate_phy_stack_interface.h"
#include "high_datarate_phy_config.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
#include "sl_rail_util_ieee802154_phy_select.h"
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT

#ifdef SL_CATALOG_MEMORY_MANAGER_PRESENT
#include "sl_memory_manager.h"
#else
#include "sl_malloc.h"
#endif // SL_CATALOG_MEMORY_MANAGER_PRESENT

static sl_zigbee_af_event_t app_cli_event;

#define MIN_PAYLOAD_LEN 2
#define MAX_PAYLOAD_LEN  (SL_HDR_PHY_MAX_PACKET_SIZE - 2 /* len bytes*/)

#define LEN_BYTES (2u)

static uint8_t local_byte_array[MAX_PAYLOAD_LEN + LEN_BYTES];
static uint32_t high_datarate_phy_rx_packet_count = 0;
static uint32_t high_datarate_phy_tx_packet_count = 0;
static void app_cli_event_handler(sl_zigbee_af_event_t *event);

uint32_t *sli_phy_rx_fifo = NULL;

// CLI Command handlers
void sl_high_datarate_phy_set_rx_enable_command(sl_cli_command_arg_t *arguments);
void sl_high_datarate_phy_clr_stats_command(sl_cli_command_arg_t *arguments);
void sl_high_datarate_phy_print_stats_command(sl_cli_command_arg_t *arguments);
void sl_high_datarate_phy_tx_command(sl_cli_command_arg_t *arguments);
void sl_high_datarate_phy_tx_sched_command(sl_cli_command_arg_t *arguments);
void sl_high_datarate_phy_set_phy_command(sl_cli_command_arg_t *arguments);
void sl_high_datarate_phy_get_phy_command(sl_cli_command_arg_t *arguments);
void sl_high_datarate_phy_force_tx_after_failed_hdr_phy_cca_command(sl_cli_command_arg_t *arguments);

void (*sl_high_datarate_phy_rx_callback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi, uint32_t pkt_rx_timestamp) = NULL;
void (*sl_high_datarate_phy_tx_complete_callback)(uint8_t mac_index, sl_status_t status, uint16_t packet_length, uint8_t *packet, uint8_t tag) = NULL;
extern sl_status_t sl_mac_send_raw_high_datarate_phy_message(uint8_t nwk_index, uint8_t *payload);
extern sl_status_t sl_mac_send_raw_high_datarate_phy_scheduled_message(uint8_t nwk_index, uint8_t *payload, sl_rail_time_t timestamp);
extern void sl_mac_lower_mac_set_high_datarate_csma_params(sl_rail_csma_config_t *csmaParams);
extern void sl_mac_lower_mac_set_high_datarate_phy_radio_priorities(sl_802154_radio_priorities_t *priorities);
extern sl_rail_status_t sl_mac_set_mode_switch_sync_detect(bool enable_f);
extern sl_status_t sl_mac_force_tx_after_failed_hdr_phy_cca(uint8_t hdr_csma_attempts, uint32_t min_bo_period_us, uint32_t max_bo_period_us);

/**
 * Default receive callback function for High-BW-phy packets
 * Note that packet does not include 4 byte CRC
 *              packet[0] packet[1] : 2 byte Length (packet[1] << 8 + packet[0])
 *              packet[2] .. packet[Length+1] payload
 *              linkQuality of received packet
 *              rssi of received packet
 *              pkt received (frame end) absolute timestamp of received packet
 *
 * @param[out] packet pointer
 */
void sl_high_datarate_phy_rx_cb(uint8_t *packet, uint8_t linkQuality, int8_t rssi, uint32_t pkt_rx_timestamp)
{
  high_datarate_phy_rx_packet_count++;
  sl_zigbee_app_debug_print(" Received Packet : ");
  for (uint16_t i = LEN_BYTES; i < (uint16_t)packet[0] + ((uint16_t)packet[1] << 8) + LEN_BYTES; i++ ) {
    sl_zigbee_app_debug_print("%02X ", packet[i]);
  }
  sl_zigbee_app_debug_println("\nLQI : %d\nRSSI : %d timestamp: 0x%04X\n", linkQuality, rssi, pkt_rx_timestamp);
}

void sl_mac_high_datarate_phy_rx_callback(uint8_t *packet, uint8_t linkQuality, int8_t rssi, uint32_t pkt_rx_timestamp)
{
  if (sl_high_datarate_phy_rx_callback != NULL) {
    (*sl_high_datarate_phy_rx_callback)(packet, linkQuality, rssi, pkt_rx_timestamp);
  }
}
/**
 * Default Tx callback function for High-BW-phy packets
 * Note that packet does not include 4 byte CRC
 *              mac_index : 1 byte mac index - usually 0
 *              status: sl_status_t indicating transmit status
 *              sli_zigbee_packet_header_t of transmitted packet
 *              tag for the transmitted packet
 *
 * @param[out] packet pointer
 */
void sl_high_datarate_phy_tx_cb(uint8_t mac_index, sl_status_t status, uint16_t packet_length, uint8_t* packet, uint8_t tag)
{
  (void)mac_index;
  (void)tag;
  high_datarate_phy_tx_packet_count++;
  sl_zigbee_app_debug_println("packet len = %d ", packet_length);

  for (uint16_t i = 0; i < packet_length; i++) {
    sl_zigbee_app_debug_print("%02X ", packet[i]);
  }

  sl_zigbee_app_debug_println("\nTx Status : 0x%02X\n", status);
}

void sl_mac_high_datarate_phy_tx_callback(uint8_t mac_index, sl_status_t status, uint16_t packet_length, uint8_t* packet, uint8_t tag)
{
  if (sl_high_datarate_phy_tx_complete_callback != NULL) {
    (*sl_high_datarate_phy_tx_complete_callback)(mac_index, status, packet_length, packet, tag);
  }
}
/**
 * Init function for High-BW-Phy events - This is called using the templates
 *
 *  @param[in] init_level
 */
void sli_high_datarate_phy_init(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT: {
      sl_zigbee_af_event_init(&app_cli_event, app_cli_event_handler);
      break;
    }
    case SL_ZIGBEE_INIT_LEVEL_DONE: {
      sl_high_datarate_phy_rx_callback = sl_high_datarate_phy_rx_cb;
      sl_high_datarate_phy_tx_complete_callback = sl_high_datarate_phy_tx_cb;
      break;
    }
  }
}
/**
 * Configures receive callback function
 *
 *  @param[in] Function pointer to receive callback
 *              packet[0] packet[1] : 2 byte Length (packet[1] << 8 + packet[0])
 *              packet[2] .. packet[Length+1] payload
 *              linkQuality of received packet
 *              rssi of received packet
 *
 * Note: By default, sl_high_datarate_phy_rx_cb fires upon receiving a High-BW-phy packet
 * The user is free to override this callback by calling this function and supplying
 * a different receive callback
 */
void sl_high_datarate_phy_config_rx_callback(void (*rx_callback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi, uint32_t pkt_rx_timestamp))
{
  sl_high_datarate_phy_rx_callback = rx_callback;
}
/**
 * Configures transmit complete callback function
 *  @param[out] Function pointer to receive tx complete callback
 *              mac_index : 1 byte mac index - usually 0
 *              status: sl_status_t indicating transmit status
 *              packet length of transmitted packet
 *              uint8_t pointer to packet
 *              tag for the transmitted packet
 *
 */
void sl_high_datarate_phy_config_tx_complete_callback(void (*tx_complete_callback)(uint8_t mac_index, sl_status_t status, uint16_t packet_length, uint8_t *packet, uint8_t tag))
{
  sl_high_datarate_phy_tx_complete_callback = tx_complete_callback;
}

/**
 * Configures CSMA parameters for high datarate phy packet
 *
 * @param[in]  Pointer to csma params used on high datarate phy packets
 *
 */
void sl_high_datarate_phy_config_csma_params(sl_rail_csma_config_t *csma_params)
{
  sl_mac_lower_mac_set_high_datarate_csma_params(csma_params);
}
/**
 * Configures priority for high datarate phy packet
 *
 * @param[in]  Pointer to csma params used on high datarate phy packets
 *
 */
void sl_high_datarate_phy_config_radio_priorities(sl_802154_radio_priorities_t *priorities)
{
  sl_mac_lower_mac_set_high_datarate_phy_radio_priorities(priorities);
}
/**
 *  @brief force cca retry for failed hdr phy tx, with the given parameters
 *  @param hdr_csma_attempts
 *  @param min_bo_period_us
 *  @param max_bo_period_us
 *  @return sl_status_t
 */
sl_status_t sl_high_datarate_phy_force_tx_after_failed_hdr_phy_cca(uint8_t hdr_csma_attempts, uint32_t min_bo_period_us, uint32_t max_bo_period_us)
{
  return sl_mac_force_tx_after_failed_hdr_phy_cca(hdr_csma_attempts, min_bo_period_us, max_bo_period_us);
}

/**
 * Transmits a High-BW-Phy packet consisting of <len> bytes of payload
 * Note that there is a 4 byte CRC which is tacked on later and is not part
 * of the packet parameter
 * @param[in] payload Pointer to bytes of transmitted data
 *              packet[0] packet[1] : 2 byte Length (packet[1] << 8 + packet[0])
 *              packet[2] .. packet[Length+1] payload
 *
 */
sl_status_t sl_high_datarate_phy_transmit(uint8_t *payload)
{
  uint16_t requested_len = (payload[1] << 8) + payload[0];
  if ( requested_len < MIN_PAYLOAD_LEN  || requested_len > MAX_PAYLOAD_LEN ) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sli_zigbee_state != SL_ZIGBEE_JOINED_NETWORK) {
    return SL_STATUS_INVALID_MODE;
  }
  return sl_mac_send_raw_high_datarate_phy_message(sli_zigbee_get_current_network_index(), payload);
}
/**
 * CLI Command handler to transmit High-BW-Phy packet
 * @param[in] length Transmits a packet that contains <length> bytes using the high-BW-phy
 *              packet[0] packet[1] : 2 byte Length (packet[1] << 8 + packet[0])
 *              packet[2] .. packet[Length+1] payload
 */
static uint16_t length;
void sl_high_datarate_phy_tx_command(sl_cli_command_arg_t *arguments)
{
  length = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_af_event_set_active(&app_cli_event);
}

/**
 * @brief Forces a transmission after a failed high data rate PHY CCA command.
 *
 * This function is invoked via CLI to force a transmission attempt after the
 * Clear Channel Assessment (CCA) fails in high data rate PHY mode. It retrieves
 * the number of CSMA attempts and the minimum and maximum backoff periods from
 * the CLI arguments, then calls the underlying PHY function to perform the operation.
 * The result status is printed for debugging purposes.
 *
 * @param arguments Pointer to the CLI command arguments structure.
 *   - arguments[0]: Number of CSMA attempts (uint8_t)
 *   - arguments[1]: Minimum backoff period in microseconds (uint32_t)
 *   - arguments[2]: Maximum backoff period in microseconds (uint32_t)
 */
void sl_high_datarate_phy_force_tx_after_failed_hdr_phy_cca_command(sl_cli_command_arg_t *arguments)
{
  uint8_t hdr_csma_attempts = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t min_bo_period_us = sl_cli_get_argument_uint32(arguments, 1);
  uint32_t max_bo_period_us = sl_cli_get_argument_uint32(arguments, 2);
  sl_status_t status = sl_high_datarate_phy_force_tx_after_failed_hdr_phy_cca(hdr_csma_attempts, min_bo_period_us, max_bo_period_us);
  sl_zigbee_app_debug_println("force_tx_after_failed_hdr_phy_cca: Status = 0x%02X", status);
}

/**
 * CLI Command handler to transmit High-BW-Phy scheduled packet
 * @param[in] length Transmits a packet that contains <length> bytes using the high-BW-phy
 *              packet[0] packet[1] : 2 byte Length (packet[1] << 8 + packet[0])
 *              packet[2] .. packet[Length+1] payload
 *              uint32_t : relative timestamp in uS for when the mode switch packet will start
 */
uint32_t timestamp = 0;
void sl_high_datarate_phy_tx_sched_command(sl_cli_command_arg_t *arguments)
{
  length = sl_cli_get_argument_uint16(arguments, 0);
  uint32_t timestamp_delta = sl_cli_get_argument_uint32(arguments, 1);
  timestamp = sl_rail_get_time(SL_RAIL_EFR32_HANDLE) + timestamp_delta;
  sl_zigbee_af_event_set_active(&app_cli_event);
}

/**
 * Event handler to call sl_high_datarate_phy_transmit function
 * This additional step is necessary because the function manipulates buffers
 * and this cannot be done from anywhere except the zigbee task context in an RTOS
 * application
 */
#include "sl_gpio.h"
static void app_cli_event_handler(sl_zigbee_af_event_t *event)
{
  (void)event;
  if ( length > MAX_PAYLOAD_LEN ) {
    sl_zigbee_app_debug_println("Status = 0x%02x", SL_STATUS_INVALID_PARAMETER);
    return;
  }
  local_byte_array[0] = (uint8_t)(length & 0xFF);
  local_byte_array[1] = (uint8_t)(length >> 8);
  for (uint16_t i = 2; i < length + LEN_BYTES; i++) {
    local_byte_array[i] = i - 1;
  }
  if (timestamp) {
    sl_status_t status = sl_high_datarate_phy_transmit_scheduled(local_byte_array, timestamp);
    timestamp = 0;
    sl_zigbee_app_debug_println("Status = 0x%02x", status);
  } else {
    sl_zigbee_app_debug_println("Status = 0x%02x", sl_high_datarate_phy_transmit(local_byte_array));
  }
}

/**
 * Enable or disable Reception of high bandwidth phy packets
 * @param[in] enable_f : set this argument to 1 to enable and 0 to disable reception of
 *                       high bandwidth phy packets
 *
 */
sl_rail_status_t sl_high_datarate_phy_set_reception_enable(bool enable_f)
{
  return (sl_mac_set_mode_switch_sync_detect(enable_f));
}

/**
 * CLI command to enable or disable Reception of high bandwidth phy packets
 * @param[in] enable_f : set this argument to 1 to enable and 0 to disable reception of
 *                       high bandwidth phy packets
 *
 */
void sl_high_datarate_phy_set_rx_enable_command(sl_cli_command_arg_t *arguments)
{
  bool enable_f = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_app_debug_println(" Enable HDR to %d Status = 0x%02X", enable_f, sl_high_datarate_phy_set_reception_enable(enable_f) );
}
/**
 * Clears count of High-BW-Phy received and transmitted packets
 *
 */
void sl_high_datarate_phy_clr_stats_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  high_datarate_phy_rx_packet_count = 0;
  high_datarate_phy_tx_packet_count = 0;
}
/**
 * Prints count of High-BW-Phy received and transmitted packets
 *
 */
void sl_high_datarate_phy_print_stats_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_app_debug_println("Total number of packets received : %d", high_datarate_phy_rx_packet_count);
  sl_zigbee_app_debug_println("Total number of packets transmitted : %d", high_datarate_phy_tx_packet_count);
}
/**
 * Transmits a High-BW-Phy packet consisting of <len> bytes of payload
 * Note that there is a 4 byte CRC which is tacked on later and is not part
 * of the packet parameter
 * @param[in] payload Pointer to bytes of transmitted data
 *              packet[0] packet[1] : 2 byte Length (packet[1] << 8 + packet[0])
 *              packet[2] .. packet[Length+1] payload
 * @param[in] absolute timestamp that packet is expected to be transmitted
 *
 */
sl_status_t sl_high_datarate_phy_transmit_scheduled(uint8_t *payload, sl_rail_time_t timestamp)
{
  uint16_t requested_len = (payload[1] << 8) + payload[0];
  if ( requested_len < MIN_PAYLOAD_LEN  || requested_len > MAX_PAYLOAD_LEN ) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sli_zigbee_state != SL_ZIGBEE_JOINED_NETWORK) {
    return SL_STATUS_INVALID_MODE;
  }
  sl_zigbee_app_debug_println("Tx sched timestamp: 0x%08lX\n", timestamp);
  return sl_mac_send_raw_high_datarate_phy_scheduled_message(sli_zigbee_get_current_network_index(), payload, timestamp);
}

static sl_rail_ieee802154_phy_features_t desired_phy_features = SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS;

// Function to get human-readable name for sl_rail_util_radio_config_t values
static const char* get_phy_name(sl_rail_util_radio_config_t phy_id)
{
  switch (phy_id) {
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ:                    return "2P4_GHZ";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV:            return "2P4_GHZ_ANT_DIV";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_COEX:               return "2P4_GHZ_COEX";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_ANT_DIV_COEX:       return "2P4_GHZ_ANT_DIV_COEX";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM:                return "2P4_GHZ_FEM";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV:        return "2P4_GHZ_FEM_ANT_DIV";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_COEX:           return "2P4_GHZ_FEM_COEX";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_FEM_ANT_DIV_COEX:   return "2P4_GHZ_FEM_ANT_DIV_COEX";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_2_MBPS:             return "2P4_GHZ_2_MBPS";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_RX_CH_SWITCHING:    return "2P4_GHZ_RX_CH_SWITCHING";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_1_MBPS_FEC:         return "2P4_GHZ_1_MBPS_FEC";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_2_MBPS:         return "2P4_GHZ_FCS_2_MBPS";
    case SL_RAIL_IEEE802154_PHY_2P4_GHZ_FCS_1_MBPS_FEC:     return "2P4_GHZ_FCS_1_MBPS_FEC";
    case SL_RAIL_IEEE802154_PHY_863_MHZ_GB868:              return "863_MHZ_GB868";
    case SL_RAIL_IEEE802154_PHY_915_MHZ_GB868:              return "915_MHZ_GB868";
    default:                                                return NULL;
  }
}

sl_rail_ieee802154_phy_features_t sl_rail_util_ieee802154_get_high_speed_phy_features(void)
{
  return desired_phy_features;
}

void sl_high_datarate_phy_set_phy_command(sl_cli_command_arg_t *arguments)
{
  uint8_t switch_rate = sl_cli_get_argument_uint8(arguments, 0);
  if ( switch_rate == 0) {
    #ifdef SL_CATALOG_SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_PRESENT
    desired_phy_features = SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING;
    #else
    desired_phy_features = SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ;
    #endif
  } else if ( switch_rate == 1) {
    desired_phy_features = SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_1_MBPS_FEC;
  } else if (switch_rate == 2) {
    desired_phy_features = SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_2_MBPS;
  } else {
    sl_zigbee_app_debug_println("No HDR phy switch to the invalid value of %d Mps.", switch_rate);
  }
}

void sl_high_datarate_phy_get_phy_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_rail_util_radio_config_t active_phy = sl_rail_util_ieee802154_get_active_radio_config();
  const char* phy_name = get_phy_name(active_phy);

  if (phy_name != NULL) {
    sl_zigbee_app_debug_println("Active Radio PHY: %s", phy_name);
  } else {
    sl_zigbee_app_debug_println("Active Radio PHY: unknown PHY combination of (0x%02X)", active_phy);
  }
}

// permanent rx_fifo allocation
void sli_mac_hdr_phy_rx_fifo_initialize(void)
{
  #if defined(_SILICON_LABS_32B_SERIES_3)
  sli_phy_rx_fifo = sl_malloc(sli_mac_max_rx_flat_pkt_size << 1); // Allocate extra space for alignment
  if (sli_phy_rx_fifo == NULL) {
    // Handle memory allocation failure
    sl_zigbee_app_debug_println("Failed to allocate memory for HDR phy rx fifo");
    return;
  }
  #endif // defined(_SILICON_LABS_32B_SERIES_3)
}
