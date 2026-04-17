/***************************************************************************//**
 * @file sl_wisun_msg_api.h
 * @brief Wi-SUN message API
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 *****************************************************************************/

#ifndef SL_WISUN_MSG_API_H
#define SL_WISUN_MSG_API_H

#include "sl_wisun_types.h"
#include "sl_wisun_connection_params_api.h"
#include "sl_wisun_lfn_params_api.h"
#include "sl_wisun_regulation_api.h"

/**************************************************************************//**
 * @addtogroup SL_WISUN_MSG_API Message API
 * @ingroup SL_WISUN_API
 *
 * Wi-SUN Message API provides a message-based API for managing and configuring
 * the Wi-SUN protocol stack for Network Co-Processor (NCP) use-cases.
 *
 * The API is functionally equivalent of @ref SL_WISUN_API. It is based on
 * requests from the host application to the NCP application running on the
 * device, which sends the requests to the stack using sl_wisun_send_request().
 * Depending on the request, the required action is either performed
 * immediately or an internal stack operation is started, terminating with an
 * event once the operation finishes. All events contain a status code,
 * indicating the result of the requested operation. Events are also used by
 * the stack to notify the application of any important information, such as
 * the state of the connection.
 *
 * The NCP application is expected to override sl_wisun_on_event() to handle
 * events from the stack.
 *
 * @{
 *****************************************************************************/

/// Wi-SUN Message API request IDs
typedef enum {
  SL_WISUN_MSG_SET_NETWORK_SIZE_REQ_ID                    = 0x01,
  SL_WISUN_MSG_GET_IP_ADDRESS_REQ_ID                      = 0x03,
  SL_WISUN_MSG_OPEN_SOCKET_REQ_ID                         = 0x04,
  SL_WISUN_MSG_CLOSE_SOCKET_REQ_ID                        = 0x05,
  SL_WISUN_MSG_SENDTO_ON_SOCKET_REQ_ID                    = 0x06,
  SL_WISUN_MSG_LISTEN_ON_SOCKET_REQ_ID                    = 0x07,
  SL_WISUN_MSG_ACCEPT_ON_SOCKET_REQ_ID                    = 0x08,
  SL_WISUN_MSG_CONNECT_SOCKET_REQ_ID                      = 0x09,
  SL_WISUN_MSG_BIND_SOCKET_REQ_ID                         = 0x0A,
  SL_WISUN_MSG_SEND_ON_SOCKET_REQ_ID                      = 0x0B,
  SL_WISUN_MSG_RECEIVE_ON_SOCKET_REQ_ID                   = 0x0C,
  SL_WISUN_MSG_DISCONNECT_REQ_ID                          = 0x0D,
  SL_WISUN_MSG_SET_TRUSTED_CERTIFICATE_REQ_ID             = 0x0E,
  SL_WISUN_MSG_SET_DEVICE_CERTIFICATE_REQ_ID              = 0x0F,
  SL_WISUN_MSG_SET_DEVICE_PRIVATE_KEY_REQ_ID              = 0x10,
  SL_WISUN_MSG_GET_STATISTICS_REQ_ID                      = 0x11,
  SL_WISUN_MSG_SET_SOCKET_OPTION_REQ_ID                   = 0x12,
  SL_WISUN_MSG_SET_TX_POWER_REQ_ID                        = 0x13,
  SL_WISUN_MSG_SET_CHANNEL_MASK_REQ_ID                    = 0x15,
  SL_WISUN_MSG_ALLOW_MAC_ADDRESS_REQ_ID                   = 0x16,
  SL_WISUN_MSG_DENY_MAC_ADDRESS_REQ_ID                    = 0x17,
  SL_WISUN_MSG_GET_SOCKET_OPTION_REQ_ID                   = 0x18,
  SL_WISUN_MSG_GET_JOIN_STATE_REQ_ID                      = 0x19,
  SL_WISUN_MSG_CLEAR_CREDENTIAL_CACHE_REQ_ID              = 0x1A,
  SL_WISUN_MSG_GET_MAC_ADDRESS_REQ_ID                     = 0x1B,
  SL_WISUN_MSG_SET_MAC_ADDRESS_REQ_ID                     = 0x1C,
  SL_WISUN_MSG_RESET_STATISTICS_REQ_ID                    = 0x1D,
  SL_WISUN_MSG_GET_NEIGHBOR_COUNT_REQ_ID                  = 0x1E,
  SL_WISUN_MSG_GET_NEIGHBORS_REQ_ID                       = 0x1F,
  SL_WISUN_MSG_GET_NEIGHBOR_INFO_REQ_ID                   = 0x20,
  SL_WISUN_MSG_SET_UNICAST_SETTINGS_REQ_ID                = 0x21,
  SL_WISUN_MSG_SET_TRACE_LEVEL_REQ_ID                     = 0x22,
  SL_WISUN_MSG_SET_TRACE_FILTER_REQ_ID                    = 0x23,
  SL_WISUN_MSG_SET_REGULATION_REQ_ID                      = 0x24,
  SL_WISUN_MSG_SET_DEVICE_PRIVATE_KEY_ID_REQ_ID           = 0x25,
  SL_WISUN_MSG_SET_ASYNC_FRAGMENTATION_REQ_ID             = 0x27,
  SL_WISUN_MSG_SET_MODE_SWITCH_REQ_ID                     = 0x28,
  SL_WISUN_MSG_SET_REGULATION_TX_THRESHOLDS_REQ_ID        = 0x29,
  SL_WISUN_MSG_SET_DEVICE_TYPE_REQ_ID                     = 0x2B,
  SL_WISUN_MSG_SET_CONNECTION_PARAMS_REQ_ID               = 0x2C,
  SL_WISUN_MSG_JOIN_REQ_ID                                = 0x2D,
  SL_WISUN_MSG_SET_POM_IE_REQ_ID                          = 0x2E,
  SL_WISUN_MSG_GET_POM_IE_REQ_ID                          = 0x2F,
  SL_WISUN_MSG_SET_LFN_PARAMS_REQ_ID                      = 0x30,
  SL_WISUN_MSG_SET_LFN_SUPPORT_REQ_ID                     = 0x31,
  SL_WISUN_MSG_SET_PTI_STATE_REQ_ID                       = 0x32,
  SL_WISUN_MSG_SET_TBU_SETTINGS_REQ_ID                    = 0x33,
  SL_WISUN_MSG_GET_GTKS_REQ_ID                            = 0x34,
  SL_WISUN_MSG_TRIGGER_FRAME_REQ_ID                       = 0x35,
  SL_WISUN_MSG_GET_STACK_VERSION_REQ_ID                   = 0x36,
  SL_WISUN_MSG_SET_SECURITY_STATE_REQ_ID                  = 0x37,
  SL_WISUN_MSG_GET_NETWORK_INFO_REQ_ID                    = 0x38,
  SL_WISUN_MSG_GET_RPL_INFO_REQ_ID                        = 0x39,
  SL_WISUN_MSG_GET_EXCLUDED_CHANNEL_MASK_REQ_ID           = 0x3A,
  SL_WISUN_MSG_SET_NEIGHBOR_TABLE_SIZE_REQ_ID             = 0x3B,
  SL_WISUN_MSG_SOCKET_RECVMSG_REQ_ID                      = 0x3C,
  SL_WISUN_MSG_SOCKET_SENDMSG_REQ_ID                      = 0x3D,
  SL_WISUN_MSG_SOCKET_GETSOCKNAME_REQ_ID                  = 0x3E,
  SL_WISUN_MSG_SOCKET_GETPEERNAME_REQ_ID                  = 0x3F,
  SL_WISUN_MSG_ENABLE_NEIGHBOUR_SOLICITATIONS_REQ_ID      = 0x40,
  SL_WISUN_MSG_TRIGGER_NEIGHBOR_CACHE_REFRESH_REQ_ID      = 0x41,
  SL_WISUN_MSG_SET_RATE_ALGORITHM_REQ_ID                  = 0x42,
  SL_WISUN_MSG_GET_RATE_ALGORITHM_STATS_REQ_ID            = 0x43,
  SL_WISUN_MSG_SET_TX_POWER_DDBM_REQ_ID                   = 0x44,
  SL_WISUN_MSG_SET_LEAF_REQ_ID                            = 0x45,
  SL_WISUN_MSG_SET_DIRECT_CONNECT_STATE_REQ_ID            = 0x46,
  SL_WISUN_MSG_ACCEPT_DIRECT_CONNECT_LINK_REQ_ID          = 0x47,
  SL_WISUN_MSG_SET_PHY_SENSITIVITY_REQ_ID                 = 0x48,
  SL_WISUN_MSG_SET_DIRECT_CONNECT_PMK_ID_REQ_ID           = 0x49,
  SL_WISUN_MSG_SET_PREFERRED_PAN_REQ_ID                   = 0x4A,
  SL_WISUN_MSG_CONFIG_NEIGHBOR_TABLE_SIZE_REQ_ID          = 0x4B,
  SL_WISUN_MSG_SET_LFN_TIMINGS_REQ_ID                     = 0x4C,
  SL_WISUN_MSG_CONFIG_CONCURRENT_DETECTION_REQ_ID         = 0x4D,
  SL_WISUN_MSG_SET_REGULATION_PARAMS_REQ_ID               = 0x4E,
  SL_WISUN_MSG_RESET_REGULATION_DUTY_CYCLE_REQ_ID         = 0x4F,
  SL_WISUN_MSG_SET_EAP_IDENTITY_REQ_ID                    = 0x50,
  SL_WISUN_MSG_SET_EVENT_FILTER_REQ_ID                    = 0x51,
  SL_WISUN_MSG_SET_FAN_TPS_VERSION_REQ_ID                 = 0x52,
  SL_WISUN_MSG_SET_RX_FIFO_SIZE_REQ_ID                    = 0x53,
  SL_WISUN_MSG_REGISTER_ADDRESS_REQ_ID                    = 0x54,
  SL_WISUN_MSG_CLEAR_EVENT_FILTERS_REQ_ID                 = 0x55,
} sl_wisun_msg_req_id_t;

/// Wi-SUN Message API confirmation IDs
typedef enum {
  SL_WISUN_MSG_SET_NETWORK_SIZE_CNF_ID                    = 0x01,
  SL_WISUN_MSG_GET_IP_ADDRESS_CNF_ID                      = 0x03,
  SL_WISUN_MSG_OPEN_SOCKET_CNF_ID                         = 0x04,
  SL_WISUN_MSG_CLOSE_SOCKET_CNF_ID                        = 0x05,
  SL_WISUN_MSG_SENDTO_ON_SOCKET_CNF_ID                    = 0x06,
  SL_WISUN_MSG_LISTEN_ON_SOCKET_CNF_ID                    = 0x07,
  SL_WISUN_MSG_ACCEPT_ON_SOCKET_CNF_ID                    = 0x08,
  SL_WISUN_MSG_CONNECT_SOCKET_CNF_ID                      = 0x09,
  SL_WISUN_MSG_BIND_SOCKET_CNF_ID                         = 0x0A,
  SL_WISUN_MSG_SEND_ON_SOCKET_CNF_ID                      = 0x0B,
  SL_WISUN_MSG_RECEIVE_ON_SOCKET_CNF_ID                   = 0x0C,
  SL_WISUN_MSG_DISCONNECT_CNF_ID                          = 0x0D,
  SL_WISUN_MSG_SET_TRUSTED_CERTIFICATE_CNF_ID             = 0x0E,
  SL_WISUN_MSG_SET_DEVICE_CERTIFICATE_CNF_ID              = 0x0F,
  SL_WISUN_MSG_SET_DEVICE_PRIVATE_KEY_CNF_ID              = 0x10,
  SL_WISUN_MSG_GET_STATISTICS_CNF_ID                      = 0x11,
  SL_WISUN_MSG_SET_SOCKET_OPTION_CNF_ID                   = 0x12,
  SL_WISUN_MSG_SET_TX_POWER_CNF_ID                        = 0x13,
  SL_WISUN_MSG_SET_CHANNEL_MASK_CNF_ID                    = 0x15,
  SL_WISUN_MSG_ALLOW_MAC_ADDRESS_CNF_ID                   = 0x16,
  SL_WISUN_MSG_DENY_MAC_ADDRESS_CNF_ID                    = 0x17,
  SL_WISUN_MSG_GET_SOCKET_OPTION_CNF_ID                   = 0x18,
  SL_WISUN_MSG_GET_JOIN_STATE_CNF_ID                      = 0x19,
  SL_WISUN_MSG_CLEAR_CREDENTIAL_CACHE_CNF_ID              = 0x1A,
  SL_WISUN_MSG_GET_MAC_ADDRESS_CNF_ID                     = 0x1B,
  SL_WISUN_MSG_SET_MAC_ADDRESS_CNF_ID                     = 0x1C,
  SL_WISUN_MSG_RESET_STATISTICS_CNF_ID                    = 0x1D,
  SL_WISUN_MSG_GET_NEIGHBOR_COUNT_CNF_ID                  = 0x1E,
  SL_WISUN_MSG_GET_NEIGHBORS_CNF_ID                       = 0x1F,
  SL_WISUN_MSG_GET_NEIGHBOR_INFO_CNF_ID                   = 0x20,
  SL_WISUN_MSG_SET_UNICAST_SETTINGS_CNF_ID                = 0x21,
  SL_WISUN_MSG_SET_TRACE_LEVEL_CNF_ID                     = 0x22,
  SL_WISUN_MSG_SET_TRACE_FILTER_CNF_ID                    = 0x23,
  SL_WISUN_MSG_SET_REGULATION_CNF_ID                      = 0x24,
  SL_WISUN_MSG_SET_DEVICE_PRIVATE_KEY_ID_CNF_ID           = 0x25,
  SL_WISUN_MSG_SET_ASYNC_FRAGMENTATION_CNF_ID             = 0x27,
  SL_WISUN_MSG_SET_MODE_SWITCH_CNF_ID                     = 0x28,
  SL_WISUN_MSG_SET_REGULATION_TX_THRESHOLDS_CNF_ID        = 0x29,
  SL_WISUN_MSG_SET_DEVICE_TYPE_CNF_ID                     = 0x2B,
  SL_WISUN_MSG_SET_CONNECTION_PARAMS_CNF_ID               = 0x2C,
  SL_WISUN_MSG_JOIN_CNF_ID                                = 0x2D,
  SL_WISUN_MSG_SET_POM_IE_CNF_ID                          = 0x2E,
  SL_WISUN_MSG_GET_POM_IE_CNF_ID                          = 0x2F,
  SL_WISUN_MSG_SET_LFN_PARAMS_CNF_ID                      = 0x30,
  SL_WISUN_MSG_SET_LFN_SUPPORT_CNF_ID                     = 0x31,
  SL_WISUN_MSG_SET_PTI_STATE_CNF_ID                       = 0x32,
  SL_WISUN_MSG_SET_TBU_SETTINGS_CNF_ID                    = 0x33,
  SL_WISUN_MSG_GET_GTKS_CNF_ID                            = 0x34,
  SL_WISUN_MSG_TRIGGER_FRAME_CNF_ID                       = 0x35,
  SL_WISUN_MSG_GET_STACK_VERSION_CNF_ID                   = 0x36,
  SL_WISUN_MSG_SET_SECURITY_STATE_CNF_ID                  = 0x37,
  SL_WISUN_MSG_GET_NETWORK_INFO_CNF_ID                    = 0x38,
  SL_WISUN_MSG_GET_RPL_INFO_CNF_ID                        = 0x39,
  SL_WISUN_MSG_GET_EXCLUDED_CHANNEL_MASK_CNF_ID           = 0x3A,
  SL_WISUN_MSG_SET_NEIGHBOR_TABLE_SIZE_CNF_ID             = 0x3B,
  SL_WISUN_MSG_SOCKET_RECVMSG_CNF_ID                      = 0x3C,
  SL_WISUN_MSG_SOCKET_SENDMSG_CNF_ID                      = 0X3D,
  SL_WISUN_MSG_SOCKET_GETSOCKNAME_CNF_ID                  = 0x3E,
  SL_WISUN_MSG_SOCKET_GETPEERNAME_CNF_ID                  = 0x3F,
  SL_WISUN_MSG_ENABLE_NEIGHBOUR_SOLICITATIONS_CNF_ID      = 0x40,
  SL_WISUN_MSG_TRIGGER_NEIGHBOR_CACHE_REFRESH_CNF_ID      = 0x41,
  SL_WISUN_MSG_SET_RATE_ALGORITHM_CNF_ID                  = 0x42,
  SL_WISUN_MSG_GET_RATE_ALGORITHM_STATS_CNF_ID            = 0x43,
  SL_WISUN_MSG_SET_TX_POWER_DDBM_CNF_ID                   = 0x44,
  SL_WISUN_MSG_SET_LEAF_CNF_ID                            = 0x45,
  SL_WISUN_MSG_SET_DIRECT_CONNECT_STATE_CNF_ID            = 0x46,
  SL_WISUN_MSG_ACCEPT_DIRECT_CONNECT_LINK_CNF_ID          = 0x47,
  SL_WISUN_MSG_SET_PHY_SENSITIVITY_CNF_ID                 = 0x48,
  SL_WISUN_MSG_SET_DIRECT_CONNECT_PMK_ID_CNF_ID           = 0x49,
  SL_WISUN_MSG_SET_PREFERRED_PAN_CNF_ID                   = 0x4A,
  SL_WISUN_MSG_CONFIG_NEIGHBOR_TABLE_SIZE_CNF_ID          = 0x4B,
  SL_WISUN_MSG_SET_LFN_TIMINGS_CNF_ID                     = 0x4C,
  SL_WISUN_MSG_CONFIG_CONCURRENT_DETECTION_CNF_ID         = 0x4D,
  SL_WISUN_MSG_SET_REGULATION_PARAMS_CNF_ID               = 0x4E,
  SL_WISUN_MSG_RESET_REGULATION_DUTY_CYCLE_CNF_ID         = 0x4F,
  SL_WISUN_MSG_SET_EAP_IDENTITY_CNF_ID                    = 0x50,
  SL_WISUN_MSG_SET_EVENT_FILTER_CNF_ID                    = 0x51,
  SL_WISUN_MSG_SET_FAN_TPS_VERSION_CNF_ID                 = 0x52,
  SL_WISUN_MSG_SET_RX_FIFO_SIZE_CNF_ID                    = 0x53,
  SL_WISUN_MSG_REGISTER_ADDRESS_CNF_ID                    = 0x54,
  SL_WISUN_MSG_CLEAR_EVENT_FILTERS_CNF_ID                 = 0x55,
} sl_wisun_msg_cnf_id_t;

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GENERIC sl_wisun_msg_generic
 * @{
 *****************************************************************************/

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_generic_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_generic_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_generic_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GENERIC) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_NETWORK_SIZE sl_wisun_msg_set_network_size
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Network size
  uint8_t size;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_network_size_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_network_size_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_network_size_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_network_size_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_network_size_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_network_size_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GENERIC) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_IP_ADDRESS sl_wisun_msg_get_ip_address
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Type of the IP address to read
  uint32_t address_type;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_ip_address_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_get_ip_address_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_ip_address_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// IP address
  in6_addr_t address;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_ip_address_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_ip_address_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_ip_address_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_IP_ADDRESS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_OPEN_SOCKET sl_wisun_msg_open_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Communication domain
  uint32_t domain;
  /// Communication semantics
  uint32_t type;
  /// Protocol to be used
  uint32_t protocol;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_open_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_open_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_open_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// ID of the opened socket
  int32_t socket_id;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_open_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_open_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_open_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_OPEN_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_CLOSE_SOCKET sl_wisun_msg_close_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_close_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_close_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_close_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_close_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_close_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_close_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_CLOSE_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SENDTO_ON_SOCKET sl_wisun_msg_sendto_on_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// IP address of the remote peer
  in6_addr_t remote_address;
  /// Port number of the remote peer
  uint16_t remote_port;
  /// Amount of data to write
  uint16_t data_length;
  /// Data to write
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_sendto_on_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_sendto_on_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_sendto_on_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Amount of data sent in bytes
  /// equal to SOCKET_RETVAL_ERROR in case of failure
  int32_t data_length;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_sendto_on_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_sendto_on_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_sendto_on_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SENDTO_ON_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SOCKET_SENDMSG sl_wisun_msg_socket_sendmsg
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// Specifies the type of message reception
  int32_t flags;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_sendmsg_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_socket_sendmsg_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_sendmsg_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Amount of data sent in bytes
  /// equal to SOCKET_RETVAL_ERROR in case of failure
  int32_t data_length;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_sendmsg_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_socket_sendmsg_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_sendmsg_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SOCKET_SENDMSG) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_LISTEN_ON_SOCKET sl_wisun_msg_listen_on_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_listen_on_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_listen_on_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_listen_on_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_listen_on_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_listen_on_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_listen_on_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_LISTEN_ON_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_ACCEPT_ON_SOCKET sl_wisun_msg_accept_on_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_on_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_accept_on_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_on_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// ID of the new connected socket
  uint32_t remote_socket_id;
  /// IP address of the remote peer
  in6_addr_t remote_address;
  /// Port number of the remote peer
  uint16_t remote_port;
  /// Reserved, set to zero
  uint16_t reserved;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_on_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_accept_on_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_on_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_ACCEPT_ON_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_CONNECT_SOCKET sl_wisun_msg_connect_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// IP address of the remote peer
  in6_addr_t remote_address;
  /// Port number of the remote peer
  uint16_t remote_port;
  /// Reserved, set to zero
  uint16_t reserved;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_connect_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_connect_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_connect_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_connect_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_connect_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_connect_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_CONNECT_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_BIND_SOCKET sl_wisun_msg_bind_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// Local IP address to use on the socket
  in6_addr_t local_address;
  /// Local port number to use on the socket
  uint16_t local_port;
  /// Reserved, set to zero
  uint16_t reserved;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_bind_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_bind_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_bind_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_bind_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_bind_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_bind_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_BIND_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SEND_ON_SOCKET sl_wisun_msg_send_on_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// Amount of data to write
  uint16_t data_length;
  /// Reserved, set to zero
  uint16_t reserved;
  /// Data to write
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_send_on_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_send_on_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_send_on_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Amount of data sent in bytes
  /// Equal to SOCKET_RETVAL_ERROR in case of failure
  int32_t data_length;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_send_on_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_send_on_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_send_on_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SEND_ON_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_RECEIVE_ON_SOCKET sl_wisun_msg_receive_on_socket
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// Amount of data to read
  uint16_t data_length;
  /// whether to get or not the remote address
  uint8_t get_remote_addr;
  /// Reserved, set to zero
  uint8_t reserved;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_receive_on_socket_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_receive_on_socket_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_receive_on_socket_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// IP address of the remote peer
  in6_addr_t remote_address;
  /// Port number of the remote peer
  uint16_t remote_port;
  /// Amount of data read
  int16_t data_length;
  /// Error code
  int32_t error_code;
  /// Data read
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_receive_on_socket_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_receive_on_socket_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_receive_on_socket_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_RECEIVE_ON_SOCKET) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SOCKET_RECVMSG sl_wisun_msg_socket_recvmsg
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// socket ID
  int32_t socket;
  /// Specifies the type of message reception
  int32_t flags;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_recvmsg_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_socket_recvmsg_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_recvmsg_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Number of bytes received;
  int16_t data_length;
  /// reserved, set to zero
  uint16_t reserved;
  /// code of the error
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_recvmsg_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_socket_recvmsg_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_recvmsg_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SOCKET_RECVMSG) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SOCKET_GETSOCKNAME sl_wisun_msg_socket_getsockname
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// socket ID
  int32_t socket;
  /// Specifies the type of message reception
  int32_t flags;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getsockname_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_socket_getsockname_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getsockname_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Local IP address
  in6_addr_t local_address;
  /// Local port
  int32_t local_port;
  /// error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getsockname_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_socket_getsockname_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getsockname_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SOCKET_GETSOCKNAME) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SOCKET_GETPEERNAME sl_wisun_msg_socket_getpeername
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// socket ID
  int32_t socket;
  /// Specifies the type of message reception
  int32_t flags;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getpeername_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_socket_getpeername_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getpeername_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Remote IP address
  in6_addr_t remote_address;
  /// Remote port
  int32_t remote_port;
  /// error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getpeername_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_socket_getpeername_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_socket_getpeername_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SOCKET_GETPEERNAME) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_DISCONNECT sl_wisun_msg_disconnect
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// 1 for non-graceful disconnection
  uint32_t non_graceful;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_disconnect_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_disconnect_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_disconnect_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_disconnect_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_disconnect_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_disconnect_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_DISCONNECT) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_TRUSTED_CERTIFICATE sl_wisun_msg_set_trusted_certificate
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Options for the certificate
  uint16_t certificate_options;
  /// Size of the certificate data
  uint16_t certificate_length;
  /// Certificate data
  uint8_t certificate[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trusted_certificate_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_trusted_certificate_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trusted_certificate_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trusted_certificate_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_trusted_certificate_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trusted_certificate_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_TRUSTED_CERTIFICATE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_DEVICE_CERTIFICATE sl_wisun_msg_set_device_certificate
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Options for the certificate
  uint16_t certificate_options;
  /// Size of the certificate data
  uint16_t certificate_length;
  /// Certificate data
  uint8_t certificate[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_certificate_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_device_certificate_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_certificate_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_certificate_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_device_certificate_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_certificate_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_DEVICE_CERTIFICATE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_DEVICE_PRIVATE_KEY sl_wisun_msg_set_device_private_key
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Options for the private key
  uint16_t key_options;
  /// Size of the private key data
  uint16_t key_length;
  /// Private key data
  uint8_t key[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_device_private_key_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_device_private_key_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_DEVICE_PRIVATE_KEY) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_STATISTICS sl_wisun_msg_get_statistics
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Type of statistics to read
  uint32_t statistics_type;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_statistics_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_get_statistics_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_statistics_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Statistics
  sl_wisun_statistics_t statistics;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_statistics_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_statistics_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_statistics_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_STATISTICS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_SOCKET_OPTION sl_wisun_msg_set_socket_option
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// Socket protocol level
  int32_t level;
  /// Socket option name
  int32_t option_name;
  /// option data length
  uint16_t option_length;
  /// option value
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_socket_option_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_socket_option_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_socket_option_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Error code
  int32_t error_code;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_socket_option_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_socket_option_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_socket_option_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_SOCKET_OPTION) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_TX_POWER sl_wisun_msg_set_tx_power
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// TX power
  int8_t tx_power;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_tx_power_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_tx_power_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_TX_POWER) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_CHANNEL_MASK sl_wisun_msg_set_channel_mask
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Channel mask
  sl_wisun_channel_mask_t channel_mask;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_channel_mask_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_channel_mask_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_channel_mask_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_channel_mask_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_channel_mask_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_channel_mask_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_CHANNEL_MASK) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_ALLOW_MAC_ADDRESS sl_wisun_msg_allow_mac_address
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// MAC address
  sl_wisun_mac_address_t address;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_allow_mac_address_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_allow_mac_address_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_allow_mac_address_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_allow_mac_address_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_allow_mac_address_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_allow_mac_address_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_ALLOW_MAC_ADDRESS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_DENY_MAC_ADDRESS sl_wisun_msg_deny_mac_address
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// MAC address
  sl_wisun_mac_address_t address;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_deny_mac_address_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_deny_mac_address_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_deny_mac_address_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_deny_mac_address_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_deny_mac_address_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_deny_mac_address_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_DENY_MAC_ADDRESS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_SOCKET_OPTION sl_wisun_msg_get_socket_option
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// ID of the socket
  int32_t socket_id;
  /// Socket protocol level
  int32_t level;
  /// Socket option to get
  int32_t option_name;
  /// Socket option length
  uint16_t option_length;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_socket_option_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_get_socket_option_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_socket_option_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Socket option length
  uint16_t option_length;
  /// reserved, set to zero
  uint16_t reserved;
  /// Error code
  int32_t error_code;
  /// option value
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_socket_option_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_socket_option_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_socket_option_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_SOCKET_OPTION) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_JOIN_STATE sl_wisun_msg_get_join_state
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_join_state_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Join state
  uint32_t join_state;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_join_state_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_join_state_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_join_state_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_JOIN_STATE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_CLEAR_CREDENTIAL_CACHE sl_wisun_msg_clear_credential_cache
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_clear_credential_cache_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_clear_credential_cache_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_clear_credential_cache_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_clear_credential_cache_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_CLEAR_CREDENTIAL_CACHE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_MAC_ADDRESS sl_wisun_msg_get_mac_address
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_mac_address_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// MAC address
  sl_wisun_mac_address_t address;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_mac_address_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_mac_address_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_mac_address_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_MAC_ADDRESS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_MAC_ADDRESS sl_wisun_msg_set_mac_address
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// MAC address
  sl_wisun_mac_address_t address;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mac_address_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_mac_address_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mac_address_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mac_address_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_mac_address_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mac_address_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_MAC_ADDRESS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_RESET_STATISTICS sl_wisun_msg_reset_statistics
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Type of statistics to read
  uint32_t statistics_type;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_reset_statistics_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_reset_statistics_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_reset_statistics_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_reset_statistics_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_reset_statistics_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_reset_statistics_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_RESET_STATISTICS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_NEIGHBOR_COUNT sl_wisun_msg_get_neighbor_count
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbor_count_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Neighbor count
  uint8_t neighbor_count;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbor_count_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_neighbor_count_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbor_count_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_NEIGHBOR_COUNT) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_NEIGHBORS sl_wisun_msg_get_neighbors
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Number of neighbors to read
  uint8_t neighbor_count;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbors_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_get_neighbors_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbors_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Number of neighbors read
  uint8_t neighbor_count;
  /// Reserved, set to zero
  uint8_t reserved[3];
  /// Neighbor MAC addresses read
  sl_wisun_mac_address_t neighbor_mac_addresses[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbors_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_neighbors_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbors_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_NEIGHBORS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_NEIGHBOR_INFO sl_wisun_msg_get_neighbor_info
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Neighbor MAC address
  sl_wisun_mac_address_t neighbor_mac_address;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbor_info_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_get_neighbor_info_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbor_info_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Information read
  sl_wisun_neighbor_info_t neighbor_info;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbor_info_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_neighbor_info_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_neighbor_info_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_NEIGHBOR_INFO) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_UNICAST_SETTINGS sl_wisun_msg_set_unicast_settings
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Unicast dwell interval
  uint8_t dwell_interval_ms;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_unicast_settings_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_unicast_settings_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_unicast_settings_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_unicast_settings_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_unicast_settings_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_unicast_settings_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_UNICAST_SETTINGS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_TRACE_LEVEL sl_wisun_msg_set_trace_level
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Number of groups to configure.
  /// Maximum SL_WISUN_TRACE_GROUP_COUNT
  uint8_t group_count;
  /// Table with group_count element filled. It indicates the
  /// trace level to be displayed for each group.
  sl_wisun_trace_group_config_t trace_config[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_level_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_trace_level_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_level_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_level_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_trace_level_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_level_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_TRACE_LEVEL) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_TRACE_FILTER sl_wisun_msg_set_trace_filter
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Bitfield. Each 1 indicates the trace group ID corresponding
  /// is selected for tracing. 0 means the trace group ID corresponding
  /// should be filtered out. It follows enum sl_wisun_trace_group_t
  /// filter[0] contains ID 0 to 7
  /// filter[1] contains ID 8 to 15...
  uint8_t filter[SL_WISUN_FILTER_BITFIELD_SIZE];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_filter_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_trace_filter_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_filter_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_filter_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_trace_filter_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_trace_filter_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_TRACE_FILTER) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_REGULATION sl_wisun_set_regulation
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Regional regulation
  uint32_t regulation;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_regulation_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_regulation_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_REGULATION) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_DEVICE_PRIVATE_KEY_ID sl_wisun_msg_set_device_private_key_id
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Private key ID
  uint32_t key_id;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_id_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_device_private_key_id_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_id_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_id_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_device_private_key_id_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_private_key_id_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_DEVICE_PRIVATE_ID_KEY) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_ASYNC_FRAGMENTATION sl_wisun_set_advert_fragment_duration
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Fragment duration (ms)
  uint32_t fragment_duration_ms;
} SL_ATTRIBUTE_PACKED  sl_wisun_msg_set_async_fragmentation_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_async_fragmentation_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_async_fragmentation_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_async_fragmentation_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_async_fragmentation_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_async_fragmentation_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_ASYNC_FRAGMENTATION) */
/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_RATE_ALGORITHM sl_wisun_msg_set_rate_algorithm
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Neighbor MAC address
  sl_wisun_mac_address_t neighbor;
  /// Indicate the rate algorithm type
  uint8_t type;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rate_algorithm_id_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_rate_algorithm_id_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rate_algorithm_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rate_algorithm_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_rate_algorithm_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rate_algorithm_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_RATE_ALGORITHM) */
/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_RATE_ALGORITHM_STATS sl_wisun_msg_get_rate_algorithm_stats
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Neighbor MAC address
  sl_wisun_mac_address_t neighbor;
  /// number of rates available in the array space
  uint16_t allocated_rate_count;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_rate_algorithm_stats_id_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_get_rate_algorithm_stats_id_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_rate_algorithm_stats_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Number of rates copied
  uint16_t rate_count;
  /// Status of the request
  uint32_t status;
  /// Reserved, set to zero
  uint8_t reserved[2];
  /// Neighbor MAC addresses read
  sl_wisun_rate_t rates[];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_rate_algorithm_stats_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_rate_algorithm_stats_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_rate_algorithm_stats_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_RATE_ALGORITHM_STATS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_MODE_SWITCH sl_wisun_msg_set_mode_switch
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Neighbor MAC address
  sl_wisun_mac_address_t neighbor_address;
  /// Indicate how the mode switching is allowed
  uint8_t mode;
  /// Indicate the PhyModeId to use for mode switching
  uint8_t phy_mode_id;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mode_switch_id_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_mode_switch_id_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mode_switch_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mode_switch_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_mode_switch_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_mode_switch_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_MODE_SWITCH) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_REGULATION_TX_THRESHOLDS sl_wisun_set_regulation_tx_thresholds
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Warning threshold in percent
  int8_t warning_threshold;
  /// Alert threshold in percent
  int8_t alert_threshold;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED  sl_wisun_msg_set_regulation_tx_thresholds_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_regulation_tx_thresholds_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_tx_thresholds_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_tx_thresholds_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_regulation_tx_thresholds_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_tx_thresholds_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_REGULATION_TX_THRESHOLDS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_DEVICE_TYPE sl_wisun_set_device_type
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Device type
  uint8_t type;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED  sl_wisun_msg_set_device_type_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_device_type_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_type_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_type_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_device_type_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_device_type_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_DEVICE_TYPE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_CONNECTION_PARAMS sl_wisun_msg_set_connection_params
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Connection parameters
  sl_wisun_connection_params_t parameters;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_connection_params_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_connection_params_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_connection_params_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_connection_params_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_connection_params_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_connection_params_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_CONNECTION_PARAMS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_JOIN sl_wisun_msg_join
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Network name as a NULL terminated string
  uint8_t name[SL_WISUN_NETWORK_NAME_SIZE];
  /// PHY configuration
  sl_wisun_phy_config_t phy_config;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_join_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_join_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_join_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_join_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_join_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_join_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_JOIN) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_POM_IE sl_wisun_msg_set_pom_ie
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Number of PhyModeId to configure
  uint8_t phy_mode_id_count;
  /// List of phy_mode_id_count PhyModeId
  uint8_t phy_mode_ids[SL_WISUN_MAX_PHY_MODE_ID_COUNT];
  /// Indicate if the device is supporting MAC mode switch.
  /// Feature currently unsupported. Must be set to 0.
  uint8_t is_mdr_command_capable;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pom_ie_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_pom_ie_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pom_ie_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pom_ie_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_pom_ie_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pom_ie_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_POM_IE_REQ) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_POM_IE sl_wisun_msg_get_pom_ie
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_pom_ie_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Number of PhyModeId to configure
  uint8_t phy_mode_id_count;
  /// List of phy_mode_id_count PhyModeId
  uint8_t phy_mode_ids[SL_WISUN_MAX_PHY_MODE_ID_COUNT];
  /// Indicate if the device is supporting MAC mode switch
  /// Feature currently unsupported. Must be set to 0.
  uint8_t is_mdr_command_capable;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_pom_ie_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_pom_ie_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_pom_ie_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_POM_IE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_STACK_VERSION sl_wisun_msg_get_stack_version
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_stack_version_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Build number
  uint16_t build;
  /// Wi-SUN stack major version
  uint8_t major;
  /// Wi-SUN stack minor version
  uint8_t minor;
  /// Wi-SUN stack patch version
  uint8_t patch;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_stack_version_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_stack_version_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_stack_version_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_STACK_VERSION) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_LFN_PARAMS sl_wisun_msg_set_lfn_params
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// LFN parameters
  sl_wisun_lfn_params_t parameters;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_params_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_lfn_params_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_params_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_params_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_lfn_params_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_params_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_LFN_PARAMS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_LFN_SUPPORT sl_wisun_msg_set_lfn_support
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Number of LFN supported [0, 10]
  uint8_t lfn_limit;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_support_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_lfn_support_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_support_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_support_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_lfn_support_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_support_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_LFN_SUPPORT) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_PTI_STATE sl_wisun_msg_set_pti_state
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Is pti enabled ?
  uint8_t pti_state;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pti_state_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_pti_state_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pti_state_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pti_state_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_pti_state_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_pti_state_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_PTI_STATE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_TBU_SETTINGS sl_wisun_set_tbu_settings
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// 6LoWPAN MTU size in bytes
  uint16_t mtu_size;
  /// Channel exclusion mode
  uint8_t channel_exclusion_mode;
  /// Reserved, set to zero
  uint8_t reserved;
} SL_ATTRIBUTE_PACKED  sl_wisun_msg_set_tbu_settings_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_tbu_settings_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tbu_settings_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tbu_settings_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_tbu_settings_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tbu_settings_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_TBU_SETTINGS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_GTKS sl_wisun_get_gtks
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_gtks_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// GTKs
  uint8_t gtks[SL_WISUN_GTK_NUM][SL_WISUN_GTK_LEN];
  /// LGTKs
  uint8_t lgtks[SL_WISUN_LGTK_NUM][SL_WISUN_GTK_LEN];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_gtks_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_gtks_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_gtks_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_GTKS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_TRIGGER_FRAME sl_wisun_msg_trigger_frame
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Frame type
  uint32_t frame_type;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_trigger_frame_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_trigger_frame_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_trigger_frame_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_trigger_frame_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_trigger_frame_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_trigger_frame_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_TRIGGER_FRAME) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_SECURITY_STATE sl_wisun_msg_set_security_state
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Security state
  uint32_t security_state;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_security_state_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_security_state_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_security_state_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_security_state_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_pti_state_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_security_state_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_SECURITY_STATE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_NETWORK_INFO sl_wisun_msg_get_network_info
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_network_info_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Wi-SUN network information
  sl_wisun_network_info_t network_info;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_network_info_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_network_info_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_network_info_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_NETWORK_INFO) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_RPL_INFO sl_wisun_msg_get_rpl_info
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_rpl_info_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// RPL information
  sl_wisun_rpl_info_t rpl_info;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_rpl_info_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_rpl_info_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_rpl_info_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_RPL_INFO) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_GET_EXCLUDED_CHANNEL_MASK sl_wisun_msg_get_excluded_channel_mask
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Type of channel mask
  uint32_t type;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_excluded_channel_mask_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_get_excluded_channel_mask_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_excluded_channel_mask_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
  /// Number of channels in mask
  uint32_t channel_count;
  /// Channel mask
  sl_wisun_channel_mask_t channel_mask;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_excluded_channel_mask_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_get_excluded_channel_mask_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_get_excluded_channel_mask_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_GET_EXCLUDED_CHANNEL_MASK) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_NEIGHBOR_TABLE_SIZE sl_wisun_msg_set_neighbor_table_size
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Neighbor table size
  uint8_t size;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_neighbor_table_size_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_network_size_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_neighbor_table_size_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_neighbor_table_size_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_neighbor_table_size_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_neighbor_table_size_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_NEIGHBOR_TABLE_SIZE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_CONFIG_NEIGHBOR_TABLE_SIZE sl_wisun_msg_config_neighbor_table_size
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Maximum number of RPL children
  uint8_t max_child_count;
  /// Maximum number of neighbors including children, parent, and temporary neighbors
  uint8_t max_neighbor_count;
  /// Maximum number of neighbors in the security table
  uint16_t max_security_neighbor_count;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_neighbor_table_size_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_config_neighbor_table_size_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_neighbor_table_size_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_neighbor_table_size_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_config_neighbor_table_size_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_neighbor_table_size_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_CONFIG_NEIGHBOR_TABLE_SIZE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_ENABLE_NEIGHBOUR_SOLICITATIONS sl_wisun_msg_enable_neighbour_solicitations
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// true to enable neighbour solicitations
  uint8_t enable;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_enable_neighbour_solicitations_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_enable_neighbour_solicitations_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_enable_neighbour_solicitations_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_enable_neighbour_solicitations_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_enable_neighbour_solicitations_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_enable_neighbour_solicitations_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_ENABLE_NEIGHBOUR_SOLICITATIONS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_TRIGGER_NEIGHBOR_CACHE_REFRESH sl_wisun_msg_trigger_neighbor_cache_refresh
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_trigger_neighbor_cache_refresh_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_trigger_neighbor_cache_refresh_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_trigger_neighbor_cache_refresh_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_trigger_neighbor_cache_refresh_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_TRIGGER_NEIGHBOR_CACHE_REFRESH) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_TX_POWER_DDBM sl_wisun_msg_set_tx_power_ddbm
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// TX power
  int16_t tx_power_ddbm;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_ddbm_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_tx_power_ddbm_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_ddbm_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_ddbm_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_tx_power_ddbm_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_tx_power_ddbm_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_TX_POWER_DDBM) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_LEAF sl_wisun_msg_set_leaf
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// True to make the node act as an RPL leaf
  uint8_t is_leaf;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_leaf_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_leaf_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_leaf_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_leaf_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_leaf_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_leaf_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_LEAF) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_EVENT_FILTER sl_wisun_msg_set_event_filter
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// neighbor MAC address
  sl_wisun_mac_address_t mac_address;
  /// Bitmask for the specified events
  uint64_t events;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_event_filter_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_event_filter_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_event_filter_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_event_filter_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_event_filter_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_event_filter_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_EVENT_FILTER) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_CLEAR_EVENT_FILTERS sl_wisun_msg_clear_event_filters
 * @{
 *****************************************************************************/


/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_clear_event_filters_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_clear_event_filters_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_clear_event_filters_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_clear_event_filters_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_clear_event_filters_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_clear_event_filters_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_CLEAR_EVENT_FILTERS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_DIRECT_CONNECT_STATE sl_wisun_msg_set_direct_connect_state
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// True to enable Direct Connect, false otherwise
  uint8_t is_enabled;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_state_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_direct_connect_state_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_state_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_state_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_direct_connect_state_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_state_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_DIRECT_CONNECT_STATE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_ACCEPT_DIRECT_CONNECT_LINK sl_wisun_msg_accept_direct_connect_link
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// LinkLocal IPv6 address of the client
  in6_addr_t link_local_ipv6;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_direct_connect_link_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_accept_direct_connect_link_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_direct_connect_link_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_direct_connect_link_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_accept_direct_connect_link_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_accept_direct_connect_link_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_ACCEPT_DIRECT_CONNECT_LINK) */

/**************************************************************************//**
 * @defgroup SL_WISUN_SET_PHY_SENSITIVITY sl_wisun_set_phy_sensitivity
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Sensitivity in dBm
  int16_t sensitivity_dbm;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// Reserved, set to zero
  uint8_t reserved[1];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_phy_sensitivity_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_phy_sensitivity_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_phy_sensitivity_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_phy_sensitivity_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_phy_sensitivity_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_phy_sensitivity_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_SET_PHY_SENSITIVITY) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_DIRECT_CONNECT_PMK_ID sl_wisun_msg_set_direct_connect_pmk_id
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Pairwise Master Key (PMK) ID
  uint32_t key_id;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_pmk_id_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_direct_connect_pmk_id_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_pmk_id_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_pmk_id_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_direct_connect_pmk_id_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_direct_connect_pmk_id_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_DIRECT_CONNECT_PMK_ID) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_PREFERRED_PAN sl_wisun_msg_set_preferred_pan
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Preferred PAN ID
  uint16_t pan_id;
  /// Reserved, set to zero
  uint16_t reserved;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_preferred_pan_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_preferred_pan_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_preferred_pan_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_preferred_pan_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_preferred_pan_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_preferred_pan_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_PREFERRED_PAN) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_LFN_TIMINGS sl_wisun_msg_set_lfn_timings
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// LFN's EUI-64
  sl_wisun_mac_address_t mac_address;
    /// The listening interval desired to be used by the LFN in milliseconds
  uint32_t listening_interval_ms;
  /// Time to advance the beginning of the LFN’s Unicast Listening Interval in milliseconds
  uint32_t listening_offset_ms;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_timings_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_lfn_timings_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_timings_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_timings_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_lfn_timings_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_lfn_timings_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_LFN_TIMINGS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_CONFIG_CONCURRENT_DETECTION sl_wisun_msg_config_concurrent_detection
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// True to select alternate PHY for transmissions
  uint8_t enable_tx;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_concurrent_detection_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_config_concurrent_detection_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_concurrent_detection_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_concurrent_detection_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_config_concurrent_detection_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_config_concurrent_detection_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_CONFIG_CONCURRENT_DETECTION) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_REGULATION_PARAMS sl_wisun_msg_set_regulation_params
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Regional regulation parameters
  sl_wisun_regulation_params_t parameters;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_params_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_regulation_params_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_params_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_params_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_regulation_params_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_regulation_params_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_REGULATION_PARAMS) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_RESET_REGULATION_DUTY_CYCLE sl_wisun_msg_reset_regulation_duty_cycle
 * @{
 *****************************************************************************/

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_reset_regulation_duty_cycle_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_reset_regulation_duty_cycle_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_reset_regulation_duty_cycle_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_reset_regulation_duty_cycle_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_RESET_REGULATION_DUTY_CYCLE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_EAP_IDENTITY sl_wisun_msg_set_eap_identity
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// EAP identity
  uint8_t identity[SL_WISUN_EAP_IDENTITY_SIZE];
  /// Length of EAP identity
  uint8_t identity_length;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_eap_identity_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_eap_identity_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_eap_identity_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_eap_identity_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_eap_identity_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_eap_identity_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_EAP_IDENTITY) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_FAN_TPS_VERSION sl_wisun_msg_set_fan_tps_version
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// FAN TPS version
  uint8_t fan_tps_version;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_fan_tps_version_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_fan_tps_version_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_fan_tps_version_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_fan_tps_version_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_fan_tps_version_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_fan_tps_version_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_FAN_TPS_VERSION) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_SET_RX_FIFO_SIZE sl_wisun_msg_set_rx_fifo_size
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Desired size of the RX FIFO in bytes in [64, 4096] (power of two)
  uint16_t size;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rx_fifo_size_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_set_rx_fifo_size_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rx_fifo_size_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rx_fifo_size_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_set_rx_fifo_size_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_set_rx_fifo_size_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_SET_RX_FIFO_SIZE) */

/**************************************************************************//**
 * @defgroup SL_WISUN_MSG_REGISTER_ADDRESS sl_wisun_msg_register_address
 * @{
 *****************************************************************************/

/// Request message body
SL_PACK_START(1)
typedef struct {
  /// Target Address
  in6_addr_t target_address;
  /// Registration lifetime in minutes
  uint16_t registration_lifetime_m;
  /// Reserved, set to zero
  uint16_t reserved;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_register_address_req_body_t;
SL_PACK_END()

/// Request message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Request message body
  sl_wisun_msg_register_address_req_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_register_address_req_t;
SL_PACK_END()

/// Confirmation message body
SL_PACK_START(1)
typedef struct {
  /// Status of the request
  uint32_t status;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_register_address_cnf_body_t;
SL_PACK_END()

/// Confirmation message
SL_PACK_START(1)
typedef struct {
  /// Common message header
  sl_wisun_msg_header_t header;
  /// Confirmation message body
  sl_wisun_msg_register_address_cnf_body_t body;
} SL_ATTRIBUTE_PACKED sl_wisun_msg_register_address_cnf_t;
SL_PACK_END()

/** @} (end SL_WISUN_MSG_REGISTER_ADDRESS) */


/**************************************************************************//**
 * @brief Send a request to the Wi-SUN stack
 *
 * This function provides an interface for sending requests to the Wi-SUN
 * stack and receiving confirmations. When using blocking requests, the function
 * blocks until the request is processed and a confirmation is available.
 *
 * REQ/CNF buffer layout:
 * request:      | Header (4B) | Request body (varies)       | Trailing data (if any) |
 * confirmation: | Header (4B) | Confirmation body (varies)  | Trailing data (if any) |
 *
 * Header structure:
 * - length: uint16_t - Total message length including header and trailing data
 * - id: uint8_t - Message identifier
 * - info: uint8_t - Message flags and metadata
 *
 * @pre req != NULL && cnf != NULL
 * @pre req_len >= sizeof(sl_wisun_msg_header_t)
 * @pre cnf_len >= sizeof(sl_wisun_msg_header_t) + sizeof(uint32_t) (header + status)
 * @pre req->header.length == req_len (length consistency check)
 *
 * @param[in] req Pointer to the request message buffer
 * @param[in] req_len Length of the request message in bytes (must match req->header.length)
 * @param[out] cnf Pointer to confirmation message buffer
 * @param[in] cnf_len Length of the confirmation message buffer in bytes
 *
 * @return SL_STATUS_OK Request processed successfully
 * @return SL_STATUS_INVALID_PARAMETER Invalid parameters or buffer size mismatch
 * @return SL_STATUS_NULL_POINTER NULL pointer parameter
 * @return SL_STATUS_FAIL Internal processing error or handler failure
 *
 * @note Blocking behavior: When using blocking requests, this function blocks the calling thread until the
 *       request is processed.
 *
 * @see sl_wisun_msg_header_t for header structure details
 *****************************************************************************/
 sl_status_t sl_wisun_send_request(const void *req,
                                   uint16_t req_len,
                                   void *cnf,
                                   uint16_t cnf_len);

/** @} (end SL_WISUN_MSG_API) */

#endif  // SL_WISUN_MSG_API_H
