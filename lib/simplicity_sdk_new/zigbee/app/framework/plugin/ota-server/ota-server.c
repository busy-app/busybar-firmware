/***************************************************************************//**
 * @file
 * @brief Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading specific file.
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
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "ota-server.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "ota-server-dynamic-block-period.h"
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"

#include "zap-cluster-command-parser.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_SERVER_PRESENT
#include "app/framework/plugin/sub-ghz-server/sub-ghz-server.h"
#endif

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

extern uint8_t appDefaultResponseStatus;

#define LAST_MESSAGE_ID ZCL_QUERY_SPECIFIC_FILE_RESPONSE_COMMAND_ID

uint8_t otaServerEndpoint = 0;  // invalid endpoint

#define QUERY_NEXT_IMAGE_HW_VER_PRESENT_MASK  0x01

// this mask is the same for both Image block request and image page request
#define IMAGE_REQUEST_IEEE_PRESENT_MASK       0x01

// This determines the maximum amount of size the server can handle in one request.
// Normally the protocol does not use Zigbee's fragmentation and thus puts as
// much data as possible in a single message.  We have to size the response
// due to static data structures that need to know the limit.
// 63 bytes is the maximum amount that can be used without APS encryption.
// The server will automatically shrink the size of the response further based on
// the other send options (e.g. APS encryption or source routing).
#define MAX_POSSIBLE_SERVER_BLOCK_SIZE 63

#if defined(MIN_BLOCK_REQUEST_SUPPORT)
  #define MIN_BLOCK_REQUEST_SERVER_SUPPORT SL_ZIGBEE_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_SERVER
#else
  #define MIN_BLOCK_REQUEST_SERVER_SUPPORT 0
#endif

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

static bool commandParse(sl_zigbee_af_cluster_command_t* command);

#define prepareClusterResponse(commandId, status)        \
  sli_zigbee_af_ota_server_prepare_response(false,       \
                                            (commandId), \
                                            (status),    \
                                            0) // defaultResponsePayloadCommandId

#define prepareDefaultResponse(status, commandId)                            \
  sli_zigbee_af_ota_server_prepare_response(true,                            \
                                            ZCL_DEFAULT_RESPONSE_COMMAND_ID, \
                                            (status),                        \
                                            (commandId))

static void addEmberAfOtaImageIdIntoResponse(const sl_zigbee_af_ota_image_id_t* id);

// -------------------------------------------------------
// OTA Server Init and Tick functions
// -------------------------------------------------------

void sl_zigbee_af_ota_bootload_cluster_server_init_cb(uint8_t endpoint)
{
  sl_zigbee_af_ota_storage_init_cb();
  otaServerEndpoint = endpoint;
  sli_zigbee_af_ota_server_dynamic_block_period_init();
}

void sl_zigbee_af_ota_bootload_cluster_server_tick_cb(uint8_t endpoint)
{
  sli_zigbee_af_ota_page_request_tick(endpoint);
}

// This tick is endpointless and declared differently in plugin.properties
void sl_zigbee_af_ota_server_tick(void)
{
  sli_zigbee_af_ota_server_dynamic_block_period_tick();
}

// -------------------------------------------------------
// OTA Server Handler functions
// -------------------------------------------------------
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_ota_server_incoming_message_raw_cb(sl_zigbee_af_cluster_command_t* command)
{
  sl_status_t status;
  if (!commandParse(command)) {
    sl_zigbee_af_ota_bootload_cluster_println("ClusterError: failed parsing cmd 0x%02X",
                                              command->commandId);
    sl_zigbee_af_ota_bootload_cluster_flush();
    status = sl_zigbee_af_send_default_response(command, SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD);
  } else {
    // Avoid sending duplicate image block responses if it is an ota page request
    // The page request responses are also handled by sendBlockRequest() in
    // ota-server-page-request.c, we don't sendResponse() again here.
    if (command->commandId == ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID
        && sl_zigbee_af_ota_page_request_server_policy_cb() == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
    status = sl_zigbee_af_send_response();
  }

  if (SL_STATUS_OK != status) {
    sl_zigbee_af_ota_bootload_cluster_println("OTA: failed sending response to cmd 0x%02X:"
                                              " error 0x%02X",
                                              command->commandId,
                                              status);
  }

  // Always return true to indicate we processed the message.
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

static uint8_t queryNextImageRequestHandler(const sl_zigbee_af_ota_image_id_t* currentImageId,
                                            uint16_t* hardwareVersion)
{
  uint8_t status;
  sl_zigbee_af_ota_image_id_t upgradeImageId;

  status = sl_zigbee_af_ota_server_query_cb(currentImageId,
                                            hardwareVersion,
                                            &upgradeImageId);

  prepareClusterResponse(ZCL_QUERY_NEXT_IMAGE_RESPONSE_COMMAND_ID,
                         status);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }

  addEmberAfOtaImageIdIntoResponse(&upgradeImageId);
  (void) sl_zigbee_af_put_int32u_in_resp(sl_zigbee_af_ota_storage_get_total_image_size_cb(&upgradeImageId));
  return status;
}

bool sl_zigbee_af_ota_server_send_image_notify_cb(sl_802154_short_addr_t dest,
                                                  uint8_t endpoint,
                                                  uint8_t payloadType,
                                                  uint8_t queryJitter,
                                                  const sl_zigbee_af_ota_image_id_t* id)
{
  sl_status_t status;

  // Clear the frame since we are initiating the conversation
  memset(&sl_zigbee_af_response_aps_frame, 0, sizeof(sl_zigbee_aps_frame_t));

  sl_zigbee_af_response_aps_frame.options = 0;
  sl_zigbee_af_response_destination = dest;
  sl_zigbee_af_response_aps_frame.destinationEndpoint = endpoint;
  prepareClusterResponse(ZCL_IMAGE_NOTIFY_COMMAND_ID,
                         payloadType);

  (void) sl_zigbee_af_put_int8u_in_resp(queryJitter);

  if (payloadType >= 1) {
    (void) sl_zigbee_af_put_int16u_in_resp(id->manufacturerId);
  }
  if (payloadType >= 2) {
    (void) sl_zigbee_af_put_int16u_in_resp(id->imageTypeId);
  }
  if (payloadType >= 3) {
    (void) sl_zigbee_af_put_int32u_in_resp(id->firmwareVersion);
  }

  if (dest >= SL_ZIGBEE_BROADCAST_ADDRESS) {
    status = sl_zigbee_af_send_broadcast(SL_ZIGBEE_NULL_NODE_ID,
                                         dest,
                                         0,
                                         &sl_zigbee_af_response_aps_frame,
                                         appResponseLength,
                                         appResponseData);
  } else {
    status = sl_zigbee_af_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                       dest,
                                       &sl_zigbee_af_response_aps_frame,
                                       appResponseLength,
                                       appResponseData);
  }
  return (status == SL_STATUS_OK);
}

static void printBlockRequestInfo(const sl_zigbee_af_ota_image_id_t* id,
                                  uint8_t maxDataSize,
                                  uint32_t offset)
{
  // To reduce the redundant data printed by the server, it will only print
  // a request for a different image than the last one.  To change this
  // behavior update the bool below.
  const bool printAllRequests = false;

  static sl_zigbee_af_ota_image_id_t lastImageId = INVALID_OTA_IMAGE_ID;

  if (!printAllRequests
      && (0 == memcmp(id, &lastImageId, sizeof(sl_zigbee_af_ota_image_id_t)))) {
    return;
  }
  memmove(&lastImageId, id, sizeof(sl_zigbee_af_ota_image_id_t));

  sl_zigbee_af_ota_bootload_cluster_println("NEW ImageBlockReq mfgId:%04X imageTypeId:%04X, file:%08X, maxDataSize:%d, offset:0x%08X",
                                            id->manufacturerId,
                                            id->imageTypeId,
                                            id->firmwareVersion,
                                            maxDataSize,
                                            offset);
  sl_zigbee_af_ota_server_update_started_cb(id->manufacturerId,
                                            id->imageTypeId,
                                            id->firmwareVersion,
                                            maxDataSize,
                                            offset);
}

// This function is made non-static for the Page request code
// It returns 0 on error, or the number of bytes sent on success.
uint8_t sli_zigbee_af_ota_image_block_request_handler(sl_zigbee_af_image_block_request_callback_struct_t* callbackData)
{
  uint8_t data[MAX_POSSIBLE_SERVER_BLOCK_SIZE];
  uint32_t actualLength;
  uint8_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint8_t serverBlockSize = sl_zigbee_af_ota_server_block_size_cb(callbackData->source);
#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_SERVER_PRESENT
  sl_zigbee_duty_cycle_state_t dcState;
#endif

  if (serverBlockSize > MAX_POSSIBLE_SERVER_BLOCK_SIZE) {
    serverBlockSize = MAX_POSSIBLE_SERVER_BLOCK_SIZE;
  }

  // Delay checks
  // We delay the OTA client if the following conditions happen, listed by order
  // 1. The duty cycle state has moved to Critical state (for SE1.4 subghz)
  // 2. We support a dynamically-treated Minimum Block Period, which means we
  //    can only support so many clients downloading at once. When we're at that
  //    max, we tell new clients to delay and come back later
  // 3. The OTA client indicates that it supports the Minimum Block Period, and
  //    its value differs from the value the server holds. We tell it to wait
  //    for data with the updated block period value
  // 4. Test code desires a delay

#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_SERVER_PRESENT
  // SE1.4 says that when in Critical Duty Cycle state... "If active, the OTA server
  // shall respond to any Image Block Request command with an Image Block Response
  // command with a status of WAIT_FOR_DATA."
  if (sl_zigbee_get_duty_cycle_state(&dcState) == SL_STATUS_OK
      && dcState >= SL_ZIGBEE_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED) {
    callbackData->waitTimeSecondsResponse = sl_zigbee_af_sub_ghz_server_suspend_zcl_messages_status(callbackData->source);
    status = SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA;
  } else
#endif
  status = sl_zigbee_af_ota_server_image_block_request_cb(callbackData);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    prepareClusterResponse(ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID,
                           status);
    if (status == SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA) {
      // Current time (0 = use relative time, not UTC)
      (void) sl_zigbee_af_put_int32u_in_resp(0);
      (void) sl_zigbee_af_put_int32u_in_resp(callbackData->waitTimeSecondsResponse);

#if defined(MIN_BLOCK_REQUEST_SUPPORT)
      // The min block request period is in milliseconds as defined in af-types.h
      // This attribute, BockRequestDelay, used to be in milliseconds
      // (09-5264-23, section 6.7.10)
      // It now (15-0324-02) is called MinimumBlockPeriod and is defined to be
      // in seconds (section 11.10.10)
      (void) sl_zigbee_af_put_int16u_in_resp(callbackData->minBlockRequestPeriod);
#endif

      // If this is a Page Request and we've told the client to delay, we can
      // abort this session. It has to come back with another Image Page Request
      // in the future anyway (NOTE: an Image Page Request gets refunneled into
      // this code as a simulated Image Block Request for processing purposes)
      if (callbackData->bitmask
          & SL_ZIGBEE_AF_IMAGE_BLOCK_REQUEST_SIMULATED_FROM_PAGE_REQUEST) {
        (void)sli_zigbee_af_ota_page_request_error_handler(); // only aborts current session
      }
    }
    return 0;
  }

  memset(data, 0, MAX_POSSIBLE_SERVER_BLOCK_SIZE);
  printBlockRequestInfo(callbackData->id,
                        callbackData->maxDataSize,
                        callbackData->offset);

  callbackData->maxDataSize = (callbackData->maxDataSize < serverBlockSize
                               ? callbackData->maxDataSize
                               : serverBlockSize);
  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
      != sl_zigbee_af_ota_storage_read_image_data_cb(callbackData->id,
                                                     callbackData->offset,
                                                     callbackData->maxDataSize,
                                                     data,
                                                     &actualLength)
      || actualLength == 0) {
    status = SL_ZIGBEE_ZCL_STATUS_NO_IMAGE_AVAILABLE;
    sl_zigbee_af_ota_server_update_complete_cb(callbackData->id->manufacturerId,
                                               callbackData->id->imageTypeId,
                                               callbackData->id->firmwareVersion,
                                               callbackData->source,
                                               status);
  }

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (!sli_zigbee_af_ota_page_request_error_handler()) {
      // If the page request code didn't handle the error (because this code
      // wasn't called due to a page request) then we send a normal
      // response.  We don't generate an error message because in that case
      // we were sending an unsolicited image block response.
      prepareDefaultResponse(status, ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID);
    }
    return 0;
  }

  prepareClusterResponse(ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID, status);
  addEmberAfOtaImageIdIntoResponse(callbackData->id);
  (void) sl_zigbee_af_put_int32u_in_resp(callbackData->offset);
  (void) sl_zigbee_af_put_int8u_in_resp((uint8_t)actualLength);
  sl_zigbee_af_put_block_in_resp(data, actualLength);
  sl_zigbee_af_set_command_endpoints(otaServerEndpoint, callbackData->clientEndpoint);

  sl_zigbee_af_ota_server_block_sent_cb((uint8_t)actualLength,
                                        callbackData->id->manufacturerId,
                                        callbackData->id->imageTypeId,
                                        callbackData->id->firmwareVersion);
  // We can't send more than 128 bytes in a packet so we can safely cast this
  // to a 1-byte number.
  return (uint8_t)actualLength;
}

static void constructUpgradeEndResponse(const sl_zigbee_af_ota_image_id_t* imageId,
                                        uint32_t upgradeTime)
{
  prepareClusterResponse(ZCL_UPGRADE_END_RESPONSE_COMMAND_ID,
                         0);      // status code (will ignore)

  appResponseLength--;  // The above function wrote an extra byte which we
                        // don't want because there is no status code for this
                        // message

  addEmberAfOtaImageIdIntoResponse(imageId);

  // We always use relative time.  There is no benefit in using
  // UTC time since the client has to support both.
  (void) sl_zigbee_af_put_int32u_in_resp(0);                   // current time
  (void) sl_zigbee_af_put_int32u_in_resp(upgradeTime);
}

static void upgradeEndRequestHandler(sl_802154_short_addr_t source,
                                     uint8_t status,
                                     const sl_zigbee_af_ota_image_id_t* imageId)
{
  uint32_t upgradeTime;
  bool goAhead;
  sl_zigbee_af_status_t defaultRespStatus = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  sl_zigbee_af_ota_bootload_cluster_println("RX UpgradeEndReq status:%02X",
                                            status);

  // This callback is considered only informative when the status
  // is a failure.
  goAhead = sl_zigbee_af_ota_server_upgrade_end_request_cb(source,
                                                           status,
                                                           &upgradeTime,
                                                           imageId);

  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (goAhead) {
      constructUpgradeEndResponse(imageId, upgradeTime);
      sl_zigbee_af_ota_server_update_complete_cb(imageId->manufacturerId,
                                                 imageId->imageTypeId,
                                                 imageId->firmwareVersion,
                                                 source,
                                                 status);
      return;
    } else {
      defaultRespStatus = SL_ZIGBEE_ZCL_STATUS_ABORT;
    }
  }

  prepareDefaultResponse(defaultRespStatus,
                         ZCL_UPGRADE_END_REQUEST_COMMAND_ID);
}

static void querySpecificFileRequestHandler(uint8_t* requestNodeAddress,
                                            const sl_zigbee_af_ota_image_id_t* imageId,
                                            uint16_t currentStackVersion)
{
  // Not supported yet.
  UNUSED_VAR(requestNodeAddress);
  UNUSED_VAR(imageId);
  UNUSED_VAR(currentStackVersion);

  prepareDefaultResponse(SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND,
                         ZCL_QUERY_SPECIFIC_FILE_REQUEST_COMMAND_ID);
}

void sli_zigbee_af_ota_server_prepare_response(bool useDefaultResponse,
                                               uint8_t commandId,
                                               uint8_t status,
                                               uint8_t defaultResponsePayloadCommandId)
{
  sl_zigbee_af_response_aps_frame.sourceEndpoint = otaServerEndpoint;
  appResponseLength = 0;
  sl_zigbee_af_response_aps_frame.clusterId = ZCL_OTA_BOOTLOAD_CLUSTER_ID;

  sl_zigbee_af_response_aps_frame.options =
    ((sli_zigbee_af_ota_server_handling_page_request()
      && commandId == ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID)
     ? SL_ZIGBEE_APS_OPTION_NONE
     : SL_ZIGBEE_APS_OPTION_RETRY);

  uint8_t disable_default_response_mask = ZCL_DISABLE_DEFAULT_RESPONSE_MASK;

  if ( ZCL_IMAGE_NOTIFY_COMMAND_ID == commandId ) {
    if ( sl_zigbee_af_response_destination < SL_ZIGBEE_BROADCAST_ADDRESS ) {
      //Optional - Not set if sent as unicast, set if sent as broadcast
      disable_default_response_mask = SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_RESPONSES;
    }
  }

  // Assume sl_zigbee_af_response_aps_frame.destinationEndpoint has already
  // been set based on the framework.  In most cases it is as simple
  // as swapping source and dest endpoints.

  (void) sl_zigbee_af_put_int8u_in_resp((useDefaultResponse
                                         ? ZCL_GLOBAL_COMMAND
                                         : (ZCL_CLUSTER_SPECIFIC_COMMAND
                                            | disable_default_response_mask))
                                        | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT);
  (void) sl_zigbee_af_put_int8u_in_resp(commandId == ZCL_IMAGE_NOTIFY_COMMAND_ID
                                        ? sl_zigbee_af_next_sequence()
                                        : sl_zigbee_af_incoming_zcl_sequence_number);

  (void) sl_zigbee_af_put_int8u_in_resp(commandId);
  if (useDefaultResponse) {
    (void) sl_zigbee_af_put_int8u_in_resp(defaultResponsePayloadCommandId);
  }
  (void) sl_zigbee_af_put_int8u_in_resp(status);

  // Assume the emberAfAppResponseDestination is already set.
}

static void addEmberAfOtaImageIdIntoResponse(const sl_zigbee_af_ota_image_id_t* id)
{
  (void) sl_zigbee_af_put_int16u_in_resp(id->manufacturerId);
  (void) sl_zigbee_af_put_int16u_in_resp(id->imageTypeId);
  (void) sl_zigbee_af_put_int32u_in_resp(id->firmwareVersion);
}

static bool commandParse(sl_zigbee_af_cluster_command_t* command)
{
  uint8_t commandId = command->commandId;
  uint8_t* buffer = command->buffer;
  uint8_t length = command->bufLen;
  uint8_t index = SL_ZIGBEE_AF_ZCL_OVERHEAD;
  sl_zigbee_af_ota_image_id_t imageId = INVALID_OTA_IMAGE_ID;

  if (commandId > LAST_MESSAGE_ID
      || (length
          < (SL_ZIGBEE_AF_ZCL_OVERHEAD + sli_zigbee_af_ota_min_message_lengths[commandId]))) {
    return false;
  }
  switch (commandId) {
    case ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID:
    case ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID:
    case ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID: {
      uint8_t fieldControl;
      uint32_t offset;
      uint8_t maxDataSize;
      uint16_t hardwareVersion;
      bool hardwareVersionPresent = false;

      fieldControl = sl_zigbee_af_get_int8u(buffer, index, length);
      index++;
      index += sli_zigbee_af_ota_parse_image_id_from_message(&imageId,
                                                             &(buffer[index]),
                                                             length);

      if (commandId == ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID) {
        if (fieldControl & QUERY_NEXT_IMAGE_HW_VER_PRESENT_MASK) {
          hardwareVersionPresent = true;
          hardwareVersion = sl_zigbee_af_get_int16u(buffer, index, length);
          index += 2;
        }

        queryNextImageRequestHandler(&imageId,
                                     (hardwareVersionPresent
                                      ? &hardwareVersion
                                      : NULL));
        return true;
      } // else // ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID
        //   or ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID

      offset = sl_zigbee_af_get_int32u(buffer, index, length);
      index += 4;
      maxDataSize = sl_zigbee_af_get_int8u(buffer, index, length);
      index++;

      if (commandId == ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID) {
        sl_zigbee_af_image_block_request_callback_struct_t callbackStruct;
        memset(&callbackStruct, 0, sizeof(sl_zigbee_af_image_block_request_callback_struct_t));

        if (fieldControl & OTA_FIELD_CONTROL_NODE_EUI64_PRESENT_BIT) {
          memcpy(callbackStruct.sourceEui, &(buffer[index]), EUI64_SIZE);
          index += EUI64_SIZE;
        }

        if (fieldControl & OTA_FIELD_CONTROL_MIN_BLOCK_REQUEST_PRESENT_BIT) {
          callbackStruct.minBlockRequestPeriod = sl_zigbee_af_get_int16u(buffer, index, length);
          callbackStruct.bitmask |= SL_ZIGBEE_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_CLIENT;
          index += 2;
        }

        callbackStruct.clientEndpoint = command->apsFrame->sourceEndpoint;
        callbackStruct.source = command->source;
        callbackStruct.id = &imageId;
        callbackStruct.offset = offset;
        callbackStruct.maxDataSize = maxDataSize;
        callbackStruct.bitmask |= MIN_BLOCK_REQUEST_SERVER_SUPPORT;

        sli_zigbee_af_ota_image_block_request_handler(&callbackStruct);
      } else { // ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID
        uint16_t pageSize;
        uint16_t responseSpacing;
        uint8_t status;

        pageSize = sl_zigbee_af_get_int16u(buffer, index, length);
        index += 2;

        responseSpacing = sl_zigbee_af_get_int16u(buffer, index, length);
        index += 2;

        status = sli_zigbee_af_ota_page_request_handler(command->apsFrame->sourceEndpoint,
                                                        otaServerEndpoint,
                                                        &imageId,
                                                        offset,
                                                        maxDataSize,
                                                        pageSize,
                                                        responseSpacing);

        if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
          prepareDefaultResponse(status,
                                 ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID);
        }
      }
      return true;
    }
    case ZCL_UPGRADE_END_REQUEST_COMMAND_ID: {
      uint8_t status = sl_zigbee_af_get_int8u(buffer, index, length);
      index++;
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        index += sli_zigbee_af_ota_parse_image_id_from_message(&imageId,
                                                               &(buffer[index]),
                                                               length);
      }
      upgradeEndRequestHandler(command->source, status, &imageId);
      return true;
    }
    case ZCL_QUERY_SPECIFIC_FILE_REQUEST_COMMAND_ID: {
      uint8_t* requestNodeAddress = &(buffer[index]);
      uint16_t currentStackVersion;
      index += 8;  // add 8 to jump over the requestNodeAddress
      index += sli_zigbee_af_ota_parse_image_id_from_message(&imageId,
                                                             &(buffer[index]),
                                                             length);
      currentStackVersion = sl_zigbee_af_get_int16u(buffer, index, length);
      index += 2;
      querySpecificFileRequestHandler(requestNodeAddress,
                                      &imageId,
                                      currentStackVersion);
      return true;
    }
  }
  return false;
}

void sl_zigbee_af_ota_server_send_upgrade_command_cb(sl_802154_short_addr_t dest,
                                                     uint8_t endpoint,
                                                     const sl_zigbee_af_ota_image_id_t* id)
{
  sl_status_t status;
  sl_zigbee_af_response_destination = dest;
  sl_zigbee_af_response_aps_frame.destinationEndpoint = endpoint;
  constructUpgradeEndResponse(id,
                              0);  // upgrade time (0 = now)
  status = sl_zigbee_af_send_response();
  if (SL_STATUS_OK != status) {
    sl_zigbee_af_ota_bootload_cluster_println("OTA: failed sending upgrade response: "
                                              "error 0x%02X", status);
  }
}

uint32_t sl_zigbee_af_ota_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                       sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = sl_zigbee_af_ota_server_incoming_message_raw_cb(cmd);

  return status;
}
