/***************************************************************************//**
 * @file
 * @brief Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading specific file.
 *
 * This handles the optional feature of a device requesting a
 * a full page (of EEPROM) and getting multiple image block responses.
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
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"

#if defined(PAGE_REQUEST_SUPPORT)

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#define MAXIMUM_PAGE_SIZE 1024

static sl_802154_short_addr_t requesterNodeId = SL_ZIGBEE_NULL_NODE_ID;
static uint32_t requesterBaseOffset;
static uint16_t requesterPageSize;
static uint16_t totalBytesSent;
static uint8_t requesterMaxDataSize;
static sl_zigbee_af_ota_image_id_t requesterImageId;
static bool handlingPageRequest = false;
static uint16_t requesterResponseSpacing;
static uint8_t requesterEndpoint = 0xFF;

#define SHORTEST_SEND_RATE 10L  // ms.

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

static void sendBlockRequest(void);
static void abortPageRequest(void);

#if defined(EM_AF_TEST_HARNESS_CODE)
  #define pageRequestTickCallback(x, y) \
  sli_zigbee_af_server_page_request_tick_callback(x, y)
#else
  #define pageRequestTickCallback(x, y) true
#endif

// -----------------------------------------------------------------------------

uint8_t sli_zigbee_af_ota_page_request_handler(uint8_t clientEndpoint,
                                               uint8_t serverEndpoint,
                                               const sl_zigbee_af_ota_image_id_t* id,
                                               uint32_t offset,
                                               uint8_t maxDataSize,
                                               uint16_t pageSize,
                                               uint16_t responseSpacing)
{
  uint32_t totalSize;
  uint8_t status;
  sl_zigbee_af_ota_bootload_cluster_println("RX ImagePageReq mfgId:%04X imageType:%04X, file:%08X, offset:%08X dataSize:%d pageSize%04X spacing:%d",
                                            id->manufacturerId,
                                            id->imageTypeId,
                                            id->firmwareVersion,
                                            offset,
                                            maxDataSize,
                                            pageSize,
                                            responseSpacing);

  // Only allow 1 page request at a time.
  if (requesterNodeId != SL_ZIGBEE_NULL_NODE_ID) {
    otaPrintln("2nd page request not supported");
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  status = sl_zigbee_af_ota_page_request_server_policy_cb();
  if (status) {
    return status;
  }

  memmove(&requesterImageId, id, sizeof(sl_zigbee_af_ota_image_id_t));
  totalSize = sl_zigbee_af_ota_storage_get_total_image_size_cb(id);

  if (totalSize == 0) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  } else if (offset > totalSize || (maxDataSize > pageSize)) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }

  requesterEndpoint = clientEndpoint;
  requesterNodeId = sl_zigbee_af_response_destination;
  requesterBaseOffset = offset;
  requesterPageSize = pageSize;
  requesterMaxDataSize = maxDataSize;
  requesterResponseSpacing = (responseSpacing < SHORTEST_SEND_RATE
                              ? SHORTEST_SEND_RATE
                              : responseSpacing);
  totalBytesSent = 0;

  sli_zigbee_af_ota_page_request_tick(serverEndpoint);

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

void sli_zigbee_af_ota_page_request_tick(uint8_t endpoint)
{
  if (requesterNodeId == SL_ZIGBEE_NULL_NODE_ID) {
    return;
  }

  sendBlockRequest();
  sl_zigbee_zcl_schedule_server_tick_extended(endpoint,
                                              ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                                              requesterResponseSpacing,
                                              SL_ZIGBEE_AF_SHORT_POLL,
                                              SL_ZIGBEE_AF_OK_TO_SLEEP);
}

bool sli_zigbee_af_ota_page_request_error_handler(void)
{
  if (handlingPageRequest) {
    abortPageRequest();
    return true;
  }
  return false;
}

static void abortPageRequest(void)
{
  requesterNodeId = SL_ZIGBEE_NULL_NODE_ID;
  requesterEndpoint = 0xFF;
}

static void sendBlockRequest(void)
{
  uint8_t bytesSentThisTime = 0;
  uint32_t totalSize = sl_zigbee_af_ota_storage_get_total_image_size_cb(&requesterImageId);
  uint8_t maxDataToSend;
  uint32_t bytesLeft;

  if (totalSize == 0) {
    // The image no longer exists.
    abortPageRequest();
    return;
  }

  bytesLeft = totalSize - (requesterBaseOffset + totalBytesSent);

  // 3 possibilities for how much data to send
  //   - Up to requesterMaxSize
  //   - As many bytes are left in the file
  //   - As many bytes are left to fill up client's page size
  if ((requesterPageSize - totalBytesSent) > requesterMaxDataSize) {
    maxDataToSend = (bytesLeft > requesterMaxDataSize
                     ? requesterMaxDataSize
                     : (uint8_t)bytesLeft);
  } else {
    maxDataToSend = requesterPageSize - totalBytesSent;
  }

  sl_zigbee_af_response_destination = requesterNodeId;
  handlingPageRequest = true;

  // To enable sending as fast as possible without the receiver
  // having to waste battery power by responding, we clear the
  // retry flag.
  sl_zigbee_af_response_aps_frame.options &= ~SL_ZIGBEE_APS_OPTION_RETRY;

  if (pageRequestTickCallback(totalBytesSent,
                              requesterMaxDataSize)) {
    // Simulate a block request to the server that we will generate
    // a response to.
    sl_zigbee_af_image_block_request_callback_struct_t callbackStruct;
    memset(&callbackStruct, 0, sizeof(sl_zigbee_af_image_block_request_callback_struct_t));
    callbackStruct.source = requesterNodeId;
    callbackStruct.id = &requesterImageId;
    callbackStruct.offset = requesterBaseOffset + totalBytesSent;
    callbackStruct.maxDataSize = maxDataToSend;
    callbackStruct.clientEndpoint = requesterEndpoint;
    callbackStruct.bitmask =
      SL_ZIGBEE_AF_IMAGE_BLOCK_REQUEST_SIMULATED_FROM_PAGE_REQUEST;

    bytesSentThisTime = sli_zigbee_af_ota_image_block_request_handler(&callbackStruct);
    sl_zigbee_af_send_response();
  } else {
    bytesSentThisTime += maxDataToSend;
  }
  handlingPageRequest = false;
  if (bytesSentThisTime == 0) {
    sl_zigbee_af_ota_bootload_cluster_println("Failed to send image block for page request");
    // We don't need to call abortPageRequest();
    // here because the server will call into our otaPageRequestErrorHandler()
    // if that occurs.
  } else {
    totalBytesSent += bytesSentThisTime;

    if (totalBytesSent >= totalSize
        || totalBytesSent >= requesterPageSize) {
      sl_zigbee_af_ota_bootload_cluster_println("Done sending blocks for page request.");
      abortPageRequest();
    }
  }
}

bool sli_zigbee_af_ota_server_handling_page_request(void)
{
  return handlingPageRequest;
}

//------------------------------------------------------------------------------
#else // No page request support

uint8_t sli_zigbee_af_ota_page_request_handler(uint8_t clientEndpoint,
                                               uint8_t serverEndpoint,
                                               const sl_zigbee_af_ota_image_id_t* id,
                                               uint32_t offset,
                                               uint8_t maxDataSize,
                                               uint16_t pageSize,
                                               uint16_t responseSpacing)
{
  UNUSED_VAR(clientEndpoint);
  UNUSED_VAR(serverEndpoint);
  UNUSED_VAR(id);
  UNUSED_VAR(offset);
  UNUSED_VAR(maxDataSize);
  UNUSED_VAR(pageSize);
  UNUSED_VAR(responseSpacing);
  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}

bool sli_zigbee_af_ota_page_request_error_handler(void)
{
  return false;
}

void sli_zigbee_af_ota_page_request_tick(uint8_t endpoint)
{
  UNUSED_VAR(endpoint);
}

bool sli_zigbee_af_ota_server_handling_page_request(void)
{
  return false;
}

#endif //  defined (PAGE_REQUEST_SUPPORT)
