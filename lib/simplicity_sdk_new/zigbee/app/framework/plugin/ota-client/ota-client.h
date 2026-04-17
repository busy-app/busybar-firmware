/***************************************************************************//**
 * @file
 * @brief API for OTA Clients
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_ZIGBEE_SCRIPTED_TEST
#include "config/ota-client-config.h"
#else
#include "ota-client-config.h"
#endif

#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_AUTO_START == 1)
#define AUTO_START
#endif
#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_USE_PAGE_REQUEST == 1)
#define USE_PAGE_REQUEST
#endif
#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_IGNORE_LOOPBACK_SERVER == 1)
#define IGNORE_LOOPBACK_SERVER
#endif
#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_IGNORE_NON_TRUST_CENTER == 1)
#define IGNORE_NON_TRUST_CENTER
#endif
#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNATURE_VERIFICATION_SUPPORT == 1)
#define SIGNATURE_VERIFICATION_SUPPORT
#endif

/**
 * @defgroup ota-client OTA Client
 * @ingroup component cluster
 * @brief API and Callbacks for the OTA Cluster Client Component
 *
 * Silicon Labs implementation of the Zigbee Over-the-air Bootload Client Cluster
 * (a multi-hop, application bootloader). This implementation finds the OTA
 * server in the network, periodically queries the server for a new image to
 * download, downloads the data, and then waits for the server command to
 * tell it to upgrade. Optionally, it can cryptographically verify the
 * image before upgrade.
 *
 */

/**
 * @addtogroup ota-client
 * @{
 */

/** @brief How often the client will ask for a piece of an upgrade image being
 * actively downloaded. A rate of 0 means the client will request the next block of data
 * as fast as it can.
 */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_DELAY_MS)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_DELAY_MS  0L
#endif

/** @brief How often the OTA client looks for an OTA server when there is NOT
 * one present in the network.  Once it has found one, it queries the
 * same one forever (or until it reboots).
 */
#if defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SERVER_DISCOVERY_DELAY_MINUTES)
  #define SL_ZIGBEE_AF_OTA_SERVER_DISCOVERY_DELAY_MS \
  (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SERVER_DISCOVERY_DELAY_MINUTES * MINUTES_IN_MS)
#else
  #define SL_ZIGBEE_AF_OTA_SERVER_DISCOVERY_DELAY_MS     (2 * MINUTES_IN_MS)
#endif

/** @brief How often the OTA client asks the OTA server if there is a new image available. */
#if defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_QUERY_DELAY_MINUTES)
  #define SL_ZIGBEE_AF_OTA_QUERY_DELAY_MS \
  (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_QUERY_DELAY_MINUTES * MINUTES_IN_MS)
#else
  #define SL_ZIGBEE_AF_OTA_QUERY_DELAY_MS (5 * MINUTES_IN_MS)
#endif

/** @brief How many sequential query errors will cause a device to look for a new OTA server. */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_QUERY_ERROR_THRESHOLD)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_QUERY_ERROR_THRESHOLD 10
#endif

/** @brief The maximum number of sequential errors when downloading an image that will
 * trigger the OTA client to abort the download.
 */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_ERROR_THRESHOLD)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_DOWNLOAD_ERROR_THRESHOLD  10
#endif

/** @brief The delay between attempts to request to initiate the bootload
 * of a successfully downloaded file.
 */
#if defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_RUN_UPGRADE_REQUEST_DELAY_MINUTES)
  #define SL_ZIGBEE_AF_RUN_UPGRADE_REQUEST_DELAY_MS \
  (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_RUN_UPGRADE_REQUEST_DELAY_MINUTES * MINUTES_IN_MS)
#else
  #define SL_ZIGBEE_AF_RUN_UPGRADE_REQUEST_DELAY_MS (10 * MINUTES_IN_MS)
#endif

/** @brief The maximum number of sequential errors when asking the OTA Server when to
 * upgrade that will cause the OTA client to apply the upgrade without the
 * server telling it to do so.
 */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_UPGRADE_WAIT_THRESHOLD)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_UPGRADE_WAIT_THRESHOLD  10
#endif
/** @brief Page Request Size. Tested page sizes: 1024, 2048, and 4096.*/
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_PAGE_REQUEST_SIZE)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_PAGE_REQUEST_SIZE 1024
#endif

/** @brief The spacing requested by the client between the image blocks sent by the
 * server to the client during a page request.
 */
#if !defined(SL_ZIGBEE_AF_OTA_CLIENT_PAGE_REQUEST_SPACING_MS)
  #define SL_ZIGBEE_AF_OTA_CLIENT_PAGE_REQUEST_SPACING_MS 50L
#endif

/** @brief This is the time delay between calls to verifying the OTA image.
 *
 * Verification can take a while (especially in the case of signature
 * checking for Smart Energy) so this provides the ability for other
 * parts of the system to run.
 */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_VERIFY_DELAY_MS)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_VERIFY_DELAY_MS    10L
#endif

#define NULL_EUI64 { 0, 0, 0, 0, 0, 0, 0, 0 }

/** @brief EUI64 of the allowed image signer. A NULL EUI64 is an invalid signer and will never match. */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI0)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI0 NULL_EUI64
#endif

/** @brief EUI64 of the allowed image signer. A NULL EUI64 is an invalid signer and will never match. */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI1)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI1 NULL_EUI64
#endif

/** @brief EUI64 of the allowed image signer. A NULL EUI64 is an invalid signer and will never match. */
#if !defined(SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI2)
  #define SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI2 NULL_EUI64
#endif

#define SECONDS_IN_MS (1000L)
#define MINUTES_IN_MS (60 * SECONDS_IN_MS)
#define HOURS_IN_MS (60 * MINUTES_IN_MS)

/** @brief Invalid hardware version.
 *
 * By default if hardware version is not defined, it is not used.
 * Most products do not limit upgrade images based on the version.
 * Instead they have different images for different hardware.  However
 * this can provide support for an image that only supports certain hardware
 * revision numbers.
 *
 */
#define SL_ZIGBEE_AF_INVALID_HARDWARE_VERSION 0xFFFF

// TODO: ungate this when the facilities are in place to do so
// for generated CLI
//#if defined(SL_ZIGBEE_TEST)
extern uint8_t sli_zigbee_af_ota_client_stop_download_percentage;
//#endif

/**
 * @name API
 * @{
 */

/** @brief Notify the server that the upgrade image has been received and validated successfully.
 * This routine is automatically called by the OTA client state machine once the entire OTA image
 * has been downloaded successfully. This API sends a message to the OTA server asking when the OTA
 * image should be applied.
 *
 */
void sl_zigbee_af_ota_server_send_upgrade_request();

/**
 * @brief Set whether OTA clients use non TC servers.
 *
 * Sets the behavior of OTA clients when determining whether or not
 * to use non Trust Center OTA servers and abort any download that is in progress.
 *
 * @param[in] IgnoreNonTc a bool determining whether client should ignore any
 *        non Trust Center OTA servers.
 */
void sl_zigbee_af_ota_client_set_ignore_non_trust_center(bool ignoreNonTc);

/** @brief Returns whether or not the device is allowed to discover OTA server services from devices
 * other than the trust center.
 *
 * @return Current value of ignoreNonTrustCenter for OTA clients.
 *
 */
bool sl_zigbee_af_ota_client_get_ignore_non_trust_center();

/**
 * @brief Set whether OTA Clients will permit firmware downgrades.
 *
 * @param[in] isDisabled A bool value indicating whether or not downgrades
 *        will be disabled.
 */
void sl_zigbee_af_set_disable_ota_downgrades(bool isDisabled);

/** @brief Returns whether or not the device is allowed to downgrade its firmware.
 *
 * @return The current value of disableOtaDowngrades.
 */
bool sl_zigbee_af_get_disable_ota_downgrades();

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup ota_client_cb OTA Client
 * @ingroup af_callback
 * @brief Callbacks for OTA Client Component
 *
 */

/**
 * @addtogroup ota_client_cb
 * @{
 */

/** @brief Pre-Bootload Callback.
 *
 * Called shortly before installing the downloaded image.
 *
 * @param srcEndpoint[out]     Ver.: always
 * @param serverEndpoint[out]  Ver.: always
 * @param serverNodeId[out]    Ver.: always
 */
void sl_zigbee_af_ota_client_pre_bootload_cb(uint8_t srcEndpoint,
                                             uint8_t serverEndpoint,
                                             sl_802154_short_addr_t serverNodeId);

/** @} */ // end of ota_client_cb
/** @} */ // end of name Callbacks
/** @} */ // end of ota-client

void sli_zigbee_af_ota_client_stop(void);
void sli_zigbee_af_ota_client_print_state(SL_CLI_COMMAND_ARG);
void sli_zigbee_af_send_image_block_request_test(SL_CLI_COMMAND_ARG);

void sli_zigbee_af_set_page_request(bool pageRequest);
bool sli_zigbee_af_using_page_request(void);

void sli_zigbee_af_ota_bootload_cluster_client_resume_after_erase(bool success);
