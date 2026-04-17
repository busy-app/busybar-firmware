/***************************************************************************//**
 * @file
 * @brief Generic code related to the receipt and processing of interpan
 * messages.
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

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "interpan.h"
#include "stack/include/zigbee-security-manager.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#if defined(ALLOW_KEY_ESTABLISHMENT)
#if defined(ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_SERVER_COMMAND_ID)
#define ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_SERVER_COMMAND_ID
#elif defined(ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_CLIENT_COMMAND_ID)
#define ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_CLIENT_COMMAND_ID
#else
#error "ZCL TerminateKeyEstablishment Command ID not defined"
#endif
#endif // ALLOW_KEY_ESTABLISHMENT

//------------------------------------------------------------------------------
// Globals

// Global enable. Defaults to true. User can set to false if s/he wishes.
// Example: user will set to false for SE 1.2 Comms Hub (GBCS)
static bool gInterpanEnabled;

#if defined(ALLOW_FRAGMENTATION)
// Global timeout value in seconds
static uint16_t gMessageTimeout;
#endif // ALLOW_FRAGMENTATION

// MAC frame control
// Bits:
// | 0-2   |   3      |    4    |  5  |  6    |   7-9    | 10-11 |  12-13   | 14-15 |
// | Frame | Security |  Frame  | Ack | Intra | Reserved | Dest. | Reserved | Src   |
// | Type  | Enabled  | Pending | Req | PAN   |          | Addr. |          | Adrr. |
// |       |          |         |     |       |          | Mode  |          | Mode  |

// Frame Type
//   000       = Beacon
//   001       = Data
//   010       = Acknwoledgement
//   011       = MAC Command
//   100 - 111 = Reserved

// Addressing Mode
//   00 - PAN ID and address field are not present
//   01 - Reserved
//   10 - Address field contains a 16-bit short address
//   11 - Address field contains a 64-bit extended address

#ifdef EZSP_HOST
#define MAC_FRAME_TYPE_DATA           0x0001

#define MAC_FRAME_SOURCE_MODE_SHORT 0x8000
#define MAC_FRAME_SOURCE_MODE_LONG  0xC000

#define MAC_FRAME_DESTINATION_MODE_SHORT   0x0800
#define MAC_FRAME_DESTINATION_MODE_LONG    0x0C00
#endif
// The two possible incoming MAC frame controls.
// Using short source address is not allowed.
#define SHORT_DEST_FRAME_CONTROL (MAC_FRAME_TYPE_DATA                \
                                  | MAC_FRAME_DESTINATION_MODE_SHORT \
                                  | MAC_FRAME_SOURCE_MODE_LONG)
#define LONG_DEST_FRAME_CONTROL  (MAC_FRAME_TYPE_DATA               \
                                  | MAC_FRAME_DESTINATION_MODE_LONG \
                                  | MAC_FRAME_SOURCE_MODE_LONG)

#define MAC_ACK_REQUIRED              0x0020

#if defined(ALLOW_APS_ENCRYPTED_MESSAGES)
  #define APS_ENCRYPTION_ALLOWED 1
#else
  #define APS_ENCRYPTION_ALLOWED 0
#endif

#define PRIMARY_ENDPOINT   0
#define ALL_ENDPOINTS      1
#define SPECIFIED_ENDPOINT 2

#if defined(SL_ZIGBEE_AF_PLUGIN_INTERPAN_DELIVER_TO)
  #if SL_ZIGBEE_AF_PLUGIN_INTERPAN_DELIVER_TO == PRIMARY_ENDPOINT
    #define ENDPOINT sl_zigbee_af_primary_endpoint()

  #elif SL_ZIGBEE_AF_PLUGIN_INTERPAN_DELIVER_TO == ALL_ENDPOINTS
    #define ENDPOINT SL_ZIGBEE_BROADCAST_ENDPOINT

  #elif SL_ZIGBEE_AF_PLUGIN_INTERPAN_DELIVER_TO == SPECIFIED_ENDPOINT
    #define ENDPOINT SL_ZIGBEE_AF_PLUGIN_INTERPAN_DELIVER_TO_SPECIFIED_ENDPOINT_VALUE

  #else
    #error Unknown value for interpan plugin option: SL_ZIGBEE_AF_PLUGIN_INTERPAN_DELIVER_TO
  #endif
#else
  #error Invalid Interpan plugin Config: SL_ZIGBEE_AF_PLUGIN_INTERPAN_DELIVER_TO not defined.
#endif

//------------------------------------------------------------------------------
// Forward Declarations

static bool isMessageAllowed(sl_zigbee_af_interpan_header_t *headerData,
                             uint8_t messageLength,
                             uint8_t *messageContents);

#if ((defined(EMBER_AF_PRINT_ENABLE) && defined(SL_ZIGBEE_AF_PRINT_APP)) || (defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT)))
static void printMessage(sl_zigbee_mac_passthrough_type_t passthroughType, sl_zigbee_af_interpan_header_t *headerData);
#else
  #define printMessage(x, y)
#endif

#if !defined SL_ZIGBEE_AF_PLUGIN_INTERPAN_CUSTOM_FILTER
  #define SL_ZIGBEE_AF_PLUGIN_INTERPAN_CUSTOM_FILTER
#endif

#if defined(ALLOW_FRAGMENTATION)
static sl_status_t interpanFragmentationSendUnicast(sl_zigbee_af_interpan_header_t* header, uint8_t* message, uint16_t messageLen);
static bool isInterpanFragment(uint8_t* payload, uint8_t payloadLen);
static sl_status_t interpanFragmentationSendIpmf(sli_zigbee_tx_fragmented_interpan_packet_t* txPacket);
static sl_status_t interpanFragmentationProcessIpmf(sl_zigbee_af_interpan_header_t header, uint8_t *message, uint8_t messageLen);
static void interpanFragmentationSendIpmfResponse(sl_zigbee_af_interpan_header_t header, uint8_t fragNum, uint8_t responseStatus);
static void interpanFragmentationProcessIpmfResponse(sl_zigbee_af_interpan_header_t header, uint8_t* payload, uint8_t payloadLen);

static sli_zigbee_tx_fragmented_interpan_packet_t txPackets[SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS];
static sli_zigbee_rx_fragmented_interpan_packet_t rxPackets[SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS];
static sli_zigbee_tx_fragmented_interpan_packet_t* getFreeTxPacketEntry(sl_802154_long_addr_t destinationEui);
static sli_zigbee_rx_fragmented_interpan_packet_t* getFreeRxPacketEntry(sl_802154_long_addr_t sourceEui);
static sli_zigbee_tx_fragmented_interpan_packet_t* txPacketLookUp(sl_802154_long_addr_t destEui);
static sli_zigbee_rx_fragmented_interpan_packet_t* rxPacketLookUp(sl_802154_long_addr_t sourceEui);
static void freeTxPacketEntry(sli_zigbee_tx_fragmented_interpan_packet_t *txPacket, sli_zigbee_e_interpan_fragmentation_status_t status);
static void freeRxPacketEntry(sli_zigbee_rx_fragmented_interpan_packet_t *rxPacket, sli_zigbee_e_interpan_fragmentation_status_t status);

sl_zigbee_af_event_t sl_zigbee_af_interpan_fragment_transmit_event;
sl_zigbee_af_event_t sl_zigbee_af_interpan_fragment_receive_event;
void sl_zigbee_af_interpan_fragment_transmit_event_handler(sl_zigbee_af_event_t * event);
void sl_zigbee_af_interpan_fragment_receive_event_handler(sl_zigbee_af_event_t * event);

#endif // ALLOW_FRAGMENTATION

// These have to stay outside the ifdef interpan frag check because AppBuilder
// generates code that calls them, which cannot be ifdeffed

#define IPMF_MSG_TIMEOUT_MS       60 * 1000

//------------------------------------------------------------------------------

// Inter-PAN messages are a security risk because they are typically not
// encrypted and yet we must accept them.  Therefore, we are very strict about
// what we allow.  Listed below are ZCL messages that are allowed, including
// the profile, cluster, direction, and command id.  Any inter-PAN messages not
// matching one of these are dropped.
// If inter-PAN fragments (IPMF) are received, a different set of checks is
// performed.
static const sl_zigbee_af_allowed_inter_pan_message_t messages[] = {
  // A custom filter can be added using this #define.
  // It should contain one or more 'sl_zigbee_af_allowed_inter_pan_message_t' const structs
  SL_ZIGBEE_AF_PLUGIN_INTERPAN_CUSTOM_FILTER

#if defined(ALLOW_REQUIRED_SMART_ENERGY_MESSAGES)

#ifdef ZCL_USING_PRICE_CLUSTER_SERVER
  {
    SE_PROFILE_ID,
    ZCL_PRICE_CLUSTER_ID,
    ZCL_GET_CURRENT_PRICE_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER,
  },
  {
    SE_PROFILE_ID,
    ZCL_PRICE_CLUSTER_ID,
    ZCL_GET_SCHEDULED_PRICES_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER,
  },
#endif

#endif // ALLOW_REQUIRED_SMART_ENERGY_MESSAGES

#if defined(ALLOW_SMART_ENERGY_RESPONSE_MESSAGES)

#ifdef ZCL_USING_PRICE_CLUSTER_CLIENT
  {
    SE_PROFILE_ID,
    ZCL_PRICE_CLUSTER_ID,
    ZCL_PUBLISH_PRICE_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT,
  },
#endif
#ifdef ZCL_USING_MESSAGING_CLUSTER_CLIENT
  {
    SE_PROFILE_ID,
    ZCL_MESSAGING_CLUSTER_ID,
    ZCL_DISPLAY_MESSAGE_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT,
  },
  {
    SE_PROFILE_ID,
    ZCL_MESSAGING_CLUSTER_ID,
    ZCL_CANCEL_MESSAGE_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT,
  },
#endif

#endif // ALLOW_SMART_ENERGY_RESPONSE_MESSAGES

#if defined(ALLOW_KEY_ESTABLISHMENT)
  // Since client and server share the same command ID's,
  // we can get away with only listing the request messages.
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_EPHEMERAL_DATA_RESPONSE_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_CONFIRM_KEY_DATA_RESPONSE_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_DEFAULT_RESPONSE_COMMAND_ID,
    SL_ZIGBEE_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER,
  },
#endif // ALLOW_KEY_ESTABLISHMENT

  { 0xFFFF, 0xFFFF, 0x00, 0x00 }, // terminator
};

//#define ENCRYPT_DEBUG

#if !defined(ENCRYPT_DEBUG)
  #define printData(...)
#endif

// Private ZLL API's
bool sli_zigbee_af_zll_commissioning_client_interpan_pre_message_received_callback(const sl_zigbee_af_interpan_header_t *header,
                                                                                   uint8_t msgLen,
                                                                                   uint8_t *message);
bool sli_zigbee_af_zll_commissioning_server_interpan_pre_message_received_callback(const sl_zigbee_af_interpan_header_t *header,
                                                                                   uint8_t msgLen,
                                                                                   uint8_t *message);

//------------------------------------------------------------------------------

void interpanPluginInit(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
#if defined(ALLOW_FRAGMENTATION)
      sl_zigbee_af_event_init(&sl_zigbee_af_interpan_fragment_transmit_event,
                              sl_zigbee_af_interpan_fragment_transmit_event_handler);
      sl_zigbee_af_event_init(&sl_zigbee_af_interpan_fragment_receive_event,
                              sl_zigbee_af_interpan_fragment_receive_event_handler);
#endif // ALLOW_FRAGMENTATION
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      gInterpanEnabled = true;

#if defined(ALLOW_FRAGMENTATION)
      gMessageTimeout  = IPMF_MSG_TIMEOUT_MS;
      uint8_t i;

      // The following two loops need adjustment if more than one packet is desired
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
        txPackets[i].messageType  = UNUSED_TX_PACKET_ENTRY;
        txPackets[i].event = &sl_zigbee_af_interpan_fragment_transmit_event;
      }
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
        memset(rxPackets[i].sourceEui, 0, EUI64_SIZE);
        rxPackets[i].event = &sl_zigbee_af_interpan_fragment_receive_event;
      }
#endif //ALLOW_FRAGMENTATION
      break;
    }
  }
}

// This function sets the global state of inter-PAN. If state is true, inter-PAN
// messages are sent and received, else, all inter-PANs are dropped.
void sli_zigbee_af_interpan_set_enable_state(bool enable)
{
  if (gInterpanEnabled != enable) {
    gInterpanEnabled = enable;
    interpanPluginSetMacMatchFilterEnable(enable);
  }
}

#if defined(ALLOW_FRAGMENTATION)
// This is a helper function for testing purposes only. It changes the inter-PAN
// fragment message timeout
void interpanPluginSetFragmentMessageTimeout(uint16_t timeout)  // seconds
{
  uint16_t timeoutMs = timeout * 1000;
  if (gMessageTimeout != timeoutMs) {
    gMessageTimeout = timeoutMs;
    sl_zigbee_af_app_println("IPMF message timeout changed to %d seconds", timeout);
  }
}
#endif //ALLOW_FRAGMENTATION

#if defined(ENCRYPT_DEBUG)

static void printData(const char * name, uint8_t* message, uint8_t length)
{
  uint8_t i;
  sl_zigbee_af_app_print("\n%s: ", name);
  for (i = 0; i < length; i++) {
    sl_zigbee_af_app_print("%02X ", message[i]);
    sl_zigbee_af_app_flush();
  }
  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_flush();
}

#endif

static uint8_t* pushInt16u(uint8_t *finger, uint16_t value)
{
  *finger++ = LOW_BYTE(value);
  *finger++ = HIGH_BYTE(value);
  return finger;
}

static uint8_t* pushEui64(uint8_t *finger, uint8_t *value)
{
  memmove(finger, value, 8);
  return finger + 8;
}

#if defined(ALLOW_APS_ENCRYPTED_MESSAGES)

static sl_status_t handleApsSecurity(bool encrypt,
                                     uint8_t* apsFrame,
                                     uint8_t apsHeaderLength,
                                     uint8_t* messageLength,
                                     uint8_t maxLengthForEncryption,
                                     sl_zigbee_af_interpan_header_t* header)
{
  sl_status_t status;

  if (!(header->options & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)) {
    // We won't encrypt/decrypt messages that don't have a long address.
    return SL_STATUS_ZIGBEE_APS_ENCRYPTION_ERROR;
  }
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_export_link_key_by_eui(header->longAddress, &context, NULL, NULL);

  if (context.key_index == 0xFF) {
    return SL_STATUS_ZIGBEE_APS_ENCRYPTION_ERROR;
  }

  if (encrypt
      && ((maxLengthForEncryption - *messageLength)
          < INTERPAN_APS_ENCRYPTION_OVERHEAD)) {
    return SL_STATUS_ZIGBEE_APS_ENCRYPTION_ERROR;
  }

  apsFrame[0] |= INTERPAN_APS_FRAME_SECURITY;

  status = sli_zigbee_af_interpan_aps_crypt_message(encrypt,
                                                    apsFrame,
                                                    messageLength,
                                                    apsHeaderLength,
                                                    header->longAddress);
  return status;
}
#endif

static sl_status_t makeInterPanMessage(sl_zigbee_af_interpan_header_t *headerData,
                                       uint8_t *message,
                                       uint16_t maxLength,
                                       uint8_t *payload,
                                       uint16_t *payloadLength,
                                       uint16_t *returnLength)
{
  uint8_t *finger = message;
  bool haveLongAddress = (headerData->options
                          & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS);
  uint16_t macFrameControl = (haveLongAddress
                              ? LONG_DEST_FRAME_CONTROL
                              : SHORT_DEST_FRAME_CONTROL);

  sl_802154_long_addr_t eui;
  uint8_t* apsFrame;

  uint8_t headerSize = (SL_ZIGBEE_AF_INTER_PAN_UNICAST == headerData->messageType)
                       ? INTERPAN_UNICAST_HEADER_SIZE
                       : INTERPAN_MULTICAST_HEADER_SIZE;

  if (maxLength < (headerSize + *payloadLength)) {
    sl_zigbee_af_app_print("Error: Inter-pan message too big (%d + %d > %d)",
                           headerSize, *payloadLength,
                           maxLength);
    return SL_STATUS_MESSAGE_TOO_LONG;
  }

  if (headerData->messageType == SL_ZIGBEE_AF_INTER_PAN_UNICAST) {
    macFrameControl |= MAC_ACK_REQUIRED;
  }

  sl_zigbee_af_get_eui64(eui);

  finger = pushInt16u(finger, macFrameControl);
  finger++;  // Skip Sequence number, stack sets the sequence number.

  finger = pushInt16u(finger, headerData->panId);
  if (headerData->options & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS) {
    finger = pushEui64(finger, headerData->longAddress);
  } else {
    finger = pushInt16u(finger, headerData->shortAddress);
  }

  finger = pushInt16u(finger, sl_zigbee_af_get_pan_id());
  finger = pushEui64(finger, eui);

  finger = pushInt16u(finger, STUB_NWK_FRAME_CONTROL);

  apsFrame = finger;
  *finger++ = (headerData->messageType
               | INTERPAN_APS_FRAME_TYPE);

  if (headerData->messageType == SL_ZIGBEE_AF_INTER_PAN_MULTICAST) {
    finger = pushInt16u(finger, headerData->groupId);
  }

  finger = pushInt16u(finger, headerData->clusterId);
  finger = pushInt16u(finger, headerData->profileId);

  uint8_t apsHeaderLength = (uint8_t)(finger - apsFrame);

  memmove(finger, payload, *payloadLength);
  finger += *payloadLength;

  if (headerData->options & SL_ZIGBEE_AF_INTERPAN_OPTION_APS_ENCRYPT) {
    sl_status_t status = SL_STATUS_INVALID_CONFIGURATION;
    uint8_t apsEncryptLength = apsHeaderLength + (uint8_t)(*payloadLength);
    #if defined(ALLOW_APS_ENCRYPTED_MESSAGES)
    printData("Before Encryption", apsFrame, apsEncryptLength);

    status = handleApsSecurity(true,  // encrypt?
                               apsFrame,
                               apsHeaderLength,
                               &apsEncryptLength,
                               maxLength - (uint8_t)(apsFrame - message),
                               headerData);
    #endif // ALLOW_APS_ENCRYPTED_MESSAGES
    if (status != SL_STATUS_OK) {
      return status;
    }

    printData("After Encryption ", apsFrame, apsEncryptLength);

    finger = apsFrame + apsEncryptLength;
  }

  *returnLength = finger - message;

  return SL_STATUS_OK;
}

static uint8_t parseInterpanMessage(uint8_t *message,
                                    uint8_t *messageLength,
                                    sl_zigbee_af_interpan_header_t *headerData)
{
  uint8_t *finger = message;
  uint16_t macFrameControl;
  uint8_t remainingLength;
  uint8_t apsFrameControl;
  uint8_t apsHeaderIndex;

  memset(headerData, 0, sizeof(sl_zigbee_af_interpan_header_t));

  // We rely on the stack to insure that the MAC frame is formatted
  // correctly and that the length is at least long enough
  // to contain that frame.

  macFrameControl = HIGH_LOW_TO_INT(finger[1], finger[0])
                    & ~(MAC_ACK_REQUIRED);

  if (macFrameControl == LONG_DEST_FRAME_CONTROL) {
    // control, sequence, dest PAN ID, long dest
    finger += 2 + 1 + 2 + 8;
  } else if (macFrameControl == SHORT_DEST_FRAME_CONTROL) {
    // control, sequence, dest PAN ID, short dest
    finger += 2 + 1 + 2 + 2;
  } else {
    return 0;
  }

  // Source PAN ID
  headerData->panId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;

  // It is expected that the long Source Address is always present and
  // that the stack MAC filters insured that to be the case.
  headerData->options |= SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS;
  memmove(headerData->longAddress, finger, 8);
  finger += 8;

  // Now that we know the correct MAC length, verify the interpan
  // frame is the correct length.

  remainingLength = *messageLength - (uint8_t)(finger - message);

  if (remainingLength < (STUB_NWK_SIZE + MIN_STUB_APS_SIZE)) {
    return 0;
  }

  if (HIGH_LOW_TO_INT(finger[1], finger[0]) != STUB_NWK_FRAME_CONTROL) {
    return 0;
  }
  finger += 2;
  apsHeaderIndex = (finger - message);
  remainingLength -= 2;

  apsFrameControl = (*finger++);

  if ((apsFrameControl & ~(INTERPAN_APS_FRAME_DELIVERY_MODE_MASK)
       &~INTERPAN_APS_FRAME_SECURITY)
      != INTERPAN_APS_FRAME_CONTROL_NO_DELIVERY_MODE) {
    sl_zigbee_af_app_println("%sBad APS frame control 0x%02X",
                             "ERR: Inter-PAN ",
                             apsFrameControl);
    return 0;
  }
  headerData->messageType = (apsFrameControl
                             & INTERPAN_APS_FRAME_DELIVERY_MODE_MASK);

  switch (headerData->messageType) {
    case SL_ZIGBEE_AF_INTER_PAN_UNICAST:
    case SL_ZIGBEE_AF_INTER_PAN_BROADCAST:
      // Broadcast and unicast have the same size messages
      if (remainingLength < INTERPAN_APS_UNICAST_SIZE) {
        return 0;
      }
      break;
    case SL_ZIGBEE_AF_INTER_PAN_MULTICAST:
      if (remainingLength < INTERPAN_APS_MULTICAST_SIZE) {
        return 0;
      }
      headerData->groupId = HIGH_LOW_TO_INT(finger[1], finger[0]);
      finger += 2;
      break;
    default:
      sl_zigbee_af_app_println("%sBad Delivery Mode 0x%02X",
                               "ERR: Inter-PAN ",
                               headerData->messageType);
      return 0;
  }

  headerData->clusterId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;
  headerData->profileId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;

  if (apsFrameControl & INTERPAN_APS_FRAME_SECURITY) {
    sl_status_t status = SL_STATUS_NOT_AVAILABLE;
    uint8_t apsEncryptLength = *messageLength - apsHeaderIndex;
    headerData->options |= SL_ZIGBEE_AF_INTERPAN_OPTION_APS_ENCRYPT;
    #if defined(ALLOW_APS_ENCRYPTED_MESSAGES)
    uint8_t UNUSED apsHeaderLength = (uint8_t)(finger - message) - apsHeaderIndex;
    printData("Before Decryption",
              message + apsHeaderIndex,
              apsEncryptLength);

    status = handleApsSecurity(false, // encrypt?
                               message + apsHeaderIndex,
                               apsHeaderLength,
                               &apsEncryptLength,
                               0,    // maxLengthForEncryption (ignored)
                               headerData);
    #endif // ALLOW_APS_ENCRYPTED_MESSAGES
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_app_println("%sAPS decryption failed (0x%02X).",
                               "ERR: Inter-PAN ",
                               status);
      return 0;
    }

    printData("After Decryption ",
              message + apsHeaderIndex,
              apsEncryptLength);

    *messageLength = apsHeaderIndex + apsEncryptLength;
  }

  return (finger - message);
}

bool sli_zigbee_af_interpan_process_message(
  sl_zigbee_mac_passthrough_type_t passthroughType,
  uint8_t messageLength,
  uint8_t *messageContents)
{
  sl_zigbee_aps_frame_t apsFrame;
  sl_zigbee_incoming_message_type_t type;
  bool loopback;
  sl_zigbee_af_interpan_header_t headerData;
  uint8_t payloadOffset, payloadLength;
  uint8_t* payload;

  sl_zigbee_af_debug_println("sli_zigbee_af_interpan_process_message");
  payloadOffset = parseInterpanMessage(messageContents,
                                       &messageLength,
                                       &headerData);
  if (payloadOffset == 0) {
    return false;
  }
  printMessage(passthroughType, &headerData);

  payload = messageContents + payloadOffset;
  payloadLength = messageLength - payloadOffset;

  if (sl_zigbee_af_interpan_pre_message_received_cb(&headerData,
                                                    payloadLength,
                                                    payload)
#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_CLIENT_PRESENT
      || (((passthroughType & SL_802154_PASSTHROUGH_INTERNAL_ZLL) == SL_802154_PASSTHROUGH_INTERNAL_ZLL)
          && (sli_zigbee_af_zll_commissioning_client_interpan_pre_message_received_callback(&headerData,
                                                                                            payloadLength,
                                                                                            payload)))
#endif // ZSL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_CLIENT_PRESENT
#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT
      || (((passthroughType & SL_802154_PASSTHROUGH_INTERNAL_ZLL) == SL_802154_PASSTHROUGH_INTERNAL_ZLL)
          && (sli_zigbee_af_zll_commissioning_server_interpan_pre_message_received_callback(&headerData,
                                                                                            payloadLength,
                                                                                            payload)))
#endif // SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT
      ) {
    return true;
  }
#if defined(SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_CLIENT_PRESENT) || defined(SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT)
  else if (headerData.profileId == SL_ZIGBEE_ZLL_PROFILE_ID
           && headerData.clusterId == ZCL_ZLL_COMMISSIONING_CLUSTER_ID) {
    // Drop any other ZLL commissioning interpans, if they get this far.
    sl_zigbee_af_debug_println("Interpan - dropping unhandled interpans");
    return true;
  }
#endif // SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_CLIENT_PRESENT || SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_SERVER_PRESENT

  if (!isMessageAllowed(&headerData, payloadLength, payload)) {
    return false;
  }

  if (headerData.options & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS) {
    headerData.shortAddress = SL_ZIGBEE_NULL_NODE_ID;
    loopback = sl_zigbee_is_local_eui64(headerData.longAddress);
  } else {
    loopback = (sl_zigbee_af_get_node_id() == headerData.shortAddress);
  }

  apsFrame.profileId = headerData.profileId;
  apsFrame.clusterId = headerData.clusterId;
  apsFrame.sourceEndpoint = 1;   // arbitrary since not sent over-the-air
  apsFrame.destinationEndpoint = ENDPOINT;
  apsFrame.options = (SL_ZIGBEE_APS_OPTION_NONE
                      | ((headerData.options
                          & SL_ZIGBEE_AF_INTERPAN_OPTION_APS_ENCRYPT)
                         ? SL_ZIGBEE_APS_OPTION_ENCRYPTION
                         : 0));
  apsFrame.groupId = headerData.groupId;
  apsFrame.sequence = 0x00; // unknown sequence number

  switch (headerData.messageType) {
    case SL_ZIGBEE_AF_INTER_PAN_UNICAST:
      type = SL_ZIGBEE_INCOMING_UNICAST;
      break;
    case SL_ZIGBEE_AF_INTER_PAN_BROADCAST:
      type = (loopback
              ? SL_ZIGBEE_INCOMING_BROADCAST_LOOPBACK
              : SL_ZIGBEE_INCOMING_BROADCAST);
      break;
    case SL_ZIGBEE_AF_INTER_PAN_MULTICAST:
      type = (loopback
              ? SL_ZIGBEE_INCOMING_MULTICAST_LOOPBACK
              : SL_ZIGBEE_INCOMING_MULTICAST);
      break;
    default:
      return false;
  }

#if defined(ALLOW_FRAGMENTATION)
  // If interpan fragment, intercept
  if (SE_PROFILE_ID == headerData.profileId) {
    if (isInterpanFragment(payload, payloadLength)) {
      uint8_t fragNum = payload[INTERPAN_FRAGMENTATION_APS_INDEX_IPMF_INDEX];

      switch (payload[INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_INDEX]) {
        case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_VAL:
        {
          sl_status_t status = interpanFragmentationProcessIpmf(headerData,
                                                                messageContents,
                                                                messageLength);
          interpanFragmentationSendIpmfResponse(headerData,
                                                fragNum,
                                                SL_STATUS_OK == status
                                                ? INTERPAN_IPMF_RESPONSE_SUCCESS
                                                : INTERPAN_IPMF_RESPONSE_FAILURE);
        }
        break;
        case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_RESPONSE_VAL:
        {
          // This function will also send the next fragment if response is OK
          interpanFragmentationProcessIpmfResponse(headerData,
                                                   payload,
                                                   payloadLength);
        }
        break;
        default:
          break;
      }

      // All inter-PAN fragments are consumed
      return true;
    }
  }
#endif // ALLOW_FRAGMENTATION

  return sl_zigbee_af_process_message(&apsFrame,
                                      type,
                                      payload,
                                      payloadLength,
                                      headerData.shortAddress,
                                      &headerData);
}

static bool isMessageAllowed(sl_zigbee_af_interpan_header_t *headerData,
                             uint8_t messageLength,
                             uint8_t *messageContents)
{
  uint8_t incomingMessageOptions = 0;
  uint8_t commandId;

  if (messageLength < SL_ZIGBEE_AF_ZCL_OVERHEAD) {
    sl_zigbee_af_app_println("%smessage too short (%d < %d)!",
                             "ERR: Inter-PAN ",
                             messageLength,
                             SL_ZIGBEE_AF_ZCL_OVERHEAD);
    return false;
  }

  if (headerData->options & SL_ZIGBEE_AF_INTERPAN_OPTION_APS_ENCRYPT) {
    return APS_ENCRYPTION_ALLOWED;
  }

  // Only the first bit is used for ZCL Frame type
  if (messageContents[0] & BIT(1)) {
    sl_zigbee_af_app_println("%sUnsupported ZCL frame type.",
                             "ERR: Inter-PAN ");
    return false;
  }

  if (messageContents[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) {
    incomingMessageOptions |= SL_ZIGBEE_AF_INTERPAN_MANUFACTURER_SPECIFIC;
  }
  if ((messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND)
      == ZCL_GLOBAL_COMMAND) {
    incomingMessageOptions |= SL_ZIGBEE_AF_INTERPAN_GLOBAL_COMMAND;
  }

  incomingMessageOptions |= ((messageContents[0] & ZCL_FRAME_CONTROL_DIRECTION_MASK)
                             ? SL_ZIGBEE_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT
                             : SL_ZIGBEE_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER);
  if (messageContents[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) {
    if (messageLength < SL_ZIGBEE_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD) {
      sl_zigbee_af_app_println("%smessage too short!", "ERR: Inter-PAN ");
      return false;
    }
    commandId = messageContents[4];
  } else {
    commandId = messageContents[2];
  }

#if defined (ALLOW_FRAGMENTATION)
  // If interpan fragmentation is enabled, we allow all messages of
  // SE profile ID, appropriate control byte value, and appropriate lengths
  if ((headerData->profileId == SE_PROFILE_ID)
      && isInterpanFragment(messageContents, messageLength)) {
    return true;
  }
#endif // ALLOW_FRAGMENTATION

  uint8_t messages_count = sizeof(messages) / sizeof(messages[0]);
  for (uint8_t i = 0; i < messages_count; i++) {
    if (messages[i].profileId == 0xFFFF) {
      break;
    }
    if (messages[i].profileId == headerData->profileId
        && messages[i].clusterId == headerData->clusterId
        && messages[i].commandId == commandId
        // Here we only check if the direction is allowed
        && (messages[i].options & incomingMessageOptions)) {
      return true;
    }
  }

  sl_zigbee_af_app_println("%sprofile 0x%04X, cluster 0x%04X, command 0x%02X not permitted",
                           "ERR: Inter-PAN ",
                           headerData->profileId,
                           headerData->clusterId,
                           commandId);
  return false;
}

#if ((defined(EMBER_AF_PRINT_ENABLE) && defined(SL_ZIGBEE_AF_PRINT_APP)) || (defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT)))

static void printMessage(sl_zigbee_mac_passthrough_type_t passthroughType, sl_zigbee_af_interpan_header_t *headerData)
{
  if (passthroughType & SL_802154_PASSTHROUGH_INTERNAL_ZLL) {
    sl_zigbee_af_app_print("RX ZLL inter-PAN message (");
  } else {
    sl_zigbee_af_app_print("RX inter-PAN message (");
  }
  if (headerData->messageType == SL_ZIGBEE_AF_INTER_PAN_UNICAST) {
    sl_zigbee_af_app_print("uni");
  } else if (headerData->messageType == SL_ZIGBEE_AF_INTER_PAN_BROADCAST) {
    sl_zigbee_af_app_print("broad");
  } else if (headerData->messageType == SL_ZIGBEE_AF_INTER_PAN_MULTICAST) {
    sl_zigbee_af_app_print("multi");
  }
  sl_zigbee_af_app_println("cast):");
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_println("  src pan id: 0x%04X", headerData->panId);
  if (headerData->options & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS) {
    sl_zigbee_af_app_print("  src long id: ");
    sl_zigbee_af_print_big_endian_eui64(headerData->longAddress);
    sl_zigbee_af_app_println("");
  } else {
    sl_zigbee_af_app_println("  src short id: 0x%04X", headerData->shortAddress);
  }
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_println("  profile id: 0x%04X", headerData->profileId);
  sl_zigbee_af_app_print("  cluster id: 0x%04X ", headerData->clusterId);
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(headerData->clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_app_println("");
  if (headerData->messageType == SL_ZIGBEE_AF_INTER_PAN_MULTICAST) {
    sl_zigbee_af_app_println("  group id: 0x%04X", headerData->groupId);
  }
  sl_zigbee_af_app_flush();
}

#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(SL_ZIGBEE_AF_PRINT_APP)

sl_status_t sl_zigbee_af_interpan_send_message_cb(sl_zigbee_af_interpan_header_t* header,
                                                  uint16_t messageLength,
                                                  uint8_t* messageBytes)
{
  sl_status_t status;
  uint8_t message[SL_ZIGBEE_AF_MAXIMUM_INTERPAN_LENGTH];
  uint16_t maxLen;
  uint8_t headerSize = SL_ZIGBEE_AF_INTER_PAN_UNICAST == header->messageType
                       ? INTERPAN_UNICAST_HEADER_SIZE
                       : INTERPAN_MULTICAST_HEADER_SIZE;

  if (!gInterpanEnabled) {
    return SL_STATUS_INVALID_STATE;
  }

  if ((headerSize + messageLength) <= SL_ZIGBEE_AF_MAXIMUM_INTERPAN_LENGTH) {
    status = makeInterPanMessage(header,
                                 message,
                                 SL_ZIGBEE_AF_MAXIMUM_INTERPAN_LENGTH,
                                 messageBytes,
                                 &messageLength,
                                 &maxLen);
    if (status == SL_STATUS_OK) {
      status = sli_zigbee_af_interpan_send_raw_message((uint8_t)maxLen, message);
    }
  }
#if defined(ALLOW_FRAGMENTATION)
  else if ((headerSize + messageLength)
           <= SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE) {
    if (SL_ZIGBEE_AF_INTER_PAN_UNICAST != header->messageType) {
      return SL_STATUS_INVALID_STATE;
    }

    status = interpanFragmentationSendUnicast(header,
                                              messageBytes,
                                              messageLength);
  }
#endif // ALLOW_FRAGMENTATION
  else {
    return SL_STATUS_MESSAGE_TOO_LONG;
  }

  if (status != SL_STATUS_OK) {
    return status;
  }

  sl_zigbee_af_app_print("T%08X:Inter-PAN TX (%d) [",
                         sl_zigbee_af_get_current_time(),
                         messageLength);
  sl_zigbee_af_app_print_buffer(messageBytes, messageLength, true);
  sl_zigbee_af_app_println("], 0x%02X", status);
  sl_zigbee_af_app_flush();

  return status;
}

#if defined(ALLOW_FRAGMENTATION)

// This function checks if packet is an inter-PAN fragment. It must do length
// checks as well since ZCL default responses come in with APS payload byte 1
// 0x00, which is also an inter-PAN IPMF control byte value.
static bool isInterpanFragment(uint8_t* payload, uint8_t payloadLen)
{
  if (!payload || (payloadLen < SL_ZIGBEE_APS_INTERPAN_FRAGMENT_MIN_LEN)) {
    return false;
  }

  switch (payload[INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_INDEX]) {
    case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_VAL:
    {
      if ((payloadLen >= SL_ZIGBEE_APS_INTERPAN_FRAGMENTATION_OVERHEAD)
          && (payload[INTERPAN_FRAGMENTATION_APS_LEN_IPMF_INDEX]
              == (payloadLen - SL_ZIGBEE_APS_INTERPAN_FRAGMENTATION_OVERHEAD))) {
        return true;
      }
    }
    break;
    case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_RESPONSE_VAL:
    {
      if (SL_ZIGBEE_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN == payloadLen) {
        return true;
      }
    }
    break;
    default:
      break;
  }

  return false;
}

// Helper function to find a free buffer used to store a message sent via frags
static sli_zigbee_tx_fragmented_interpan_packet_t* getFreeTxPacketEntry(sl_802154_long_addr_t destEui)
{
  uint8_t i;

  if (!destEui) {
    return NULL;
  }

  // Interpan fragmentation cannot handle sending mutliple fragmented
  // transmissions to the same partner due to the lack of APS ACKS with sequence
  // numbers. Ensure that we're not in the middle of sending to partner already
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    sli_zigbee_tx_fragmented_interpan_packet_t *txPacket = &(txPackets[i]);
    if (txPacket->messageType != UNUSED_TX_PACKET_ENTRY) {
      if (!memcmp(txPacket->destEui, destEui, EUI64_SIZE)) {
        return NULL;
      }
    }
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    sli_zigbee_tx_fragmented_interpan_packet_t *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      memcpy(txPacket->destEui, destEui, EUI64_SIZE);
      return txPacket;
    }
  }
  return NULL;
}

// Helper function to free a buffer used to store a message sent via frags
static void freeTxPacketEntry(sli_zigbee_tx_fragmented_interpan_packet_t *txPacket, sli_zigbee_e_interpan_fragmentation_status_t reason)
{
  if (!txPacket) {
    return;
  }
  txPacket->messageType = UNUSED_TX_PACKET_ENTRY;

  if (IPMF_SUCCESS != reason) {
    // User callback: something went wrong
    sl_zigbee_af_interpan_fragment_transmission_failed_cb(reason,
                                                          txPacket->fragmentNum);
  }
}

// Helper function to find an existing buffer used to store a msg sent via frags
static sli_zigbee_tx_fragmented_interpan_packet_t* txPacketLookUp(sl_802154_long_addr_t destEui)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    sli_zigbee_tx_fragmented_interpan_packet_t *txPacket = &(txPackets[i]);
    if (!memcmp(txPacket->destEui, destEui, EUI64_SIZE)) {
      return txPacket;
    }
  }
  return NULL;
}

// Helper function to find a free buffer used to store a message rcvd via frags
static sli_zigbee_rx_fragmented_interpan_packet_t* getFreeRxPacketEntry(sl_802154_long_addr_t sourceEui)
{
  uint8_t i;
  sl_802154_long_addr_t nullEui = { 0 };
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    sli_zigbee_rx_fragmented_interpan_packet_t *rxPacket = &(rxPackets[i]);
    if (!memcmp(rxPacket->sourceEui, nullEui, EUI64_SIZE)) {
      memcpy(rxPacket->sourceEui, sourceEui, EUI64_SIZE);
      return rxPacket;
    }
  }
  return NULL;
}

// Helper function to free a buffer used to store a message received via frags
static void freeRxPacketEntry(sli_zigbee_rx_fragmented_interpan_packet_t *rxPacket, sli_zigbee_e_interpan_fragmentation_status_t reason)
{
  if (!rxPacket) {
    return;
  }
  memset(rxPacket->sourceEui, 0, EUI64_SIZE);

  if (IPMF_SUCCESS != reason) {
    // User callback: something went wrong
    sl_zigbee_af_interpan_fragment_transmission_failed_cb(reason,
                                                          rxPacket->lastFragmentNumReceived + 1);
  }
}

// Helper function to find an existing buffer used to store a msg rcvd via frags
static sli_zigbee_rx_fragmented_interpan_packet_t* rxPacketLookUp(sl_802154_long_addr_t sourceEui)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    sli_zigbee_rx_fragmented_interpan_packet_t *rxPacket = &(rxPackets[i]);
    if (!memcmp(rxPacket->sourceEui, sourceEui, EUI64_SIZE)) {
      return rxPacket;
    }
  }
  return NULL;
}

// This function sends a whole message. It takes care of the fragmenting.
static sl_status_t interpanFragmentationSendUnicast(sl_zigbee_af_interpan_header_t* header,
                                                    uint8_t* message,
                                                    uint16_t messageLen)
{
  sli_zigbee_tx_fragmented_interpan_packet_t *txPacket;
  sl_status_t status;
  uint8_t headerLen = INTERPAN_UNICAST_HEADER_SIZE;
  uint8_t maxFragLen, numFragments;

  if (!header || !message || (0 == messageLen)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (messageLen > SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE) {
    return SL_STATUS_MESSAGE_TOO_LONG;
  }

  // Claim a tx packet buffer. Only 1 tx packet per partner.
  txPacket = getFreeTxPacketEntry(header->longAddress);
  if (txPacket == NULL) {
    return SL_STATUS_ZIGBEE_MAX_MESSAGE_LIMIT_REACHED;
  }

  // No interpan frag APS encryption allowed (payload assumed already encrypted)
  header->options &= ~SL_ZIGBEE_AF_INTERPAN_OPTION_APS_ENCRYPT;

  // Build a big packet
  status = makeInterPanMessage(header,
                               txPacket->buffer,
                               SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE,
                               message,
                               &messageLen,
                               &(txPacket->bufLen));
  if (SL_STATUS_OK != status) {
    return status;
  }

  maxFragLen = SL_ZIGBEE_AF_MAXIMUM_INTERPAN_LENGTH
               - headerLen
               - SL_ZIGBEE_APS_INTERPAN_FRAGMENTATION_OVERHEAD;

  numFragments = messageLen / maxFragLen;
  if (messageLen % maxFragLen) {
    numFragments++;
  }

  // Fill in some info before we start the transmit process
  txPacket->messageType    = header->messageType;
  txPacket->fragmentMaxLen = maxFragLen;
  txPacket->numFragments   = numFragments;
  txPacket->fragmentNum    = 0;

  // Forward it to the frag-sending function
  status = interpanFragmentationSendIpmf(txPacket);

  return status;
}

// This function sends an individual fragment. It expects the caller to handle
// the setting up and handling of txPacket metadata.
static sl_status_t interpanFragmentationSendIpmf(sli_zigbee_tx_fragmented_interpan_packet_t *txPacket)
{
  if (!txPacket || !txPacket->bufLen) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  uint8_t  message[SL_ZIGBEE_AF_MAXIMUM_INTERPAN_LENGTH];
  uint8_t* finger = message;
  uint8_t  headerLen = SL_ZIGBEE_AF_INTER_PAN_UNICAST == txPacket->messageType
                       ? INTERPAN_UNICAST_HEADER_SIZE
                       : INTERPAN_MULTICAST_HEADER_SIZE;
  uint16_t messageSentLen, messageLeftLen;
  uint8_t  messageToBeSentLen, packetLen;

  // Copy header info from pre-built packet
  memmove(finger, txPacket->buffer, headerLen);
  finger += headerLen;

  // Write the IPMF payload with a fragment of the whole message
  messageSentLen = txPacket->fragmentMaxLen * txPacket->fragmentNum;
  messageLeftLen = txPacket->bufLen - headerLen - messageSentLen;
  messageToBeSentLen =  messageLeftLen >= txPacket->fragmentMaxLen
                       ? txPacket->fragmentMaxLen : messageLeftLen;

  *finger++ = INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_VAL;
  *finger++ = txPacket->fragmentNum;
  *finger++ = txPacket->numFragments;
  *finger++ = messageToBeSentLen;
  memmove(finger,
          (txPacket->buffer + headerLen + messageSentLen),
          messageToBeSentLen);
  finger += messageToBeSentLen;

  packetLen = finger - message;

  sl_zigbee_af_app_println("Sending interpan fragment %d of %d",
                           txPacket->fragmentNum, txPacket->numFragments - 1);

  status = sli_zigbee_af_interpan_send_raw_message(packetLen, message);

  // Start a timer waiting for the IPMF response
  sl_zigbee_af_event_set_delay_ms(txPacket->event, gMessageTimeout);

  return status;
}

// This function reads an IPMF message and stores necessary data. The caller
// will need to send an IPMF response based on the return from this function.
static sl_status_t interpanFragmentationProcessIpmf(sl_zigbee_af_interpan_header_t header,
                                                    uint8_t *message, uint8_t messageLen)
{
  uint8_t* finger;
  uint8_t fragNum, numFrags, fragLen, headerLen = INTERPAN_UNICAST_HEADER_SIZE;
  sli_zigbee_rx_fragmented_interpan_packet_t *rxPacket;

  if (!message || !messageLen) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  finger = message + headerLen;
  finger++;   // Skip the control byte
  fragNum  = *finger++;
  numFrags = *finger++;
  fragLen  = *finger++;

  sl_zigbee_af_app_println("Receiving inter-PAN fragment %d of %d", fragNum, numFrags - 1);

  // See if we already have an entry for this fragment transmission
  rxPacket = rxPacketLookUp(header.longAddress);
  if (!rxPacket && (0 != fragNum)) {
    sl_zigbee_af_app_println("ERR: expected first IPMF index 0, got %d", fragNum);
    return SL_STATUS_INVALID_INDEX;
  } else if (rxPacket && (0 == fragNum)) {
    sl_zigbee_af_app_print("ERR: ");
    sl_zigbee_af_print_little_endian_eui64(header.longAddress);
    sl_zigbee_af_app_println(" sending multiple IPMF index 0 fragments");
    freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
    return SL_STATUS_INVALID_INDEX;
  }

  // If no entry, see if we have room to take a buffer
  if (NULL == rxPacket) {
    rxPacket = getFreeRxPacketEntry(header.longAddress);
    if (rxPacket) {
      // If we've allocated a new buffer, copy the header over and set data
      memmove(rxPacket->buffer, message, headerLen);
      rxPacket->bufLen                  = headerLen;
      rxPacket->numFragments            = numFrags;
      rxPacket->lastFragmentNumReceived = fragNum;
    }
  }

  if (NULL == rxPacket) {
    sl_zigbee_af_app_print("ERR: RX interpan fragment from ");
    sl_zigbee_af_print_little_endian_eui64(header.longAddress);
    sl_zigbee_af_app_println(" but no available buffers");
    return SL_STATUS_ZIGBEE_MAX_MESSAGE_LIMIT_REACHED;
  }

  // We received an IPMF, so turn off the timer for now
  sl_zigbee_af_event_set_inactive(rxPacket->event);

  if (fragLen == 0) {
    sl_zigbee_af_app_println("ERR: RX interpan fragment from ");
    sl_zigbee_af_print_little_endian_eui64(header.longAddress);
    sl_zigbee_af_app_println(" with IPMF length 0");
    freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
    return SL_STATUS_INVALID_INDEX;
  }

  if (0 != fragNum) {
    if (rxPacket->lastFragmentNumReceived != (fragNum - 1)) {
      sl_zigbee_af_app_println("ERR: RX IPMF fragment %d after %d, expecting %d",
                               fragNum, rxPacket->lastFragmentNumReceived,
                               rxPacket->lastFragmentNumReceived + 1);
      freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
      return SL_STATUS_INVALID_INDEX;
    }
    if (rxPacket->numFragments != numFrags) {
      sl_zigbee_af_app_println("ERR: expecting total number of IPMFs %d, but received"
                               " %d in IPMF fragment %d",
                               rxPacket->numFragments, numFrags, fragNum);
      freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
      return SL_STATUS_INVALID_INDEX;
    }
    // Note that we allow sequential fragments to specify different lengths.
    // In theory, frags 0 to n-1 should be of the same maximum length, and the
    // last fragment n should be of shorter length. For now, we'll allow that
    // any of those middle packets may come in with a different length.
  }

  // Strip the payload and add it to the whole message buffer
  memmove(rxPacket->buffer + rxPacket->bufLen, finger, fragLen);
  rxPacket->bufLen += fragLen;
  rxPacket->lastFragmentNumReceived = fragNum;

  if (rxPacket->lastFragmentNumReceived == (rxPacket->numFragments - 1)) {
    // Full message received
    sl_zigbee_af_app_print("T%08X:Inter-PAN RX (%d B, %d fragments) [",
                           sl_zigbee_af_get_current_time(),
                           rxPacket->bufLen - headerLen,
                           rxPacket->numFragments);
    sl_zigbee_af_app_print_buffer(rxPacket->buffer + headerLen,
                                  rxPacket->bufLen - headerLen,
                                  true);
    sl_zigbee_af_app_println("], 0x%02X", SL_STATUS_OK);
    sl_zigbee_af_app_flush();

    // User callback: all frags received, message reconstructed
    sl_zigbee_af_interpan_message_received_over_fragments_cb(
      &header,
      rxPacket->bufLen - headerLen,
      rxPacket->buffer + headerLen);

    freeRxPacketEntry(rxPacket, IPMF_SUCCESS);
  }

  return SL_STATUS_OK;
}

// This function sends an IPMF response to the sender of the IPMF
static void interpanFragmentationSendIpmfResponse(sl_zigbee_af_interpan_header_t header,
                                                  uint8_t fragNum, uint8_t response)
{
  sl_status_t status;
  uint8_t message[SL_ZIGBEE_AF_MAXIMUM_INTERPAN_LENGTH];
  uint8_t* finger;
  uint16_t messageLength, noPayloadLen = 0;
  uint8_t noPayload = 0;

  if ((INTERPAN_IPMF_RESPONSE_SUCCESS != response)
      && (INTERPAN_IPMF_RESPONSE_FAILURE != response)) {
    return;
  }

  // Construct a new IPMF message, but no payload
  status = makeInterPanMessage(&header,
                               message,
                               SL_ZIGBEE_AF_MAXIMUM_INTERPAN_LENGTH,
                               &noPayload,
                               &noPayloadLen,
                               &messageLength);
  if (SL_STATUS_OK != status) {
    return;
  }

  // Move past the header and write the IPMF response payload
  finger = message + messageLength;
  *finger++ = INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_RESPONSE_VAL;
  *finger++ = fragNum;
  *finger++ = response;

  messageLength += SL_ZIGBEE_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN;

  // Send the response
  sli_zigbee_af_interpan_send_raw_message(messageLength, message);

  // Start a timer waiting for the next IPMF
  sli_zigbee_rx_fragmented_interpan_packet_t* rxPacket = rxPacketLookUp(header.longAddress);
  if (rxPacket) {
    sl_zigbee_af_event_set_delay_ms(rxPacket->event, gMessageTimeout);
  }
}

// This function processes the IPMF response and either sends the next fragment
// or finishes the transmission
static void interpanFragmentationProcessIpmfResponse(sl_zigbee_af_interpan_header_t header,
                                                     uint8_t* payload,
                                                     uint8_t payloadLen)
{
  if (!payload
      || (SL_ZIGBEE_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN != payloadLen)) {
    return;
  }

  uint8_t fragmentNumAcked = payload[INTERPAN_FRAGMENTATION_APS_INDEX_IPMF_INDEX];
  uint8_t response = payload[INTERPAN_FRAGMENTATION_APS_IPMF_RESPONSE_INDEX];

  sli_zigbee_tx_fragmented_interpan_packet_t* txPacket = txPacketLookUp(header.longAddress);
  if (!txPacket) {
    return;
  }

  // We received an IPMF, so turn off the timer for now
  sl_zigbee_af_event_set_inactive(txPacket->event);

  if (fragmentNumAcked != txPacket->fragmentNum) {
    // Partner is not acking what we've sent it - error
    freeTxPacketEntry(txPacket, IPMF_TX_BAD_RESPONSE);
    return;
  }

  if (INTERPAN_IPMF_RESPONSE_SUCCESS != response) {
    // Partner acking bad response - error
    freeTxPacketEntry(txPacket, IPMF_TX_BAD_RESPONSE);
    return;
  }

  if (txPacket->fragmentNum == (txPacket->numFragments - 1)) {
    // Done!
    freeTxPacketEntry(txPacket, IPMF_SUCCESS);
    return;
  }

  txPacket->fragmentNum++;
  interpanFragmentationSendIpmf(txPacket);
}

// This function is called when an inter-PAN fragment transmission fails to
// receive a response within the allotted time
void sl_zigbee_af_interpan_fragment_transmit_event_handler(sl_zigbee_af_event_t * event)
{
  sl_zigbee_af_event_set_inactive(event);

  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    sli_zigbee_tx_fragmented_interpan_packet_t *txPacket = &(txPackets[i]);
    if (txPacket->event == event) {
      sl_zigbee_af_app_println("ERR: TX inter-PAN: fragment %d of %d has timed out "
                               "waiting for a fragment response",
                               txPacket->fragmentNum, txPacket->numFragments - 1);
      freeTxPacketEntry(txPacket, IPMF_TX_TIMEOUT);
    }
  }
}

// This function is called when an inter-PAN fragment reception fails to receive
// the next fragment within the allotted time
void sl_zigbee_af_interpan_fragment_receive_event_handler(sl_zigbee_af_event_t * event)
{
  sl_zigbee_af_event_set_inactive(event);

  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    sli_zigbee_rx_fragmented_interpan_packet_t *rxPacket = &(rxPackets[i]);
    if (rxPacket->event == event) {
      sl_zigbee_af_app_println("ERR: RX inter-PAN: fragment %d of %d has timed out "
                               "waiting for the next fragment",
                               rxPacket->lastFragmentNumReceived,
                               rxPacket->numFragments - 1);
      freeRxPacketEntry(rxPacket, IPMF_RX_TIMEOUT);
    }
  }
}

#endif // ALLOW_FRAGMENTATION

//------------------------------------------------------------------------------
// Weak callback definitions.

WEAK(bool sl_zigbee_af_interpan_pre_message_received_cb(const sl_zigbee_af_interpan_header_t *header,
                                                        uint8_t msgLen,
                                                        uint8_t *message))
{
  (void)header;
  (void)msgLen;
  (void)message;
  return false;
}
