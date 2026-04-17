/***************************************************************************//**
 * @file
 * @brief Network report and update commands.
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

#ifndef MANAGEMENT_H
#define MANAGEMENT_H

void sli_zigbee_network_management_init(void);
sl_802154_short_addr_t sli_zigbee_get_network_management_state(uint8_t *nwkUpdateIdLoc,
                                                               uint32_t *activeChannelsLoc);
#define sli_zigbee_get_network_manager_id() (sli_zigbee_get_network_management_state(NULL, NULL))

uint8_t sli_zigbee_stack_get_nwk_update_id(void);
uint8_t sli_zigbee_set_nwk_update_id(uint8_t id);
#define sli_zigbee_increment_nwk_update_id() \
  (sli_zigbee_set_nwk_update_id(sli_zigbee_stack_get_nwk_update_id() + 1))

void sli_zigbee_set_network_management_state(uint8_t newNwkUpdateId,
                                             sl_802154_short_addr_t newNetworkManager,
                                             uint32_t newActiveChannels);
void sli_zigbee_process_management_command(sli_zigbee_packet_header_t header);
void sli_zigbee_management_event_handler(void);
void sli_zigbee_send_pan_id_conflict_report(void);

bool sli_zigbee_am_network_manager(void);
void sli_zigbee_decrement_pan_conflict_meter(void);
void slxi_zigbee_stack_change_pan_id_now(sl_802154_pan_id_t panId);

// Exported for internal testing
bool sli_zigbee_send_report_or_update(uint8_t command,
                                      uint8_t updateId,
                                      uint16_t panId);

/**
 * @brief get the number of pan id conflicts detected
 */
uint16_t sli_zigbee_get_pan_id_conflict_count(void);

/**
 * @brief get the number of seconds since the last count reset
 */
uint16_t sli_zigbee_get_pan_id_conflict_last_reset_seconds(void);

/**
 * @brief resets the count and reset timer for pan id conflicts
 */
void sli_zigbee_reset_pan_id_conflict_count(void);

/**
 * @brief enable or disable the pan id conflict reports
 * returns boolean - success or failure to enable network reporting based on network index.
 */

void sli_zigbee_stack_set_pan_id_conflict_report(boolean set_value, uint8_t nwk_index);
#endif // MANAGEMENT_H
