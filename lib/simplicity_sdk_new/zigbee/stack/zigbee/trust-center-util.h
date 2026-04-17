/***************************************************************************//**
 * @file
 * @brief Security code that is of use only on trust centers.
 * Devices that will never be trust centers can use stubs in place
 * of this file.
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

#ifndef TRUST_CENTER_UTIL_H
#define TRUST_CENTER_UTIL_H

void sli_zigbee_set_trust_center_aps_encryption(sl_zigbee_tc_aps_encrypt_mode_t option);

extern const sl_zigbee_library_status_t sli_zigbee_security_core_library_status;

void sli_zigbee_device_joining_security_handler(sl_zigbee_device_update_t status,
                                                sl_802154_short_addr_t parentShortAddress,
                                                sl_802154_short_addr_t childShortAddress,
                                                uint8_t* childLongAddress,
                                                void*    joinerEncaptlv);

sl_status_t sli_zigbee_remove_device(sl_802154_long_addr_t device);

bool sli_zigbee_send_network_key(sl_802154_short_addr_t destinationShortId,
                                 sl_802154_long_addr_t destinationLongId,
                                 sli_zigbee_send_aps_command_options_t options);

void sli_zigbee_process_request_key(sl_802154_short_addr_t source,
                                    uint8_t* frame,
                                    sli_zigbee_send_aps_command_options_t packetSecurity);

void sli_zigbee_trust_center_request_key_cleanup(void);

bool sli_zigbee_send_link_key(sl_802154_short_addr_t targetNodeId,
                              sl_802154_long_addr_t targetEui64,
                              sli_zigbee_key_type_t keyType,
                              sl_zigbee_key_data_t* key,
                              bool useApsEncryption);

bool sli_zigbee_add_tunnelled_aps_header(sli_zigbee_packet_header_t *header,
                                         sl_802154_long_addr_t longDestination);

#endif // TRUST_CENTER_UTIL_H
