/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef ENHANCED_BEACON_REQUEST_H
#define ENHANCED_BEACON_REQUEST_H

#include "mac/multi-mac.h"
#include "stack/core/sl_zigbee_multi_phy.h"
#include "mac/mac-info-element-parsing.h"

extern const sl_zigbee_library_status_t sli_zigbee_enhanced_beacon_request_library_status;

sl_status_t sli_zigbee_send_enhanced_beacon_request(uint8_t mac_index, bool firstTimeJoin);

bool sli_zigbee_process_enhanced_beacon_request(sli_zigbee_packet_header_t header,
                                                sli_802154mac_frame_info_element_parse_result result,
                                                sli_802154mac_info_element_field* infoElementsArray);

void sli_zigbee_join_list_add(sl_802154_long_addr_t longId);
void sli_zigbee_join_list_delete(sl_802154_long_addr_t longId);
void sli_zigbee_join_list_clear(void);
sli_buffer_manager_buffer_t sli_zigbee_get_join_list_pointer(void);

#define LPD_DEBUG(x)  //x

// This value is used to initialize sli_zigbee_child_power table and later check against to return
// MAX_RADIO_POWER_USER_PROVIDED if there is a child which does not have an entry in
// sli_zigbee_child_power table. We may come across this scenario when coordinator power cycle
// for some reason as sli_zigbee_child_power is not persistent.
#define INVALID_RADIO_POWER_VALUE   (-128)

#define POWER_CONTROL_TARGET_RSSI (-77) //-79 as per specs
#define MIN_POWER_CONTROL_TX_POWER (-15)

void sli_zigbee_remove_ebr_power_and_eui64_entry(sl_802154_long_addr_t eui64);
uint8_t sli_zigbee_get_ebr_power_and_eui_list_pointer(uint8_t **buffer);
int8_t sli_zigbee_find_ebr_power_by_eui64(sl_802154_long_addr_t eui64);
bool sli_zigbee_add_ebr_eui_and_power_entry(sl_802154_long_addr_t eui64, int8_t newPower);

void sli_802154mac_send_enhanced_beacon(uint8_t mac_index);

#endif // ENHANCED_BEACON_REQUEST_H
