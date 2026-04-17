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

#ifndef SILABS_ZIGBEE_MAC_COMMAND_H
#define SILABS_ZIGBEE_MAC_COMMAND_H

// 15.4 command definitions
//
//

#include "multi-mac.h"
#include "mac-command.h"

// Association status responses.
typedef enum {
  EM_ASSOCIATION_SUCCESSFUL = 0x00,
  EM_ASSOCIATION_PAN_AT_CAPACITY = 0x01,
  EM_ASSOCIATION_PAN_ACCESS_DENIED = 0x02,
  EM_ASSOCIATION_CHANNEL_ACCESS_FAILURE = 0x80,
  EM_ASSOCIATION_NO_ACK = 0x81,
  EM_ASSOCIATION_NO_DATA = 0x82,
  EM_ASSOCIATION_UNAVAILABLE_KEY = 0x83,
  EM_ASSOCIATION_FAILED_SECURITY_CHECK = 0x84,
  EM_ASSOCIATION_INVALID_PARAMETER = 0x85
} sli_zigbee_association_status;

// Bits in the capability information byte of a MAC_ASSOCIATION_REQUEST
#define CAPABILITY_ALTERNATE_PAN_COORDINATOR    ((uint8_t)(BIT(0)))
#define CAPABILITY_DEVICE_TYPE                  ((uint8_t)(BIT(1)))
#define CAPABILITY_POWER_SOURCE                 ((uint8_t)(BIT(2)))
#define CAPABILITY_RECEIVER_ON_WHEN_IDLE        ((uint8_t)(BIT(3)))
// We are using this bit to store mac index in token to determine which
// interface child belongs to when MAC_DUAL_PRESENT
#define CAPABILITY_CHILD_MAC_INDEX              ((uint8_t)(BIT(4)))
// bits 4 and 5 are reserved
#define CAPABILITY_SECURITY_CAPABILITY_DEPRECATED          ((uint8_t)(BIT(6)))
#define CAPABILITY_ALLOCATE_ADDRESS             ((uint8_t)(BIT(7)))

#define MAC_SUPERFRAME_BEACON_ORDER_MASK  (0x000F)
#define MAC_SUPERFRAME_BEACON_ORDER_SHIFT (0)
#define MAC_SUPERFRAME_ORDER_MASK         (0x00F0)
#define MAC_SUPERFRAME_ORDER_SHIFT        (4)
#define MAC_SUPERFRAME_FINAL_CAP_MASK     (0x0F00)
#define MAC_SUPERFRAME_FINAL_CAP_SHIFT    (8)
#define MAC_SUPERFRAME_BATTERY_EXTENSION  (0x1000)
#define MAC_SUPERFRAME_PAN_COORD          (0x4000)
#define MAC_SUPERFRAME_ASSOC_PERMIT       (0x8000)

#define MAC_BEACON_GTS_PERMIT    (0x80)

#define sli_zigbee_has_router_capability(cap) (cap & CAPABILITY_DEVICE_TYPE)

// The number of symbols to wait for the data frame to arrive after receiving an
// ack to a data request with the frame pending bit set. In 802.15.4-2003, this
// is called aMaxFrameResponseTime and has a value of 1220 symbols (1220 x 16 us
// = 19.5 ms).
// We wait longer than this: 41.056 mS (320 uS per backoff period * 115
// maximum backoff periods + 133 maximum bytes * 32 uS per byte)
// 2566 * 16 us = 41056 us = 41.056 ms.
// If you change this be sure to update ZIG_ON_ETH_MAX_FRAME_RESPONSE_MULTIPLIER
// in phy/bridge/zigbee-bridge-ethernet.c as well.
#define MAX_FRAME_RESPONSE_TIME 2566
#define MAX_FRAME_RESPONSE_TIME_MS 41

// Using latest R22 1.0 parameters, I get 17212 symbols (172.12 ms) for GB868
// MAX_FRAME_RESPONSE_TIME -- a factor of 6.7 * the 2.4GHz's 2566 value we've
// been using for a long time.
#define MAX_FRAME_RESPONSE_TIME_SUBGHZ 17212
#define MAX_FRAME_RESPONSE_TIME_SUBGHZ_MS 172

sli_zigbee_packet_header_t sli_802154mac_make_message_with_info_element(uint8_t command,
                                                                        uint16_t macFrameControl,
                                                                        uint8_t *destination,
                                                                        uint16_t destinationPanId,
                                                                        uint16_t sourcePanId,
                                                                        uint8_t *frame,
                                                                        uint8_t frameLength,
                                                                        sli_buffer_manager_buffer_t payload,
                                                                        sli_buffer_manager_buffer_t infoElement);

sl_status_t sli_802154mac_send_message_with_info_element(uint8_t mac_index,
                                                         uint8_t command,
                                                         uint16_t macFrameControl,
                                                         uint8_t *destination,
                                                         uint16_t destinationPanId,
                                                         uint16_t sourcePanId,
                                                         uint8_t *frame,
                                                         uint8_t frameLength,
                                                         sli_buffer_manager_buffer_t payload,
                                                         sli_buffer_manager_buffer_t infoElement,
                                                         uint8_t priority);

sl_status_t sli_802154mac_send_message(uint8_t mac_index,
                                       uint8_t command,
                                       uint16_t macFrameControl,
                                       uint8_t *destination,
                                       uint16_t destinationPanId,
                                       uint16_t sourcePanId,
                                       uint8_t *frame,
                                       uint8_t frameLength,
                                       sli_buffer_manager_buffer_t payload,
                                       uint8_t priority);

sl_status_t sli_802154mac_send_orphan_notification(uint8_t mac_index);
sl_status_t sli_802154mac_send_coordinator_realignment(uint8_t mac_index,
                                                       sl_802154_long_addr_t longAddr,
                                                       sl_802154_short_addr_t shortAddr);

sl_status_t sli_802154mac_send_beacon_request(uint8_t mac_index);

sli_zigbee_packet_header_t sli_802154mac_make_message(uint8_t command,
                                                      uint16_t macFrameControl,
                                                      uint8_t *destination,
                                                      uint16_t destinationPanId,
                                                      uint16_t sourcePanId,
                                                      uint8_t *frame,
                                                      uint8_t frameLength,
                                                      sli_buffer_manager_buffer_t payload);

#if !(defined(SL_ZIGBEE_LEAF_STACK))
void sli_802154mac_set_permit_association(bool permitAssn);
#endif

void sli_802154mac_send_beacon(uint8_t mac_index);

sl_status_t sli_802154mac_associate_respond(uint8_t mac_index,
                                            sl_802154_long_addr_t eui64,
                                            sl_802154_short_addr_t shortAddr,
                                            uint8_t status);

sl_status_t sli_802154mac_associate_request(uint8_t mac_index,
                                            sl_802154_short_addr_t parentId,
                                            uint16_t panId,
                                            uint8_t capabilities);

uint16_t sli_zigbee_beacon_superframe(uint8_t mac_index);

#endif // SILABS_ZIGBEE_MAC_COMMAND_H
