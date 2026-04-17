/***************************************************************************//**
 * @file sl_wisun_coap_meter.c
 * @brief Wi-SUN CoAP Meter
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sl_assert.h"
#include "sl_status.h"
#include "sl_mempool.h"
#include "sl_string.h"
#include "sl_sleeptimer.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_app_core.h"
#if defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
#include "sl_wisun_app_br_core.h"
#include "border_router/sl_wisun_br_api.h"
#endif
#include "sl_wisun_coap.h"
#include "sl_wisun_coap_rhnd.h"
#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
#include "sl_wisun_coap_notify.h"
#endif
#include "sl_wisun_config.h"
#include "sl_wisun_trace_util.h"
#include "sl_wisun_coap_meter.h"
#include "sli_wisun_meter_collector.h"
#include "sl_wisun_meter_collector_config.h"
#include "sl_memory_manager.h"

#if !defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_wisun_led_driver.h"
#endif

#if defined(SL_CATALOG_TEMP_SENSOR_PRESENT)
#include "sl_wisun_rht_measurement.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Print to buff macro function
#define __print_to_buff(__r, __pkt, __buf, __buf_len, __format, ...) \
  do {                                                               \
    if (__buf_len <= 0) {                                            \
      break;                                                         \
    }                                                                \
    __r = snprintf(__buf, __buf_len, __format, ##__VA_ARGS__);       \
    if (__r > 0L) {                                                  \
      (__buf) += __r;                                                \
      (__buf_len) -= __r;                                            \
      __pkt->payload_len += __r;                                     \
    }                                                                \
  } while (0)

/// Json measurement begin format string
#define SL_WISUN_COAP_METER_JSON_MEAS_RESP_BEGIN_FORMAT_STR \
  "{\"%s\" : \n["

/// Json measurement line format string
#define SL_WISUN_COAP_METER_JSON_MEAS_RESP_LINE_FORMAT_STR \
  "\n\"#%u: %lu.%luC %lu.%lu%% %ulux\""

/// Json measurement end format string
#define SL_WISUN_COAP_JSON_MEAS_RESP_END_FORMAT_STR \
  "\n]}"

/// JSON formatted measurement data maximum size
#define SL_WISUN_COAP_METER_JSON_REQUIRED_PAYLOAD_SIZE \
  (SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE * 36 + 64)

#if (SL_WISUN_COAP_METER_JSON_REQUIRED_PAYLOAD_SIZE > (SL_WISUN_COAP_NOTIFY_SOCK_BUFF_SIZE - 32U))
#define SL_WISUN_COAP_METER_JOSN_PAYLOAD_SIZE (SL_WISUN_COAP_NOTIFY_SOCK_BUFF_SIZE - 32U)
#else
#define SL_WISUN_COAP_METER_JOSN_PAYLOAD_SIZE SL_WISUN_COAP_METER_JSON_REQUIRED_PAYLOAD_SIZE
#endif

/// JSON formatted measurement data maximum size
#define SL_WISUN_COAP_METER_JSON_MEAS_DATA_SIZE         350U

/// JSON format string for measurement
#define SL_WISUN_COAP_METER_JSON_MEAS_FORMAT_STR \
  "  {\n"                                        \
  "    \"%s\" :\n"                               \
  "    {\n"                                      \
  "      \"id\": %u,\n"                          \
  "      \"temp\": %lu.%lu,\n"                   \
  "      \"hum\": %lu.%lu,\n"                    \
  "      \"lx\": %u\n"                           \
  "    }\n"                                      \
  "  }"

/// Measurement plain text buffer size
#define SL_WISUN_COAP_METER_MEAS_PLAIN_TXT_DATA_SIZE    32U

/// CoAP Meter measurement resource type ALL
#define SL_WISUN_COAP_METER_RESOURCE_RT_ALL             "all"

/// CoAP Meter measurement resource type temperature
#define SL_WISUN_COAP_METER_RESOURCE_RT_TEMPERATURE     "temperature"

/// CoAP Meter measurement resource type humidity
#define SL_WISUN_COAP_METER_RESOURCE_RT_HUMIDITY        "humidity"

/// CoAP Meter measurement resource type light
#define SL_WISUN_COAP_METER_RESOURCE_RT_LIGHT           "light"

/// CoAP Meter measurement interface sensor
#define SL_WISUN_COAP_METER_RESOURCE_IF_SENSOR          "sensor"

/// CoAP Meter gpio resource type LED
#define SL_WISUN_COAP_METER_RESOURCE_RT_LED             "led"

/// CoAP Meter gpio interface
#define SL_WISUN_COAP_METER_RESOURCE_IF_LED             "gpio"

/// CoAP Meter resource type help
#define SL_WISUN_COAP_METER_RESOURCE_RT_HELP            "help"

/// CoAP Meter help interface
#define SL_WISUN_COAP_METER_RESOURCE_IF_HELP            "meter"

/// Not valid help request response string
#define SL_WISUN_COAP_METER_NOT_VALID_HELP_RESP         "[Not valid help request]"

/// Measurement failed response string
#define SL_WISUN_COAP_METER_MEASUREMENT_FAILED_RESP     "[Measurement failed]"

/// Not valid measurement request response string
#define SL_WISUN_COAP_METER_NOT_VALID_MEAS_RESP         "[Not valid measurement request]"

/// Parsing payload failed response string
#define SL_WISUN_COAP_METER_PARSE_PAYLOAD_FAILED_RESP   "[Parsing payload failed]"

/// Invalid source address response string
#define SL_WISUN_COAP_METER_INVALID_SRC_ADDR_RESP       "[Invalid source address]"

/// Registering collector failed response string
#define SL_WISUN_COAP_METER_REG_COLLECTOR_FAILED_RESP   "[Registering collector failed]"

/// Collector registered response string
#define SL_WISUN_COAP_METER_COLLECTOR_REGISTERED_RESP   "[Collector registered]"

/// Getting collector failed response string
#define SL_WISUN_COAP_METER_GET_COLLECTOR_FAILED_RESP   "[Getting collector failed]"

///Removing collector failed response string
#define SL_WISUN_COAP_METER_RM_COLLECTOR_FAILED_RESP    "[Removing collector failed]"

/// Collector removed response string
#define SL_WISUN_COAP_METER_COLLECTOR_REMOVED_RESP      "[Collector removed]"

/// Temperature measurement failed response string
#define SL_WISUN_COAP_METER_TEMP_MEAS_FAILED_RESP       "[Temperature measurement failed]"

/// Humidity measurement failed response string
#define SL_WISUN_COAP_METER_HUM_MEAS_FAILED_RESP        "[Humidity measurement failed]"

/// Light measurement failed response string
#define SL_WISUN_COAP_METER_LIGHT_MEAS_FAILED_RESP      "[Light measurement failed]"

/// Not valid LED ID response string
#define SL_WISUN_COAP_METER_NOT_VALID_LED_ID_RESP       "[Not valid LED ID]"

/// LED toggle failed response string
#define SL_WISUN_COAP_METER_LED_TOGGLE_FAILED_RESP      "[LED toggle failed]"

/// LED toggled response string
#define SL_WISUN_COAP_METER_LED_TOGGLED_RESP            "[LED toggled]"

/// Help response string
#define SL_WISUN_COAP_METER_HELP_RESP                                                                \
  "Command example:\n"                                                                               \
  "coap-client -m get -N -B 10 -t text coap://[$METER_IPV6_ADDR]:5683/$URI_PATH -e \"$PAYLOAD\"\n\n" \
  "Requests:\n"                                                                                      \
  "  [PUT/POST/GET] sensor/all:\n"                                                                   \
  "    - no payload or \"async\": GET request to get measurement data\n"                             \
  "    - \"register\": POST/PUT request to register collector\n"                                     \
  "    - \"remove\": POST/PUT request to remove collector\n"                                         \
  "  [GET]       sensor/temperature: Request to get temperature\n"                                   \
  "  [GET]       sensor/humidity: Request to get humidity\n"                                         \
  "  [GET]       sensor/light: Request to get light\n"                                               \
  "  [PUT/POST]  gpio/led: Toggle LED0/LED1. Payload is index of the LED: \"0\" or \"1\"\n"

/// Measurement type enumeration
typedef enum sl_wisun_coap_meter_measurement_type {
  /// Measure All
  SL_WISUN_COAP_METER_MEASUREMENT_TYPE_ALL,
  /// Measure temperature
  SL_WISUN_COAP_METER_MEASUREMENT_TYPE_TEMPERATURE,
  /// Measure humidity
  SL_WISUN_COAP_METER_MEASUREMENT_TYPE_HUMIDITY,
  /// Measure light
  SL_WISUN_COAP_METER_MEASUREMENT_TYPE_LIGHT
} sl_wisun_coap_meter_measurement_type_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Create formatted json string from measurement packet
 * @details Use snprintf to static buffer
 * @param[in] packet packet
 * @param[in] ip_str_global node global ipv6 address
 * @return const char* pointer to the static buff
 *****************************************************************************/
static const char *_meter_packet2json(const sl_wisun_meter_packet_t * const packet,
                                      const char *ip_str_global);

/**************************************************************************//**
 * @brief Prepare help response packet
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_help_response_cb(const sl_wisun_coap_packet_t * const req_packet);

/**************************************************************************//**
 * @brief Prepare measurement response packet from all sensor values
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_measurement_response_cb(const sockaddr_in6_t * const src_addr,
                                                        const sl_wisun_coap_packet_t * const req_packet);

/**************************************************************************//**
 * @brief Prepare Temperature measurement response packet
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_temperature_response_cb(const sl_wisun_coap_packet_t * const req_packet);

/**************************************************************************//**
 * @brief Prepare Humidity measurement response packet
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_humidity_response_cb(const sl_wisun_coap_packet_t * const req_packet);

/**************************************************************************//**
 * @brief Prepare Light measurement response packet
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_light_response_cb(const sl_wisun_coap_packet_t * const req_packet);

#if !defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/**************************************************************************//**
 * @brief Prepare measurement response packet
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_led_toggle_response_cb(const sl_wisun_coap_packet_t * const req_packet);
#endif

/**************************************************************************//**
 * @brief Prepare measurement by measurement type
 * @details Helper function
 * @param[in] req_packet Request packet
 * @param[in] measurement Measurement type
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t *_prepare_measurement_resp(const sl_wisun_coap_packet_t * const req_packet,
                                                         const sl_wisun_coap_meter_measurement_type_t measurement);

/**************************************************************************//**
 * @brief Redirect response to new address
 * @details Callback function for resource handler service
 * @param[out] new_addr New address
 * @param[in] req_packet Request packet
 *****************************************************************************/
static void _redirect_resp(sockaddr_in6_t * const new_addr,
                           const sl_wisun_coap_packet_t * const req_packet);

#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT) \
  && SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/**************************************************************************//**
 * @brief Handle LFN wakeup event
 * @details Callback for custom event handling.
 *          The callback triggers notification service for LFN devices.
 * @param[in] evt Event
 *****************************************************************************/
static void _lfn_wakeup_evt_cb(sl_wisun_evt_t * evt);
#endif

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/**************************************************************************//**
 * @brief Handle notification event
 * @details Callback for CoAP Notification Service.
 *          It builds the response packet with the measurement data.
 * @param[in] notify Notification
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t * _notify_hnd_cb(const sl_wisun_coap_notify_t *notify);

/**************************************************************************//**
 * @brief Check notification condition
 * @details Callback for CoAP Notification Service. It returns always true
 * @param[in] notify Notification
 * @return bool true: send, false: not send
 *****************************************************************************/
static bool _notify_condition_cb(const sl_wisun_coap_notify_t *notify);
#endif

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

#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT) \
  && SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/**************************************************************************//**
 * @brief Get LFN profile
 * @details Get LFN profile from configuration
 * @return sl_wisun_lfn_params_t * LFN profile constant pointer or NULL on error
 *****************************************************************************/
static const sl_wisun_lfn_params_t * _get_lfn_profile(void);

/**************************************************************************//**
 * @brief Calculate LFN threshold in milliseconds
 * @details Calculate LFN threshold in milliseconds from LFN profile parameters
 * @param[in] lfn_params LFN profile parameters
 * @return uint32_t LFN threshold in milliseconds
 *****************************************************************************/
__STATIC_INLINE uint32_t _calc_lfn_threshold_ms(const sl_wisun_lfn_params_t * const lfn_params);
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Metrics counter
static sl_wisun_meter_metrics_cnt_t _metrics_cnt = {
  .tot_cnt = 0UL,
  .curr_cnt = 0UL,
  .err_cnt = 0UL,
  .fire_cnt = SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE,
  .reg_coll_cnt = 0UL,
  .max_reg_coll_cnt = SL_WISUN_METER_MAX_REG_COLLECTOR
};

/// Metrics storage mempool
static sl_mempool_t _metrics_mpool = { 0 };

/// Metrics storage buffer
static uint8_t _metrics_buff[SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE * sizeof(sl_wisun_meter_packet_t)] = { 0 };

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/// Notification payload buffer
static uint8_t _payload_buff[SL_WISUN_COAP_METER_JOSN_PAYLOAD_SIZE] = { 0 };
#endif

#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT) \
  && SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/// LFN Parameters
static const sl_wisun_lfn_params_t *_lfn_params = NULL;

/// LFN threshold in milliseconds to send
static uint32_t _lfn_threshold_ms = 0;
#endif
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_wisun_coap_meter_init(void)
{
  static sl_wisun_coap_rhnd_resource_t coap_resource = { 0 };
#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT) \
  && SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
#if defined(WISUN_CONFIG_DEVICE_TYPE)
  uint8_t device_type = WISUN_CONFIG_DEVICE_TYPE;
#else
  uint8_t device_type = SL_WISUN_ROUTER;
#endif
#endif

#if defined(SL_CATALOG_TEMP_SENSOR_PRESENT)
  sl_wisun_rht_init();
#endif

  // Init meter help
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path = SL_WISUN_COAP_METER_COLLECTOR_HELP_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_COAP_METER_RESOURCE_RT_HELP;
  coap_resource.data.interface = SL_WISUN_COAP_METER_RESOURCE_IF_HELP;
  coap_resource.auto_response = _help_response_cb;
  coap_resource.redirect_response = _redirect_resp;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);

  // Init sensor all
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path = SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_COAP_METER_RESOURCE_RT_ALL;
  coap_resource.data.interface = SL_WISUN_COAP_METER_RESOURCE_IF_SENSOR;
  coap_resource.auto_response_ext = _measurement_response_cb;
  coap_resource.redirect_response = _redirect_resp;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);

  // Init sensor temperature
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path = SL_WISUN_COAP_METER_COLLECTOR_TEMPERATURE_SENSOR_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_COAP_METER_RESOURCE_RT_TEMPERATURE;
  coap_resource.data.interface = SL_WISUN_COAP_METER_RESOURCE_IF_SENSOR;
  coap_resource.auto_response = _temperature_response_cb;
  coap_resource.redirect_response = _redirect_resp;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);

  // Init sensor humidity
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path = SL_WISUN_COAP_METER_COLLECTOR_HUMIDITY_SENSOR_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_COAP_METER_RESOURCE_RT_HUMIDITY;
  coap_resource.data.interface = SL_WISUN_COAP_METER_RESOURCE_IF_SENSOR;
  coap_resource.auto_response = _humidity_response_cb;
  coap_resource.redirect_response = _redirect_resp;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);

  // Init sensor light
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path = SL_WISUN_COAP_METER_COLLECTOR_LIGHT_SENSOR_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_COAP_METER_RESOURCE_RT_LIGHT;
  coap_resource.data.interface = SL_WISUN_COAP_METER_RESOURCE_IF_SENSOR;
  coap_resource.auto_response = _light_response_cb;
  coap_resource.redirect_response = _redirect_resp;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
#if !defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  // Init led resource
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path  = SL_WISUN_COAP_METER_COLLECTOR_LED_TOGGLE_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_COAP_METER_RESOURCE_RT_LED;
  coap_resource.data.interface = SL_WISUN_COAP_METER_RESOURCE_IF_LED;
  coap_resource.auto_response = _led_toggle_response_cb;
  coap_resource.redirect_response = _redirect_resp;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
#endif

#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT) \
  && SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
  // For LFN devices, tick event should be enabled
  if (device_type == SL_WISUN_LFN) {
    sl_wisun_coap_notify_tick_evt_enable(true);
    EFM_ASSERT(app_wisun_em_custom_callback_register(SL_WISUN_MSG_LFN_WAKE_UP_IND_ID,
                                                     _lfn_wakeup_evt_cb) == SL_STATUS_OK);
    _lfn_params = _get_lfn_profile();
    EFM_ASSERT(_lfn_params != NULL);
    _lfn_threshold_ms = _calc_lfn_threshold_ms(_lfn_params);
  }
#endif

  // Init metrics mempool
  EFM_ASSERT(sl_mempool_create(&_metrics_mpool,
                               SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE,
                               sizeof(sl_wisun_meter_packet_t),
                               _metrics_buff,
                               sizeof(_metrics_buff)) == SL_STATUS_OK);

#if (SL_WISUN_COAP_METER_JSON_REQUIRED_PAYLOAD_SIZE > SL_WISUN_COAP_METER_JOSN_PAYLOAD_SIZE)
  printf("[Warning: Payload buffer is truncated to %lu bytes]\n", SL_WISUN_COAP_METER_JOSN_PAYLOAD_SIZE);
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static const char *_meter_packet2json(const sl_wisun_meter_packet_t * const packet,
                                      const char *ip_str_global)
{
  static char buff[SL_WISUN_COAP_METER_JSON_MEAS_DATA_SIZE] = { 0 };
  const char *ip_str = (ip_str_global != NULL) ? ip_str_global : "unknown";
  snprintf(buff,
           SL_WISUN_COAP_METER_JSON_MEAS_DATA_SIZE,
           SL_WISUN_COAP_METER_JSON_MEAS_FORMAT_STR,
           ip_str,
           packet->id,
           packet->temperature / 1000,
           (packet->temperature % 1000) / 10,
           packet->humidity / 1000,
           (packet->humidity % 1000) / 10,
           packet->light);
  return buff;
}

static sl_wisun_coap_packet_t *_measurement_response_cb(const sockaddr_in6_t * const src_addr,
                                                        const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;
  char *payload_str = NULL;
#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
  static sl_wisun_coap_notify_t notify = { 0 };
  const sl_wisun_coap_notify_t *tmp_notify = NULL;
  const char *addr_str = NULL;
#else
  (void) src_addr;
#endif

  payload_str = sl_wisun_coap_get_payload_str(req_packet);

  // Handle async request
  if (req_packet->msg_code == COAP_MSG_CODE_REQUEST_GET) {
    if (payload_str == NULL
        || !strncmp(payload_str, SL_WISUN_METER_REQUEST_TYPE_STR_ASYNC, SL_WISUN_METER_REQUEST_RESPONSE_STR_MAX_LEN)) {
      resp_packet = _prepare_measurement_resp(req_packet, SL_WISUN_COAP_METER_MEASUREMENT_TYPE_ALL);
      if (resp_packet == NULL) {
        _metrics_cnt.err_cnt++;
        resp_packet = _build_const_resp(req_packet,
                                        SL_WISUN_COAP_METER_MEASUREMENT_FAILED_RESP,
                                        COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
      }
      return resp_packet;
    }
  }

  if (!(req_packet->msg_code == COAP_MSG_CODE_REQUEST_POST
        || req_packet->msg_code == COAP_MSG_CODE_REQUEST_PUT)) {
    return _build_const_resp(req_packet,
                             SL_WISUN_COAP_METER_NOT_VALID_MEAS_RESP,
                             COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }

  // Parse payload: payload is required for registration and remove request
  if (payload_str == NULL) {
    return _build_const_resp(req_packet,
                             SL_WISUN_COAP_METER_PARSE_PAYLOAD_FAILED_RESP,
                             COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }
#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
  // Register request handling
  if (!strncmp(payload_str, SL_WISUN_METER_REQUEST_TYPE_STR_REGISTER,
               SL_WISUN_METER_REQUEST_RESPONSE_STR_MAX_LEN)) {
    // Create notification entry
    // Set remote address
    memcpy(&notify.remote_addr, src_addr, sizeof(sockaddr_in6_t));
    notify.remote_addr.sin6_port = htons(SL_WISUN_METER_COLLECTOR_PORT);

    // Set notification id to IPv6 address
    notify.id = app_wisun_trace_util_get_ip_str(&notify.remote_addr.sin6_addr);
    if (notify.id == NULL) {
      sl_wisun_coap_destroy_payload_str(payload_str);
      return _build_const_resp(req_packet,
                               SL_WISUN_COAP_METER_INVALID_SRC_ADDR_RESP,
                               COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
    }

    // Set notification callback
    notify.hnd_cb = _notify_hnd_cb;
    notify.condition_cb = _notify_condition_cb;
    notify.schedule_time_ms = SL_WISUN_METER_SCHEDULE_TIME;
    notify.tick_ms = 0;

    if (sl_wisun_coap_notify_add(&notify) != SL_STATUS_OK) {
      sl_wisun_coap_destroy_payload_str(payload_str);
      app_wisun_trace_util_destroy_ip_str(notify.id);
      return _build_const_resp(req_packet,
                               SL_WISUN_COAP_METER_REG_COLLECTOR_FAILED_RESP,
                               COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
    }

    _metrics_cnt.reg_coll_cnt++;

    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_COLLECTOR_REGISTERED_RESP,
                                    COAP_MSG_CODE_RESPONSE_CREATED);

    // Remove request handling
  } else if (!strncmp(payload_str, SL_WISUN_METER_REQUEST_TYPE_STR_REMOVE,
                      SL_WISUN_METER_REQUEST_RESPONSE_STR_MAX_LEN)) {
    // Free notification id
    addr_str = app_wisun_trace_util_get_ip_str(&src_addr->sin6_addr);
    tmp_notify = sl_wisun_coap_notify_get_by_id(addr_str);
    if (tmp_notify == NULL || addr_str == NULL) {
      app_wisun_trace_util_destroy_ip_str(addr_str);
      sl_wisun_coap_destroy_payload_str(payload_str);
      return _build_const_resp(req_packet,
                               SL_WISUN_COAP_METER_GET_COLLECTOR_FAILED_RESP,
                               COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
    }
    sl_free((void *) tmp_notify->id);

    // Remove meter
    if (sl_wisun_coap_notify_remove_by_id(addr_str) != SL_STATUS_OK) {
      app_wisun_trace_util_destroy_ip_str(addr_str);
      sl_wisun_coap_destroy_payload_str(payload_str);
      return _build_const_resp(req_packet,
                               SL_WISUN_COAP_METER_RM_COLLECTOR_FAILED_RESP,
                               COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
    }
    _metrics_cnt.reg_coll_cnt--;
    app_wisun_trace_util_destroy_ip_str(addr_str);

    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_COLLECTOR_REMOVED_RESP,
                                    COAP_MSG_CODE_RESPONSE_CREATED);
  }
#endif
  // Free payload string
  sl_wisun_coap_destroy_payload_str(payload_str);

  return resp_packet;
}

static sl_wisun_coap_packet_t *_temperature_response_cb(const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;

  if (req_packet->msg_code != COAP_MSG_CODE_REQUEST_GET) {
    return _build_const_resp(req_packet,
                             SL_WISUN_COAP_METER_NOT_VALID_MEAS_RESP,
                             COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }

  resp_packet = _prepare_measurement_resp(req_packet, SL_WISUN_COAP_METER_MEASUREMENT_TYPE_TEMPERATURE);
  if (resp_packet == NULL) {
    _metrics_cnt.err_cnt++;
    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_TEMP_MEAS_FAILED_RESP,
                                    COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }
  return resp_packet;
}

static sl_wisun_coap_packet_t *_humidity_response_cb(const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;

  if (req_packet->msg_code != COAP_MSG_CODE_REQUEST_GET) {
    return _build_const_resp(req_packet,
                             SL_WISUN_COAP_METER_NOT_VALID_MEAS_RESP,
                             COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }

  resp_packet = _prepare_measurement_resp(req_packet, SL_WISUN_COAP_METER_MEASUREMENT_TYPE_HUMIDITY);
  if (resp_packet == NULL) {
    _metrics_cnt.err_cnt++;
    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_HUM_MEAS_FAILED_RESP,
                                    COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }
  return resp_packet;
}

static sl_wisun_coap_packet_t *_light_response_cb(const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;

  if (req_packet->msg_code != COAP_MSG_CODE_REQUEST_GET) {
    return _build_const_resp(req_packet,
                             SL_WISUN_COAP_METER_NOT_VALID_MEAS_RESP,
                             COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }

  resp_packet = _prepare_measurement_resp(req_packet, SL_WISUN_COAP_METER_MEASUREMENT_TYPE_LIGHT);
  if (resp_packet == NULL) {
    _metrics_cnt.err_cnt++;
    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_LIGHT_MEAS_FAILED_RESP,
                                    COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }
  return resp_packet;
}

static sl_wisun_coap_packet_t *_help_response_cb(const sl_wisun_coap_packet_t * const req_packet)
{
  if (req_packet->msg_code != COAP_MSG_CODE_REQUEST_GET) {
    return _build_const_resp(req_packet, SL_WISUN_COAP_METER_NOT_VALID_HELP_RESP, COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  }

  return _build_const_resp(req_packet, SL_WISUN_COAP_METER_HELP_RESP, COAP_MSG_CODE_RESPONSE_CREATED);
}

#if !defined(SL_CATALOG_POWER_MANAGER_PRESENT)
static sl_wisun_coap_packet_t *_led_toggle_response_cb(const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;
  int32_t led_num = 0;
  sl_wisun_led_id_t led_id = SL_WISUN_LED_UNKNOW_ID;
  char *payload_str = NULL;

  // Check request packet
  if (!(req_packet->msg_code == COAP_MSG_CODE_REQUEST_POST
        || req_packet->msg_code == COAP_MSG_CODE_REQUEST_PUT)) {
    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_NOT_VALID_MEAS_RESP,
                                    COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
    return resp_packet;
  }

  // Toggle LEDn
  payload_str = sl_wisun_coap_get_payload_str(req_packet);

  if (payload_str == NULL) {
    return resp_packet;
  }

  led_num = atoi(payload_str);
  sl_wisun_coap_destroy_payload_str(payload_str);

  switch (led_num) {
    case 0L:
      led_id = SL_WISUN_LED0_ID;
      break;
    case 1L:
      led_id = SL_WISUN_LED1_ID;
      break;
    default:
      resp_packet = _build_const_resp(req_packet,
                                      SL_WISUN_COAP_METER_NOT_VALID_LED_ID_RESP,
                                      COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
      return resp_packet;
  }

  if (sl_wisun_led_toggle(led_id) == SL_STATUS_FAIL) {
    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_LED_TOGGLE_FAILED_RESP,
                                    COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  } else {
    resp_packet = _build_const_resp(req_packet,
                                    SL_WISUN_COAP_METER_LED_TOGGLED_RESP,
                                    COAP_MSG_CODE_RESPONSE_CHANGED);
  }

  return resp_packet;
}
#endif

static sl_wisun_coap_packet_t *_prepare_measurement_resp(const sl_wisun_coap_packet_t * const req_packet,
                                                         const sl_wisun_coap_meter_measurement_type_t measurement)
{
  sl_wisun_coap_packet_t * resp_packet = NULL;
  sn_coap_content_format_e ct_format = COAP_CT_JSON;
  size_t max_content_size = 0U;
  const char *ip_str_global = NULL;
  char *content = NULL;
  static sl_wisun_meter_packet_t packet = { 0 };
  #if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
  static sl_wisun_app_core_current_addr_t addresses = { 0 };
  #else
  static sl_wisun_app_br_core_current_addr_t br_addresses = { 0 };
  #endif

  // Init packet
  resp_packet = sl_wisun_coap_build_response(req_packet, COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
  if (resp_packet == NULL) {
    return NULL;
  }

  if (measurement == SL_WISUN_COAP_METER_MEASUREMENT_TYPE_ALL) {
    ct_format = COAP_CT_JSON;
    max_content_size = SL_WISUN_COAP_METER_JSON_MEAS_DATA_SIZE;

    // Get IP address
    #if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
    sl_wisun_app_core_get_current_addresses(&addresses);
    ip_str_global = app_wisun_trace_util_get_ip_str(&addresses.global);
    #else
    if (sl_wisun_br_get_ip_addresses(br_addresses.addr_ll.address,
                                     br_addresses.addr_gua.address,
                                     br_addresses.addr_dodagid.address) == SL_STATUS_OK) {
      ip_str_global = app_wisun_trace_util_get_ip_str(&br_addresses.addr_gua);
    }
    #endif
  } else {
    ct_format = COAP_CT_TEXT_PLAIN;
    max_content_size = SL_WISUN_COAP_METER_MEAS_PLAIN_TXT_DATA_SIZE;
    // Allocate string buffer for particular sensor measurement
    content = sl_malloc(SL_WISUN_COAP_METER_MEAS_PLAIN_TXT_DATA_SIZE);
  }

  // Measure all parameters
  if (measurement == SL_WISUN_COAP_METER_MEASUREMENT_TYPE_ALL) {
    sli_wisun_meter_gen_packet_id(&packet);
    sli_wisun_meter_get_temperature(&packet);
    sli_wisun_meter_get_humidity(&packet);
    sli_wisun_meter_get_light(&packet);
    content = (char *)_meter_packet2json(&packet, ip_str_global);
    app_wisun_trace_util_destroy_ip_str(ip_str_global);

    // Temperature measurement
  } else if (measurement == SL_WISUN_COAP_METER_MEASUREMENT_TYPE_TEMPERATURE) {
    sli_wisun_meter_get_temperature(&packet);
    snprintf(content,
             max_content_size,
             "%lu.%lu Cdeg", packet.temperature / 1000,
             (packet.temperature % 1000) / 10);

    // Humidity measurement
  } else if (measurement == SL_WISUN_COAP_METER_MEASUREMENT_TYPE_HUMIDITY) {
    sli_wisun_meter_get_humidity(&packet);
    snprintf(content,
             max_content_size,
             "%lu.%lu %%", packet.humidity / 1000,
             (packet.humidity % 1000) / 10);

    // Light measurement
  } else if (measurement == SL_WISUN_COAP_METER_MEASUREMENT_TYPE_LIGHT) {
    sli_wisun_meter_get_light(&packet);
    snprintf(content,
             max_content_size,
             "%u lux", packet.light);
  }

  resp_packet->msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
  resp_packet->content_format = ct_format;
  resp_packet->payload_ptr = (uint8_t *) content;
  resp_packet->payload_len = (uint16_t) sl_strnlen(content, max_content_size);
  return resp_packet;
}

static void _redirect_resp(sockaddr_in6_t * const new_addr,
                           const sl_wisun_coap_packet_t * const req_packet)
{
  (void) req_packet;
  // Redirect to resource handler service if the message comes from  Wi-SUN CoAP Collector
  if (req_packet->msg_id == SL_WISUN_COAP_METER_COLLECTOR_DEFAULT_MESSAGE_ID) {
    new_addr->sin6_port = htons(SL_WISUN_METER_COLLECTOR_PORT);
  }
}

#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT) \
  && SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
static void _lfn_wakeup_evt_cb(sl_wisun_evt_t * evt)
{
  uint32_t time = 0UL;
  static uint32_t prev_time = 0UL;
  uint32_t elapsed_time = 0UL;

  (void) evt;

  time = sl_sleeptimer_get_tick_count();
  elapsed_time = sl_sleeptimer_tick_to_ms(time - prev_time);

  if (elapsed_time >= _lfn_threshold_ms) {
    prev_time = time;
    (void) sl_wisun_coap_notify_tick();
  }
}
#endif

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
static sl_wisun_coap_packet_t * _notify_hnd_cb(const sl_wisun_coap_notify_t *notify)
{
  sl_wisun_meter_packet_t *meas_pkt = NULL;
  sl_wisun_coap_packet_t *pkt_ptr = NULL;
  char *tmp_buff = NULL;
  int32_t res = 0L;
  int32_t payload_len = SL_WISUN_COAP_METER_JOSN_PAYLOAD_SIZE;
  const char *ip_str_global = NULL;
  uint32_t cnt = 0U;

  #if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
  static sl_wisun_app_core_current_addr_t addresses = { 0 };
  #else
  static sl_wisun_app_br_core_current_addr_t br_addresses = { 0 };
  #endif
  static sl_wisun_coap_packet_t notify_coap_pkt = {
    .msg_code = COAP_MSG_CODE_REQUEST_PUT,
    .msg_id = SL_WISUN_COAP_METER_COLLECTOR_DEFAULT_MESSAGE_ID,
    .msg_type = COAP_MSG_TYPE_NON_CONFIRMABLE,
    .content_format = COAP_CT_JSON,
    .uri_path_ptr = (uint8_t *)SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH,
    .uri_path_len = sizeof(SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH) - 1,
    .payload_ptr = _payload_buff,
    .payload_len = 0U,
    .token_ptr = NULL,
    .token_len = 0U,
    .options_list_ptr = NULL
  };

  (void) notify;

  meas_pkt = (sl_wisun_meter_packet_t *)sl_mempool_alloc(&_metrics_mpool);
  if (meas_pkt == NULL) {
    return NULL;
  }

  sli_wisun_meter_gen_packet_id(meas_pkt);
  sli_wisun_meter_get_temperature(meas_pkt);
  sli_wisun_meter_get_humidity(meas_pkt);
  sli_wisun_meter_get_light(meas_pkt);

  _metrics_cnt.curr_cnt++;
  _metrics_cnt.tot_cnt++;

  // Check metrics count, if the current count is equal to the fire count,
  // send the packet
  if (_metrics_cnt.curr_cnt < _metrics_cnt.fire_cnt) {
    return &notify_coap_pkt;
  }

  // Build JSON payload
  tmp_buff = (char *) notify_coap_pkt.payload_ptr;
  pkt_ptr = &notify_coap_pkt;
  #if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
  sl_wisun_app_core_get_current_addresses(&addresses);
  ip_str_global = app_wisun_trace_util_get_ip_str(&addresses.global);
  #else
  if (sl_wisun_br_get_ip_addresses(br_addresses.addr_ll.address,
                                   br_addresses.addr_gua.address,
                                   br_addresses.addr_dodagid.address) == SL_STATUS_OK) {
    ip_str_global = app_wisun_trace_util_get_ip_str(&br_addresses.addr_gua);
  }
  #endif
  if (ip_str_global == NULL) {
    return NULL;
  }

  __print_to_buff(res, pkt_ptr, tmp_buff, payload_len,
                  SL_WISUN_COAP_METER_JSON_MEAS_RESP_BEGIN_FORMAT_STR,
                  ip_str_global);
  app_wisun_trace_util_destroy_ip_str(ip_str_global);

  cnt = 0U;
  for (sl_mempool_block_hnd_t *block = _metrics_mpool.blocks;
       block != NULL;
       block = block->next) {
    meas_pkt = (sl_wisun_meter_packet_t *)block->start_addr;

    __print_to_buff(res, pkt_ptr, tmp_buff, payload_len,
                    cnt != (_metrics_mpool.used_block_count - 1)
                    ? SL_WISUN_COAP_METER_JSON_MEAS_RESP_LINE_FORMAT_STR ","
                    : SL_WISUN_COAP_METER_JSON_MEAS_RESP_LINE_FORMAT_STR,
                    meas_pkt->id,
                    meas_pkt->temperature / 1000,
                    (meas_pkt->temperature % 1000) / 10,
                    meas_pkt->humidity / 1000,
                    (meas_pkt->humidity % 1000) / 10,
                    meas_pkt->light);
    ++cnt;
  }

  __print_to_buff(res, pkt_ptr, tmp_buff, payload_len,
                  SL_WISUN_COAP_JSON_MEAS_RESP_END_FORMAT_STR);

  // Calculate packet payload length
  pkt_ptr->payload_len = sl_strnlen((char *)pkt_ptr->payload_ptr, SL_WISUN_COAP_METER_JOSN_PAYLOAD_SIZE);

  if (payload_len < 0) {
    snprintf((char *)(pkt_ptr->payload_ptr + (pkt_ptr->payload_len - 4)), 4, "...");
    pkt_ptr->content_format = COAP_CT_TEXT_PLAIN;
  }

  // Free the metrics table
  for (sl_mempool_block_hnd_t *block = _metrics_mpool.blocks;
       block != NULL;
       block = block->next) {
    sl_mempool_free(&_metrics_mpool, block->start_addr);
  }

  return &notify_coap_pkt;
}

static bool _notify_condition_cb(const sl_wisun_coap_notify_t *notify)
{
  (void) notify;
  if (_metrics_cnt.curr_cnt == _metrics_cnt.fire_cnt) {
    _metrics_cnt.curr_cnt = 0;
    return true;
  } else {
    return false;
  }
}
#endif

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
  resp_packet->payload_len = (uint16_t)sl_strnlen((char *)resp_str, SL_WISUN_METER_REQUEST_RESPONSE_STR_MAX_LEN);
  return resp_packet;
}
#if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT) \
  && SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
static const sl_wisun_lfn_params_t * _get_lfn_profile(void)
{
#if !defined(WISUN_CONFIG_DEVICE_PROFILE)
  return &SL_WISUN_PARAMS_LFN_TEST;
#else
  switch (WISUN_CONFIG_DEVICE_PROFILE) {
    case SL_WISUN_LFN_PROFILE_TEST:
      return &SL_WISUN_PARAMS_LFN_TEST;
    case SL_WISUN_LFN_PROFILE_BALANCED:
      return &SL_WISUN_PARAMS_LFN_BALANCED;
    case SL_WISUN_LFN_PROFILE_ECO:
      return &SL_WISUN_PARAMS_LFN_ECO;
    default:
      return NULL;
  }
#endif
}
__STATIC_INLINE uint32_t _calc_lfn_threshold_ms(const sl_wisun_lfn_params_t * const lfn_params)
{
  // 90% of the LFN unicast interval
  return (uint32_t)(((uint64_t)lfn_params->data_layer.unicast_interval_ms * 90UL) / 100UL);
}
#endif
