/***************************************************************************//**
 * @file sl_wisun_app_status.c
 * @brief Wi-SUN Application Status CoAP notification
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <string.h>
#include <stdio.h>

#include "sl_assert.h"
#include "em_device.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_string.h"
#include "socket/socket.h"
#include "app_project_info.h"
#include "sl_component_catalog.h"
#include "sl_wisun_types.h"
#include "sl_wisun_api.h"
#include "sl_wisun_app_status.h"
#include "sl_wisun_app_status_config.h"
#include "sl_wisun_coap.h"
#include "sl_wisun_coap_notify.h"
#include "sl_wisun_trace_util.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_app_core_util.h"
#include "sl_memory_manager.h"

#if SL_WISUN_APP_STATUS_COAP_RESOURCE_ENABLE
#include "sl_wisun_coap_rhnd.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#if !defined(SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE) \
  || !SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
  #warning 'SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE' is not set. Value must be 1.
#endif

/// Notification payload size
#define SL_WISUN_APP_STATUS_NOTIF_PAYLOAD_SIZE            1024UL

/// Max neighbor count included in the neighbour info
#define SL_WISUN_APP_STATUS_NOTIF_MAX_NEIGHBOUR_CNT       8UL

/// App status notification coap msg ID
#define SL_WISUN_APP_STATUS_NOTIF_MSG_ID                  40001U

/// Application version string length
#define SL_WISUN_APP_STATUS_VERSION_STR_LEN               64U

/// Application version format string
#define SL_WISUN_APP_STATUS_VERSION_FORMAT_STR            "v%lu.%lu.%lu"

#if SL_WISUN_APP_STATUS_COAP_RESOURCE_ENABLE
/// Resource type for application status
#define SL_WISUN_APP_STATUS_RESOURCE_RT_STATUS            "status"

/// App status interface app
#define SL_WISUN_APP_STATUS_RESOURCE_IF_STATUS            "app"

/// Not valid app status request response string
#define SL_WISUN_APP_STATUS_NOT_VALID_STATUS_RESP         "[Not valid app status request]"

/// App status failed response string
#define SL_WISUN_APP_STATUS_FAILED_RESP                   "[App status failed]"
#endif

/// Neighbour info notification json header str
#define SL_WISUN_APP_STATUS_NOTIF_NEIGHBOUR_INFO_JSON_HEADER_STR \
  "\"nb_info\": {\n"                                             \
  "  \"ip\":[\"t\",\"lt\",\"txc\",\"txf\",\"txms\",\"txmsf\",\"rpl\",\"etx\",\"rslo\",\"rsli\"],\n"

/// Neighbour json entry format string
#define SL_WISUN_APP_STATUS_NEIGHBOUR_FORMAT_STR \
  "  \"%s\":[%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u]"

/// Time statistic json format string
#define SL_WISUN_APP_STATUS_TIME_STAT_JSON_STR \
  "\"stat\": {\n"                              \
  "  \"run\": \"%s\",\n"                       \
  "  \"conn_cnt\": %lu,\n"                     \
  "  \"conn\": \"%s\",\n"                      \
  "  \"tot_conn\": \"%s\",\n"                  \
  "  \"disconn\": \"%s\",\n"                   \
  "  \"tot_disconn\": \"%s\",\n"               \
  "  \"available\": \"%u.%02u%%\"\n"           \
  "}"

/// Device info json format string
#define SL_WISUN_APP_STATUS_DEVICE_INFO_JSON_STR    \
  "\"dev_info\": {\n"                               \
  "  \"hw\": \"%s %s %s\",\n"                       \
  "  \"sw\": \"%s %s (stack: v%lu.%lu.%lu %s)\",\n" \
  "  \"mac\": \"%s\",\n"                            \
  "  \"ip\": \"%s\"\n"                              \
  "}"

/// Default application name
#define SL_WISUN_APP_STATUS_DEFAULT_APP_NAME        "Wi-SUN"

/// Print to buff macro function
#define __print_to_buff(__r, __buf, __buf_len, __format, ...)              \
  do {                                                                     \
    __r = snprintf((char *)(__buf), (__buf_len), __format, ##__VA_ARGS__); \
    if (__r > 0L) {                                                        \
      (__buf) += __r;                                                      \
      (__buf_len) -= __r;                                                  \
    }                                                                      \
  } while (0)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Notify handler function
 * @details Callback for notification service
 * @param[in] notify Notification instance
 * @return sl_wisun_coap_packet_t * Built CoAP packet
 *****************************************************************************/
static sl_wisun_coap_packet_t * _notify_hnd(const struct sl_wisun_coap_notify *notify);

#if SL_WISUN_APP_STATUS_COAP_RESOURCE_ENABLE
/**************************************************************************//**
 * @brief Build constant response packet
 * @details Build response packet with constant string and with given response code
 * @param[in] req_packet Request packet
 * @param[in] resp_str Response string
 * @param[in] code Response code
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t * _build_const_resp(const sl_wisun_coap_packet_t * const req_packet,
                                                  const char * const resp_str,
                                                  const sn_coap_msg_code_e code);
#endif

/**************************************************************************//**
 * @brief Build neighbour info
 * @details Helper function
 * @param[in,out] buf Buffer pointer to start, pointer is updated
 * @param[in,out] buf_len Available buffer length, value is updated
 *****************************************************************************/
static void _build_neighbour_info(uint8_t **buf, uint16_t *buf_len);

#if SL_WISUN_APP_STATUS_COAP_RESOURCE_ENABLE
/**************************************************************************//**
 * @brief Prepare app status response packet
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_app_status_response_cb(const sl_wisun_coap_packet_t * const req_packet);
#endif

/**************************************************************************//**
 * @brief Build device info
 * @details Helper function
 * @param[in,out] buf Buffer pointer to start, pointer is updated
 * @param[in,out] buf_len Available buffer length, value is updated
 *****************************************************************************/
static void _build_device_info(uint8_t **buf, uint16_t *buf_len);

/**************************************************************************//**
 * @brief Build time statistic
 * @details Helper function
 * @param[in,out] buf Buffer pointer to start, pointer is updated
 * @param[in,out] buf_len Available buffer length, value is updated
 *****************************************************************************/
static void _build_time_stat(uint8_t **buf, uint16_t *buf_len);

/**************************************************************************//**
 * @brief Build payload
 * @details Helper function
 * @return uint16_t Size of the built CoAP payload
 *****************************************************************************/
static uint16_t _build_payload(void);

/**************************************************************************//**
 * @brief Acquire application mutex
 * @details Internal mutex lock
 *****************************************************************************/
__STATIC_INLINE void _app_wisun_mutex_acquire(void);

/**************************************************************************//**
 * @brief Release application mutex
 * @details Internal mutex release
 *****************************************************************************/
__STATIC_INLINE void _app_wisun_mutex_release(void);

/**************************************************************************//**
 * @brief Updates the notify settings for CoAP app status notification
 * @details Removes and add the new notification settings
 *****************************************************************************/
__STATIC_INLINE sl_status_t _update_notify_settings(void);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Notification instance
static sl_wisun_coap_notify_t _notify = { 0U };

/// Notification payload buffer
static uint8_t _notif_buff[SL_WISUN_APP_STATUS_NOTIF_PAYLOAD_SIZE] = { 0U };

///  App status mutex
static osMutexId_t _app_wisun_app_status_mtx = NULL;

///  App status mutex attribute
static const osMutexAttr_t _app_wisun_app_status_mtx_attr = {
  .name      = "AppWisunAppStatusMutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_wisun_app_status_init(void)
{
#if SL_WISUN_APP_STATUS_COAP_RESOURCE_ENABLE
  static sl_wisun_coap_rhnd_resource_t coap_resource = { 0 };

  // init app status resource
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path = SL_WISUN_APP_STATUS_DEFAULT_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_APP_STATUS_RESOURCE_RT_STATUS;
  coap_resource.data.interface = SL_WISUN_APP_STATUS_RESOURCE_IF_STATUS;
  coap_resource.auto_response = _app_status_response_cb;
  coap_resource.redirect_response = NULL;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
#endif

  // init wisun network mutex
  _app_wisun_app_status_mtx = osMutexNew(&_app_wisun_app_status_mtx_attr);
  EFM_ASSERT(_app_wisun_app_status_mtx != NULL);

  // init notification instance
  EFM_ASSERT(inet_pton(AF_INET6, SL_WISUN_APP_STATUS_DEFAULT_REMOTE_ADDR, &_notify.remote_addr.sin6_addr) == 1);
  _notify.id = SL_WISUN_APP_STATUS_DEFAULT_NOTIFICATION_ID;
  _notify.remote_addr.sin6_port = htons(SL_WISUN_APP_STATUS_DEFAULT_REMOTE_PORT);
  _notify.schedule_time_ms = SL_WISUN_APP_STATUS_DEFAULT_SCHEDULE_TIME_MS;
  _notify.tick_ms = 0UL;
  _notify.condition_cb = sl_wisun_app_status_condition_cb;
  _notify.hnd_cb = _notify_hnd;

  _update_notify_settings();
}

SL_WEAK bool sl_wisun_app_status_condition_cb(const sl_wisun_coap_notify_t * notify)
{
  (void) notify;
  return true;
}

sockaddr_in6_t* sl_wisun_app_status_get_remote_address(void)
{
  return &(_notify.remote_addr);
}

sl_status_t sl_wisun_app_status_get_remote_addr(sockaddr_in6_t * const addr)
{
  if (addr == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  _app_wisun_mutex_acquire();
  memcpy(addr, &_notify.remote_addr, sizeof(sockaddr_in6_t));
  _app_wisun_mutex_release();

  return SL_STATUS_OK;
}

uint32_t sl_wisun_app_status_get_schedule_time_ms(void)
{
  return _notify.schedule_time_ms;
}

sl_status_t sl_wisun_app_status_set_remote_address(const char *remote_address, const uint16_t port)
{
  sl_status_t result = SL_STATUS_OK;
  int32_t ip_result = 0;
  _app_wisun_mutex_acquire();

  if (remote_address == NULL) {
    result =  SL_STATUS_NULL_POINTER;
  } else {
    ip_result = inet_pton(AF_INET6, remote_address, &_notify.remote_addr.sin6_addr);
    if (ip_result != 1) {
      result = SL_STATUS_FAIL;
    }
    _notify.remote_addr.sin6_port = htons(port);
  }
  _update_notify_settings();
  _app_wisun_mutex_release();
  return result;
}

void sl_wisun_app_status_set_schedule_time_ms(const uint32_t new_schedule_time_ms)
{
  _app_wisun_mutex_acquire();
  _notify.schedule_time_ms = new_schedule_time_ms;
  _update_notify_settings();
  _app_wisun_mutex_release();
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void _build_neighbour_info(uint8_t **buf, uint16_t *buf_len)
{
  int32_t r = 0L;
  uint8_t neighbors_count = 0U;
  static sl_wisun_neighbor_info_t info = { 0U };
  static sl_wisun_mac_address_t mac_addrs[SL_WISUN_APP_STATUS_NOTIF_MAX_NEIGHBOUR_CNT] = { 0U };
  const char *ip_str = NULL;

  if (sl_wisun_get_neighbor_count(&neighbors_count) != SL_STATUS_OK) {
    return;
  }

  if (neighbors_count > SL_WISUN_APP_STATUS_NOTIF_MAX_NEIGHBOUR_CNT) {
    neighbors_count = SL_WISUN_APP_STATUS_NOTIF_MAX_NEIGHBOUR_CNT;
  }

  if (sl_wisun_get_neighbors(&neighbors_count, mac_addrs) != SL_STATUS_OK) {
    return;
  }

  if (neighbors_count > SL_WISUN_APP_STATUS_NOTIF_MAX_NEIGHBOUR_CNT) {
    return;
  }

  // print header
  __print_to_buff(r, *buf, *buf_len, SL_WISUN_APP_STATUS_NOTIF_NEIGHBOUR_INFO_JSON_HEADER_STR);

  for (uint8_t idx = 0U; idx < neighbors_count; ++idx) {
    if (sl_wisun_get_neighbor_info(&mac_addrs[idx], &info) != SL_STATUS_OK) {
      continue;
    }

    ip_str = app_wisun_trace_util_get_ip_str(&info.global_address);
    if (ip_str == NULL) {
      continue;
    }
    __print_to_buff(r, *buf, *buf_len,
                    SL_WISUN_APP_STATUS_NEIGHBOUR_FORMAT_STR,
                    ip_str,
                    info.type,
                    info.lifetime,
                    info.mac_tx_count,
                    info.mac_tx_failed_count,
                    info.mac_tx_ms_count,
                    info.mac_tx_ms_failed_count,
                    info.rpl_rank,
                    info.etx,
                    info.rsl_out,
                    info.rsl_in);

    sl_free((void *) ip_str);

    if (idx < (neighbors_count - 1)) {
      __print_to_buff(r, *buf, *buf_len, ",\n");
    }
  }

  __print_to_buff(r, *buf, *buf_len, "\n}");
}

static void _build_device_info(uint8_t **buf, uint16_t *buf_len)
{
  int32_t r = 0L;
  static bool initialized = false;
  static const app_project_info_t *pinf = NULL;
  static const app_project_info_version_t *ver_stack = NULL;
  static const app_project_info_version_t *ver_app = NULL;
  static sl_wisun_mac_address_t mac_addr = { 0U };
  static in6_addr_t ip_addr = { 0U };
  static const char *mac_addr_str = NULL;
  static const char *ip_addr_str = NULL;
  static sl_wisun_device_type_t dev_type = SL_WISUN_ROUTER;
  static char *ver_app_str = NULL;
#if defined(SL_BOARD_NAME)
  static const char *board_name = SL_BOARD_NAME;
#else
  static const char *board_name = "unknown";
#endif
#if defined(SL_BOARD_REV)
  static const char *board_rev = SL_BOARD_REV;
#else
  static const char *board_rev = "";
#endif
#if defined(PART_NUMBER)
  static const char *part_number = PART_NUMBER;
#else
  static const char *part_number = "";
#endif

  if (!initialized) {
#if defined(SL_CATALOG_WISUN_LFN_DEVICE_SUPPORT_PRESENT)
    dev_type = sl_wisun_app_core_get_device_type();
#endif

    pinf = sl_wisun_app_core_util_project_info_get();
    ver_stack = app_project_info_get_version(APP_PROJECT_INFO_VERSION_ID_WISUN, pinf);
    ver_app = app_project_info_get_version(APP_PROJECT_INFO_VERSION_ID_APP, pinf);

    if (pinf == NULL || ver_stack == NULL) {
      return;
    }

    if (sl_wisun_get_mac_address(&mac_addr) != SL_STATUS_OK) {
      return;
    }

    if (sl_wisun_get_ip_address(SL_WISUN_IP_ADDRESS_TYPE_GLOBAL, &ip_addr) != SL_STATUS_OK) {
      return;
    }

    mac_addr_str = app_wisun_mac_addr_to_str(&mac_addr);
    ip_addr_str = app_wisun_trace_util_get_ip_str(&ip_addr);

    if (mac_addr_str == NULL || ip_addr_str == NULL) {
      sl_free((void *) mac_addr_str);
      sl_free((void *) ip_addr_str);
      return;
    }

    if (ver_app == NULL) {
      ver_app_str = "";
    } else {
      ver_app_str = sl_malloc(SL_WISUN_APP_STATUS_VERSION_STR_LEN);
      if (ver_app_str == NULL) {
        ver_app_str = "";
      }

      (void) snprintf(ver_app_str, SL_WISUN_APP_STATUS_VERSION_STR_LEN,
                      SL_WISUN_APP_STATUS_VERSION_FORMAT_STR,
                      ver_app->major.val,
                      ver_app->minor.val,
                      ver_app->patch.val);
    }

    initialized = true;
  }

  __print_to_buff(r, *buf, *buf_len,
                  SL_WISUN_APP_STATUS_DEVICE_INFO_JSON_STR,
                  board_name, board_rev, part_number,
                  pinf->project_name == NULL ? SL_WISUN_APP_STATUS_DEFAULT_APP_NAME : pinf->project_name,
                  ver_app_str,
                  ver_stack->major.val, ver_stack->minor.val, ver_stack->patch.val,
                  app_wisun_trace_util_device_type_to_str(dev_type),
                  mac_addr_str,
                  ip_addr_str);
}

static void _build_time_stat(uint8_t **buf, uint16_t *buf_len)
{
  int32_t r = 0L;
  static sl_wisun_app_core_time_stat_t stat = { 0U };
  static sl_wisun_trace_util_time_t time = { 0U };
  const char *run_str = NULL;
  const char *conn_str = NULL;
  const char *tot_conn_str = NULL;
  const char *disconn_str = NULL;
  const char *tot_disconn_str = NULL;
  uint16_t tmp = 0U;
  uint8_t avail_i = 0U;
  uint8_t avail_f = 0U;
  uint64_t total_time = 0ULL;

  sl_wisun_app_core_get_time_stat(&stat);

  app_wisun_trace_util_timestamp_init(stat.curr_ms, &time);
  run_str = app_wisun_trace_util_time_to_str(&time);

  app_wisun_trace_util_timestamp_init(stat.connected_ms, &time);
  conn_str = app_wisun_trace_util_time_to_str(&time);

  app_wisun_trace_util_timestamp_init(stat.tot_connected_ms, &time);
  tot_conn_str = app_wisun_trace_util_time_to_str(&time);

  app_wisun_trace_util_timestamp_init(stat.disconnected_ms, &time);
  disconn_str = app_wisun_trace_util_time_to_str(&time);

  app_wisun_trace_util_timestamp_init(stat.tot_disconnected_ms, &time);
  tot_disconn_str = app_wisun_trace_util_time_to_str(&time);

  total_time = stat.tot_connected_ms + stat.tot_disconnected_ms;
  if (total_time > 0ULL) {
    tmp = (uint16_t) ((stat.tot_connected_ms * 10000U) / total_time);
  } else {
    tmp = 0U;
  }
  avail_i = (uint8_t) (tmp / 100U);
  avail_f = (uint8_t) (tmp - avail_i * 100U);

  __print_to_buff(r, *buf, *buf_len,
                  SL_WISUN_APP_STATUS_TIME_STAT_JSON_STR,
                  run_str,
                  stat.conn_cnt,
                  conn_str,
                  tot_conn_str,
                  disconn_str,
                  tot_disconn_str,
                  avail_i,
                  avail_f);

  sl_free((void *) run_str);
  sl_free((void *) conn_str);
  sl_free((void *) tot_conn_str);
  sl_free((void *) disconn_str);
  sl_free((void *) tot_disconn_str);
}

static uint16_t _build_payload(void)
{
  int32_t r = 0L;
  uint16_t free_bytes = SL_WISUN_APP_STATUS_NOTIF_PAYLOAD_SIZE - 1U;
  uint16_t buf_len = 0U;
  uint8_t *buf_ptr = _notif_buff;

  __print_to_buff(r, buf_ptr, free_bytes, "\n{\n");

#if SL_WISUN_APP_STATUS_DEVICE_INFO_ENABLE
  _build_device_info(&buf_ptr, &free_bytes);
#endif

#if SL_WISUN_APP_STATUS_EXECUTION_TIME_ENABLE
  #if (SL_WISUN_APP_STATUS_DEVICE_INFO_ENABLE)
  __print_to_buff(r, buf_ptr, free_bytes, ",\n");
  #endif
  _build_time_stat(&buf_ptr, &free_bytes);
#endif

#if SL_WISUN_APP_STATUS_NEIGHBOR_INFO_ENABLE
  #if (SL_WISUN_APP_STATUS_EXECUTION_TIME_ENABLE || SL_WISUN_APP_STATUS_DEVICE_INFO_ENABLE)
  __print_to_buff(r, buf_ptr, free_bytes, ",\n");
  #endif
  _build_neighbour_info(&buf_ptr, &free_bytes);
#endif

  __print_to_buff(r, buf_ptr, free_bytes, "\n}\n");

  buf_len = SL_WISUN_APP_STATUS_NOTIF_PAYLOAD_SIZE - free_bytes;
  _notif_buff[buf_len] = 0U;

  return buf_len;
}

static sl_wisun_coap_packet_t * _notify_hnd(const struct sl_wisun_coap_notify *notify)
{
  static sl_wisun_coap_packet_t pkt = {
    .msg_code = COAP_MSG_CODE_REQUEST_PUT,
    .msg_id = SL_WISUN_APP_STATUS_NOTIF_MSG_ID,
    .msg_type = COAP_MSG_TYPE_NON_CONFIRMABLE,
    .content_format = COAP_CT_JSON,
    .uri_path_ptr = (uint8_t *)SL_WISUN_APP_STATUS_DEFAULT_URI_PATH,
    .uri_path_len = sizeof(SL_WISUN_APP_STATUS_DEFAULT_URI_PATH) - 1,
    .payload_ptr = _notif_buff,
    .token_ptr = NULL,
    .token_len = 0U,
    .options_list_ptr = NULL
  };

  (void) notify;

  // calculate the new length of payload
  pkt.payload_len = _build_payload();

  return &pkt;
}

#if SL_WISUN_APP_STATUS_COAP_RESOURCE_ENABLE
static sl_wisun_coap_packet_t *_app_status_response_cb(const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;

  if (req_packet->msg_code != COAP_MSG_CODE_REQUEST_GET) {
    return _build_const_resp(req_packet,
                             SL_WISUN_APP_STATUS_NOT_VALID_STATUS_RESP,
                             COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }

  // Init packet
  resp_packet = sl_wisun_coap_build_response(req_packet, COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  if (resp_packet == NULL) {
    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_APP_STATUS_FAILED_RESP,
                                    COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }

  resp_packet->msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
  resp_packet->content_format = COAP_CT_JSON;
  _app_wisun_mutex_acquire();
  resp_packet->payload_len = _build_payload();
  _app_wisun_mutex_release();
  resp_packet->payload_ptr = _notif_buff;

  return resp_packet;
}

static sl_wisun_coap_packet_t * _build_const_resp(const sl_wisun_coap_packet_t * const req_packet,
                                                  const char * const resp_str,
                                                  const sn_coap_msg_code_e code)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;

  resp_packet = sl_wisun_coap_build_response(req_packet, code);
  if (resp_packet == NULL) {
    return NULL;
  }

  resp_packet->payload_ptr = (uint8_t *)resp_str;
  resp_packet->payload_len = (uint16_t)sl_strnlen((char *)resp_str, SL_WISUN_APP_STATUS_NOTIF_PAYLOAD_SIZE);

  return resp_packet;
}
#endif

/* Mutex acquire */
__STATIC_INLINE void _app_wisun_mutex_acquire(void)
{
  EFM_ASSERT(osMutexAcquire(_app_wisun_app_status_mtx, osWaitForever) == osOK);
}

/* Mutex release */
__STATIC_INLINE void _app_wisun_mutex_release(void)
{
  EFM_ASSERT(osMutexRelease(_app_wisun_app_status_mtx) == osOK);
}

/* update coap notify instance*/
__STATIC_INLINE sl_status_t _update_notify_settings(void)
{
  sl_wisun_coap_notify_remove_by_id(SL_WISUN_APP_STATUS_DEFAULT_NOTIFICATION_ID);
  return sl_wisun_coap_notify_add(&_notify);
}
