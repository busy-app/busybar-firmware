/***************************************************************************//**
 * @file
 * @brief Definitions for the Standalone Bootloader Server plugin.
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
 * @defgroup standalone-bootloader-server Standalone Bootloader Server
 * @ingroup component
 * @brief API and Callbacks for the Standalone Bootloader Server Component
 *
 * This component implements the legacy Ember over-the-air, 1-hop, bootload protocol.
 * This can bootload a target node that has a standalone bootloader client
 * running on it.  It first scans for neighboring devices supporting the protocol.
 * Once a client target has been identified, it sends the request to bootload,
 * answers the challenge using the configured encryption key, and then
 * transfers a new EBL file via Xmodem using 802.15.4 MAC data frames.
 * It utilizes the OTA Storage code to retrieve the EBL file for sending to
 * the target node.  This is not a Zigbee-certifiable protocol.
 *
 */

/**
 * @addtogroup standalone-bootloader-server
 * @{
 */

/**
 * @name API
 * @{
 */

// Public API
sl_status_t sl_zigbee_af_standalone_bootloader_server_broadcast_query(void);
sl_status_t sl_zigbee_af_standalone_bootloader_server_start_client_bootload(sl_802154_long_addr_t longId,
                                                                            const sl_zigbee_af_ota_image_id_t* id,
                                                                            uint16_t tag);
sl_status_t sl_zigbee_af_standalone_bootloader_server_start_client_bootload_with_current_target(const sl_zigbee_af_ota_image_id_t* id,
                                                                                                uint16_t tag);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup standalone_bl_server_cb Standalone Bootloader Server
 * @ingroup af_callback
 * @brief Callbacks for Standalone Bootloader Server Component
 *
 */

/**
 * @addtogroup standalone_bl_server_cb
 * @{
 */

/** @brief Query Response.
 *
 * Allows the application to control whether the plugin can respond to incoming
 * bootload messages.
 *
 * @param queryWasBroadcast Boolean indicating whether the query was broadcast
 * or unicast Ver.: always
 * @param queryData A data structure providing the details about the local
 * device's bootloader. Ver.: always
 */
void sl_zigbee_af_standalone_bootloader_server_query_response_cb(bool queryWasBroadcast,
                                                                 const sl_zigbee_af_standalone_bootloader_query_response_data_t*queryData);

/** @brief Finished.
 *
 * Notifies the application when the standalone bootload has completed,
 * successfully or otherwise.
 *
 * @param success Boolean indicating the success or failure of the bootload.
 * Ver.: always
 */
void sl_zigbee_af_standalone_bootloader_server_finished_cb(bool success);

/** @} */ // end of standalone_bl_server_cb
/** @} */ // end of name Callbacks
/** @} */ // end of standalone-bootloader-server

void sli_zigbee_af_standalone_bootloader_server_print_target_client_info_command(void);
void sli_zigbee_af_standalone_bootloader_server_print_status(void);
