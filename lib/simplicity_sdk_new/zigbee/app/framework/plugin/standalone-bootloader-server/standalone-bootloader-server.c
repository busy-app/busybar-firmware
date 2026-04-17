/***************************************************************************//**
 * @file
 * @brief This file defines the server behavior for the Ember proprietary bootloader
 * protocol.
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
#include "app/framework/plugin/standalone-bootloader-common/bootloader-protocol.h"
#include "standalone-bootloader-server.h"
#include "app/framework/plugin/xmodem-sender/xmodem-sender.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#include "app/framework/plugin/ota-common/ota.h"

#include "standalone-bootloader-server-config.h"

//------------------------------------------------------------------------------
// Globals

static const sl_802154_long_addr_t broadcastEui64 = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const sl_zigbee_key_data_t bootloadKey = {
  SL_ZIGBEE_AF_PLUGIN_STANDALONE_BOOTLOADER_SERVER_ENCRYPTION_KEY,
};

typedef enum {
  SERVER_BOOTLOAD_STATE_NONE = 0,
  SERVER_BOOTLOAD_STATE_UNICAST_QUERY_SENT,
  SERVER_BOOTLOAD_STATE_LAUNCH_REQUEST_SENT,
  SERVER_BOOTLOAD_STATE_DELAY_BEFORE_CHECK_IF_BOOTLOADER_ACTIVE,
  SERVER_BOOTLOAD_STATE_CHECK_IF_BOOTLOADER_ACTIVE_SENT,
  SERVER_BOOTLOAD_STATE_SENDING_DATA,
} sli_zigbee_bootload_server_state_t;

static  const char * serverStateStrings[] = {
  "None",
  "Unicast Query Sent",
  "Launch Request Sent",
  "Delay before checking if bootloader active",
  "Check if Bootloader active sent",
  "Sending Data",
};

static sli_zigbee_bootload_server_state_t serverBootloadState = SERVER_BOOTLOAD_STATE_NONE;

// We don't make these configurable since they could effect the ability
// to work with bootloaders already in the field.  And bootloaders
// are not updated.
#define QUERY_TIMEOUT_SECONDS 2
#define LAUNCH_REQUEST_TIMEOUT_SECONDS 2
#define DELAY_BEFORE_CHECKING_IF_BOOTLOADER_ACTIVE_SECONDS 2
#define DELAY_BEFORE_SENDING_BOOTLOAD_DATA_SECONDS 2

sl_zigbee_af_event_t myEvent;
static void myEventHandler(sl_zigbee_af_event_t * event);

// We remember the last target client as an optimization.
// This way a CLI user can broadcast the query and then simply bootload the
// lone nearby client.
// However the application may tell any specific node to bootload via its
// long ID.  This performs a unicast query to obtain the current bootloader
// data just in case.
static sl_zigbee_af_standalone_bootloader_query_response_data_t targetClient;

// Since the max 802.15.4 packet size is 128 bytes, we can't fit a single
// Xmodem packet plus overhead in an over-the-air packet.  Split them in half.
#define OTA_XMODEM_MESSAGE_SIZE 64

static sl_zigbee_af_ota_image_id_t imageToBootload;
static uint32_t tagOffset;
static uint32_t tagSize;

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Functions

void sli_zigbee_af_standalone_bootloader_server_print_status(void)
{
  bootloadPrintln("Server State: %s", serverStateStrings[serverBootloadState]);
  bootloadPrint("Key Data: ");
  sl_zigbee_af_print_zigbee_key(sl_zigbee_key_contents(&bootloadKey));
}

static void resetServerState(bool success)
{
  sl_zigbee_af_standalone_bootloader_server_finished_cb(success);

  // We don't clear the targetClient struct so it can potentially be re-used
  // in a subsequent bootload.
  serverBootloadState = SERVER_BOOTLOAD_STATE_NONE;
  sl_zigbee_af_event_set_inactive(&myEvent);
}

static bool isTargetClientDataValid(void)
{
  return (0 != memcmp(broadcastEui64, targetClient.eui64, EUI64_SIZE));
}

void sl_zigbee_af_standalone_bootloader_server_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(&myEvent,
                              myEventHandler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      memset(&targetClient, 0xFF, sizeof(sl_zigbee_af_standalone_bootloader_query_response_data_t));
      break;
    }
  }
}

static sl_status_t sendQuery(const sl_802154_long_addr_t targetEui)
{
  sl_802154_long_addr_t eui64Copy;
  uint8_t outgoingBlock[MAX_BOOTLOAD_MESSAGE_SIZE];
  uint8_t length = sl_zigbee_af_standalone_bootloader_common_make_header(outgoingBlock, XMODEM_QUERY);
  bool isBroadcast = (0 == memcmp(broadcastEui64, targetEui, EUI64_SIZE));

  // We need to cast away the const to avoid compiler warning.
  memmove(eui64Copy, targetEui, EUI64_SIZE);

  return sl_zigbee_af_standalone_bootloader_common_send_message(isBroadcast,
                                                                eui64Copy,
                                                                length,
                                                                outgoingBlock);
}

// Query all nearby nodes for bootload info.
sl_status_t sl_zigbee_af_standalone_bootloader_server_broadcast_query(void)
{
  return sendQuery(broadcastEui64);
}

static void decodeAndPrintServerMessageType(uint8_t command)
{
  const bool printAcks = false;
  uint8_t id = 0xFF;
  const char * commandStrings[] = {
    "Query Response",
    "Auth Challenge",
    "SOH",
    "EOT",
    "ACK",
    "NAK",
    "CANCEL",
    "BLOCK_OK",
    "FILEDONE",
  };

  switch (command) {
    case (XMODEM_QRESP):
      id = 0;
      break;
    case (XMODEM_AUTH_CHALLENGE):
      id = 1;
      break;
    case (XMODEM_SOH):
      id = 2;
      break;
    case (XMODEM_EOT):
      id = 3;
      break;
    case (XMODEM_ACK):
      id = 4;
      break;
    case (XMODEM_NAK):
      id = 5;
      break;
    case (XMODEM_CANCEL):
      id = 6;
      break;
    case (XMODEM_BLOCKOK):
      id = 7;
      break;
    case (XMODEM_FILEDONE):
      id = 8;
      break;

    default:
      break;
  }
  if (printAcks || command != XMODEM_ACK) {
    bootloadPrintln("Received Standalone Bootloader message (%d): %s",
                    command,
                    (id == 0xFF
                     ? "??"
                     : commandStrings[id]));
  }
}

static void printTargetClientInfo(const char * prefixString)
{
  bootloadPrint("%s", prefixString);
  if (isTargetClientDataValid()) {
    sl_zigbee_af_print_big_endian_eui64(targetClient.eui64);
    bootloadPrintln("\n  Bootloader Active: %s",
                    (targetClient.bootloaderActive
                     ? "yes"
                     : "no"));
    bootloadPrintln("  MFG ID:       0x%04X", targetClient.mfgId);
    bootloadPrint("  Hardware Tag: ");
    sli_zigbee_af_standalone_bootloader_common_print_hardware_tag(targetClient.hardwareTag);
    bootloadPrintln("  Capabilities: 0x%02X", targetClient.capabilities);
    bootloadPrintln("  Platform:     0x%02X", targetClient.platform);
    bootloadPrintln("  Micro:        0x%02X", targetClient.micro);
    bootloadPrintln("  Phy:          0x%02X", targetClient.phy);
  } else {
    bootloadPrintln("-");
  }
}

void sli_zigbee_af_standalone_bootloader_server_print_target_client_info_command(void)
{
  printTargetClientInfo("Current target client: ");
}

static void sendLaunchRequest(void)
{
  uint8_t outgoingBlock[LAUNCH_REQUEST_LENGTH];
  uint8_t index = sl_zigbee_af_standalone_bootloader_common_make_header(outgoingBlock, XMODEM_LAUNCH_REQUEST);
  sl_status_t status;
  bootloadPrintln("Sending launch request to current target");
  outgoingBlock[index++] = LOW_BYTE(targetClient.mfgId);
  outgoingBlock[index++] = HIGH_BYTE(targetClient.mfgId);
  memmove(&(outgoingBlock[index]),
          targetClient.hardwareTag,
          SL_ZIGBEE_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH);
  index += SL_ZIGBEE_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH;

  status = sl_zigbee_af_standalone_bootloader_common_send_message(false, // is broadcast?
                                                                  targetClient.eui64,
                                                                  index,
                                                                  outgoingBlock);
  if (status == SL_STATUS_OK) {
    serverBootloadState = SERVER_BOOTLOAD_STATE_LAUNCH_REQUEST_SENT;
    sl_zigbee_af_event_set_delay_qs(&myEvent,
                                    LAUNCH_REQUEST_TIMEOUT_SECONDS << 2);
  } else {
    resetServerState(false);
  }
}

static sl_status_t xmodemSendRoutine(uint8_t* data, uint8_t length)
{
  uint8_t outgoingBlock[MAX_BOOTLOAD_MESSAGE_SIZE];
  uint8_t index = sl_zigbee_af_standalone_bootloader_common_make_header(outgoingBlock,
                                                                        1); // message type (ignored)

  // We ignore the type of message that we passed in the above function.
  // The Xmodem plugin already formatted the entire buffer for us.
  index--;

  memmove(&(outgoingBlock[index]), data, length);
  index += length;
  return sl_zigbee_af_standalone_bootloader_common_send_message(false,
                                                                targetClient.eui64,
                                                                index,
                                                                outgoingBlock);
}

static sl_status_t getNextBootloaderBlock(uint32_t address, // relative to 0
                                          uint8_t length,
                                          uint8_t* returnData,
                                          uint8_t* returnLength,
                                          bool* done)
{
  uint32_t realReturnLength;
  sl_zigbee_af_ota_storage_status_t status;

  if (address == 0) {
    sli_zigbee_af_print_percentage_set_start_and_end(0,    // starting offset
                                                     tagSize); // end offset
  }
  sli_zigbee_af_print_percentage_update("Xmodem Transfer",
                                        5, // update frequency (e.g. every 5%)
                                        address);

  status =
    sl_zigbee_af_ota_storage_read_image_data_cb(&imageToBootload,
                                                address + tagOffset,
                                                length,
                                                returnData,
                                                &realReturnLength);
  *returnLength = (uint8_t)realReturnLength;
  if (address + length > tagSize) {
    *done = true;
  } else {
    *done = false;
  }

  return (status == SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
          ? SL_STATUS_OK
          : SL_STATUS_FAIL);
}

static void xmodemComplete(bool success)
{
  bootloadPrintln("Xmodem transfer complete, status: %s", (success ? "Success" : "FAILED"));
  resetServerState(success);
}

static void startSendingXmodemData(void)
{
  sl_zigbee_af_event_set_inactive(&myEvent);
  serverBootloadState = SERVER_BOOTLOAD_STATE_SENDING_DATA;
  if (SL_STATUS_OK != sl_zigbee_af_xmodem_sender_start(xmodemSendRoutine,
                                                       getNextBootloaderBlock,
                                                       xmodemComplete,
                                                       OTA_XMODEM_MESSAGE_SIZE,
                                                       false)) { // wait for ready char?
    bootloadPrintln("Error: Could not start Xmodem transfer.  Aborting.");
    resetServerState(false);
  }
}

// Passed message includes overhead bytes
static void processQueryResponse(sl_802154_long_addr_t longId,
                                 uint8_t* message)
{
  memset(&(targetClient), 0, sizeof(sl_zigbee_af_standalone_bootloader_query_response_data_t));
  memmove(targetClient.eui64, longId, EUI64_SIZE);
  targetClient.bootloaderActive = message[QRESP_OFFSET_BL_ACTIVE];
  targetClient.mfgId = HIGH_LOW_TO_INT(message[QRESP_OFFSET_MFG_ID],
                                       message[QRESP_OFFSET_MFG_ID + 1]);
  memmove(targetClient.hardwareTag,
          &(message[QRESP_OFFSET_HARDWARE_TAG]),
          SL_ZIGBEE_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH);
  targetClient.capabilities = message[QRESP_OFFSET_BL_CAPS];
  targetClient.platform = message[QRESP_OFFSET_PLATFORM];
  targetClient.micro = message[QRESP_OFFSET_MICRO];
  targetClient.phy = message[QRESP_OFFSET_PHY];
  targetClient.bootloaderVersion = HIGH_LOW_TO_INT(message[QRESP_OFFSET_BL_VERSION],
                                                   message[QRESP_OFFSET_BL_VERSION + 1]);

  printTargetClientInfo("Received Query response from: ");
  sl_zigbee_af_standalone_bootloader_server_query_response_cb((serverBootloadState == SERVER_BOOTLOAD_STATE_NONE),  // broadcast?
                                                              &targetClient);

  if (serverBootloadState == SERVER_BOOTLOAD_STATE_UNICAST_QUERY_SENT) {
    // If the bootloader is already active in the device, skip to the chase and
    // start downloading new data to it.
    if (targetClient.bootloaderActive) {
      startSendingXmodemData();
    } else {
      sendLaunchRequest();
    }
  } else if (serverBootloadState == SERVER_BOOTLOAD_STATE_CHECK_IF_BOOTLOADER_ACTIVE_SENT) {
    if (targetClient.bootloaderActive) {
      startSendingXmodemData();
    } else {
      bootloadPrintln("Error: Bootloader of client not active.  Aborting.");
      resetServerState(false);
    }
  }
}

static void processChallenge(sl_802154_long_addr_t longId,
                             uint8_t* message)
{
  uint8_t outgoingBlock[XMODEM_AUTH_RESPONSE_LENGTH];
  sl_status_t status;
  uint8_t index = sl_zigbee_af_standalone_bootloader_common_make_header(outgoingBlock,
                                                                        XMODEM_AUTH_RESPONSE);

  if (message[XMODEM_AUTH_CHALLENGE_REQUEST_INDEX] != CHALLENGE_REQUEST_VERSION) {
    bootloadPrintln("Error: Challenge request version (%d) not the same as mine (%d)",
                    message[XMODEM_AUTH_CHALLENGE_REQUEST_INDEX],
                    CHALLENGE_REQUEST_VERSION);
    resetServerState(false);
    return;
  }

  memmove(&(outgoingBlock[index]),
          &(message[BOOTLOAD_MESSAGE_OVERHEAD]),
          BOOTLOAD_AUTH_CHALLENGE_SIZE);

  sli_zigbee_af_standalone_bootloader_client_encrypt(&(outgoingBlock[index]),
                                                     (uint8_t*)sl_zigbee_key_contents(&bootloadKey));

  index += BOOTLOAD_AUTH_RESPONSE_SIZE;
  status = sl_zigbee_af_standalone_bootloader_common_send_message(false,
                                                                  longId,
                                                                  index,
                                                                  outgoingBlock);
  if (status == SL_STATUS_OK) {
    serverBootloadState = SERVER_BOOTLOAD_STATE_DELAY_BEFORE_CHECK_IF_BOOTLOADER_ACTIVE;
    sl_zigbee_af_event_set_delay_qs(&myEvent,
                                    DELAY_BEFORE_SENDING_BOOTLOAD_DATA_SECONDS << 2);
  } else {
    resetServerState(false);
  }
}

sl_status_t sl_zigbee_af_standalone_bootloader_server_start_client_bootload_with_current_target(const sl_zigbee_af_ota_image_id_t* id,
                                                                                                uint16_t tag)
{
  if (!isTargetClientDataValid()) {
    bootloadPrintln("Error: No current target.");
    return SL_STATUS_FAIL;
  }

  return sl_zigbee_af_standalone_bootloader_server_start_client_bootload(targetClient.eui64,
                                                                         id,
                                                                         tag);
}

sl_status_t sl_zigbee_af_standalone_bootloader_server_start_client_bootload(sl_802154_long_addr_t longId,
                                                                            const sl_zigbee_af_ota_image_id_t* id,
                                                                            uint16_t tag)
{
  sl_status_t status;

  if (serverBootloadState != SERVER_BOOTLOAD_STATE_NONE) {
    return SL_STATUS_INVALID_STATE;
  }

  if (!sl_zigbee_af_is_ota_image_id_valid(id)) {
    bootloadPrintln("Error: Passed Image ID is invalid");
    return SL_STATUS_FAIL;
  }
  if (0 == sl_zigbee_af_ota_storage_get_total_image_size_cb(id)) {
    bootloadPrintln("Error:  Cannot find image ID within storage.");
    return SL_STATUS_INVALID_STATE;
  }
  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS != sli_zigbee_af_ota_storage_get_tag_offset_and_size(id,
                                                                                            tag,
                                                                                            &tagOffset,
                                                                                            &tagSize)) {
    bootloadPrintln("Error: Cannot find tag 0x%04X within passed image ID", tag);
    return SL_STATUS_INVALID_STATE;
  }

  memmove(&imageToBootload, id, sizeof(sl_zigbee_af_ota_image_id_t));

  status = sendQuery(longId);
  if (status == SL_STATUS_OK) {
    serverBootloadState = SERVER_BOOTLOAD_STATE_UNICAST_QUERY_SENT;
    sl_zigbee_af_event_set_delay_qs(&myEvent,
                                    QUERY_TIMEOUT_SECONDS << 2);
  } else {
    resetServerState(false);
  }
  return status;
}

static void myEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  sl_zigbee_af_event_set_inactive(&myEvent);
  if (serverBootloadState == SERVER_BOOTLOAD_STATE_UNICAST_QUERY_SENT) {
    bootloadPrint("Error: Timed out waiting for device to respond to bootloader query");
  } else if (serverBootloadState == SERVER_BOOTLOAD_STATE_DELAY_BEFORE_CHECK_IF_BOOTLOADER_ACTIVE) {
    bootloadPrintln("Checking if bootloader is now active on client");
    if (SL_STATUS_OK == sendQuery(targetClient.eui64)) {
      serverBootloadState = SERVER_BOOTLOAD_STATE_CHECK_IF_BOOTLOADER_ACTIVE_SENT;
      sl_zigbee_af_event_set_delay_qs(&myEvent,
                                      QUERY_TIMEOUT_SECONDS << 2);
    } else {
      bootloadPrintln("Error: Giving up bootload of client");
      resetServerState(false);
    }
    return;
  }
  serverBootloadState = SERVER_BOOTLOAD_STATE_NONE;
}

static bool validateSenderEui(sl_802154_long_addr_t longId)
{
  if (isTargetClientDataValid()) {
    return (0 == memcmp(longId, targetClient.eui64, EUI64_SIZE));
  }

  // If target data hasn't been, then we allow anyone to send us bootloader messages.
  // With the assumption is that the server's state will prevent unwanted messages
  // from triggering real behavior.
  return true;
}

bool sl_zigbee_af_standalone_bootloader_common_incoming_message_cb(sl_802154_long_addr_t longId,
                                                                   uint8_t length,
                                                                   uint8_t* message)
{
  if (!sl_zigbee_af_standalone_bootloader_common_check_incoming_message(length,
                                                                        message)) {
    return false;
  }

  if (!validateSenderEui(longId)) {
    bootloadPrintln("Error: Got incoming bootload message from invalid sender");
    return false;
  }

  decodeAndPrintServerMessageType(message[OFFSET_MESSAGE_TYPE]);

  switch (message[OFFSET_MESSAGE_TYPE]) {
    case (XMODEM_QRESP): {
      if (serverBootloadState <= SERVER_BOOTLOAD_STATE_UNICAST_QUERY_SENT
          || serverBootloadState == SERVER_BOOTLOAD_STATE_CHECK_IF_BOOTLOADER_ACTIVE_SENT) {
        if (length < QUERY_RESPONSE_LENGTH) {
          bootloadPrint("Error: Query response too short from: ");
          return true;
        }
        processQueryResponse(longId,
                             message);
      }
      break;
    }
    case (XMODEM_AUTH_CHALLENGE): {
      if (serverBootloadState == SERVER_BOOTLOAD_STATE_LAUNCH_REQUEST_SENT) {
        if (length < XMODEM_AUTH_CHALLENGE_LENGTH) {
          bootloadPrintln("Error: Bootloader Challenge too short (%d < %d)",
                          length,
                          XMODEM_AUTH_CHALLENGE_LENGTH);
        } else {
          processChallenge(longId,
                           message);
        }
      }
      break;
    }

    default:
      if (serverBootloadState == SERVER_BOOTLOAD_STATE_SENDING_DATA) {
        sl_zigbee_af_xmodem_sender_incoming_block(&(message[OFFSET_MESSAGE_TYPE]), length);
      }
      break;
  }
  return true;
}
