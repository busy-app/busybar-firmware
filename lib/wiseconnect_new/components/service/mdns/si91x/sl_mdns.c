/***************************************************************************/ /**
 * @file  sl_mdns.c
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_mdns.h"
#include "sl_slist.h"
#include "sl_si91x_driver.h"
#include "sl_string.h"
#include "sli_wifi_utility.h"

#define MAX_MDNS_SERVICE_COUNT 1
#define MDNSD_BUFFER_SIZE      1000

#define SI91X_MDNSD_INIT                    1 //! MDNSD Initialize
#define SI91X_MDNS_COMMAND_DISCOVER_SERVICE 2 //! MDNSD Discover Service
#define SI91X_MDNSD_REGISTER_SERVICE        3 //! MDNSD register service
#define SI91X_MDNSD_DEINIT                  6 //! MDNSD deinitialize
#define SI91X_MDNS_COMMAND_STOP_DISCOVERY   7 //! MDNSD Stop Service Discovery

// mdnsd init structure
typedef struct {
  uint8_t ip_version; // ip version
  uint8_t ttl[2];     // time to live
} SL_ATTRIBUTE_PACKED sl_si91x_mdns_init_t;

// mdnsd register-service structure
typedef struct {
  uint8_t port[2]; // port number
  uint8_t ttl[2];  // time to live
  uint8_t more;    // reset if it is last service in the list
} SL_ATTRIBUTE_PACKED sl_si91x_mdns_reg_srv_t;

// mdnsd discover-service structure
typedef struct {
  char host_srv_name[31]; // Full service name (e.g., "_http._tcp.local.")
  uint8_t type;           // Query type (e.g., PTR, SRV, A, AAAA)
  uint32_t timeout;       // Timeout for discovery in milliseconds
} SL_ATTRIBUTE_PACKED sl_si91x_mdns_discover_t;

// mdnsd structure
typedef struct {
  uint8_t command_type; // command type 1-MDNSD init, 3- Register service, 6- Deinit
  union {
    sl_si91x_mdns_init_t init;             // mdns init
    sl_si91x_mdns_reg_srv_t reg_srv;       // mdns register
    sl_si91x_mdns_discover_t discover_srv; // mdns discovery
  } req_conf;
  uint8_t buffer[MDNSD_BUFFER_SIZE]; //Data buffer
} SL_ATTRIBUTE_PACKED sl_si91x_mdns_req_t;

/**********************************************************************************************************************
 *                                                Static Functions                                                    *
**********************************************************************************************************************/
static void sli_si91x_clean_service_handle(sl_wifi_buffer_t *service_handle)
{
  const sl_mdns_service_t *service = NULL;
  uint16_t buffer_length           = 0;

  service = (sl_mdns_service_t *)sli_wifi_host_get_buffer_data(service_handle, 0, &buffer_length);
  free((char *)service->instance_name);
  free((char *)service->service_type);
  free((char *)service->service_message);
  sli_si91x_host_free_buffer(service_handle);

  return;
}

static void sli_si91x_clean_service_list(sl_mdns_interface_t *interface)
{
  sl_wifi_buffer_t *service = NULL;
  sl_wifi_buffer_t *block   = NULL;

  service = interface->service_list;

  while (service != NULL) {
    block   = service;
    service = (sl_wifi_buffer_t *)service->node.node;
    sli_si91x_clean_service_handle(block);
  }

  return;
}

static void sli_si91x_clean_mdns_handle(sl_mdns_t *mdns)
{
  sl_wifi_buffer_t *interface = NULL;
  sl_wifi_buffer_t *block     = NULL;
  sl_mdns_interface_t *in     = NULL;
  uint16_t buffer_length;

  interface = mdns->interface_list;

  while (interface != NULL) {
    block     = interface;
    interface = (sl_wifi_buffer_t *)interface->node.node;
    in        = (sl_mdns_interface_t *)sli_wifi_host_get_buffer_data(block, 0, &buffer_length);
    sli_si91x_clean_service_list(in);
    sli_si91x_host_free_buffer(block);
  }

  return;
}
/**********************************************************************************************************************
 *                                                Public Functions                                                    *
**********************************************************************************************************************/
sl_status_t sl_mdns_init(sl_mdns_t *mdns, const sl_mdns_configuration_t *config, sl_mdns_event_handler_t event_handler)
{
  sl_si91x_mdns_req_t req   = { 0 };
  sl_status_t status        = SL_STATUS_FAIL;
  uint32_t length           = 0;
  uint16_t ttl              = 0;
  uint32_t length_host_name = 0;

  memset(mdns, 0, sizeof(sl_mdns_t));
  memcpy(&(mdns->configuration), config, sizeof(sl_mdns_configuration_t));
  mdns->event_handler = event_handler;

  req.command_type         = SI91X_MDNSD_INIT;
  req.req_conf.init.ttl[0] = ttl & 0x00ff;
  req.req_conf.init.ttl[1] = (ttl >> 8) & 0x00ff;

  if (SL_IPV4_VERSION == config->type) {
    req.req_conf.init.ip_version = 4;
  } else if (SL_IPV6_VERSION == config->type) {
    req.req_conf.init.ip_version = 6;
  } else {
    return SL_STATUS_INVALID_TYPE;
  }

  length_host_name = sl_strnlen((char *)config->host_name, 32) + 1; // +1 for null terminator
  memcpy(req.buffer, config->host_name, length_host_name);
  length = sizeof(sl_si91x_mdns_req_t) - MDNSD_BUFFER_SIZE + length_host_name;

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_MDNSD,
                                         SLI_SI91X_NETWORK_CMD,
                                         &req,
                                         length,
                                         SLI_WLAN_RSP_MDNSD_WAIT_TIME,
                                         NULL,
                                         NULL);

  return status;
}

sl_status_t sl_mdns_deinit(sl_mdns_t *mdns)
{
  sl_si91x_mdns_req_t req = { 0 };
  sl_status_t status      = SL_STATUS_FAIL;
  uint32_t length         = 0;

  req.command_type = SI91X_MDNSD_DEINIT;
  length           = sizeof(sl_si91x_mdns_req_t) - MDNSD_BUFFER_SIZE;

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_MDNSD,
                                         SLI_SI91X_NETWORK_CMD,
                                         &req,
                                         length,
                                         SLI_WLAN_RSP_MDNSD_WAIT_TIME,
                                         NULL,
                                         NULL);

  sli_si91x_clean_mdns_handle(mdns);
  memset(mdns, 0, sizeof(sl_mdns_t));
  return status;
}

sl_status_t sl_mdns_add_interface(sl_mdns_t *mdns, sl_net_interface_t interface)
{
  sl_status_t status              = SL_STATUS_FAIL;
  sl_wifi_buffer_t *new_interface = NULL;
  sl_mdns_interface_t *in         = NULL;
  uint16_t buffer_length          = 0;

  status = sli_si91x_host_allocate_buffer(&new_interface, SL_WIFI_CONTROL_BUFFER, sizeof(sl_mdns_interface_t), 1000);
  VERIFY_STATUS_AND_RETURN(status);
  in               = (sl_mdns_interface_t *)sli_wifi_host_get_buffer_data(new_interface, 0, &buffer_length);
  in->interface    = interface;
  in->service_list = NULL;

  new_interface->node.node = (sl_slist_node_t *)mdns->interface_list;
  mdns->interface_list     = new_interface;

  return SL_STATUS_OK;
}

sl_status_t sl_mdns_remove_interface(const sl_mdns_t *mdns, sl_net_interface_t interface)
{
  UNUSED_PARAMETER(mdns);
  UNUSED_PARAMETER(interface);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_mdns_register_service(sl_mdns_t *mdns, sl_net_interface_t interface, const sl_mdns_service_t *service)
{
  sl_status_t status              = SL_STATUS_FAIL;
  sl_wifi_buffer_t *new_service   = NULL;
  sl_mdns_interface_t *in         = NULL;
  sl_mdns_service_t *srv          = NULL;
  sl_si91x_mdns_req_t req         = { 0 };
  uint32_t length                 = 0;
  uint16_t buffer_length          = 0;
  uint32_t length_service_type    = 0;
  uint32_t length_instance_name   = 0;
  uint32_t length_service_message = 0;

  if (NULL == mdns->interface_list) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (mdns->service_count >= MAX_MDNS_SERVICE_COUNT) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == service->service_type) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == service->service_type) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == service->service_message) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  length_service_type    = sl_strnlen((char *)service->service_type, MDNSD_BUFFER_SIZE) + 1;
  length_instance_name   = sl_strnlen((char *)service->instance_name, MDNSD_BUFFER_SIZE) + 1;
  length_service_message = sl_strnlen((char *)service->service_message, MDNSD_BUFFER_SIZE) + 1;

  if ((length_service_type + length_instance_name + length_service_message) > MDNSD_BUFFER_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (sl_wifi_buffer_t *target_in = mdns->interface_list; (NULL != target_in);
       target_in                   = (sl_wifi_buffer_t *)target_in->node.node) {
    in = (sl_mdns_interface_t *)sli_wifi_host_get_buffer_data(target_in, 0, &buffer_length);

    if (in->interface == interface) {
      break;
    } else {
      in = NULL;
    }
  }

  if (NULL == in) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sli_si91x_host_allocate_buffer(&new_service, SL_WIFI_CONTROL_BUFFER, sizeof(sl_mdns_service_t), 1000);
  VERIFY_STATUS_AND_RETURN(status);
  srv                = (sl_mdns_service_t *)sli_wifi_host_get_buffer_data(new_service, 0, &buffer_length);
  srv->instance_name = malloc(length_instance_name);
  memcpy((char *)srv->instance_name, service->instance_name, length_instance_name);

  srv->service_type = malloc(length_service_type);
  memcpy((char *)srv->service_type, service->service_type, length_service_type);

  srv->service_message = malloc(length_service_message);
  memcpy((char *)srv->service_message, service->service_message, length_service_message);

  srv->port = service->port;
  srv->ttl  = service->ttl;

  // Prepare Firmware request
  req.command_type             = SI91X_MDNSD_REGISTER_SERVICE;
  req.req_conf.reg_srv.port[0] = service->port & 0x00ff;
  req.req_conf.reg_srv.port[1] = (service->port >> 8) & 0x00ff;
  req.req_conf.reg_srv.ttl[0]  = service->ttl & 0x00ff;
  req.req_conf.reg_srv.ttl[1]  = (service->ttl >> 8) & 0x00ff;
  req.req_conf.reg_srv.more    = 0;
  length                       = 0;

  memcpy((char *)req.buffer, service->service_type, length_service_type);
  length += length_service_type;

  memcpy((char *)((req.buffer) + length), service->instance_name, length_instance_name);
  length += length_instance_name;

  memcpy((char *)((req.buffer) + length), service->service_message, length_service_message);
  length += length_service_message + (sizeof(sl_si91x_mdns_req_t) - MDNSD_BUFFER_SIZE);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_MDNSD,
                                         SLI_SI91X_NETWORK_CMD,
                                         &req,
                                         length,
                                         SLI_WLAN_RSP_MDNSD_WAIT_TIME,
                                         NULL,
                                         NULL);
  if (SL_STATUS_OK != status) {
    sli_si91x_clean_service_handle(new_service);
    VERIFY_STATUS_AND_RETURN(status);
  }

  new_service->node.node = (sl_slist_node_t *)in->service_list;
  in->service_list       = new_service;
  mdns->service_count++;
  return SL_STATUS_OK;
}

sl_status_t sl_mdns_unregister_service(const sl_mdns_t *mdns, const sl_mdns_service_query_t *service_query)
{
  UNUSED_PARAMETER(mdns);
  UNUSED_PARAMETER(service_query);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_mdns_update_service_message(const sl_mdns_t *mdns,
                                           const sl_mdns_service_query_t *service_query,
                                           const char *message,
                                           uint32_t message_length)
{
  UNUSED_PARAMETER(mdns);
  UNUSED_PARAMETER(service_query);
  UNUSED_PARAMETER(message);
  UNUSED_PARAMETER(message_length);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_mdns_service_discovery_start(const sl_mdns_t *mdns,
                                            sl_net_interface_t interface,
                                            const sl_mdns_service_query_t *service_query)
{
  UNUSED_PARAMETER(mdns);
  UNUSED_PARAMETER(interface);

  // NULL check for service_query
  if (service_query == NULL || service_query->service_type == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (service_query->service_type[0] == '\0') {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_si91x_mdns_req_t req = { 0 };
  sl_status_t status      = SL_STATUS_FAIL;

  // Set the discover command type
  req.command_type = SI91X_MDNS_COMMAND_DISCOVER_SERVICE;

  // Calculate length of service_type string
  uint32_t service_type_len =
    sl_strnlen((char *)service_query->service_type, sizeof(req.req_conf.discover_srv.host_srv_name) - 1);

  memcpy((char *)req.req_conf.discover_srv.host_srv_name, service_query->service_type, service_type_len);

  // Ensure null termination
  req.req_conf.discover_srv.host_srv_name[service_type_len] = '\0';

  // Fill the discover service fields
  req.req_conf.discover_srv.type    = (uint8_t)service_query->query_type;
  req.req_conf.discover_srv.timeout = service_query->timeout;

  // Send command to firmware
  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_DISCOVER_SERVICE,
                                         SLI_SI91X_NETWORK_CMD,
                                         &req,
                                         sizeof(sl_si91x_mdns_req_t),
                                         SLI_WIFI_RETURN_IMMEDIATELY,
                                         NULL,
                                         NULL);

  return status;
}

sl_status_t sl_mdns_service_discovery_stop(const sl_mdns_t *mdns, const sl_net_interface_t interface)
{
  UNUSED_PARAMETER(mdns);
  UNUSED_PARAMETER(interface);
  sl_si91x_mdns_req_t req = { 0 };
  sl_status_t status      = SL_STATUS_FAIL;

  // Set the discover command type
  req.command_type = SI91X_MDNS_COMMAND_STOP_DISCOVERY;

  // Send command to firmware
  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_MDNSD,
                                         SLI_SI91X_NETWORK_CMD,
                                         &req,
                                         sizeof(sl_si91x_mdns_req_t),
                                         SLI_WLAN_RSP_MDNSD_WAIT_TIME,
                                         NULL,
                                         NULL);

  return status;
}