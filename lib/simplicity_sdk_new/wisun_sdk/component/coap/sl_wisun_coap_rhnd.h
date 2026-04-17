/***************************************************************************//**
 * @file sl_wisun_coap_rhnd.h
 * @brief Wi-SUN CoAP resource handler
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

#ifndef SL_WISUN_COAP_RHND_H
#define SL_WISUN_COAP_RHND_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdbool.h>
#include <stdint.h>

#include "sl_status.h"
#include "sl_wisun_coap.h"
#include "sl_wisun_coap_config.h"
#include "socket/socket.h"

/**************************************************************************//**
 * @addtogroup SL_WISUN_COAP_RHND_API CoAP Resource Handler API
 * @ingroup SL_WISUN_COAP_API
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * @addtogroup SL_WISUN_COAP_RHND_TYPES Type definitions
 * @ingroup SL_WISUN_COAP_RHND_API
 * @{
 *****************************************************************************/

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Auto-Response callback for resources
typedef sl_wisun_coap_packet_t *
(*sl_wisun_coap_rhnd_auto_resp_t)(const sl_wisun_coap_packet_t * const req_packet);

/// Extended auto response callback for resources.
typedef sl_wisun_coap_packet_t *
(*sl_wisun_coap_rhnd_auto_resp_ext_t)(const sockaddr_in6_t * const src_addr,
                                      const sl_wisun_coap_packet_t * const req_packet);

/// Redirect response callback type to create new destination socket address
typedef void (*sl_wisun_coap_rhnd_redirect_resp_t)(sockaddr_in6_t * const new_addr,
                                                   const sl_wisun_coap_packet_t * const req_packet);

/// Resource table data
typedef struct sl_wisun_coap_rhnd_resource_data {
  /// resource URI path string
  const char * uri_path;
  /// resource type string
  const char * resource_type;
  /// inteface type string
  const char * interface;
} sl_wisun_coap_rhnd_resource_data_t;

/// Resource table entry
typedef struct sl_wisun_coap_rhnd_resource {
  /// Resource data
  sl_wisun_coap_rhnd_resource_data_t data;
  /// Resource discoverable
  bool discoverable;
  /// Auto response
  sl_wisun_coap_rhnd_auto_resp_t auto_response;
  /// Auto response extended
  /// If it's registered in the resource descriptor,
  /// it will be called instead of auto_response
  sl_wisun_coap_rhnd_auto_resp_ext_t auto_response_ext;
  /// Redirect response callback
  sl_wisun_coap_rhnd_redirect_resp_t redirect_response;
  /// Next pointer
  struct sl_wisun_coap_rhnd_resource * next;
} sl_wisun_coap_rhnd_resource_t;

/** @} (end SL_WISUN_COAP_RESOURCE_TYPES) */

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Initialization of the CoAP Resource Discovery internals.
 *****************************************************************************/
void sl_wisun_coap_rhnd_init(void);

/**************************************************************************//**
 * @brief Initialise empty resource
 * @details Set pointers to NULL and flags to false
 * @param[in] src_resource Resource descriptor
 *****************************************************************************/
__STATIC_INLINE void sl_wisun_coap_rhnd_resource_init(sl_wisun_coap_rhnd_resource_t * const src_resource)
{
  src_resource->data.uri_path = NULL;
  src_resource->data.resource_type = NULL;
  src_resource->data.interface = NULL;
  src_resource->discoverable = false;
  src_resource->auto_response = NULL;
  src_resource->auto_response_ext = NULL;
  src_resource->redirect_response = NULL;
  src_resource->next = NULL;
}

/**************************************************************************//**
 * @brief Register a new resource into the CoAP Resource table
 * @param[in] src_resource the new resource's descriptor
 * @return True if the registration is successful
 *****************************************************************************/
sl_status_t sl_wisun_coap_rhnd_resource_add(const sl_wisun_coap_rhnd_resource_t * const src_resource);

/**************************************************************************//**
 * @brief Remove a resource from the CoAP Resource table referenced by name
 * @param[in] uri_path URI path of the resource to be removed
 * @return True if removal is successful
 *****************************************************************************/
sl_status_t sl_wisun_coap_rhnd_resource_remove_by_uri(const char * uri_path);

/**************************************************************************//**
 * @brief Get resources
 * @details Getting linked list of resources
 * @return const sl_wisun_coap_rhnd_resource_t* Resource list, on error is NULL ptr
 *****************************************************************************/
const sl_wisun_coap_rhnd_resource_t * sl_wisun_coap_rhnd_get_resources(void);

/**************************************************************************//**
 * @brief Set auto-response callback
 * @details Set callback for resource
 * @param[in] uri_path URI path
 * @param[in] response Response callback
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_FAIL on error
 *****************************************************************************/
sl_status_t sl_wisun_coap_rhnd_set_auto_response(const char * uri_path,
                                                 sl_wisun_coap_rhnd_auto_resp_t response);

/**************************************************************************//**
 * @brief Reset auto-response callback
 * @details Set callback ptr to NULL in resource descriptor
 * @param[in] uri_path URI path
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_FAIL on error
 *****************************************************************************/
sl_status_t sl_wisun_coap_rhnd_reset_auto_response(const char * uri_path);

/**************************************************************************//**
 * @brief Print resources
 * @details Print resource URI paths
 *****************************************************************************/
void sl_wisun_coap_rhnd_print_resources(void);

#if SL_WISUN_COAP_RESOURCE_HND_SERVICE_ENABLE

/**************************************************************************//**
 * @brief Response packet received handler
 * @details Weak function, used in the service.
 *          If simple handler is implemented: simple handler will be executed
 *          If extended handler is implemented: extended handler will be executed
 *          If both are implemented: extended handler will be executed
 * @param[in,out] req_packet Request packet
 *****************************************************************************/
void sl_wisun_coap_rhnd_service_resp_received_hnd(sl_wisun_coap_packet_t * req_packet);

/**************************************************************************//**
 * @brief Response packet received extended handler
 * @details Weak function, used in the service
 *          If simple handler is implemented: simple handler will be executed
 *          If extended handler is implemented: extended handler will be executed
 *          If both are implemented: extended handler will be executed
 * @param[in] src_addr Address of the source
 * @param[in,out] req_packet Request packet
 *****************************************************************************/
void sl_wisun_coap_rhnd_service_resp_received_ext_hnd(const sockaddr_in6_t * const src_addr,
                                                      sl_wisun_coap_packet_t * req_packet);

/**************************************************************************//**
 * @brief URI path string error handler
 * @details Weak function, used in the service
 * @param[in,out] req_packet Request packet
 *****************************************************************************/
void sl_wisun_coap_rhnd_service_uri_path_error_hnd(sl_wisun_coap_packet_t * req_packet);

/**************************************************************************//**
 * @brief Response message buffer overflow error handler
 * @details Weak function, used in the service
 * @param[in,out] req_packet Request packet
 * @param[in,out] resp_packet Response packet
 *****************************************************************************/
void sl_wisun_coap_rhnd_service_buff_ovflow_error_hnd(sl_wisun_coap_packet_t * req_packet,
                                                      sl_wisun_coap_packet_t * resp_packet);

/**************************************************************************//**
 * @brief Response message build error handler
 * @details Weak function, used in the service
 * @param[in,out] req_packet Request packet
 * @param[in,out] resp_packet Response packet
 *****************************************************************************/
void sl_wisun_coap_rhnd_service_packet_build_error_hnd(sl_wisun_coap_packet_t * req_packet,
                                                       sl_wisun_coap_packet_t * resp_packet);

/**************************************************************************//**
 * @brief Response message senderror handler
 * @details Weak function, used in the service
 * @param[in,out] req_packet Request packet
 * @param[in,out] resp_packet Response packet
 *****************************************************************************/
void sl_wisun_coap_rhnd_service_resp_send_error_hnd(sl_wisun_coap_packet_t * req_packet,
                                                    sl_wisun_coap_packet_t * resp_packet);

#endif

/**************************************************************************//**
 * @brief Is request packet?
 * @details helper function
 * @param[in] packet Packet
 * @return true Request packet
 * @return false Response or Empty packet
 *****************************************************************************/
bool sl_wisun_coap_rhnd_is_request_packet(const sl_wisun_coap_packet_t * const packet);

/** @}*/

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_COAP_RHND_H
