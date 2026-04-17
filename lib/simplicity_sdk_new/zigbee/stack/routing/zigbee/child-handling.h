/***************************************************************************//**
 * @file
 * @brief Code for configuring end device timeouts
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

#ifndef CHILD_HANDLING_H
#define CHILD_HANDLING_H

#define POWER_NEGOTIATION_SUPPORTED    0x04
#define PARENT_MAC_INDEX               0x8000

// PARENT_MAC_INDEX is to store parent's mac index, it is needed when
// a node rejoining a network which was associated over alternate MAC interface.
// This is to avoid breaking tree network by rejoining on wrong interface.
// We would not require this bit once we have tc connectivity bit as a part of spec.

// Most significant (i.e. 15th) bit is for parent mac index.
#define ZIGBEE_PARENT_NETWORK_INFORMATION_MASK 0x7FFF

#define ZIGBEE_TELL_PARENT_TO_PERSIST_MY_CHILD_DATA 0x01

//We use the top bit of the sli_zigbee_end_device_timeout byte per child to record keep alives received.
#define ZIGBEE_END_DEVICE_KEEP_ALIVE_RECEIVED 0xF0
//----------------------------------------------------------------
//Macros to manipulate the parentNwkInformation variable.
//Outside of child-handling.c, these are used in pro-compliance.c to test
//different use cases.
#define sli_zigbee_set_parent_information(parentInformation, options) \
  ((parentInformation) |= (options))
#define sli_zigbee_disable_current_keep_alive_mode(parentInformation) \
  ((parentInformation) &= ~SL_ZIGBEE_KEEP_ALIVE_SUPPORT_ALL)
#define sli_zigbee_power_negotiation_not_supported(parentInformation) \
  ((parentInformation) &= ~POWER_NEGOTIATION_SUPPORTED)
#define sli_zigbee_check_parent_information(parentInformation, options) \
  ((parentInformation) & (options))
#define sli_zigbee_check_mac_data_poll_keep_alive_support(parentInformation) \
  sli_zigbee_check_parent_information(parentInformation, SL_802154_DATA_POLL_KEEP_ALIVE)
#define sli_zigbee_check_end_device_timeout_keep_alive_support(parentInformation) \
  sli_zigbee_check_parent_information(parentInformation, SL_ZIGBEE_END_DEVICE_TIMEOUT_KEEP_ALIVE)
#define sli_zigbee_check_power_negotiation_support(parentInformation) \
  sli_zigbee_check_parent_information(parentInformation, POWER_NEGOTIATION_SUPPORTED)
#define sli_zigbee_set_parent_mac_index(parentInformation) \
  sli_zigbee_set_parent_information(parentInformation, PARENT_MAC_INDEX)
#define sli_zigbee_clear_parent_mac_index(parentInformation) \
  ((parentInformation) &= ~PARENT_MAC_INDEX)
#define sli_zigbee_check_parent_mac_index(parentInformation) \
  sli_zigbee_check_parent_information(parentInformation, PARENT_MAC_INDEX)

//----------------------------------------------------------------

//This function is called each time a sli_zigbee_network_up() is reached in association.c.
//It checks if the feature is enabled and then if the device is an end device or
//a sleepy end device. Returns false if either condition is not met.
void sli_zigbee_send_timeout_request(void);

// Sends a network timeout request to the parent device
void sli_zigbee_send_network_timeout_request(uint8_t requestedTimeoutValue);

//Called from network.c when the zigbee command header matches
//ZIGBEE_NETWORK_TIMEOUT_REQUEST. Calls sli_zigbee_set_end_device_timeout to process
//end device timeout and then sends a response using sli_zigbee_send_network_timeout_response.
void sli_zigbee_process_network_timeout_request(sli_zigbee_packet_header_t header);

//It sets the corresponding index in enEndDeviceTimeout and writes it to the token
//if the persisent bit is set. Returns ZIGBEE_TIMEOUT_REQUEST_SUCCESS if a valid
//child and enumeration value were found. Returns ZIGBEE_TIMEOUT_REQUEST_INVALID_VALUE
//otherwise.
uint8_t sli_zigbee_set_end_device_timeout(sl_802154_short_addr_t sender, uint8_t timeoutValue, uint8_t endDeviceConfiguration);
//Sends a Network Timeout Response. The Zigbee command header is
//ZIGBEE_NETWORK_TIMEOUT_RESPONSE. It sends 2 bytes, one byte that represents whether
//the request was successful(either holds ZIGBEE_TIMEOUT_REQUEST_SUCCESS or
//ZIGBEE_TIMEOUT_REQUEST_INVALID_VALUE).The other byte holds sli_zigbee_what_i_support_as_a_parent
//which represents which Keep Alive mechanism the parent supports. See enumeraton above
//for possible values.
void sli_zigbee_send_network_timeout_response(uint8_t endDeviceTimeoutRequestStatus, sl_802154_short_addr_t sender);
void sli_zigbee_process_network_timeout_response(sli_zigbee_packet_header_t header);

void sli_zigbee_schedule_next_network_timeout_event(void);
void initializeParentNwkInformation(void);
void sli_zigbee_note_successful_coordinator_realignment(void);

//Support functions for sli_zigbee_what_i_support_as_a_parent.
bool sli_zigbee_set_parent_keep_alive_support_unknown(void);
bool sli_zigbee_is_parent_keep_alive_support_known(void);
uint16_t sli_zigbee_get_parent_nwk_information(void);

//Ram array for saving end device enumeration. For each child it will hold
//one value from 0 to 14. The actual value in seconds is the timeoutOptions[value]
//which is defined in child-handling.c
extern uint8_t sli_zigbee_end_device_timeout_data[];

extern uint16_t sli_zigbee_what_i_support_as_a_parent;

extern sli_zigbee_event_t sli_zigbee_network_timeout_request_event;
//This is a constant array that records the different values end devices
//can choose to have in seconds.
extern const uint32_t timeoutOptions[];

void sli_zigbee_process_link_power_delta(sli_zigbee_packet_header_t header);
sl_status_t sli_zigbee_send_link_power_delta(uint8_t options, sl_802154_short_addr_t sender, uint8_t delta);

// Setters and getters for new power calculated using link power delta command.
//This is intended use on end device only.
int8_t sli_zigbee_get_new_power_calculated_using_lpd(void);
void sli_zigbee_set_new_power_calculated_using_lpd(int8_t power);
void sli_zigbee_clear_negotiated_power_by_end_device(void);

#endif // CHILD_HANDLING_H
