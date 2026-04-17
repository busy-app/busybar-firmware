/***************************************************************************//**
 * @file
 * @brief Splits long messages into smaller blocks for transmission and reassembles
 * received blocks.
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

/**
 * @defgroup fragmentation Fragmentation
 * @ingroup component
 * @brief API and Callbacks for the Fragmentation Component
 *
 * This component supports sending long packets through fragmentation and
 * reassembly.
 *
 */

/**
 * @addtogroup fragmentation
 * @{
 */
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  #include "app/framework/include/af.h"
#else
// Include events because we use the event type in a lot of .h files.
  #include "app/framework/common/zigbee_app_framework_event.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
#include "fragmentation-config.h"
#else
#include "config/fragmentation-config.h"
#endif

#ifndef ZIGBEE_APSC_MAX_TRANSMIT_RETRIES
#define ZIGBEE_APSC_MAX_TRANSMIT_RETRIES 3
#endif //ZIGBEE_APSC_MAX_TRANSMIT_RETRIES

#ifndef SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE
#define SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE 1
#endif //SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE

//------------------------------------------------------------------------------
// Sending
#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef struct {
  sl_zigbee_outgoing_message_type_t  messageType;
  uint16_t                    indexOrDestination;
  uint8_t                     sequence;
  sl_zigbee_aps_frame_t             apsFrame;
  uint8_t*                    bufferPtr;
  uint16_t                    bufLen;
  uint8_t                     fragmentLen;
  uint8_t                     fragmentCount;
  uint8_t                     fragmentBase;
  uint8_t                     fragmentsInTransit;
}sli_zigbee_tx_fragmented_packet_t;

sl_status_t sli_zigbee_af_fragmentation_send_unicast(sl_zigbee_outgoing_message_type_t type,
                                                     uint16_t indexOrDestination,
                                                     sl_zigbee_aps_frame_t *apsFrame,
                                                     uint8_t *buffer,
                                                     uint16_t bufLen,
                                                     uint16_t *messageTag);

bool sli_zigbee_af_fragmentation_message_sent(sl_zigbee_aps_frame_t *apsFrame,
                                              sl_status_t status);

void sli_zigbee_af_fragmentation_message_sent_handler(sl_status_t status,
                                                      sl_zigbee_outgoing_message_type_t type,
                                                      uint16_t indexOrDestination,
                                                      sl_zigbee_aps_frame_t *apsFrame,
                                                      uint16_t messageTag,
                                                      uint8_t *buffer,
                                                      uint16_t bufLen);

#endif //DOXYGEN_SHOULD_SKIP_THIS

//------------------------------------------------------------------------------
// Receiving.
#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef enum {
  SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE         = 0,
  SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED             = 1,
  SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE            = 2,
  SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE = 3
}sli_zigbee_rx_packet_status_t;

typedef struct {
  sli_zigbee_rx_packet_status_t status;
  uint8_t       ackedPacketAge;
  uint8_t       buffer[SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE];
  sl_802154_short_addr_t fragmentSource;
  uint8_t       fragmentSequenceNumber;
  uint8_t       fragmentBase; // first fragment inside the rx window.
  uint16_t      windowFinger; //points to the first byte inside the rx window.
  uint8_t       fragmentsExpected; // total number of fragments expected.
  uint8_t       fragmentsReceived; // fragments received so far.
  uint8_t       fragmentMask; // bitmask of received fragments inside the rx window.
  uint8_t       lastfragmentLen; // Length of the last fragment.
  uint8_t       fragmentLen; // Length of the fragment inside the rx window.
                             // All the fragments inside the rx window should have
                             // the same length.
  sl_zigbee_af_event_t fragmentEventControl;
}sli_zigbee_rx_fragmented_packet_t;

bool sli_zigbee_af_fragmentation_incoming_message(sl_zigbee_incoming_message_type_t type,
                                                  sl_zigbee_aps_frame_t *apsFrame,
                                                  sl_802154_short_addr_t sender,
                                                  uint8_t **buffer,
                                                  uint16_t *bufLen);

void sli_zigbee_af_fragmentation_abort_reception(sl_zigbee_af_event_t* control);

extern uint8_t  sl_zigbee_fragment_window_size;

extern uint16_t sl_zigbee_mac_indirect_timeout;
#if defined(EZSP_HOST)
extern uint16_t sl_zigbee_aps_ack_timeout_ms;
#endif

void sli_zigbee_af_fragmentation_platform_init_callback(void);

sl_status_t sli_zigbee_af_fragmentation_send(sli_zigbee_tx_fragmented_packet_t* txPacket,
                                             uint8_t fragmentNumber,
                                             uint8_t fragmentLen,
                                             uint16_t offset);

void sli_zigbee_af_fragmentation_handle_source_route(sli_zigbee_tx_fragmented_packet_t* txPacket,
                                                     uint16_t indexOrDestination);

void sli_zigbee_af_fragmentation_send_reply(sl_802154_short_addr_t sender,
                                            sl_zigbee_aps_frame_t* apsFrame,
                                            sli_zigbee_rx_fragmented_packet_t* rxPacket);

#if defined(SL_ZIGBEE_TEST) || defined(SL_ZIGBEE_GOLDEN_UNIT)
extern uint8_t sli_zigbee_af_fragmentation_artificially_drop_block_number;
#endif

#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // end of fragmentation
