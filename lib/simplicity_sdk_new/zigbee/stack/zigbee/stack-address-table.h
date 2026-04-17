/***************************************************************************//**
 * @file
 * @brief Address and multicast tables.
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

#ifndef SILABS_STACK_ADDRESS_TABLE_H
#define SILABS_STACK_ADDRESS_TABLE_H

// We use three bits for Multi-Network to address the network index.
// However, in the future we might extend this. To this purpose, if you need
// to add flags here, add them starting from the end of the byte, so that we
// leave room for potential increase of the number of supported network.
#define EM_TABLE_ENTRY_EXTENDED_TIMEOUT  BIT(0)
#define EM_TABLE_ENTRY_NETWORK_INDEX     (BIT(1) | BIT(2) | BIT(3))

#if defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
#define sli_zigbee_get_address_entry_network_index(flags) (0)
#else
#define sli_zigbee_get_address_entry_network_index(flags) \
  (((flags) & EM_TABLE_ENTRY_NETWORK_INDEX) >> 1)
#endif // defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)

extern uint8_t sli_zigbee_address_table_size;
extern sli_zigbee_address_table_entry_t sli_zigbee_address_table[];
#define addressTable (sli_zigbee_address_table)

// Sets all the short IDs in the table to SL_ZIGBEE_UNKNOWN_NODE_ID.
void sli_zigbee_initialize_address_table(void);

// Changes all instances of shortId to SL_ZIGBEE_UNKNOWN_NODE_ID.
void sli_zigbee_forget_short_id(sl_802154_short_addr_t shortId);

// Updating sets the short ID of the given long ID.
//
// When a short ID is found to be incorrect we update with
// SL_ZIGBEE_UNKNOWN_NODE_ID.
void sli_zigbee_update_address_table(sl_802154_long_addr_t longId, sl_802154_short_addr_t shortId);

// Looking up addresses
sl_802154_short_addr_t sli_zigbee_find_address_by_eui64(sl_802154_long_addr_t eui64);
bool sli_zigbee_find_address_by_node_id(sl_802154_short_addr_t nodeId,
                                        sl_802154_long_addr_t eui64Return);

// Checking if a remote device is known to be sleepy.
bool sli_zigbee_child_extended_timeout(uint8_t flags, sl_802154_long_addr_t longId);

// Multicast table.
bool sli_zigbee_am_multicast_member(sl_zigbee_multicast_id_t multicastId);

#endif // SILABS_STACK_ADDRESS_TABLE_H
