/***************************************************************************//**
 * @file sl_zigbee_mac_app_events.h
 * @brief EmberZNet API for receiving MAC layer events.
 * See @ref mac_layer for documentation.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <inttypes.h>
#include "sl_status.h"
#include "sl_zigbee_types.h"

/** @brief handle incoming updates about the status of an ongoing communication
 * @param mac_index The index of the MAC layer entity that is the source of the status update
 * @param status The status of the communication channel
 * @param packet_length The length of the packet data
 * @param packet_data The packet data associated with the update
 *
 * @internal SL_ZIGBEE_IPC_ARGS
 * {# packet_data | length: packet_length | max: MAX_IPC_VEC_ARG_CAPACITY #}
 */
void sl_802154mac_communication_status_indication_handler(uint8_t mac_index,
                                                          sl_status_t status,
                                                          uint8_t packet_length,
                                                          uint8_t *packet_data);
