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
#include "app/framework/plugin/ota-common/ota-cli.h"
#include "app/framework/plugin/ota-client/ota-client.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#include "app/framework/plugin/ota-client/ota-client-signature-verify.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

/**
 * @addtogroup cli
 * @{
 */
/**
 * @brief OTA Client CLI commands. Used to drive the OTA client.
 *        <b>plugin ota-client bootload &lt;index&gt;</b>
 *        - <i> bootloads the image at the specified index by calling the OTA
 *              bootload callback.
 *           - index - uint8_t. The index at which to bootload the image.</i>
 *
 *        <b>plugin ota-client verify &lt;index&gt;</b>
 *        - <i> Performs signature verification on the image at the specified
 *              index.
 *           - index - uint8_t. The index of the image to be verified.</i>
 *
 *        <b>plugin ota-client info</b>
 *        - <i>Prints the manufacturer ID, Image Type ID, and Version information
 *             that are used when a query next image is sent to the server by the
 *             client.</i>
 *
 *        <b>plugin ota-client start</b>
 *        - <i>Starts the ota client state machine. The state machine discovers
 *             the OTA server, queries for new images, downloads the images
 *             and waits for the server command to upgrade.</i>
 *
 *        <b>plugin ota-client stop</b>
 *        - <i>Stops the OTA state machine.</i>
 *
 *        <b>plugin ota-client status</b>
 *        - <i>Prints information on the current state of the OTA client
 *             download.</i>
 *
 *        <b>plugin ota-client block-test</b>
 *        - <i></i>
 *
 *        <b>plugin ota-client page-request</b>
 *        - <i></i>
 *
 */
static void otaStartStopClientCommand(bool starting)
{
  sl_zigbee_af_core_println("%s" "ing OTA client state machine",
                            starting ? "start" : "stopp");
  if (starting) {
    sl_zigbee_af_ota_client_start_cb();
  } else {
    sli_zigbee_af_ota_client_stop();
  }
}

void otaStartClientCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  otaStartStopClientCommand(true);
}

void otaStopClientCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  otaStartStopClientCommand(false);
}

void otaCliBootload(sl_cli_command_arg_t *arguments)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_ota_image_id_t id = sli_zigbee_af_ota_find_image_id_by_index(index);
  if (!sl_zigbee_af_is_ota_image_id_valid(&id)) {
    otaPrintln("Error: No image at index %d", index);
    return;
  }
  sl_zigbee_af_ota_client_bootload_cb(&id);
}

void otaCliVerify(sl_cli_command_arg_t *arguments)
{
#if defined(SIGNATURE_VERIFICATION_SUPPORT)
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_ota_image_id_t id = sli_zigbee_af_ota_find_image_id_by_index(index);
  if (!sl_zigbee_af_is_ota_image_id_valid(&id)) {
    otaPrintln("Error: No image at index %d", index);
    return;
  }
  sli_zigbee_af_ota_image_signature_verify(0,       // max number of hash calculations
                                           &id, //   (0 = keep going until hashing is done)
                                           true); // new verification?
#else
  UNUSED_VAR(arguments);
  otaPrintln("Not supported.");
#endif
}

void otaPrintClientInfo(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_af_ota_image_id_t myId;
  uint16_t hardwareVersion;
  sl_zigbee_af_ota_client_version_info_cb(&myId, &hardwareVersion);
  otaPrintln("Client image query info");
  otaPrintln("Manuf ID:         0x%04X", myId.manufacturerId);
  otaPrintln("Image Type ID:    0x%04X", myId.imageTypeId);
  otaPrintln("Current Version:  0x%08X", myId.firmwareVersion);
  sl_zigbee_af_ota_bootload_cluster_print("Hardware Version: ");
  if (hardwareVersion != SL_ZIGBEE_AF_INVALID_HARDWARE_VERSION) {
    otaPrintln("0x%04X", hardwareVersion);
  } else {
    otaPrintln("NA");
  }
  sl_zigbee_af_core_flush();

  otaPrintln("Query Delay ms:            %ld", (uint32_t)SL_ZIGBEE_AF_OTA_QUERY_DELAY_MS);
  sl_zigbee_af_core_flush();
  otaPrintln("Server Discovery Delay ms: %ld", (uint32_t)SL_ZIGBEE_AF_OTA_SERVER_DISCOVERY_DELAY_MS);
  otaPrintln("Download Delay ms:         %ld", (uint32_t)SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_DELAY_MS);
  otaPrintln("Run Upgrade Delay ms:      %ld", (uint32_t)SL_ZIGBEE_AF_RUN_UPGRADE_REQUEST_DELAY_MS);
  sl_zigbee_af_core_flush();
  otaPrintln("Verify Delay ms:           %ld", (uint32_t)SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_VERIFY_DELAY_MS);
  otaPrintln("Download Error Threshold:  %d", SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_ERROR_THRESHOLD);
  otaPrintln("Upgrade Wait Threshold:    %d", SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_UPGRADE_WAIT_THRESHOLD);
  sl_zigbee_af_core_flush();

#if defined(USE_PAGE_REQUEST)
  otaPrintln("Use Page Request: %s", sli_zigbee_af_using_page_request() ? "yes" : "no");
  otaPrintln("Page Request Size: %d bytes",
             SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_PAGE_REQUEST_SIZE);
  otaPrintln("Page Request Timeout: %d sec.",
             SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_PAGE_REQUEST_TIMEOUT_SECONDS);
#endif

#if defined(SIGNATURE_VERIFICATION_SUPPORT)
  otaPrintln("");
  sli_zigbee_af_ota_client_signature_verify_print_signers();
#endif
}

void setPageRequest(sl_cli_command_arg_t *arguments)
{
  bool pageRequest = sl_cli_get_argument_uint8(arguments, 0);
  sli_zigbee_af_set_page_request(pageRequest);
}

// TODO: need to gate this when we're better equipped to do so
// with the generated CLI
//#if defined(SL_ZIGBEE_TEST)
void setPausePercentage(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_ota_client_stop_download_percentage = sl_cli_get_argument_uint8(arguments, 0);
}
//#endif

void otaSendUpgradeRequest(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_af_ota_server_send_upgrade_request();
}

void sli_zigbee_af_ota_client_disable_downgrades(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  otaPrintln("OTA Downgrades: Disabled");
  sl_zigbee_af_set_disable_ota_downgrades(true);
}

void sli_zigbee_af_ota_client_enable_downgrades(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  otaPrintln("OTA Downgrades: Enabled");
  sl_zigbee_af_set_disable_ota_downgrades(false);
}
