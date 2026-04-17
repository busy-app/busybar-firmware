/***************************************************************************//**
 * @file  sli_zigbee_zdo_security.h
 * @brief ZDO Security functionality described in R23 including retrieving and
 * generating Authentication Token (INTERNAL)
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_ZIGBEE_ZDO_SECURITY_H
#define SLI_ZIGBEE_ZDO_SECURITY_H

#include "stack/include/sl_zigbee_stack_specific_tlv.h"

// Local TLV Defines
// Authentication Token ID
#define SLI_ZIGBEE_ZDO_AUTHENTICATION_TOKEN_ID_TAG_ID   0
#define SLI_ZIGBEE_ZDO_AUTHENTICATION_TOKEN_ID_MIN_LEN  1
#define SLI_ZIGBEE_ZDO_AUTHENTICATION_TOKEN_ID_MAX_LEN  1
typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_AUTHENTICATION_TOKEN_ID_MAX_LEN) sl_zigbee_local_tlv_authentication_token_id_t;

// Ssecurity decommissioning
#define SLI_ZIGBEE_ZDO_SECURITY_DECOMMISSION_TAG_ID   0
#define SLI_ZIGBEE_ZDO_SECURITY_DECOMMISSION_MIN_LEN  1
#define SLI_ZIGBEE_ZDO_SECURITY_DECOMMISSION_MAX_LEN  255
typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_SECURITY_DECOMMISSION_MAX_LEN) sl_zigbee_local_tlv_security_decommission_t;

// Target IEEE Address
#define SLI_ZIGBEE_ZDO_TARGET_IEEE_ADDRESS_TAG_ID 0
#define SLI_ZIGBEE_ZDO_TARGET_IEEE_ADDRESS_MIN_LEN 8
#define SLI_ZIGBEE_ZDO_TARGET_IEEE_ADDRESS_MAX_LEN 8
typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_TARGET_IEEE_ADDRESS_MAX_LEN) sl_zigbee_local_tlv_target_ieee_address_t;

// Device Authentication Level
#define SLI_ZIGBEE_ZDO_DEVICE_AUTHENTICATION_LEVEL_TAG_ID 0
#define SLI_ZIGBEE_ZDO_DEVICE_AUTHENTICATION_LEVEL_MIN_LEN 10
#define SLI_ZIGBEE_ZDO_DEVICE_AUTHENTICATION_LEVEL_MAX_LEN 10
typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_DEVICE_AUTHENTICATION_LEVEL_MAX_LEN) sl_zigbee_local_tlv_device_authentication_level_t;

// Link Key Features and Capabilities
#define SLI_ZIGBEE_ZDO_LINK_KEY_FEATURES_AND_CAPABILITIES_TAG_ID 0
#define SLI_ZIGBEE_ZDO_LINK_KEY_FEATURES_AND_CAPABILITIES_MIN_LEN 1
#define SLI_ZIGBEE_ZDO_LINK_KEY_FEATURES_AND_CAPABILITIES_MAX_LEN 1
typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_LINK_KEY_FEATURES_AND_CAPABILITIES_MAX_LEN) sl_zigbee_local_tlv_link_key_features_and_capabilities_id_t;

// Security Challenge
#define SLI_ZIGBEE_ZDO_SECURITY_CHALLENGE_TAG_ID 0
#define SLI_ZIGBEE_ZDO_SECURITY_CHALLENGE_MIN_LEN 32
#define SLI_ZIGBEE_ZDO_SECURITY_CHALLENGE_MAX_LEN 32
typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_SECURITY_CHALLENGE_MAX_LEN) sl_zigbee_local_tlv_security_challenge_t;

/** @brief Internal function for generating Get Authentication Level Request
 *
 * @param dest Short address of where you want to send the request
 * @param aps_options APS options
 * @param target EUI64 of queried device
 *
 * @return An ::sl_status value that indicates whether the request was
 * successful or if an error status needs to be sent back.
 */
sl_status_t sli_zigbee_zdo_generate_get_authentication_level_req(sl_802154_short_addr_t dest,
                                                                 sl_zigbee_aps_option_t aps_options,
                                                                 sl_802154_long_addr_t target);

/** @brief Handler for Security Get Authentication Level Request.
 * This function will check if the request is valid and call functions
 * to get the initial join method and active link key update method of
 * the inquired device
 *
 * @param request Buffer containing the ZDO payload
 * @param payload_index Index pointing to the TLV data in the payload
 * @param source address of the device sending the request
 * @param options APS Options
 * @param sequence Sequence number of the ZDO message
 *
 * @return An ::sl_status value that indicates whether the request was
 * successful or if an error status needs to be sent back.
 */
sl_status_t sli_zigbee_zdo_handle_get_authentication_level_req(sli_buffer_manager_buffer_t request,
                                                               uint8_t payload_index,
                                                               sl_802154_short_addr_t source,
                                                               sl_zigbee_aps_option_t options,
                                                               uint8_t sequence);

/** @brief Handler for Security Get Authentication Level Response.
 * This function will check if the response is valid and pass the received
 * information up to an application callback function.
 *
 * @param request Buffer containing the ZDO payload
 * @param payload_index Index pointing to the TLV data in the payload
 * @param source Short address of the device sending the request
 *
 * @return An ::sl_status value that indicates whether the response was
 * successful or if an error status needs to be sent back.
 */
sl_status_t sli_zigbee_zdo_handle_get_authentication_level_rsp(sli_buffer_manager_buffer_t response,
                                                               uint8_t payload_index,
                                                               sl_802154_short_addr_t source);

/** @brief Sends the Security Retrieve Authentication Token Request.
 *
 * This function will generate the Authentication Token ID TLV and
 * send the Security Retrieve Authentication Token Request ZDO Command.
 *
 * @param destination Device to send the request to
 * @param options APS Options
 *
 * @return An ::sl_status value that indicates the success or failure of
 * generating and sending the ZDO command.
 */
sl_status_t sli_zigbee_zdo_generate_retrieve_authentication_token_req(sl_802154_short_addr_t destination,
                                                                      sl_zigbee_aps_option_t options);

/** @brief Handler for Security Retrieve Authentication Token Request.
 * This function will check if the request is valid and call functions
 * to generate the authentication token and send it to the destination
 * address passed in.
 *
 * @param request Buffer containing the ZDO payload
 * @param payload_index Index pointing to the TLV data in the payload
 * @param source Destination Address to send the response to
 * @param options APS Options
 *
 * @return An ::sl_status value that indicates whether the request was
 * successful or if an error status needs to be sent back.
 */
sl_status_t sli_zigbee_zdo_handle_retrieve_authentication_token_req(sli_buffer_manager_buffer_t request,
                                                                    uint8_t payload_index,
                                                                    sl_802154_short_addr_t source,
                                                                    sl_zigbee_aps_option_t options,
                                                                    uint8_t sequence);

/** @brief Sends the Security Device Decommissioning Request.
 *
 * This function will generate the Security Decommissioning TLV and
 * send the Security Decommissioning Request ZDO Command.
 *
 * @param destination Device to send the request to
 * @param encrypt should this message be APS encrypted or not
 * @param eui64_list list of EUI64 of devices that need to be decommisioned
 *
 * @return An ::sl_status value that indicates the success or failure of
 * generating and sending the ZDO command.
 */
sl_status_t sli_zigbee_zdo_generate_security_decommission_req(sl_802154_short_addr_t destination,
                                                              bool encrypt,
                                                              void* eui64_list,
                                                              uint8_t counts);

/** @brief Sends the Clear Binding Request.
 *
 * This function will generate the Clear Binding TLV and
 * send the Clear Binding Request ZDO Command.
 *
 * @param destination Device to send the request to
 * @param encrypt should this message be APS encrypted or not
 * @param eui64_list list of EUI64 of devices that need to be decommisioned
 *
 * @return An ::sl_status value that indicates the success or failure of
 * generating and sending the ZDO command.
 */
sl_status_t sli_zigbee_zdo_generate_clear_all_bindings_req(sl_802154_short_addr_t destination,
                                                           bool encrypt,
                                                           void* eui64_list,
                                                           uint8_t counts);

/** @brief Handler for Security Device Decommissioning Request.
 * This function will check if the request is valid and call functions
 * to remove binding and link key table entries for given devices
 *
 * @param request Buffer containing the ZDO payload
 * @param payload_index Index pointing to the TLV data in the payload
 * @param source Destination Address to send the response to
 * @param options APS Options
 *
 * @return An ::sl_status value that indicates whether the request was
 * successful or if an error status needs to be sent back.
 */
sl_status_t sli_zigbee_zdo_handle_security_decommission_req(sli_buffer_manager_buffer_t request,
                                                            uint8_t payload_index,
                                                            sl_802154_short_addr_t source,
                                                            sl_zigbee_aps_option_t options,
                                                            uint8_t sequence);

/** @brief Handler for Clear Binding Request.
 * This function will check if the request is valid and call functions
 * to remove binding entries for given devices, including all devices given wildcard eui64
 *
 * @param request Buffer containing the ZDO payload
 * @param payload_index Index pointing to the TLV data in the payload
 * @param source Destination Address to send the response to
 * @param options APS Options
 *
 * @return An ::sl_status value that indicates whether the request was
 * successful or if an error status needs to be sent back.
 */
sl_status_t sli_zigbee_zdo_handle_clear_all_bindings_req(sli_buffer_manager_buffer_t request,
                                                         uint8_t payload_index,
                                                         sl_802154_short_addr_t source,
                                                         sl_zigbee_aps_option_t options,
                                                         uint8_t sequence);

/** @brief Handler for Security Retrieve Authentication Token Response.
 * This function will handle the Retrieve Authentication Token Response, check
 * the Authentication Token provided and save it.
 *
 * @param response Response Buffer containing the response data
 * @param payload_index Index pointing to the TLV data in the payload
 * @param source Short address of the device sending the response
 *
 * @return An ::sl_status value that indicates whether the response was
 * successful or if an error status needs to be sent back.
 */
sl_status_t sli_zigbee_zdo_handle_retrieve_authentication_token_rsp(sli_buffer_manager_buffer_t response,
                                                                    sl_zigbee_aps_frame_t *apsFrame,
                                                                    uint8_t payload_index,
                                                                    sl_802154_short_addr_t source);

// security challenge request and response
/**
 * @brief sends a security challenge request out over the air
 * @param destShort the short id of the message destination
 * @param context Reference to the key being challenged (synchronized)
 * @return A valid sl_status_t
 */
sl_status_t sli_zigbee_zdo_generate_security_challenge_req(sl_802154_short_addr_t destShort,
                                                           sl_zigbee_sec_man_context_t *context);
/**
 * @brief handles an incoming security challenge by validating the contents
 * and issuing a response
 * @param request a message buffer containing the request
 * @param index the location of the message payload within the buffer
 * @param sourceShort the short id of the node who sent the request
 * @param rspSequence the sequence number of the request frame
 * @return a status code indicating whether processing was successful
 */
sl_status_t sli_zigbee_zdo_handle_security_challenge_req(sli_buffer_manager_buffer_t request,
                                                         uint16_t index,
                                                         sl_802154_short_addr_t sourceShort,
                                                         uint8_t rspSequence);

/**
 * @brief handles an incoming security challenge response, validating the challenge
 * and updating the corresponding frame counter
 * @param response a message buffer containing the response
 * @param index the location of the response payload within the buffer
 * @param sourceShort the short id of the responder
 * // TODO response sequence number?
 */
void sli_zigbee_zdo_handle_security_challenge_rsp(sli_buffer_manager_buffer_t response,
                                                  uint16_t index,
                                                  sl_802154_short_addr_t sourceShort);

#endif // SLI_ZIGBEE_ZDO_SECURITY_H
