/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

/** @brief UpdateStarted
 *
 * Called when an OTA upgrade starts.
 *
 * @param manufacturerId   Ver.: always
 * @param imageTypeId      Ver.: always
 * @param firmwareVersion  Ver.: always
 * @param maxDataSize      Ver.: always
 * @param offset           Ver.: always
 */
WEAK(void sl_zigbee_af_ota_server_update_started_cb(uint16_t manufacturerId,
                                                    uint16_t imageTypeId,
                                                    uint32_t firmwareVersion,
                                                    uint8_t maxDataSize,
                                                    uint32_t offset))
{
  UNUSED_VAR(manufacturerId);
  UNUSED_VAR(imageTypeId);
  UNUSED_VAR(firmwareVersion);
  UNUSED_VAR(maxDataSize);
  UNUSED_VAR(offset);
}

/** @brief OTA Server Block Sent Callback
 *
 * This function will be called when a block is sent to a device.
 *
 * @param actualLength      Ver.: always
 * @param manufacturerId    Ver.: always
 * @param imageTypeId       Ver.: always
 * @param firmwareVersion   Ver.: always
 */
WEAK(void sl_zigbee_af_ota_server_block_sent_cb(uint8_t actualLength,
                                                uint16_t manufacturerId,
                                                uint16_t imageTypeId,
                                                uint32_t firmwareVersion))
{
  UNUSED_VAR(actualLength);
  UNUSED_VAR(manufacturerId);
  UNUSED_VAR(imageTypeId);
  UNUSED_VAR(firmwareVersion);
}

/** @brief OTA Server Update Complete Callback
 *
 * This function will be called when an OTA update has finished.
 *
 * @param manufacturerId  ManufacturerId.
 * @param imageTypeId  Image Type Id.
 * @param firmwareVersion  Firmware Version.
 * @param source  Source node id.
 * @param status  Status of update.
 */
WEAK(void sl_zigbee_af_ota_server_update_complete_cb(uint16_t manufacturerId,
                                                     uint16_t imageTypeId,
                                                     uint32_t firmwareVersion,
                                                     sl_802154_short_addr_t source,
                                                     uint8_t status))
{
  UNUSED_VAR(manufacturerId);
  UNUSED_VAR(imageTypeId);
  UNUSED_VAR(firmwareVersion);
  UNUSED_VAR(source);
  UNUSED_VAR(status);
}

/** @brief Ota Server Image Block Request
 *
 * This function is called when the server application receives an image block
 * request by a client.
 *
 * @param data A struct containing the details of the image block response and
 * values that can be returned by the application to effect the behavior of the
 * server's response.
 */
WEAK(uint8_t sl_zigbee_af_ota_server_image_block_request_cb(sl_zigbee_af_image_block_request_callback_struct_t* data))
{
  // This function should return one of the following:
  //   SL_ZIGBEE_ZCL_STATUS_SUCCESS - This indicates that the OTA server code should
  //     respond to the image block request normally.
  //   SL_ZIGBEE_ZCL_STATUS_ABORT - This indicates to the OTA server that it should
  //     tell the client to abort the download and make no more image block requests.
  //   SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA - This indicates to the OTA server to tell
  //     the client it should delay before asking for that data block again.
  //     The 'waitTimeInMinutesResponse' value should be filled in with the delay value.
  UNUSED_VAR(data);
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}
