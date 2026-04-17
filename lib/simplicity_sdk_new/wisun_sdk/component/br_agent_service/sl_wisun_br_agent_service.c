/***************************************************************************//**
 * @file sl_wisun_br_agent_service.c
 * @brief Wi-SUN Border Router Agent Service
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include "sl_assert.h"
#include "sl_string.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "sl_memory_manager.h"
#include "sl_wisun_types.h"
#include "border_router/sl_wisun_br_api.h"
#include "sl_wisun_br_wifi.h"
#include "sl_wisun_br_agent_service_config.h"
#include "sl_wisun_br_agent_service.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_app_br_core.h"
#include "sl_wisun_app_setting_br.h"
#include "errno.h"

#ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
#include "sl_wisun_br_dhcpv6_server.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Agent Service Thread stack size in words
#define SL_WISUN_BR_AGENT_SERVICE_STACK_SIZE_WORD         (256UL)

// Agent Service buffer size
#define SL_WISUN_BR_AGENT_SERVICE_BUFF_SIZE               (2048U)

// Agent Service Request/Response codes
#define SL_WISUN_BR_AGENT_SERVICE_CODE_GET_TOPOLOGY       (0x01U)
#define SL_WISUN_BR_AGENT_SERVICE_CODE_GET_CONFIG_PARAMS  (0x02U)
#define SL_WISUN_BR_AGENT_SERVICE_CODE_SET_CONFIG_PARAMS  (0x03U)
#define SL_WISUN_BR_AGENT_SERVICE_CODE_RESTART_BR         (0x04U)
#define SL_WISUN_BR_AGENT_SERVICE_CODE_STOP_BR            (0x05U)

// Agent Service event flags
#define SL_WISUN_BR_AGENT_WIFI_CONNECTED_EVT_FLAG         (1U << 0U)

// Agent Service message type
typedef struct sl_wisun_br_agent_service_msg {
  // Request/Response code
  uint32_t msg_code;
  // Payload length
  uint32_t payload_len;
  // Payload data
  uint8_t *payload;
} sl_wisun_br_agent_service_msg_t;
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Agent Service task function
 * @details This function is the main task for the Agent Service
 *
 * @param[in] args Arguments
 *****************************************************************************/
static void _agent_service_task_fnc(void *args);

/**************************************************************************//**
 * @brief Parse received TCP message
 * @details This function parses the received TCP message
 *
 * @param[in] buff Pointer to the received message buffer
 * @param[in] buff_len Length of the received message buffer
 * @param[out] parsed_msg Pointer to the output message structure
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
static sl_status_t _parse_received_msg(const uint8_t * const buff,
                                       uint32_t buff_len,
                                       sl_wisun_br_agent_service_msg_t * const parsed_msg);

/**************************************************************************//**
 * @brief Create and send response message
 * @details This function creates and sends the response message based on the request type
 *
 * @param[in] parsed_msg Pointer to the parsed message structure
 * @param[in] clnt_conn Client connection
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
static sl_status_t _create_and_send_resp_msg(const sl_wisun_br_agent_service_msg_t * const parsed_msg,
                                             struct netconn *clnt_conn);

/**************************************************************************//**
 * @brief Send message
 * @details This function sends a message to the specified socket
 *
 * @param[in] clnt_conn Client connection
 * @param[in] resp_msg Pointer to the response message structure
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
static sl_status_t _send_msg(struct netconn *clnt_conn,
                             const sl_wisun_br_agent_service_msg_t * const resp_msg);

/**************************************************************************//**
 * @brief Get network topology
 * @details This function retrieves the network topology information
 *
 * @param[out] resp_msg Pointer to the response message structure
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
static sl_status_t _get_network_topology(sl_wisun_br_agent_service_msg_t * const resp_msg);

/**************************************************************************//**
 * @brief Get configuration parameters
 * @details This function retrieves the configuration parameters information
 *
 * @param[out] resp_msg Pointer to the response message structure
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
static sl_status_t _get_config_params(sl_wisun_br_agent_service_msg_t * const resp_msg);

/**************************************************************************//**
 * @brief Set configuration parameters
 * @details This function sets the configuration parameters information
 *
 * @param[in] parsed_msg Pointer to the parsed message structure
 * @return SL_STATUS_OK on success, error code otherwise
 *****************************************************************************/
static sl_status_t _set_config_params(const sl_wisun_br_agent_service_msg_t * const parsed_msg);

/**************************************************************************//**
 * @brief Check if a pointer is within a buffer
 *
 * @param ptr Pointer to check
 * @param buff_ptr Pointer to the buffer
 * @param buff_size Size of the buffer
 * @return bool True if the pointer is within the buffer, false otherwise
 *****************************************************************************/
__STATIC_INLINE bool _is_ptr_in_buff(const uint8_t * const ptr,
                                     const uint8_t * const buff_ptr,
                                     const uint16_t buff_size);

/**************************************************************************//**
 * @brief Acquire Agent Service mutex
 * @details Internal mutex lock
 *****************************************************************************/
__STATIC_INLINE void _agent_service_mutex_acquire(void);

/**************************************************************************//**
 * @brief Release Agent Service mutex
 * @details Internal mutex release
 *****************************************************************************/
__STATIC_INLINE void _agent_service_mutex_release(void);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// Agent Service task ID
static osThreadId_t _agent_service_task = NULL;

// Agent Service task attributes
static const osThreadAttr_t _agent_service_task_attr = {
  .name       = "AgentServiceTask",
  .attr_bits  = osThreadDetached,
  .cb_mem     = NULL,
  .cb_size    = 0UL,
  .stack_mem  = NULL,
  .stack_size = (SL_WISUN_BR_AGENT_SERVICE_STACK_SIZE_WORD * sizeof(void *)) & 0xFFFFFFF8U,
  .priority   = osPriorityNormal1,
  .tz_module  = 0UL,
  .reserved   = 0UL
};

// Remote address of the host Agent Service
static ip_addr_t _remote_addr = { 0 };

// Agent Service mutex
static osMutexId_t _agent_service_mtx = NULL;

// Agent Service mutex attribute
static const osMutexAttr_t _agent_service_mtx_attr = {
  .name      = "AgentService_mtx",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0UL
};

static osEventFlagsId_t agent_evt_flags;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_wisun_br_agent_service_init(void)
{
  // Create mutex
  _agent_service_mtx = osMutexNew(&_agent_service_mtx_attr);
  EFM_ASSERT(_agent_service_mtx != NULL);

  // Create Agent Service thread
  _agent_service_task = osThreadNew(_agent_service_task_fnc,
                                    NULL,
                                    &_agent_service_task_attr);
  EFM_ASSERT(_agent_service_task != NULL);

  agent_evt_flags = osEventFlagsNew(NULL);
  EFM_ASSERT(agent_evt_flags != NULL);

  // Init remote address to default value
  (void) ipaddr_aton(SL_WISUN_BR_BRIDGE_AGENT_DEFAULT_ADDR, &_remote_addr);
}

sl_status_t sl_wisun_br_agent_service_send_graph_info(void)
{
  sl_wisun_br_agent_service_msg_t resp_msg = { 0 };
  struct netconn *conn = NULL;

  if (_get_network_topology(&resp_msg) != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  conn = netconn_new(NETCONN_TCP_IPV6);
  if (conn == NULL) {
    sl_free(resp_msg.payload);
    return SL_STATUS_FAIL;
  }

  // connect to the server
  if (netconn_connect(conn, &_remote_addr,
                      SL_WISUN_BR_BRIDGE_AGENT_DEFAULT_PORT) != ERR_OK) {
    sl_free(resp_msg.payload);
    netconn_close(conn);
    netconn_delete(conn);
    return SL_STATUS_FAIL;
  }

  // wait for connection to be established
  osDelay(1000U);

  // send the response message and cleanup
  (void)_send_msg(conn, &resp_msg);
  sl_free(resp_msg.payload);
  netconn_close(conn);
  netconn_delete(conn);

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_agent_service_set_bridge_agent_addr(const char *remote_address)
{
  sl_status_t result = SL_STATUS_OK;

  _agent_service_mutex_acquire();
  if (remote_address == NULL) {
    result =  SL_STATUS_NULL_POINTER;
  } else {
    if (ipaddr_aton(remote_address, &_remote_addr) == 0) {
      result = SL_STATUS_FAIL;
    }
  }
  _agent_service_mutex_release();

  return result;
}

const char *sl_wisun_br_agent_service_get_bridge_agent_addr(void)
{
  static char *addr_str = NULL;
  const size_t buf_size = 40U;
  addr_str = sl_malloc(buf_size);
  if (addr_str == NULL) {
    return NULL;
  }

  _agent_service_mutex_acquire();
  if (ipaddr_ntoa_r(&_remote_addr, addr_str, buf_size) == NULL) {
    addr_str[0] = '\0';
  }
  _agent_service_mutex_release();

  return addr_str;
}

sl_status_t sl_wisun_br_agent_service_send_reg(void)
{
  struct netconn *conn = NULL;
  err_t err = ERR_OK;
  sl_wisun_br_agent_service_msg_t resp_msg = { 0 };
  sl_status_t status = SL_STATUS_OK;

  // get new network topology
  if (_get_config_params(&resp_msg) != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  // change message code to set config params
  resp_msg.msg_code = SL_WISUN_BR_AGENT_SERVICE_CODE_SET_CONFIG_PARAMS;
  // create client socket
  conn = netconn_new(NETCONN_TCP_IPV6);
  if (conn == NULL) {
    sl_free(resp_msg.payload);
    return SL_STATUS_FAIL;
  }

  // connect to the server
  err = netconn_connect(conn, &_remote_addr, SL_WISUN_BR_BRIDGE_AGENT_DEFAULT_PORT);
  if (err != ERR_OK) {
    sl_free(resp_msg.payload);
    netconn_close(conn);
    netconn_delete(conn);
    return SL_STATUS_FAIL;
  }

  // wait for connection to be established
  osDelay(1000U);

  // send the response message and cleanup
  if (_send_msg(conn, &resp_msg) != SL_STATUS_OK) {
    sl_free(resp_msg.payload);
    netconn_close(conn);
    netconn_delete(conn);
    return SL_STATUS_FAIL;
  }

  sl_free(resp_msg.payload);
  netconn_close(conn);
  netconn_delete(conn);

  // Send the graph info too
  status = sl_wisun_br_agent_service_send_graph_info();

  return status;
}

void sl_wisun_agent_start_service(void)
{
  (void) osEventFlagsSet(agent_evt_flags, SL_WISUN_BR_AGENT_WIFI_CONNECTED_EVT_FLAG);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void _agent_service_task_fnc(void *args)
{
  uint8_t *data = NULL;
  struct netbuf *buf = NULL;
  struct netconn *conn = NULL;
  struct netconn *newconn = NULL;
  sl_wisun_br_agent_service_msg_t recv_msg = { 0 };
  ip_addr_t srv_ipaddr = { 0U };
  uint16_t len = 0U;
  err_t err = ERR_OK;

  // Wi-Fi related params
  bool wifi_connected = false;
  uint16_t wifi_channel_number = 0U;
  uint8_t wifi_mac_address[6] = { 0U };

  (void) args;
  (void) osEventFlagsWait(agent_evt_flags,
                          SL_WISUN_BR_AGENT_WIFI_CONNECTED_EVT_FLAG,
                          osFlagsWaitAny,
                          osWaitForever);
  sl_wisun_br_wifi_get_info(&wifi_connected, &wifi_channel_number,
                            wifi_mac_address, (uint8_t *)&srv_ipaddr.addr);
  if (!wifi_connected) {
    ip_addr_set_any(IPADDR_TYPE_V6, &srv_ipaddr);
  }

#if LWIP_IPV6_SCOPES
  srv_ipaddr.zone = 0;
#endif

  // create TCP socket
  conn = netconn_new(NETCONN_TCP_IPV6);
  EFM_ASSERT(conn != NULL);

  // bind address to the socket
  err = netconn_bind(conn, &srv_ipaddr, SL_WISUN_BR_AGENT_SERVICE_SERVER_PORT);
  EFM_ASSERT(err == ERR_OK);

  // listen on socket
  err = netconn_listen(conn);
  EFM_ASSERT(err == ERR_OK);

  printf("[Border Router Agent Service started. Listen on port %u]\n",
         SL_WISUN_BR_AGENT_SERVICE_SERVER_PORT);

  // waiting for connection request
  SL_WISUN_BR_AGENT_SERVICE_LOOP {
    err = netconn_accept(conn, &newconn);
    if (err != ERR_OK) {
      osDelay(1000UL);
      continue;
    }

    // receiver loop
    err = netconn_recv(newconn, &buf);
    if (err != ERR_OK) {
      netconn_close(newconn);
      netconn_delete(newconn);
      continue;
    }

    netbuf_data(buf, (void**)&data, &len);
    switch (len) {
      case 0L: // socket closed, EOF
        break;

      default: // default: data received
        data[len] = '\0';
        // parse the received message
        if (_parse_received_msg(data,
                                (size_t)len,
                                &recv_msg) != SL_STATUS_OK) {
          break;
        }

        // create and send response message
        if (_create_and_send_resp_msg(&recv_msg,
                                      newconn) != SL_STATUS_OK) {
          break;
        }
        break;
    }

    netbuf_delete(buf);
    netconn_close(newconn);
    netconn_delete(newconn);
    osDelay(1UL);
  }

  netconn_close(conn);
  netconn_delete(conn);
}

static sl_status_t _parse_received_msg(const uint8_t * const buff,
                                       uint32_t buff_len,
                                       sl_wisun_br_agent_service_msg_t * const parsed_msg)
{
  const uint8_t *ptr = buff;

  if ((buff == NULL)
      || (buff_len == 0U)
      || (parsed_msg == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // parse request/response code
  parsed_msg->msg_code = ntohl(*((uint32_t *)ptr));
  ptr += sizeof(uint32_t);
  if (!_is_ptr_in_buff(ptr, buff, buff_len)) {
    return SL_STATUS_FAIL;
  }

  // parse payload length
  parsed_msg->payload_len = ntohl(*((uint32_t *)ptr));

  // parse payload if present
  if (parsed_msg->payload_len > 0U) {
    ptr += sizeof(uint32_t);
    if (!_is_ptr_in_buff(ptr, buff, buff_len)) {
      return SL_STATUS_FAIL;
    }
    parsed_msg->payload = (uint8_t *)ptr;
  }
  return SL_STATUS_OK;
}

static sl_status_t _create_and_send_resp_msg(const sl_wisun_br_agent_service_msg_t * const parsed_msg,
                                             struct netconn *clnt_conn)
{
  sl_status_t ret = SL_STATUS_OK;
  sl_wisun_br_agent_service_msg_t resp_msg = { 0 };
  sl_wisun_br_state_t br_state = SL_WISUN_BR_STATE_INITIALIZED;

  if (parsed_msg == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // process get/set request
  switch (parsed_msg->msg_code) {
    case SL_WISUN_BR_AGENT_SERVICE_CODE_GET_TOPOLOGY:
      if (_get_network_topology(&resp_msg) != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }
      printf("[Border Router Agent: Topology requested]\n");
      break;

    case SL_WISUN_BR_AGENT_SERVICE_CODE_GET_CONFIG_PARAMS:
      if (_get_config_params(&resp_msg) != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }
      printf("[Border Router Agent: Configuration requested]\n");
      break;

    case SL_WISUN_BR_AGENT_SERVICE_CODE_SET_CONFIG_PARAMS:
      ret = _set_config_params(parsed_msg);
      printf("[Border Router Agent: Configuration %s]\n",
             (ret == SL_STATUS_OK) ? "updated" : "update failed");
      return ret;

    case SL_WISUN_BR_AGENT_SERVICE_CODE_RESTART_BR:
      (void) sl_wisun_br_stop();
    #ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
      (void) sl_wisun_br_dhcpv6_server_stop();
    #endif
      osDelay(1000U);
      if ((sl_wisun_br_get_state(&br_state) == SL_STATUS_OK)
          && (br_state != SL_WISUN_BR_STATE_OPERATIONAL)) {
        sl_wisun_app_br_core_start();
        return SL_STATUS_OK;
      } else {
        // BR is already running
        return SL_STATUS_FAIL;
      }

    case SL_WISUN_BR_AGENT_SERVICE_CODE_STOP_BR:
      if (sl_wisun_br_stop() != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }
    #ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
      (void)sl_wisun_br_dhcpv6_server_stop();
    #endif
      return SL_STATUS_OK;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  // send the response message
  ret = _send_msg(clnt_conn, &resp_msg);

  // cleanup
  sl_free(resp_msg.payload);

  return ret;
}

static sl_status_t _send_msg(struct netconn *clnt_conn,
                             const sl_wisun_br_agent_service_msg_t * const resp_msg)
{
  uint8_t *buff = NULL;
  uint8_t *ptr = NULL;
  uint32_t total_msg_size = 0U;
  err_t err = ERR_OK;

  if (resp_msg == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // calculate total message size
  total_msg_size = sizeof(resp_msg->msg_code)
                   + sizeof(resp_msg->payload_len)
                   + resp_msg->payload_len;

  // allocate buffer for response message
  buff = (uint8_t *)sl_malloc(total_msg_size);
  if (!buff) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // clear buffer
  memset(buff, 0U, total_msg_size);

  // fill buffer with response message structure
  ptr = buff;

  // copy response code
  *((uint32_t *)ptr) = htonl(resp_msg->msg_code);
  ptr += sizeof(uint32_t);

  // copy payload length
  *((uint32_t *)ptr) = htonl(resp_msg->payload_len);
  ptr += sizeof(uint32_t);

  // copy payload if present
  if ((resp_msg->payload_len > 0U) && (resp_msg->payload != NULL)) {
    memcpy(ptr, resp_msg->payload, resp_msg->payload_len);
  }

  // send the response message
  err = netconn_write(clnt_conn, buff, total_msg_size, NETCONN_COPY);
  if (err != ERR_OK) {
    sl_free(buff);
    return SL_STATUS_FAIL;
  }

  // cleanup buffer
  sl_free(buff);

  return SL_STATUS_OK;
}

static sl_status_t _get_network_topology(sl_wisun_br_agent_service_msg_t * const resp_msg)
{
  sl_wisun_br_routing_table_entry_t *routing_table = NULL;
  uint16_t routing_table_size = 0U;
  static sl_wisun_ip_address_t ll_addr = { 0 };
  static sl_wisun_ip_address_t gua_addr = { 0 };
  static sl_wisun_ip_address_t dodagid_addr = { 0 };

  if (resp_msg == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // set response code
  resp_msg->msg_code = SL_WISUN_BR_AGENT_SERVICE_CODE_GET_TOPOLOGY;

  // get the number of routing table entries
  if (sl_wisun_br_get_routing_table_entry_count(&routing_table_size) != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  // include the BR itself
  routing_table_size += 1U;

  // allocate memory for the routing table
  routing_table = (sl_wisun_br_routing_table_entry_t *)sl_malloc(routing_table_size
                                                                 * sizeof(sl_wisun_br_routing_table_entry_t));
  if (!routing_table) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // set payload length
  resp_msg->payload_len = routing_table_size * sizeof(sl_wisun_br_routing_table_entry_t);

  // clear routing table
  memset(routing_table, 0, routing_table_size * sizeof(sl_wisun_br_routing_table_entry_t));

  // add the BR entry as the first entry
  (void)sl_wisun_br_get_ip_addresses(ll_addr.address, gua_addr.address, dodagid_addr.address);
  memcpy(routing_table->target.address, gua_addr.address, sizeof(routing_table->target.address));
  memset(routing_table->preferred.address, 0U, sizeof(routing_table->preferred.address));
  memset(routing_table->backup.address, 0U, sizeof(routing_table->backup.address));

  // get the routing table entries
  if (routing_table_size > 1U) {
    routing_table_size -= 1U;
    if (sl_wisun_br_get_routing_table(&routing_table_size,
                                      &routing_table[1]) != SL_STATUS_OK) {
      sl_free(routing_table);
      return SL_STATUS_FAIL;
    }
  }

  // set payload
  resp_msg->payload = (uint8_t *)routing_table;

  return SL_STATUS_OK;
}

static sl_status_t _get_config_params(sl_wisun_br_agent_service_msg_t * const resp_msg)
{
  if (resp_msg == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  resp_msg->payload = (uint8_t *)sl_malloc(sizeof(app_setting_br_t));

  if (!resp_msg->payload) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // get current BR settings
  if (app_wisun_setting_br_get((app_setting_br_t *)resp_msg->payload) != SL_STATUS_OK) {
    sl_free(resp_msg->payload);
    return SL_STATUS_FAIL;
  }

  resp_msg->msg_code = SL_WISUN_BR_AGENT_SERVICE_CODE_GET_CONFIG_PARAMS;
  resp_msg->payload_len = sizeof(app_setting_br_t);

  return SL_STATUS_OK;
}

static sl_status_t _set_config_params(const sl_wisun_br_agent_service_msg_t * const parsed_msg)
{
  sl_status_t ret = SL_STATUS_OK;
  app_setting_br_t *new_settings = NULL;
  app_setting_br_t *br_settings = NULL;

  if (!parsed_msg || !parsed_msg->payload) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  br_settings = (app_setting_br_t *)sl_malloc(sizeof(app_setting_br_t));
  if (!br_settings) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // get current BR settings
  if (app_wisun_setting_br_get(br_settings) != SL_STATUS_OK) {
    sl_free(br_settings);
    return SL_STATUS_FAIL;
  }

  // preserve PAN ID
  new_settings = (app_setting_br_t *)parsed_msg->payload;
  new_settings->pan_id = br_settings->pan_id;

  ret = app_wisun_setting_br_set(new_settings);

  sl_free(br_settings);

  return ret;
}

__STATIC_INLINE bool _is_ptr_in_buff(const uint8_t * const ptr,
                                     const uint8_t * const buff_ptr,
                                     const uint16_t buff_size)
{
  return (bool) ((ptr >= buff_ptr) && (ptr < (buff_ptr + buff_size)));
}

__STATIC_INLINE void _agent_service_mutex_acquire(void)
{
  EFM_ASSERT(osMutexAcquire(_agent_service_mtx, osWaitForever) == osOK);
}

__STATIC_INLINE void _agent_service_mutex_release(void)
{
  EFM_ASSERT(osMutexRelease(_agent_service_mtx) == osOK);
}
