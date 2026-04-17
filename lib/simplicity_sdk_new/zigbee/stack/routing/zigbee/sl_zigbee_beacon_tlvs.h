/***************************************************************************//**
 * @file beacon-tlvs.h
 * @brief This file contains code for constructing and parsing TLVs present in
 *  beacons.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SILABS_BEACON_TLVS_H
#define SILABS_BEACON_TLVS_H
#include "core/sl_zigbee_stack.h"
#include "core/sl_zigbee_multi_network.h"
#include "internal/inc/internal-defs-patch.h"
#include "child-handling.h"

//-----------------------------------------------------------------------------
// Externs
#include "include/library.h"
extern const sl_zigbee_library_status_t sli_zb_r23_support_library_status;

//-----------------------------------------------------------------------------
// Macros

// Bits in byte 0 of Router Information TLV. Bytes 1-3 are reserved
#define HUB_CONNECTIVITY_BIT            0x01
#define UPTIME_BIT                      0x02
// NOTE this allows for a device to indicate whether or not it is ready
// and able to support new children
#define PREFER_PARENT_BIT               0x04
#define BATTERY_BACKUP_BIT              0x08
#define ENHANCED_BEACON_REQUEST_BIT     0x10
#define MAC_DATA_POLL_KEEPALIVE_BIT     0x20
#define END_DEVICE_KEEPALIVE_BIT        0x40
#define POWER_NEGOTIATION_BIT           0x80

#define macDataPollKeepaliveSupport \
  sli_zigbee_check_mac_data_poll_keep_alive_support(sli_zigbee_what_i_support_as_a_parent)
#define endDeviceKeepaliveSupport \
  sli_zigbee_check_end_device_timeout_keep_alive_support(sli_zigbee_what_i_support_as_a_parent)
#define powerNegotiationSupport \
  sli_zigbee_check_power_negotiation_support(sli_zigbee_what_i_support_as_a_parent)
#define enhancedBeaconSupport \
  (sli_zigbee_stack_get_library_status(SL_ZIGBEE_ENHANCED_BEACON_REQUEST_LIBRARY_ID) & SL_ZIGBEE_LIBRARY_PRESENT_MASK)

//-----------------------------------------------------------------------------
// Internal (sli_zigbee prefixed) APIs

/**
 * @brief take a beacon header and append the stack beacon tlvs to it
 * @param[in|out] beacon
 */
// TODO make this sl_status_t / sl_status
bool sli_zigbee_append_beacon_tlvs(sli_zigbee_packet_header_t* beacon);
/**
 * @brief take a beacon and parse payload data from tlvs
 */
void sli_zigbee_parse_beacon_tlvs(sli_zigbee_packet_header_t beacon,
                                  uint8_t tlvPayloadStartIndex,
                                  sl_zigbee_beacon_data_t* beaconData);
/**
 * @brief adds the beacon appendix encapsulation tlvs
 */
// TODO where is this used?
uint8_t sli_zigbee_append_beacon_appendix_with_encapsulated_tlvs(uint8_t* payload, uint8_t length);
/**
 * @brief marker function for the cached beacon appendix
 */
void sli_zigbee_stack_mark_beacon_appendix_tlv_buffer(void);
/**
 * @brief configuration getter used to populate beacon appendix
 */
uint16_t sli_zigbee_stack_get_router_info_bitmask(void);
/**
 * @brief given a buffer containing permit joining request
 * parse and cache the beacon appendix tlvs
 */
void sli_zigbee_stack_parse_permit_joining_beacon_appendix(uint8_t *payload,
                                                           uint8_t payload_len);

/**
 * @brief set whether to advertise the "prefer parent" bit
 * @param[in] the boolean value corresponding to the current state
 */
void sli_zigbee_stack_set_parent_preference(bool preferParent);
/**
 * @brief return the current status of the "prefer parent" bit
 */
bool sli_zigbee_stack_is_parent_preferred(void);
bool sli_zigbee_prioritize_on_parent_selection(sl_zigbee_beacon_data_t storedBeacon, sl_zigbee_beacon_data_t rxBeacon);
void sl_disable_beacon_tlvs(bool disable);
#endif // SILABS_BEACON_TLVS_H
