/***************************************************************************/ /**
 * @file
 * @brief SL Network API
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#pragma once
#include "sl_status.h"
#include "sl_net_types.h"
#include "sli_net_types.h"
#include "sl_net_wifi_types.h"

/**
 * @brief Register an event handler for network events.
 *
 * @param[in] function Pointer to the event handler function to register.
 * @return Status of the operation.
 *         - SL_STATUS_OK: Event handler registered successfully.
 *         - SL_STATUS_FAIL: Failed to register the event handler.
 */
sl_status_t sli_net_register_event_handler(sl_net_event_handler_t function);

/**
 * @brief Notify the registered network event handler.
 *
 * @param[in] event Network event type.
 * @param[in] status Status of the event.
 * @param[in] data Pointer to event-specific data.
 * @param[in] data_size Size of the event data.
 */
void sli_notify_net_event_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t data_size);

/**
 * @brief Set a network credential.
 *
 * @param[in] id Credential ID to identify the credential.
 * @param[in] type Type of the credential (for example, password or certificate).
 * @param[in] credential Pointer to the credential data.
 * @param[in] credential_length Length of the credential data in bytes.
 * @return Status of the operation.
 *         - SL_STATUS_OK: Credential set successfully.
 *         - SL_STATUS_FAIL: Failed to set the credential.
 */
sl_status_t sli_net_set_credential(sl_net_credential_id_t id,
                                   sl_net_credential_type_t type,
                                   const void *credential,
                                   uint32_t credential_length);

/**
 * @brief Retrieve a network credential.
 *
 * @param[in] id Credential ID to identify the credential.
 * @param[out] type Pointer to store the type of the credential (for example, password or certificate).
 * @param[out] credential Pointer to store the retrieved credential data.
 * @param[out] credential_length Pointer to store the length of the retrieved credential data in bytes.
 * @return Status of the operation.
 *         - SL_STATUS_OK: Credential retrieved successfully.
 *         - SL_STATUS_FAIL: Failed to retrieve the credential.
 *         - SL_STATUS_INVALID_PARAMETER: Invalid input parameters.
 */
sl_status_t sli_net_get_credential(sl_net_credential_id_t id,
                                   const sl_net_credential_type_t *type,
                                   const void *credential,
                                   const uint32_t *credential_length);

/**
 * @brief Delete a network credential.
 *
 * @param[in] id Credential ID to identify the credential to delete.
 * @param[in] type Type of the credential to delete (for example, password or certificate).
 * @return Status of the operation.
 *         - SL_STATUS_OK: Credential deleted successfully.
 *         - SL_STATUS_FAIL: Failed to delete the credential.
 *         - SL_STATUS_INVALID_PARAMETER: Invalid input parameters.
 */
sl_status_t sli_net_delete_credential(sl_net_credential_id_t id, sl_net_credential_type_t type);

/**
 * @brief Event handler for network manager events.
 *
 * @param[in] arg Pointer to the argument passed to the event handler (can be NULL).
 * 
 * @details
 * This function processes network manager events, such as connection, disconnection, 
 * and failure events. It handles auto-join processes and notifies the user of the 
 * network status through registered event handlers.
 */
void sli_network_manager_event_handler(const void *arg);

/**
 * @brief Configures Network Address Translation (NAT) for the Wi-Fi interfaces.
 *
 * @param[in] sli_nat_config Pointer to a NAT configuration structure.
 * 
 * @details
 * This function enables or disables NAT for the Wi-Fi interfaces based on the
 * configuration provided in the sli_nat_config structure. It checks if both the STA
 * (Station) and AP (Access Point) interfaces are up before sending the NAT
 * configuration command to the driver.
 * 
 * @return Status of the operation.
 *        - SL_STATUS_OK: NAT configured successfully.
 *        - SL_STATUS_FAIL: Failed to configure NAT.
 *        - SL_STATUS_INVALID_PARAMETER: If the sli_nat_config parameter is NULL.
 */
sl_status_t sli_net_nat_configure(const sli_net_nat_config_t *sli_nat_config);

/**
 * @brief Configure the IP address for a specified virtual AP.
 *
 * This function configures the IP address settings for the given virtual access point (AP) using the provided
 * IP configuration structure. The function applies the configuration to the specified virtual AP interface and
 * waits for the operation to complete or timeout.
 *
 * @param[in]  ip_config      Pointer to the IP configuration structure to be applied. This structure should
 *                            contain the desired IP address, subnet mask, gateway, and DNS settings.
 * @param[in]  virtual_ap_id  Identifier for the virtual AP to configure. This value selects which virtual AP
 *                            interface will receive the new IP configuration.
 * @param[in]  timeout        Timeout value (in milliseconds) for the configuration operation. If the operation
 *                            does not complete within this period, the function will return a timeout error.
 *
 *  @details
 * This API is typically used to assign or update the IP address and related network parameters for a virtual
 * access point (AP) interface. It is important to ensure that the provided configuration is valid and that the
 * virtual AP is initialized before calling this function. The function will block until the configuration is
 * applied or the timeout is reached.
 * 
 * @return sl_status_t
 *         - SL_STATUS_OK on success.
 *         - SL_STATUS_TIMEOUT if the operation times out.
 *         - SL_STATUS_INVALID_PARAMETER if any argument is invalid.
 *         - Other error codes on failure.
 *
 */
sl_status_t sli_net_configure_ip_address(sl_net_ip_configuration_t *ip_config,
                                         uint8_t virtual_ap_id,
                                         const uint32_t timeout);

/**
 * @brief Initialize the network manager for handling network events and operations.
 *
 * This function sets up the necessary resources for the network manager, including
 * creating message queues and event flags. It prepares the network manager to handle
 * various network-related tasks such as auto-joining networks and managing connections.
 *
 * @return Status of the operation.
 *         - SL_STATUS_OK: Network manager initialized successfully.
 *         - SL_STATUS_ALREADY_INITIALIZED: Network manager is already initialized.
 *         - SL_STATUS_FAIL: Failed to initialize the network manager due to resource allocation issues.
 */
sl_status_t sli_network_manager_init(void);

/**
 * @brief Deinitializes the network manager and cleans up all allocated resources.
 *
 * @details
 * This function frees all resources allocated by the network manager, including:
 * - Terminating the network manager thread
 * - Deleting the network manager message queue
 * - Deleting event flags and other resources used for network management and auto-join processes
 * 
 * @return Status of the operation.
 *         - SL_STATUS_OK: Network manager deinitialized successfully.
 *         - SL_STATUS_FAIL: Failed to deinitialize the network manager.
 */
sl_status_t sli_network_manager_deinit(void);

/**
 * @brief Perform an auto-join request and wait for completion.
 *
 * Wraps the message queue interaction needed to request an auto-join
 * operation. This function blocks until the network manager responds
 * with success or failure.
 *
 * @param[in] interface   Network interface initiating the auto-join.
 * @param[in] profile_id  Profile identifier; must be SL_NET_AUTO_JOIN.
 *
 * @return sl_status_t
 *         - SL_STATUS_OK on successful auto-join
 *         - SL_STATUS_FAIL on failure or unexpected response
 *         - SL_STATUS_ABORT if wait was aborted (e.g. network manager deinitialized during wait)
 *         - SL_STATUS_INVALID_PARAMETER if profile_id is not SL_NET_AUTO_JOIN
 */
sl_status_t sli_network_manager_auto_join_request(sl_net_interface_t interface, sl_net_profile_id_t profile_id);

/**
 * @brief Initiate an asynchronous network interface bring-up request.
 *
 * Posts an async bring-up request to the network manager thread queue.
 * This function returns immediately after posting the request; completion
 * is reported via the registered global event handler.
 *
 * @param[in] interface   Network interface to bring up.
 * @param[in] profile_id  Profile identifier for the interface configuration.
 *
 * @return sl_status_t
 *         - SL_STATUS_IN_PROGRESS if request was successfully posted
 *         - SL_STATUS_BUSY if interface is already processing an async operation
 *         - SL_STATUS_INVALID_PARAMETER if interface or profile_id is invalid
 *         - SL_STATUS_NOT_INITIALIZED if network manager is not initialized
 *         - SL_STATUS_FAIL if queue operation fails
 */
sl_status_t sli_net_up_async_start(sl_net_interface_t interface, sl_net_profile_id_t profile_id);

/**
 * @brief Reset the async state for all interfaces.
 *
 * Clears the async state for all network interfaces.
 */
void sli_net_async_reset_all(void);