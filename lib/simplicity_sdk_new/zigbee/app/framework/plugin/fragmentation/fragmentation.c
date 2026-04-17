/***************************************************************************//**
 * @file
 * @brief Splits long messages into smaller fragments for transmission and
 * reassembles received fragments into full messages.
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

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
    #include "app/framework/include/af.h"
    #include "app/framework/util/util.h"
#else
    #include "sl_zigbee_types.h"
    #include "stack/include/message.h"
    #include "app/framework/common/zigbee_app_framework_event.h"
    #include "app/framework/common/sl_zigbee_system_common.h"
extern uint8_t sl_zigbee_af_maximum_aps_payload_length(sl_zigbee_outgoing_message_type_t type,
                                                       uint16_t indexOrDestination,
                                                       sl_zigbee_aps_frame_t *apsFrame);
#ifndef EZSP_HOST
    #include "stack-info.h"
#endif // EZSP_HOST
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "fragmentation.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "app/framework/plugin/debug-print/sl_zigbee_debug_print.h"
#endif //SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#if (SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK == 1)
#define FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK
#endif

//------------------------------------------------------------------------------
// Globals

#if defined(FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK)
#define FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK_BOOLEAN true
#else
#define FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK_BOOLEAN false
#endif

static const bool freeOutgoingMessagePriorToFinalAck = FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK_BOOLEAN;

#define UNUSED_TX_PACKET_ENTRY 0xFF

//------------------------------------------------------------------------------
// Forward Declarations

static sl_status_t sendNextFragments(sli_zigbee_tx_fragmented_packet_t* txPacket);
static void abortTransmission(sli_zigbee_tx_fragmented_packet_t *txPacket, sl_status_t status);
static sli_zigbee_tx_fragmented_packet_t* getFreeTxPacketEntry(void);
static sli_zigbee_tx_fragmented_packet_t* txPacketLookUp(sl_zigbee_aps_frame_t *apsFrame);
static void updateCurrentAppTasksForFragmentationState(void);

// We separate the outgoing buffer from the txPacket entry to allow us to keep around
// data about previous fragmented messages that have sent their last packet
// but not yet been acknowledged.  This saves space by not replicating the entire
// buffer required to store the outgoing message.  However, in that case we do not
// pass the complete message to the message sent handler.
static sli_zigbee_tx_fragmented_packet_t txPackets[SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS];
static uint8_t txMessageBuffers[SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS]
[SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE];

static sli_zigbee_tx_fragmented_packet_t txPacketAwaitingFinalAck = {
  .messageType = UNUSED_TX_PACKET_ENTRY
};

#if defined(SL_ZIGBEE_TEST) || defined(SL_ZIGBEE_GOLDEN_UNIT)
  #define NO_BLOCK_TO_DROP 0xFF
uint8_t sli_zigbee_af_fragmentation_artificially_drop_block_number = NO_BLOCK_TO_DROP;
  #define artificiallyDropBlock(block) (block == sli_zigbee_af_fragmentation_artificially_drop_block_number)
  #define clearArtificiallyDropBlock() sli_zigbee_af_fragmentation_artificially_drop_block_number = NO_BLOCK_TO_DROP;
  #define artificiallyDropBlockPrintln(format, arg) sl_zigbee_legacy_af_debug_print((format), (arg))

#else
  #define artificiallyDropBlock(block) false
  #define clearArtificiallyDropBlock()
  #define artificiallyDropBlockPrintln(format, arg)

#endif

#define messageTag(txPacket) ((txPacket)->apsFrame.sequence)

extern uint16_t sl_zigbee_get_aps_ack_timeout_ms(void);
//------------------------------------------------------------------------------
// Functions

sl_status_t sli_zigbee_af_fragmentation_send_unicast(sl_zigbee_outgoing_message_type_t type,
                                                     uint16_t indexOrDestination,
                                                     sl_zigbee_aps_frame_t *apsFrame,
                                                     uint8_t *buffer,
                                                     uint16_t bufLen,
                                                     uint16_t *messageTag)
{
  sl_status_t status;
  uint16_t fragments;
  sli_zigbee_tx_fragmented_packet_t* txPacket;

  if (sl_zigbee_fragment_window_size == 0) {
    return SL_STATUS_INVALID_STATE;
  }

  if (bufLen > SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE) {
    return SL_STATUS_MESSAGE_TOO_LONG;
  }

  txPacket = getFreeTxPacketEntry();
  if (txPacket == NULL) {
    return SL_STATUS_ZIGBEE_MAX_MESSAGE_LIMIT_REACHED;
  }

  txPacket->messageType = type;
  txPacket->indexOrDestination = indexOrDestination;
  memmove(&txPacket->apsFrame, apsFrame, sizeof(sl_zigbee_aps_frame_t));
  txPacket->apsFrame.options |=
    (SL_ZIGBEE_APS_OPTION_FRAGMENT | SL_ZIGBEE_APS_OPTION_RETRY);

  sli_zigbee_af_fragmentation_handle_source_route(txPacket,
                                                  indexOrDestination);

  memmove(txPacket->bufferPtr, buffer, bufLen);
  txPacket->bufLen = bufLen;
  txPacket->fragmentLen = sl_zigbee_af_maximum_aps_payload_length(type,
                                                                  indexOrDestination,
                                                                  &txPacket->apsFrame);
  if (txPacket->fragmentLen == 0) {
    return SL_STATUS_MESSAGE_TOO_LONG;
  }
  fragments = ((bufLen + txPacket->fragmentLen - 1) / txPacket->fragmentLen);
  if (fragments > MAX_INT8U_VALUE) {
    return SL_STATUS_MESSAGE_TOO_LONG;
  }

  txPacket->fragmentCount = (uint8_t)fragments;
  txPacket->fragmentBase = 0;
  txPacket->fragmentsInTransit = 0;

  status = sendNextFragments(txPacket);

  if (status == SL_STATUS_OK) {
    // Set the APS sequence number in the passed apsFrame.
    apsFrame->sequence = txPacket->sequence;
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
    sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_FRAGMENTATION_IN_PROGRESS);
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  } else {
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }

  *messageTag = messageTag(txPacket);

  return status;
}

bool sli_zigbee_af_fragmentation_message_sent(sl_zigbee_aps_frame_t *apsFrame,
                                              sl_status_t status)
{
  if (apsFrame->options & SL_ZIGBEE_APS_OPTION_FRAGMENT) {
    // If the outgoing APS frame is fragmented, we should always have a
    // a corresponding record in the sli_zigbee_tx_fragmented_packet_t array.
    sli_zigbee_tx_fragmented_packet_t *txPacket = txPacketLookUp(apsFrame);
    if (txPacket == NULL) {
      return true;
    }

    if (status == SL_STATUS_OK) {
      txPacket->fragmentsInTransit--;
      if (txPacket->fragmentsInTransit == 0) {
        txPacket->fragmentBase += sl_zigbee_fragment_window_size;
        abortTransmission(txPacket, sendNextFragments(txPacket));
      }
    } else {
      abortTransmission(txPacket, status);
    }
    updateCurrentAppTasksForFragmentationState();
    return true;
  } else {
    return false;
  }
}

static sl_status_t sendNextFragments(sli_zigbee_tx_fragmented_packet_t* txPacket)
{
  uint8_t i;
  uint16_t offset;

  sl_zigbee_legacy_af_debug_print("Sending fragment %d of %d",
                                  txPacket->fragmentBase,
                                  txPacket->fragmentCount);

  offset = txPacket->fragmentBase * txPacket->fragmentLen;

  // Send fragments until the window is full.
  for (i = txPacket->fragmentBase;
       i < txPacket->fragmentBase + sl_zigbee_fragment_window_size
       && i < txPacket->fragmentCount;
       i++) {
    sl_status_t status;

    // For a message requiring n fragments, the length of each of the first
    // n - 1 fragments is the maximum fragment size.  The length of the last
    // fragment is whatever is leftover.
    uint8_t fragmentLen = (offset + txPacket->fragmentLen < txPacket->bufLen
                           ? txPacket->fragmentLen
                           : txPacket->bufLen - offset);

    txPacket->apsFrame.groupId = HIGH_LOW_TO_INT(txPacket->fragmentCount, i);

    status = sli_zigbee_af_fragmentation_send(txPacket,
                                              i,
                                              fragmentLen,
                                              offset);
    if (status != SL_STATUS_OK) {
      return status;
    }

    txPacket->fragmentsInTransit++;
    offset += fragmentLen;
  } // close inner for

  if (txPacket->fragmentsInTransit == 0) {
    sli_zigbee_af_fragmentation_message_sent_handler(SL_STATUS_OK,
                                                     txPacket->messageType,
                                                     txPacket->indexOrDestination,
                                                     &txPacket->apsFrame,
                                                     messageTag(txPacket),
                                                     txPacket->bufferPtr,
                                                     txPacket->bufLen);
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  } else if (freeOutgoingMessagePriorToFinalAck
             && txPacket->bufferPtr != NULL
             && offset >= txPacket->bufLen
             && sl_zigbee_fragment_window_size == 1
             && txPacketAwaitingFinalAck.messageType == UNUSED_TX_PACKET_ENTRY) {
    // Awaiting final fragment
    memmove(&txPacketAwaitingFinalAck, txPacket, sizeof(sli_zigbee_tx_fragmented_packet_t));
    txPacketAwaitingFinalAck.bufferPtr = NULL;
    txPacketAwaitingFinalAck.bufLen = 0;
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }

  return SL_STATUS_OK;
}

static void abortTransmission(sli_zigbee_tx_fragmented_packet_t *txPacket,
                              sl_status_t status)
{
  if (status != SL_STATUS_OK && txPacket->messageType != UNUSED_TX_PACKET_ENTRY) {
    sli_zigbee_af_fragmentation_message_sent_handler(status,
                                                     txPacket->messageType,
                                                     txPacket->indexOrDestination,
                                                     &txPacket->apsFrame,
                                                     messageTag(txPacket),
                                                     txPacket->bufferPtr,
                                                     txPacket->bufLen);
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }
}

static sli_zigbee_tx_fragmented_packet_t* getFreeTxPacketEntry(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    sli_zigbee_tx_fragmented_packet_t *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      txPacket->bufferPtr = txMessageBuffers[i];
      return txPacket;
    }
  }
  return NULL;
}

static sli_zigbee_tx_fragmented_packet_t* txPacketLookUp(sl_zigbee_aps_frame_t *apsFrame)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    sli_zigbee_tx_fragmented_packet_t *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      continue;
    }

    // Each node has a single source APS counter.
    if (apsFrame->sequence == txPacket->apsFrame.sequence) {
      return txPacket;
    }
  }

  if (txPacketAwaitingFinalAck.messageType != UNUSED_TX_PACKET_ENTRY
      && apsFrame->sequence == txPacketAwaitingFinalAck.apsFrame.sequence) {
    return &txPacketAwaitingFinalAck;
  }
  return NULL;
}

//------------------------------------------------------------------------------
// Receiving.

#define lowBitMask(n) ((1u << (n)) - 1)
static void setFragmentMask(sli_zigbee_rx_fragmented_packet_t *rxPacket);
static bool storeRxFragment(sli_zigbee_rx_fragmented_packet_t *rxPacket,
                            uint8_t fragment,
                            uint8_t *buffer,
                            uint16_t bufLen);
static void moveRxWindow(sli_zigbee_rx_fragmented_packet_t *rxPacket);
static sli_zigbee_rx_fragmented_packet_t* getFreeRxPacketEntry(void);
static sli_zigbee_rx_fragmented_packet_t* rxPacketLookUp(sl_zigbee_aps_frame_t *apsFrame,
                                                         sl_802154_short_addr_t sender);

static sli_zigbee_rx_fragmented_packet_t rxPackets[SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS];

static void ageAllAckedRxPackets(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    if (rxPackets[i].status == SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED) {
      rxPackets[i].ackedPacketAge++;
    }
  }
}

static uint16_t retryTimeoutMs(sl_802154_short_addr_t nodeId)
{
  sl_802154_long_addr_t eui64;
  uint16_t retryTimeoutMs = sl_zigbee_get_aps_ack_timeout_ms();

#if defined(SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT)
  sl_zigbee_node_type_t nodeType;
  sl_status_t status = SL_STATUS_OK;
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  sl_zigbee_network_parameters_t networkParameters;
  status = sl_zigbee_get_network_parameters(&nodeType, &networkParameters);
#else
  nodeType = sli_zigbee_af_current_zigbee_pro_network->nodeType;
#endif
  if (SL_STATUS_OK == status && SL_ZIGBEE_SLEEPY_END_DEVICE <= nodeType) {
    retryTimeoutMs += sl_zigbee_mac_indirect_timeout;
  }
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

  if (sl_zigbee_lookup_eui64_by_node_id(nodeId, eui64) == SL_STATUS_OK
      && (sl_zigbee_get_extended_timeout(eui64) == SL_STATUS_OK)) {
    retryTimeoutMs += sl_zigbee_mac_indirect_timeout;
  }
  return retryTimeoutMs;
}

bool sli_zigbee_af_fragmentation_incoming_message(sl_zigbee_incoming_message_type_t type,
                                                  sl_zigbee_aps_frame_t *apsFrame,
                                                  sl_802154_short_addr_t sender,
                                                  uint8_t **buffer,
                                                  uint16_t *bufLen)
{
  (void)type;
  static bool rxWindowMoved = false;
  bool newFragment;
  uint8_t fragment;
  uint8_t mask;
  sli_zigbee_rx_fragmented_packet_t *rxPacket;
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  sl_zigbee_af_cluster_command_t cmd;
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

  if (!(apsFrame->options & SL_ZIGBEE_APS_OPTION_FRAGMENT)) {
    return false;
  }

  assert(*bufLen <= MAX_INT8U_VALUE);

  rxPacket = rxPacketLookUp(apsFrame, sender);
  fragment = LOW_BYTE(apsFrame->groupId);

  if (artificiallyDropBlock(fragment)) {
    artificiallyDropBlockPrintln("Artificially dropping block %d", fragment);
    clearArtificiallyDropBlock();
    if (sl_zigbee_fragment_window_size == 1) {
      // Create a fake rxPacket structure to send proper enhanced APS ACK
      sli_zigbee_rx_fragmented_packet_t dropResponse;
      dropResponse.fragmentMask = 0xFE;  // Indicate dropped fragment
      dropResponse.fragmentBase = fragment;
      sli_zigbee_af_fragmentation_send_reply(sender, apsFrame, &dropResponse);
    }

    return true;
  }

  // First fragment for this packet, we need to set up a new entry.
  if (rxPacket == NULL) {
    rxPacket = getFreeRxPacketEntry();
    if (rxPacket == NULL || fragment >= sl_zigbee_fragment_window_size) {
      return true;
    }

    rxPacket->status = SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE;
    rxPacket->fragmentSource = sender;
    rxPacket->fragmentSequenceNumber = apsFrame->sequence;
    rxPacket->fragmentBase = 0;
    rxPacket->windowFinger = 0;
    rxPacket->fragmentsReceived = 0;
    rxPacket->fragmentsExpected = 0xFF;
    rxPacket->fragmentLen = (uint8_t)(*bufLen);
    setFragmentMask(rxPacket);

    sl_zigbee_af_event_set_delay_ms(&(rxPacket->fragmentEventControl),
                                    (retryTimeoutMs(sender)
                                     * ZIGBEE_APSC_MAX_TRANSMIT_RETRIES));
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
    sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_FRAGMENTATION_IN_PROGRESS);
#endif // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  }

  // All fragments inside the rx window have been received and the incoming
  // fragment is outside the receiving window: let's move the rx window.
  if (rxPacket->fragmentMask == 0xFF
      && rxPacket->fragmentBase + sl_zigbee_fragment_window_size <= fragment) {
    moveRxWindow(rxPacket);
    setFragmentMask(rxPacket);
    rxWindowMoved = true;

    sl_zigbee_af_event_set_delay_ms(&(rxPacket->fragmentEventControl),
                                    (retryTimeoutMs(sender)
                                     * ZIGBEE_APSC_MAX_TRANSMIT_RETRIES));
  }

  // Fragment outside the rx window.
  if (fragment < rxPacket->fragmentBase
      || fragment >= rxPacket->fragmentBase + sl_zigbee_fragment_window_size) {
    return true;
  } else { // Fragment inside the rx window.
    if (rxWindowMoved) {
      // We assume that the fragment length for the new rx window is the length
      // of the first fragment received inside the window. However, if the first
      // fragment received is the last fragment of the packet, we do not
      // consider it for setting the fragment length.
      if (fragment < rxPacket->fragmentsExpected - 1) {
        rxPacket->fragmentLen = (uint8_t)(*bufLen);
        rxWindowMoved = false;
      }
    } else {
      // We enforce that all the subsequent fragments (except for the last
      // fragment) inside the rx window have the same length as the first one.
      if (fragment < rxPacket->fragmentsExpected - 1
          && rxPacket->fragmentLen != (uint8_t)(*bufLen)) {
        goto kickout;
      }
    }
  }

  mask = 1 << (fragment % sl_zigbee_fragment_window_size);
  newFragment = !(mask & rxPacket->fragmentMask);

  // First fragment, setting the total number of expected fragments.
  if (fragment == 0) {
    rxPacket->fragmentsExpected = HIGH_BYTE(apsFrame->groupId);
    if (rxPacket->fragmentsExpected < sl_zigbee_fragment_window_size) {
      setFragmentMask(rxPacket);
    }
  }

  sl_zigbee_legacy_af_debug_print("Receiving fragment %d of %d", fragment, rxPacket->fragmentsExpected);

  // If it's a new fragment, try to buffer it
  // If the payload is too long, we wait to hear all fragments (and ack each
  // one) and then respond with an INSUFFICIENT_SPACE
  rxPacket->fragmentMask |= mask;
  if (newFragment) {
    rxPacket->fragmentsReceived++;
    if (!storeRxFragment(rxPacket, fragment, *buffer, *bufLen)
        && (SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE != rxPacket->status)) {
      goto kickout;
    }
  }

  if (fragment == rxPacket->fragmentsExpected - 1
      || (rxPacket->fragmentMask
          | lowBitMask(fragment % sl_zigbee_fragment_window_size)) == 0xFF) {
    sli_zigbee_af_fragmentation_send_reply(sender,
                                           apsFrame,
                                           rxPacket);
  }

  // Received all the expected fragments.
  if (rxPacket->fragmentsReceived == rxPacket->fragmentsExpected) {
    uint8_t fragmentsInLastWindow =
      rxPacket->fragmentsExpected % sl_zigbee_fragment_window_size;
    if (fragmentsInLastWindow == 0) {
      fragmentsInLastWindow = sl_zigbee_fragment_window_size;
    }

    // Pass the reassembled packet only once to the application.
    if (rxPacket->status == SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE) {
      //Age all acked packets first
      ageAllAckedRxPackets();
      // Mark the packet entry as acked.
      rxPacket->status = SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED;
      // Set the age of the new acked packet as the youngest one.
      rxPacket->ackedPacketAge = 0;
      // This library sends replies for all fragments, so, before passing on the
      // reassembled message, clear the retry bit to prevent the application
      // from sending a duplicate reply.
      apsFrame->options &= ~SL_ZIGBEE_APS_OPTION_RETRY;

      // The total size is the window finger + (n-1) full fragments + the last
      // fragment.
      *bufLen = rxPacket->windowFinger + rxPacket->lastfragmentLen
                + (fragmentsInLastWindow - 1) * rxPacket->fragmentLen;
      *buffer = rxPacket->buffer;
      updateCurrentAppTasksForFragmentationState();
      return false;
    }
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
    else if ( (apsFrame->destinationEndpoint != 0) && (rxPacket->status
                                                       == SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE)) {
      // Send a default response with INSUFFICIENT_SPACE
      *bufLen = rxPacket->windowFinger + rxPacket->lastfragmentLen
                + (fragmentsInLastWindow - 1) * rxPacket->fragmentLen;
      *buffer = rxPacket->buffer;
      sl_zigbee_af_process_message_into_zcl_cmd(apsFrame,
                                                type,
                                                *buffer,
                                                *bufLen,
                                                sender,
                                                NULL, // inter-PAN fragments don't reach here
                                                &cmd);
      sl_zigbee_af_send_default_response(&cmd, SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE);

      // Finally, free the buffer
      sli_zigbee_af_fragmentation_abort_reception(&(rxPacket->fragmentEventControl));
    }
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  }
  return true;

  kickout:
  sli_zigbee_af_fragmentation_abort_reception(&(rxPacket->fragmentEventControl));

  return true;
}

void sli_zigbee_af_fragmentation_abort_reception(sl_zigbee_af_event_t* control)
{
  uint8_t i;
  sl_zigbee_af_event_set_inactive(control);

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    sli_zigbee_rx_fragmented_packet_t *rxPacket = &(rxPackets[i]);
    if (&(rxPacket->fragmentEventControl) == control) {
      rxPacket->status = SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE;
    }
  }
  updateCurrentAppTasksForFragmentationState();
}

static void setFragmentMask(sli_zigbee_rx_fragmented_packet_t *rxPacket)
{
  // Unused bits must be 1.
  uint8_t highestZeroBit = sl_zigbee_fragment_window_size;
  // If we are in the final window, there may be additional unused bits.
  if (rxPacket->fragmentsExpected
      < rxPacket->fragmentBase + sl_zigbee_fragment_window_size) {
    highestZeroBit = (rxPacket->fragmentsExpected % sl_zigbee_fragment_window_size);
  }
  rxPacket->fragmentMask = ~lowBitMask(highestZeroBit);
}

static bool storeRxFragment(sli_zigbee_rx_fragmented_packet_t *rxPacket,
                            uint8_t fragment,
                            uint8_t *buffer,
                            uint16_t bufLen)
{
  uint16_t index = rxPacket->windowFinger;

  if (index + bufLen > SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE) {
    rxPacket->status = SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE;
    return false;
  }

  index += (fragment - rxPacket->fragmentBase) * rxPacket->fragmentLen;
  memmove(rxPacket->buffer + index, buffer, bufLen);

  // If this is the last fragment of the packet, store its length.
  if (fragment == rxPacket->fragmentsExpected - 1) {
    rxPacket->lastfragmentLen = (uint8_t)bufLen;
  }

  return true;
}

static void moveRxWindow(sli_zigbee_rx_fragmented_packet_t *rxPacket)
{
  rxPacket->fragmentBase += sl_zigbee_fragment_window_size;
  rxPacket->windowFinger += sl_zigbee_fragment_window_size * rxPacket->fragmentLen;
}

static sli_zigbee_rx_fragmented_packet_t* getFreeRxPacketEntry(void)
{
  uint8_t i;
  sli_zigbee_rx_fragmented_packet_t* ackedPacket = NULL;

  // Available entries first.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    sli_zigbee_rx_fragmented_packet_t *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE) {
      return rxPacket;
    }
  }

  // Acked packets: Look for the oldest one.
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    sli_zigbee_rx_fragmented_packet_t *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED) {
      if (ackedPacket == NULL
          || ackedPacket->ackedPacketAge < rxPacket->ackedPacketAge) {
        ackedPacket = rxPacket;
      }
    }
  }

  return ackedPacket;
}

static sli_zigbee_rx_fragmented_packet_t* rxPacketLookUp(sl_zigbee_aps_frame_t *apsFrame,
                                                         sl_802154_short_addr_t sender)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    sli_zigbee_rx_fragmented_packet_t *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE) {
      continue;
    }
    // Each packet is univocally identified by the pair (node id, seq. number).
    if (apsFrame->sequence == rxPacket->fragmentSequenceNumber
        && sender == rxPacket->fragmentSource) {
      return rxPacket;
    }
  }
  return NULL;
}

//------------------------------------------------------------------------------
// Initialization

void sl_zigbee_af_fragmentation_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      uint8_t i;
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
        sl_zigbee_af_event_init(&(rxPackets[i].fragmentEventControl),
                                sli_zigbee_af_fragmentation_abort_reception);
        rxPackets[i].status = SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE;
      }
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      uint8_t i;
      sli_zigbee_af_fragmentation_platform_init_callback();
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
        txPackets[i].messageType = 0xFF;
      }
      break;
    }
  }
}

//------------------------------------------------------------------------------
// Utility

// Clear app task frag bit only if neither tx nor rx frag is in progress.
static void updateCurrentAppTasksForFragmentationState(void)
{
  uint8_t i;
  bool fragmenting;

  fragmenting = false;

  if (txPacketAwaitingFinalAck.messageType != UNUSED_TX_PACKET_ENTRY) {
    fragmenting = true;
  }

  if (!fragmenting) {
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
      if (txPackets[i].messageType != UNUSED_TX_PACKET_ENTRY) {
        fragmenting = true;
        break;
      }
    }
  }

  if (!fragmenting) {
    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
      if (rxPackets[i].status == SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE) {
        fragmenting = true;
        break;
      }
    }
  }
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  if (!fragmenting) {
    sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_FRAGMENTATION_IN_PROGRESS);
  }
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
}
