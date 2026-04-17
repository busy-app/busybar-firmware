/***************************************************************************//**
 * @file sl_wisun_ota_dfu.c
 * @brief Wi-SUN OTA DFU Service
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
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_status.h"
#include "btl_interface.h"
#include "btl_reset_info.h"
#include "sl_ftp.h"
#include "sl_tftp_clnt.h"
#include "sl_ftp_config.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_coap_config.h"
#include "sl_wisun_coap_rhnd.h"
#include "sl_wisun_coap_notify.h"
#include "sl_wisun_ota_dfu.h"
#include "sl_wisun_ota_dfu_config.h"
#include "sl_wisun_coap.h"
#include "sl_string.h"
#include "sl_sleeptimer.h"
#include "sl_memory_manager.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// OTA DFU Event all mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_ALL_MSK                    0x00FFFFFFUL

/// OTA DFU error mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_ERROR_MSK                  (1UL << 31UL)

/// OTA DFU start firmware update mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_START_FW_UPDATE_MSK        (1UL << SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STARTED)

/// OTA DFU firmware downloaded mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOADED_MSK          (1UL << SL_WISUN_OTA_DFU_STATUS_FW_DOWNLOADED)

/// OTA DFU firmware verified mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_FW_VERIFIED_MSK            (1UL << SL_WISUN_OTA_DFU_STATUS_FW_VERIFIED)

/// OTA DFU firmware set mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_MSK                 (1UL << SL_WISUN_OTA_DFU_STATUS_FW_SET)

/// OTA DFU firmware update stopped/finished mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_STOP_FW_UPDATE_MSK         (1UL << SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STOPPED)

/// OTA DFU firmware download error mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOAD_ERROR_MSK      (1UL << SL_WISUN_OTA_DFU_STATUS_FW_DOWNLOAD_ERROR)

/// OTA DFU firmware verification error mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_FW_VERIFY_ERROR_MSK        (1UL << SL_WISUN_OTA_DFU_STATUS_FW_VERIFY_ERROR)

/// OTA DFU firmware set error mask
#define SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_ERROR_MSK           (1UL << SL_WISUN_OTA_DFU_STATUS_FW_SET_ERROR)

/// OTA DFU status json format string
#define SL_WISUN_OTA_DFU_STATUS_JSON_FORMAT_STR         \
  "{\n"                                                 \
  "\"ip\": \"%s\",\n"                                   \
  "\"elapsed_downl_t\": \"%u-%02u:%02u:%02u\",\n"       \
  "\"elapsed_upd_t\": \"%u-%02u:%02u:%02u\",\n"         \
  "\"elapsed_since_rst_t\": \"%u-%02u:%02u:%02u\",\n"   \
  "\"downl_bytes\": %lu,\n"                             \
  "\"flags\": \"0x%.8lx\",\n"                           \
  "\"fw_update_started\": %u,\n"                        \
  "\"fw_downloaded\": %u,\n"                            \
  "\"fw_verified\": %u,\n"                              \
  "\"fw_set\": %u,\n"                                   \
  "\"fw_stopped\": %u,\n"                               \
  "\"fw_download_error\": %u,\n"                        \
  "\"fw_verify_error\": %u,\n"                          \
  "\"fw_set_error\": %u,\n"                             \
  "\"resent/received\": \"%ld/%ld\"\n"                  \
  "}\n"

/// OTA DFU status json string max length
#define SL_WISUN_OTA_DFU_STATUS_JSON_STR_MAX_LEN                512UL

/// OTA DFU response string max length
#define SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN                   (SL_WISUN_OTA_DFU_STATUS_JSON_STR_MAX_LEN)

/// OTA DFU delay in ms
#define SL_WISUN_OTA_DFU_DELAY_MS                               1000UL

/// OTA DFU shutdown delay in ms
#define SL_WISUN_OTA_DFU_SHUTDOWN_DELAY_MS                      5000UL

/// OTA DFU resource type
#define SL_WISUN_OTA_DFU_RT                                     "dfu"

/// OTA DFU resource interface
#define SL_WISUN_OTA_DFU_RESOURCE_IF_OTA                        "ota"

/// OTA DFU command delimiter
#define SL_WISUN_OTA_DFU_CMD_DELIMITER                          " "

/// OTA DFU firmware update start payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_START                      "start"

/// OTA DFU firmware update stop payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_STOP                       "stop"

/// OTA DFU firmware reboot and install
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_REBOOT_AND_INSTALL         "install"

/// OTA DFU host address payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS               "host_address"

/// OTA DFU gbl path payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_GBL_PATH                   "gbl"

/// OTA DFU uri path payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_URI                        "uri"

/// OTA DFU help payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_HELP                       "help"

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED

/// OTA DFU notify host address payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_HOST_ADDRESS        "notify_host_address"

/// OTA DFU notify uri path payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_URI                 "notify_uri"

/// OTA DFU notify download chunk count payload string
#define SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_DOWNLOAD_CHUNK_CNT  "notify_chunk_cnt"

/// Help string if host notify is enabled
#define SL_WISUN_OTA_DFU_RESPONSE_PAYLOAD_STR_HELP                                \
  "Commands:\n\n"                                                                 \
  "[P: POST (set <arg>), G: GET (get)]\n"                                         \
  "[G] " SL_WISUN_OTA_DFU_PAYLOAD_STR_HELP "\n"                                   \
  "[G] (empty payload for status json)\n"                                         \
  "[P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_START "\n"                                  \
  "[P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_STOP "\n"                                   \
  "[P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_REBOOT_AND_INSTALL "\n"                     \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS " <ipv6> <port>\n"           \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_GBL_PATH " <path>\n"                      \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_URI " <path>\n"                           \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_HOST_ADDRESS " <ipv6> <port>\n"    \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_URI " <path>\n"                    \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_DOWNLOAD_CHUNK_CNT " <count>\n\n"  \
  "Examples:\n"                                                                   \
  "GET " SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS "\n"                           \
  "POST " SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS " 2001:db8::1 69\n"


#else
/// Help string if host notify is disabled
#define SL_WISUN_OTA_DFU_RESPONSE_PAYLOAD_STR_HELP                                \
  "Commands:\n\n"                                                                 \
  "[P: POST (set <arg>), G: GET (get)]\n"                                         \
  "[G] " SL_WISUN_OTA_DFU_PAYLOAD_STR_HELP "\n"                                   \
  "[G] (empty payload for status json)\n"                                         \
  "[P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_START "\n"                                  \
  "[P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_STOP "\n"                                   \
  "[P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_REBOOT_AND_INSTALL "\n"                     \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS " <ipv6> <port>\n"           \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_GBL_PATH " <path>\n"                      \
  "[G|P] " SL_WISUN_OTA_DFU_PAYLOAD_STR_URI " <path>\n\n"                         \
  "Examples:\n"                                                                   \
  "GET " SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS "\n"                           \
  "POST " SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS " 2001:db8::1 69\n"
#endif

/// OTA DFU unknown cmd response payload string
#define SL_WISUN_OTA_DFU_RESPONSE_PAYLOAD_STR_ERROR      "[Error: %s]"

/// OTA DFU firmware started response payload string
#define SL_WISUN_OTA_DFU_RESPONSE_PAYLOAD_STR_ACK        "[Ack: %s]"

/// Error firmware download format string
#define SL_WISUN_OTA_DFU_ERROR_FW_DOWNLOAD_FORMAT_STR \
  "error (%u): write flash (ret_val: %ld, offset: %lu, data_size: %u)\n"

/// Error firmware verify format string
#define SL_WISUN_OTA_DFU_ERROR_FW_VERIFY_FORMAT_STR \
  "error (%u): verify fw (ret_val: %ld)\n"

/// Error firmware set format string
#define SL_WISUN_OTA_DFU_ERROR_FW_SET_FORMAT_STR \
  "error (%u): set fw (ret_val: %ld)\n"

/// Notification CoAP message ID
#define SL_WISUN_OTA_DFU_HOST_NOTIFY_COAP_MSG_ID          9001U

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
#define SL_WISUN_OTA_DFU_NOTIFY_ID                        "ota_notify"

#define SL_WISUN_OTA_DFU_NOTIFY_SCHEDULE_TIME_MS          1000UL
#endif

/// OTA DFU log macro function
#if SL_WISUN_OTA_DFU_VERBOSE_MODE_ENABLED
# define sl_wisun_ota_dfu_log(format, ...)               \
  do {                                                   \
    printf("[wisun-ota] (%lu) ", _get_elapsed_time_s()); \
    printf(format, ##__VA_ARGS__);                       \
  } while (0)
#else
# define sl_wisun_ota_dfu_log(format, ...) \
  do {                                     \
    (void) format;                         \
  } while (0)
#endif

/// OTA DFU Remote control argument typedef
typedef struct sl_wisun_ota_dfu_remote_arg {
  /// Command ey
  char *key;
  /// Argument value 1
  char *val1;
  /// Argument value 2
  char *val2;
} sl_wisun_ota_dfu_remote_arg_t;

/// OTA DFU response typedef
typedef struct sl_wisun_ota_dfu_response {
  /// Response code
  sn_coap_msg_code_e msg_code;
  /// Response payload
  const char *format_str;
} sl_wisun_ota_dfu_response_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief OTA DFU TFTP Data handler
 * @details Callback to store received data in SPI flash
 * @param[in,out] clnt Client
 * @param[in] data_ptr Data ptr
 * @param[in] data_size Data size
 ******************************************************************************/
static void _tftp_data_hnd(sl_tftp_clnt_t * const clnt,
                           const uint8_t * const data_ptr,
                           const uint16_t data_size);

/***************************************************************************//**
 * @brief Parse Remote CLI arguments
 * @details Split argument string to key and values
 * @param[in] cmd Source pointer
 * @param[out] arg Argument struct destination ptr
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
static void _parse_arguments(char * const cmd,
                             sl_wisun_ota_dfu_remote_arg_t * const arg);

/***************************************************************************//**
 * @brief Create response
 * @details Create response. Response packet payload contains
 *          the requested payload and the appropriate response code
 * @param[in] response Response definition
 * @param[in] req_packet Request packet
 * @param[out] resp_packet Response packet
 ******************************************************************************/
static void _create_response(const sl_wisun_ota_dfu_response_t * const response,
                             const sl_wisun_coap_packet_t * const req_packet,
                             sl_wisun_coap_packet_t * const resp_packet);

/***************************************************************************//**
 * @brief GET request handler
 * @details GET request handler
 * @param[in] req_packet Request packet
 * @param[out] resp_packet Response packet
 ******************************************************************************/
static void _handle_get_requests(const sl_wisun_coap_packet_t * const req_packet,
                                 sl_wisun_coap_packet_t * const resp_packet);

/***************************************************************************//**
 * @brief POST request handler
 * @details POST request handler
 * @param[in] req_packet Request packet
 * @param[out] resp_packet Response packet
 ******************************************************************************/
static void _handle_post_requests(const sl_wisun_coap_packet_t * const req_packet,
                                  sl_wisun_coap_packet_t * const resp_packet);

/**************************************************************************//**
 * @brief OTA DFU CoAP callback.
 * @details Handle incoming firmware update request from CoAP message.
 * @param[in] src_addr Source address.
 * @param[in] req_packet Request packet.
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t * _ota_dfu_coap_response_cb(const sl_wisun_coap_packet_t * const req_packet);

/**************************************************************************//**
 * @brief Thread function of OTA DFU Service
 * @details Thread function
 * @param[in] args Arguments (not used)
 *****************************************************************************/
static void _ota_dfu_thr_fnc(void * args);

/**************************************************************************//**
 * @brief Get status string in json format
 * @details Helper function
 * @return const char * Allocated string pointer on success, otherwise NULL
 *****************************************************************************/
static const char *_get_status_json_string(void);

/**************************************************************************//**
* @brief Change Status
* @details Change status flags and notify host
* @param[in] status_mask Status flags to update
******************************************************************************/
static void _change_status(const uint32_t status_mask);

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
/**************************************************************************//**
 * @brief Notify host callback
 * @details Prepare notification packet for notification service
 * @param[in] notify Notify descriptor
 * @return sl_wisun_coap_packet_t * CoAP packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t * _notify_cb(const struct sl_wisun_coap_notify *notify);

/**************************************************************************//**
 * @brief Notify condition callback
 * @details Check if notification is required for notification service
 * @param[in] notify Notify descriptor
 * @return bool True if notification is required, otherwise false
 *****************************************************************************/
static bool _notify_condition_cb(const struct sl_wisun_coap_notify *notify);

/**************************************************************************//**
 * @brief Notify host
 * @details Trigger notification sending
 *****************************************************************************/
__STATIC_INLINE void _notify_host(void);
#endif
/**************************************************************************//**
 * @brief Get elapsed time in seconds
 * @details Get difference between now and stored reference
 *****************************************************************************/
__STATIC_INLINE sl_sleeptimer_timestamp_t _get_elapsed_time_s(void);

/**************************************************************************//**
 * @brief Lock OTA DFU mutex
 * @details Helper function
 *****************************************************************************/
__STATIC_INLINE void _ota_dfu_mtx_acquire(void);

/**************************************************************************//**
 * @brief Release OTA DFU mutex
 * @details Helper function
 *****************************************************************************/
__STATIC_INLINE void _ota_dfu_mtx_release(void);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Thread ID
static osThreadId_t _ota_dfu_thr = NULL;

/// OTA DFU task attribute
static const osThreadAttr_t _ota_dfu_thr_attr = {
  .name        = "DfuThread",
  .attr_bits   = osThreadDetached,
  .cb_mem      = NULL,
  .cb_size     = 0,
  .stack_mem   = NULL,
  .stack_size  = app_stack_size_word_to_byte(SL_WISUN_OTA_DFU_STACK_SIZE_WORD),
  .priority    = osPriorityNormal7,
  .tz_module   = 0
};

/// OTA DFU event flag
static osEventFlagsId_t _ota_dfu_evt = NULL;

/// OTA DFU event flag attributes
static const osEventFlagsAttr_t _evt_attr = {
  .name      = "DfuEvt",
  .attr_bits = 0,
  .cb_mem    = NULL,
  .cb_size   = 0
};

/// OTA DFU mutex
static osMutexId_t _ota_dfu_mtx = NULL;

/// OTA DFU mutex attribute
static const osMutexAttr_t _ota_dfu_mtx_attr = {
  .name      = "DfuMtx",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0
};

/// OTA DFU CoAP resource
static sl_wisun_coap_rhnd_resource_t _ota_dfu_resource = {
  .data.uri_path      = SL_WISUN_OTA_DFU_URI_PATH,
  .data.resource_type = SL_WISUN_OTA_DFU_RT,
  .data.interface     = SL_WISUN_OTA_DFU_RESOURCE_IF_OTA,
  .auto_response      = _ota_dfu_coap_response_cb,
  .redirect_response  = NULL,
  .discoverable       = true
};

/// Settings storage
sl_wisun_ota_dfu_settings_t _settings = { 0U };

/// OTA DFU statistics
static sl_wisun_ota_dfu_statistics_t _stats = { 0 };

///  OTA DFU error response
static const sl_wisun_ota_dfu_response_t _response_error = {
  .msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST,
  .format_str = SL_WISUN_OTA_DFU_RESPONSE_PAYLOAD_STR_ERROR
};

/// OTA DFU ack response
static const sl_wisun_ota_dfu_response_t _response_ack = {
  .msg_code = COAP_MSG_CODE_RESPONSE_CONTENT,
  .format_str = SL_WISUN_OTA_DFU_RESPONSE_PAYLOAD_STR_ACK
};

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
/// OTA DFU Notification instance
static sl_wisun_coap_notify_t _ota_notify = {
  .id = SL_WISUN_OTA_DFU_NOTIFY_ID,
  .remote_addr = { 0U },
  .hnd_cb = _notify_cb,
  .condition_cb = _notify_condition_cb,
  .schedule_time_ms = SL_WISUN_OTA_DFU_NOTIFY_SCHEDULE_TIME_MS,
  .tick_ms = 0
};

/// Internal flag to notify host required
static bool _is_notify_required = false;
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_wisun_ota_dfu_init(void)
{
  _ota_dfu_mtx = osMutexNew(&_ota_dfu_mtx_attr);
  EFM_ASSERT(_ota_dfu_mtx != NULL);

  _ota_dfu_evt = osEventFlagsNew(&_evt_attr);
  EFM_ASSERT(_ota_dfu_evt != NULL);

  _ota_dfu_thr = osThreadNew(_ota_dfu_thr_fnc, NULL, &_ota_dfu_thr_attr);
  EFM_ASSERT(_ota_dfu_thr != NULL);
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&_ota_dfu_resource) == SL_STATUS_OK);

  // Init settings with default values
  snprintf(_settings.host_addr_str,
           SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN,
           "%s",
           SL_WISUN_OTA_DFU_HOST_ADDR);
  _settings.host_port = SL_WISUN_OTA_DFU_TFTP_PORT;
  snprintf(_settings.gbl_path_str,
           SL_WISUN_OTA_DFU_GBL_PATH_STR_BUF_LEN,
           "%s",
           SL_WISUN_OTA_DFU_GBL_FILE);
  snprintf(_settings.coap_uri_path,
           SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN,
           "%s",
           SL_WISUN_OTA_DFU_URI_PATH);

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
  _Static_assert(SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE,
                 "SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE must be enabled also to use OTA DFU host notification");

  snprintf(_settings.notify_host_addr_str,
           SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN,
           "%s",
           SL_WISUN_OTA_DFU_NOTIFY_HOST_ADDR);
  _settings.notify_host_port = SL_WISUN_OTA_DFU_NOTIFY_PORT;
  _ota_notify.remote_addr.sin6_port = htons(_settings.notify_host_port);
  inet_pton(AF_INET6, _settings.notify_host_addr_str, &_ota_notify.remote_addr.sin6_addr);

  snprintf(_settings.notify_coap_uri_path,
           SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN,
           "%s",
           SL_WISUN_OTA_DFU_NOTIFY_URI_PATH);

  _settings.notify_dwnld_chunk_cnt = SL_WISUN_OTA_DFU_NOTIFY_DOWNLOAD_CHUNK_CNT;

  EFM_ASSERT(sl_wisun_coap_notify_add(&_ota_notify) == SL_STATUS_OK);
#endif
}

sl_status_t sl_wisun_ota_dfu_start_fw_update(void)
{
  uint32_t flags = 0UL;

  _stats.received_cnt = 0UL;
  _stats.resent_cnt = 0UL;

  flags = osEventFlagsGet(_ota_dfu_evt);

  // error or already started
  if ((flags & SL_WISUN_OTA_DFU_EVT_FLAG_ERROR_MSK)
      || (flags & SL_WISUN_OTA_DFU_EVT_FLAG_START_FW_UPDATE_MSK)) {
    return SL_STATUS_FAIL;
  }

  (void) osEventFlagsSet(_ota_dfu_evt, SL_WISUN_OTA_DFU_EVT_FLAG_START_FW_UPDATE_MSK);

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_stop_fw_update(void)
{
  uint32_t flags = 0UL;

  flags = osEventFlagsGet(_ota_dfu_evt);

  // error or already stopped
  if ((flags & SL_WISUN_OTA_DFU_EVT_FLAG_ERROR_MSK)
      || (flags & SL_WISUN_OTA_DFU_EVT_FLAG_STOP_FW_UPDATE_MSK)) {
    return SL_STATUS_FAIL;
  }

  (void) osEventFlagsSet(_ota_dfu_evt, SL_WISUN_OTA_DFU_EVT_FLAG_STOP_FW_UPDATE_MSK);

  return SL_STATUS_OK;
}

#if !SL_WISUN_OTA_DFU_AUTO_INSTALL_ENABLED
sl_status_t sl_wisun_ota_dfu_reboot_and_install(void)
{
  bootloader_rebootAndInstall();

  // If the function returns, error occurred.
  return SL_STATUS_FAIL;
}
#endif

uint32_t sl_wisun_ota_dfu_get_fw_update_status(void)
{
  return osEventFlagsGet(_ota_dfu_evt);
}

const char* sl_wisun_ota_dfu_get_fw_update_status_json_str(void)
{
  return _get_status_json_string();
}

void sl_wisun_ota_dfu_free_fw_update_status_json_str(const char * str)
{
  sl_free((void *)str);
}

bool sl_wisun_ota_dfu_get_fw_update_status_flag(const sl_wisun_ota_dfu_status_t status_flag)
{
  uint32_t flags = 0UL;

  flags = osEventFlagsGet(_ota_dfu_evt);
  return (flags & SL_WISUN_OTA_DFU_EVT_FLAG_ERROR_MSK) || !(flags & 1UL << status_flag) ? false : true;
}

SL_WEAK void sl_wisun_ota_dfu_error_hnd(const sl_wisun_ota_dfu_error_code_t error_code,
                                        sl_wisun_ota_dfu_error_ctx_t * const error_ctx)
{
  switch (error_code) {
    case SL_WISUN_OTA_DFU_ERROR_FW_DOWNLOAD:
      sl_wisun_ota_dfu_log(SL_WISUN_OTA_DFU_ERROR_FW_DOWNLOAD_FORMAT_STR,
                           error_code,
                           error_ctx->download.ret_val,
                           error_ctx->download.offset,
                           error_ctx->download.data_size);
      break;

    case SL_WISUN_OTA_DFU_ERROR_FW_VERIFY:
      sl_wisun_ota_dfu_log(SL_WISUN_OTA_DFU_ERROR_FW_VERIFY_FORMAT_STR,
                           error_code,
                           error_ctx->verify.ret_val);
      break;

    case SL_WISUN_OTA_DFU_ERROR_FW_SET:
      sl_wisun_ota_dfu_log(SL_WISUN_OTA_DFU_ERROR_FW_SET_FORMAT_STR,
                           error_code,
                           error_ctx->set.ret_val);
      break;

    default:
      (void) error_ctx;
      break;
  }
}

sl_status_t sl_wisun_ota_dfu_set_host_addr(const char *host, const uint16_t port)
{
  if (host == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(_settings.host_addr_str, SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN, host);
  _settings.host_port = port;
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_get_host_addr(char * const dst_host,
                                           const uint16_t dst_host_size,
                                           uint16_t *dst_port)
{
  if (dst_host == NULL
      || dst_host_size == 0U
      || dst_host_size > SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN
      || dst_port == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(dst_host, dst_host_size, _settings.host_addr_str);
  *dst_port = _settings.host_port;
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_set_host_uri(const char *uri)
{
  if (uri == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(_settings.coap_uri_path, SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN, uri);
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_get_host_uri(char * const dst_uri,
                                          const uint16_t dst_uri_size)
{
  if (dst_uri == NULL
      || dst_uri_size == 0U
      || dst_uri_size > SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(dst_uri, dst_uri_size, _settings.coap_uri_path);
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_set_gbl_path(const char *gbl_path)
{
  if (gbl_path == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(_settings.gbl_path_str, SL_WISUN_OTA_DFU_GBL_PATH_STR_BUF_LEN, gbl_path);
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_get_gbl_path(char * const dst_gbl_path,
                                          const uint16_t dst_gbl_path_size)
{
  if (dst_gbl_path == NULL
      || dst_gbl_path_size == 0U) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(dst_gbl_path, dst_gbl_path_size, _settings.gbl_path_str);
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_get_fw_update_statistics(sl_wisun_ota_dfu_statistics_t * const stats)
{
  if (stats == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  _ota_dfu_mtx_acquire();
  // Update elapsed time since reset first
  (void) sl_sleeptimer_convert_time_to_date(sl_sleeptimer_get_time(), 0, &_stats.elapsed_time_since_rst);
  memcpy(stats, &_stats, sizeof(sl_wisun_ota_dfu_statistics_t));
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
sl_status_t sl_wisun_ota_dfu_set_notify_host_addr(const char *host,
                                                  const uint16_t port)
{
  sl_status_t status = SL_STATUS_OK;

  if (host == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();

  sl_strcpy_s(_settings.notify_host_addr_str, SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN, host);
  _settings.notify_host_port = port;
  _ota_notify.remote_addr.sin6_port = htons(_settings.notify_host_port);

  if (inet_pton(AF_INET6, _settings.notify_host_addr_str, &_ota_notify.remote_addr.sin6_addr) == 1L) {
    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  if (status != SL_STATUS_OK) {
    _ota_dfu_mtx_release();
    return status;
  }

  status = sl_wisun_coap_notify_remove_by_id(SL_WISUN_OTA_DFU_NOTIFY_ID);

  if (status != SL_STATUS_OK) {
    _ota_dfu_mtx_release();
    return status;
  }

  status = sl_wisun_coap_notify_add(&_ota_notify);

  _ota_dfu_mtx_release();

  return status;
}

sl_status_t sl_wisun_ota_dfu_get_notify_host_addr(char * const dst_host,
                                                  const uint16_t dst_host_size,
                                                  uint16_t *dst_port)
{
  if (dst_host == NULL
      || dst_host_size == 0U
      || dst_host_size > SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN
      || dst_port == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(dst_host, dst_host_size, _settings.notify_host_addr_str);
  *dst_port = _settings.notify_host_port;
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_set_notify_host_uri(const char *uri)
{
  if (uri == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(_settings.notify_coap_uri_path, SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN, uri);
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_get_notify_host_uri(char * const dst_uri,
                                                 const uint16_t dst_uri_size)
{
  if (dst_uri == NULL
      || dst_uri_size == 0U
      || dst_uri_size > SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  sl_strcpy_s(dst_uri, dst_uri_size, _settings.notify_coap_uri_path);
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_set_notify_download_chunk(const uint16_t chunk_cnt)
{
  if (!chunk_cnt) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  _settings.notify_dwnld_chunk_cnt = chunk_cnt;
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_ota_dfu_get_notify_download_chunk(uint16_t * const dst_chunk_cnt)
{
  if (dst_chunk_cnt == NULL) {
    return SL_STATUS_FAIL;
  }

  _ota_dfu_mtx_acquire();
  *dst_chunk_cnt = _settings.notify_dwnld_chunk_cnt;
  _ota_dfu_mtx_release();

  return SL_STATUS_OK;
}

#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static const char *_get_status_json_string(void)
{
  char *str = NULL;
  uint32_t flags = 0UL;

  str = (char *)sl_malloc(SL_WISUN_OTA_DFU_STATUS_JSON_STR_MAX_LEN);

  if (str == NULL) {
    return NULL;
  }

  flags = osEventFlagsGet(_ota_dfu_evt);

  // Evt flag error
  if (flags & SL_WISUN_OTA_DFU_EVT_FLAG_ERROR_MSK) {
    return NULL;
  }

  // Update elapsed time since reset
  (void) sl_sleeptimer_convert_time_to_date(sl_sleeptimer_get_time(), 0, &_stats.elapsed_time_since_rst);

  (void) snprintf(str,
                  SL_WISUN_OTA_DFU_STATUS_JSON_STR_MAX_LEN,
                  SL_WISUN_OTA_DFU_STATUS_JSON_FORMAT_STR,
                  _stats.global_ip_str,
                  (_stats.fw_downl_time.day_of_year > 0U ? _stats.fw_downl_time.day_of_year - 1U : 0U),
                  _stats.fw_downl_time.hour,
                  _stats.fw_downl_time.min,
                  _stats.fw_downl_time.sec,
                  (_stats.fw_update_time.day_of_year > 0U ? _stats.fw_update_time.day_of_year - 1U : 0U),
                  _stats.fw_update_time.hour,
                  _stats.fw_update_time.min,
                  _stats.fw_update_time.sec,
                  (_stats.elapsed_time_since_rst.day_of_year > 0U ? _stats.elapsed_time_since_rst.day_of_year - 1U : 0U),
                  _stats.elapsed_time_since_rst.hour,
                  _stats.elapsed_time_since_rst.min,
                  _stats.elapsed_time_since_rst.sec,
                  _stats.downl_bytes,
                  flags,
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_START_FW_UPDATE_MSK),
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOADED_MSK),
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_FW_VERIFIED_MSK),
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_MSK),
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_STOP_FW_UPDATE_MSK),
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOAD_ERROR_MSK),
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_FW_VERIFY_ERROR_MSK),
                  (bool)(flags & SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_ERROR_MSK),
                  _stats.resent_cnt, _stats.received_cnt);

  return (const char *)str;
}

static void _create_response(const sl_wisun_ota_dfu_response_t * const response,
                             const sl_wisun_coap_packet_t * const req_packet,
                             sl_wisun_coap_packet_t * const resp_packet)
{
  char *req_payload_str = NULL;

  resp_packet->msg_code = response->msg_code;

  req_payload_str = sl_wisun_coap_get_payload_str(req_packet);

  if (req_payload_str != NULL) {
    (void) snprintf((char *)resp_packet->payload_ptr,
                    SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN,
                    response->format_str,
                    req_payload_str);
    sl_wisun_coap_destroy_payload_str(req_payload_str);
  }
}

static void _handle_get_requests(const sl_wisun_coap_packet_t * const req_packet,
                                 sl_wisun_coap_packet_t * const resp_packet)
{
  sl_status_t status = SL_STATUS_FAIL;
  uint16_t tmp_val = 0U;
  static char tmp_addr[SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN] = { 0 };
  static sl_wisun_ota_dfu_remote_arg_t arg = { 0 };
  char *cmd = NULL;

  // Get status request handling
  if (req_packet->payload_ptr == NULL
      || req_packet->payload_len == 0) {
    resp_packet->payload_ptr = (uint8_t * ) _get_status_json_string();
    return;
  }

  resp_packet->payload_ptr = (uint8_t *)sl_malloc(SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN);

  if (resp_packet->payload_ptr == NULL) {
    return;
  }

  cmd = sl_wisun_coap_get_payload_str(req_packet);
  if (cmd == NULL) {
    sl_free(resp_packet->payload_ptr);
    return;
  }
  _parse_arguments(cmd, &arg);

  if (arg.key == NULL) {
    _create_response(&_response_error, req_packet, resp_packet);
    sl_wisun_coap_destroy_payload_str(cmd);
    return;
  }

  // Get help
  if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_HELP,
               arg.key,
               SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    (void) snprintf((char *)resp_packet->payload_ptr,
                    SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN,
                    SL_WISUN_OTA_DFU_RESPONSE_PAYLOAD_STR_HELP);
    sl_wisun_coap_destroy_payload_str(cmd);
    return;

    // Get host address
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_get_host_addr(tmp_addr,
                                            SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN,
                                            &tmp_val);
    (void) snprintf((char *)resp_packet->payload_ptr,
                    SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN,
                    "%s %u",
                    tmp_addr,
                    tmp_val);

    // Get gbl path
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_GBL_PATH,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_get_gbl_path((char *)resp_packet->payload_ptr,
                                           SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN);

    // Get uri path
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_URI,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_get_host_uri((char *)resp_packet->payload_ptr,
                                           SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN);

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
    // Get notification host address
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_HOST_ADDRESS,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_get_notify_host_addr(tmp_addr,
                                                   SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN,
                                                   &tmp_val);
    (void) snprintf((char *)resp_packet->payload_ptr,
                    SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN,
                    "%s %u",
                    tmp_addr,
                    tmp_val);

    // Get notification uri path
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_URI,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_get_notify_host_uri((char *)resp_packet->payload_ptr,
                                                  SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN);

    // Get notification download chunk cnt
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_DOWNLOAD_CHUNK_CNT,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_get_notify_download_chunk(&tmp_val);
    (void) snprintf((char *)resp_packet->payload_ptr,
                    SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN,
                    "%u",
                    tmp_val);
#endif
  }

  if (status != SL_STATUS_OK) {
    _create_response(&_response_error, req_packet, resp_packet);
  }

  sl_wisun_coap_destroy_payload_str(cmd);
}

static void _parse_arguments(char * const cmd,
                             sl_wisun_ota_dfu_remote_arg_t * const arg)
{
  arg->key = strtok(cmd, SL_WISUN_OTA_DFU_CMD_DELIMITER);
  arg->val1 = strtok(NULL, SL_WISUN_OTA_DFU_CMD_DELIMITER);
  arg->val2 = strtok(NULL, SL_WISUN_OTA_DFU_CMD_DELIMITER);
}

static void _handle_post_requests(const sl_wisun_coap_packet_t * const req_packet,
                                  sl_wisun_coap_packet_t * const resp_packet)
{
  static sl_wisun_ota_dfu_remote_arg_t arg = { 0 };
  sl_status_t status = SL_STATUS_FAIL;
  char *cmd = NULL;

  resp_packet->payload_ptr = (uint8_t *)sl_malloc(SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN);

  if (resp_packet->payload_ptr == NULL) {
    return;
  }

  cmd = sl_wisun_coap_get_payload_str(req_packet);
  if (cmd == NULL) {
    sl_free(resp_packet->payload_ptr);
    return;
  }
  _parse_arguments(cmd, &arg);

  if (arg.key == NULL) {
    _create_response(&_response_error, req_packet, resp_packet);
    sl_wisun_coap_destroy_payload_str(cmd);
    return;
  }

  // Start fw update
  if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_START,
               arg.key,
               SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_start_fw_update();
    // Stop fw update
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_STOP,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_stop_fw_update();

#if !SL_WISUN_OTA_DFU_AUTO_INSTALL_ENABLED
    // Reboot and install firmware
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_REBOOT_AND_INSTALL,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    status = sl_wisun_ota_dfu_reboot_and_install();
#endif

    // Set host address
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_HOST_ADDRESS,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    if (arg.val1 == NULL || arg.val2 == NULL) {
      _create_response(&_response_error, req_packet, resp_packet);
      sl_wisun_coap_destroy_payload_str(cmd);
      return;
    }

    status = sl_wisun_ota_dfu_set_host_addr(arg.val1, (uint16_t) atol(arg.val2));

    // Set gbl path
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_GBL_PATH,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    if (arg.val1 == NULL) {
      _create_response(&_response_error, req_packet, resp_packet);
      sl_wisun_coap_destroy_payload_str(cmd);
      return;
    }

    status = sl_wisun_ota_dfu_set_gbl_path(arg.val1);

    // Set uri path
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_URI,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    if (arg.val1 == NULL) {
      _create_response(&_response_error, req_packet, resp_packet);
      sl_wisun_coap_destroy_payload_str(cmd);
      return;
    }

    (void) sl_wisun_coap_rhnd_resource_remove_by_uri(_settings.coap_uri_path);
    status = sl_wisun_ota_dfu_set_host_uri(arg.val1);
    _ota_dfu_resource.data.uri_path = _settings.coap_uri_path;
    (void) sl_wisun_coap_rhnd_resource_add(&_ota_dfu_resource);

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
    // Set notification host address
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_HOST_ADDRESS,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    if (arg.val1 == NULL || arg.val2 == NULL) {
      _create_response(&_response_error, req_packet, resp_packet);
      sl_wisun_coap_destroy_payload_str(cmd);
      return;
    }

    status = sl_wisun_ota_dfu_set_notify_host_addr(arg.val1, (uint16_t) atol(arg.val2));

    // Set notification uri path
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_URI,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    if (arg.val1 == NULL) {
      _create_response(&_response_error, req_packet, resp_packet);
      sl_wisun_coap_destroy_payload_str(cmd);
      return;
    }

    status = sl_wisun_ota_dfu_set_notify_host_uri(arg.val1);

    // Set notification download chunk cnt
  } else if (!strncmp(SL_WISUN_OTA_DFU_PAYLOAD_STR_NOTIFY_DOWNLOAD_CHUNK_CNT,
                      arg.key,
                      SL_WISUN_OTA_DFU_RESPONSE_STR_MAX_LEN)) {
    if (arg.val1 == NULL) {
      _create_response(&_response_error, req_packet, resp_packet);
      sl_wisun_coap_destroy_payload_str(cmd);
      return;
    }

    status = sl_wisun_ota_dfu_set_notify_download_chunk((uint16_t) atol(arg.val1));
#endif
  }

  _create_response(status == SL_STATUS_OK
                   ? &_response_ack : &_response_error,
                   req_packet,
                   resp_packet);
  sl_wisun_coap_destroy_payload_str(cmd);
}

static sl_wisun_coap_packet_t * _ota_dfu_coap_response_cb(const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet = NULL;

  resp_packet = sl_wisun_coap_build_response(req_packet,
                                             COAP_MSG_CODE_RESPONSE_NOT_ACCEPTABLE);

  // Packet cannot be allocated
  if (resp_packet == NULL) {
    return NULL;
  }

  // Handle incoming empty payload
  if (req_packet->payload_ptr == NULL
      && req_packet->msg_code == COAP_MSG_CODE_REQUEST_POST) {
    return resp_packet;
  }

  // Init global IP string
  if (_stats.global_ip_str == NULL) {
    _stats.global_ip_str = sl_tftp_get_global_addr();
  }

  resp_packet->msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
  resp_packet->content_format = COAP_CT_JSON;

  if (req_packet->msg_code == COAP_MSG_CODE_REQUEST_GET) {
    _handle_get_requests(req_packet, resp_packet);
  } else if (req_packet->msg_code == COAP_MSG_CODE_REQUEST_POST) {
    _handle_post_requests(req_packet, resp_packet);
  }

  if (resp_packet->payload_ptr != NULL) {
    resp_packet->payload_len = (uint16_t)sl_strnlen((char *) resp_packet->payload_ptr,
                                                    SL_WISUN_OTA_DFU_STATUS_JSON_STR_MAX_LEN);
  } else {
    resp_packet->payload_len = 0U;
  }

  return resp_packet;
}

static void _tftp_data_hnd(sl_tftp_clnt_t * const clnt,
                           const uint8_t * const data_ptr,
                           const uint16_t data_size)
{
  uint32_t offset = 0UL;
  static uint32_t prev_offset = 0xFFFFFFFFUL;
  static sl_wisun_ota_dfu_error_ctx_t error_ctx = { 0U };
  static uint16_t chunk_cnt = 1U;

  // Check stop request and terminate the session if stopped
  if (sl_wisun_ota_dfu_get_fw_update_status_flag(SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STOPPED)) {
    (void) sl_tftp_clnt_terminate_session(clnt);
    return;
  }

  // Calculate offset
  offset = (uint32_t)((clnt->packet.content.data.block_num - 1U) * clnt->options.blksize);

  _stats.received_cnt++;

  // Try to write the same offset
  // TFTP Server resent data packet (ack from client has not been received in time)
  if (offset == prev_offset) {
    _stats.resent_cnt++;
    sl_wisun_ota_dfu_log("download: resent   chunk %u, offset: 0x%.8lx, resent/received: (%lu/%lu) %lu.%02lu %%\n",
                         clnt->packet.content.data.block_num,
                         offset,
                         _stats.resent_cnt,
                         _stats.received_cnt,
                         ((100UL * ((_stats.received_cnt * 100UL) - (_stats.resent_cnt * 100UL))) / _stats.received_cnt) / 100UL,
                         ((100UL * ((_stats.received_cnt * 100UL) - (_stats.resent_cnt * 100UL))) / _stats.received_cnt) % 100UL);
    return;
  }
  prev_offset = offset;

  if (clnt->packet.content.data.block_num == 1U) {
    _stats.downl_bytes = clnt->packet.content.data.data_size;
  } else {
    _stats.downl_bytes += clnt->packet.content.data.data_size;
    ++chunk_cnt;
  }

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
  if (_settings.notify_dwnld_chunk_cnt == chunk_cnt) {
    chunk_cnt = 0U;
    _notify_host();
  }
#endif

  error_ctx.download.ret_val = bootloader_writeStorage(SL_WISUN_OTA_DFU_STORAGE_SLOT_ID,
                                                       offset,
                                                       (uint8_t *)data_ptr,
                                                       data_size);

  if (error_ctx.download.ret_val != BOOTLOADER_OK) {
    _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOAD_ERROR_MSK);
    error_ctx.download.offset = offset;
    error_ctx.download.data_size = data_size;
    sl_wisun_ota_dfu_error_hnd(SL_WISUN_OTA_DFU_ERROR_FW_DOWNLOAD, &error_ctx);
    return;
  }

  // Update fw download time
  (void)sl_sleeptimer_convert_time_to_date(_get_elapsed_time_s(), 0, &_stats.fw_downl_time);

  // Update fw update time
  (void) sl_sleeptimer_convert_time_to_date(_get_elapsed_time_s(), 0, &_stats.fw_update_time);

  sl_wisun_ota_dfu_log("download: received chunk %u, offset: 0x%.8lx (%d bytes)\n",
                       clnt->packet.content.data.block_num, offset, data_size);
}

static void _tftp_error_hnd(sl_tftp_clnt_t * const clnt,
                            const uint16_t error_code,
                            const char *error_msg)
{
  (void)clnt;
#if !SL_WISUN_OTA_DFU_VERBOSE_MODE_ENABLED
  (void)error_code;
  (void)error_msg;
#endif

  sl_wisun_ota_dfu_log("TFTP error code: %u, %s\n", error_code, error_msg);
}

static void _ota_dfu_thr_fnc(void * args)
{
  static BootloaderStorageInformation_t storage_info = { 0U };
  static BootloaderStorageSlot_t slot_info = { 0U };
  static sl_tftp_clnt_t tftp_clnt = { 0U };
  static sl_wisun_ota_dfu_error_ctx_t error_ctx = { 0U };
  uint32_t flags = 0UL;

  (void) args;

  // Fetch reset reason at startup for debug purposes
  sl_wisun_ota_dfu_log("Reset cause debug info - {reason: 0x%.4x, signature: 0x%.4x}\n",
                       bootloader_getResetReason().reason,
                       bootloader_getResetReason().signature);

  // Get bootloader storage and slot info
  bootloader_getStorageInfo(&storage_info);
  EFM_ASSERT(storage_info.numStorageSlots >= 1);
  EFM_ASSERT(bootloader_getStorageSlotInfo(SL_WISUN_OTA_DFU_STORAGE_SLOT_ID, &slot_info) == BOOTLOADER_OK);

  SL_WISUN_OTA_DFU_SERVICE_LOOP() {
    osEventFlagsClear(_ota_dfu_evt, SL_WISUN_OTA_DFU_EVT_FLAG_START_FW_UPDATE_MSK);
    flags = osEventFlagsWait(_ota_dfu_evt,
                             SL_WISUN_OTA_DFU_EVT_FLAG_START_FW_UPDATE_MSK,
                             osFlagsWaitAny | osFlagsNoClear,
                             osWaitForever);

    // Start time
    _stats.start_time = sl_sleeptimer_get_time();

    if (flags & SL_WISUN_OTA_DFU_EVT_FLAG_ERROR_MSK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_ERROR_MSK);
      sl_wisun_ota_dfu_log("OTA DFU evt flags (0x%08lX) failure\n", flags);
      continue;
    }

    // Clear all mask except started flag
    osEventFlagsClear(_ota_dfu_evt,
                      SL_WISUN_OTA_DFU_EVT_FLAG_ALL_MSK
                      ^ SL_WISUN_OTA_DFU_EVT_FLAG_START_FW_UPDATE_MSK);

    // Erase storage slot before firmware download (necessary for delta ota dfu)
    if (bootloader_eraseStorageSlot(SL_WISUN_OTA_DFU_STORAGE_SLOT_ID) != BOOTLOADER_OK) {
      sl_wisun_ota_dfu_log("Warning: erase storage slot failed\n");
    }

    sl_wisun_ota_dfu_log("Storage info: version: %lu, capabilities: %lu, storageType: %lu, numStorageSlots: %lu\n",
                         storage_info.version,
                         storage_info.capabilities,
                         (uint32_t)storage_info.storageType,
                         storage_info.numStorageSlots);
    sl_wisun_ota_dfu_log("Storage slot info: address: 0x%lx, size: %lu\n", slot_info.address, slot_info.length);
    sl_wisun_ota_dfu_log("Firmware upgrade started\n");

    // Check stop request
    if (sl_wisun_ota_dfu_get_fw_update_status_flag(SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STOPPED)) {
#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
      _notify_host();
#endif
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_STOP_FW_UPDATE_MSK);
      sl_wisun_ota_dfu_log("Firmware update stopped\n");
      continue;
    }

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
    _notify_host();
#endif

    // Initialize TFTP client
    if (sl_tftp_clnt_init(&tftp_clnt,
                          _settings.host_addr_str,
                          _settings.host_port,
                          _tftp_data_hnd,
                          _tftp_error_hnd) != SL_STATUS_OK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOAD_ERROR_MSK);
      sl_wisun_ota_dfu_log("TFTP init failed\n");
      continue;
    }
    sl_wisun_ota_dfu_log("TFTP init done\n");

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
    _notify_host();
#endif

    sl_wisun_ota_dfu_log("TFTP download started: tftp://[%s]:%u/%s\n",
                         _settings.host_addr_str,
                         _settings.host_port,
                         _settings.gbl_path_str);

    // Set TFTP options block size
    if (sl_tftp_clnt_set_option(&tftp_clnt,
                                SL_TFTP_OPT_EXT_BLOCKSIZE,
                                SL_WISUN_OTA_DFU_TFTP_DATA_BLOCK_SIZE) != SL_STATUS_OK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_ERROR_MSK);
      sl_wisun_ota_dfu_log("TFTP set 'blksize' option failed\n");
      continue;
    }

    // Set TFTP options timeout
    if (sl_tftp_clnt_set_option(&tftp_clnt,
                                SL_TFTP_OPT_EXT_TIMEOUT_INTERVAL,
                                SL_WISUN_OTA_DFU_TFTP_TIMEOUT_SEC) != SL_STATUS_OK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_ERROR_MSK);
      sl_wisun_ota_dfu_log("TFTP set 'timeout' option failed\n");
      continue;
    }

    // Send RRQ request
    if (sl_tftp_clnt_request(&tftp_clnt,
                             SL_TFTP_OPCODE_RRQ,
                             _settings.gbl_path_str,
                             SL_TFTP_MODE_OCTET_STR) != SL_STATUS_OK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_ERROR_MSK);
      sl_wisun_ota_dfu_log("TFTP RRQ request failed\n");
      continue;
    }

    // Wait for firmware download to finish
    if (sl_tftp_clnt_wait_op_finished(&tftp_clnt) != SL_STATUS_OK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_ERROR_MSK);
      sl_wisun_ota_dfu_log("TFTP wait for firmware download to finish failed\n");
      continue;
    }

    // Check download error
    if (sl_tftp_clnt_is_op_rrq_wrq_failed(&tftp_clnt)
        || sl_wisun_ota_dfu_get_fw_update_status_flag(SL_WISUN_OTA_DFU_STATUS_FW_DOWNLOAD_ERROR)) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOAD_ERROR_MSK);
      sl_wisun_ota_dfu_log("TFTP download failed\n");
      continue;
    }

    // Check stop request
    if (sl_wisun_ota_dfu_get_fw_update_status_flag(SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STOPPED)) {
#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
      _notify_host();
#endif
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_STOP_FW_UPDATE_MSK);
      sl_wisun_ota_dfu_log("Firmware update stopped\n");
      continue;
    }
    _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_DOWNLOADED_MSK);
    sl_wisun_ota_dfu_log("TFTP download finished\n");
    osDelay(SL_WISUN_OTA_DFU_DELAY_MS);

    // Verify image
    error_ctx.verify.ret_val = bootloader_verifyImage(SL_WISUN_OTA_DFU_STORAGE_SLOT_ID, NULL);
    if (error_ctx.verify.ret_val != BOOTLOADER_OK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_VERIFY_ERROR_MSK);
      sl_wisun_ota_dfu_log("Verify image failed\n");
      sl_wisun_ota_dfu_error_hnd(SL_WISUN_OTA_DFU_ERROR_FW_VERIFY, &error_ctx);
      continue;
    }
    _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_VERIFIED_MSK);
    sl_wisun_ota_dfu_log("Verify image finished\n");
    osDelay(SL_WISUN_OTA_DFU_DELAY_MS);

    // Set image
    error_ctx.set.ret_val = bootloader_setImageToBootload(SL_WISUN_OTA_DFU_STORAGE_SLOT_ID);
    if (error_ctx.set.ret_val != BOOTLOADER_OK) {
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_ERROR_MSK);
      sl_wisun_ota_dfu_log("Set image failed");
      sl_wisun_ota_dfu_error_hnd(SL_WISUN_OTA_DFU_ERROR_FW_SET, &error_ctx);
      continue;
    }
    _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_FW_SET_MSK);
    sl_wisun_ota_dfu_log("Set image finished\n");
    osDelay(SL_WISUN_OTA_DFU_SHUTDOWN_DELAY_MS);

    // Check stop request
    if (sl_wisun_ota_dfu_get_fw_update_status_flag(SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STOPPED)) {
#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
      _notify_host();
#endif
      _change_status(SL_WISUN_OTA_DFU_EVT_FLAG_STOP_FW_UPDATE_MSK);
      sl_wisun_ota_dfu_log("Firmware update stopped\n");
      continue;
    }

    // Reboot and install firmware
#if SL_WISUN_OTA_DFU_AUTO_INSTALL_ENABLED
    sl_wisun_ota_dfu_log("Starting reboot and install...\n");
    osDelay(SL_WISUN_OTA_DFU_DELAY_MS);
    bootloader_rebootAndInstall();
#endif
  }
}

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
static sl_wisun_coap_packet_t * _notify_cb(const struct sl_wisun_coap_notify *notify)
{
  static sl_wisun_coap_packet_t notify_pkt = {
    .msg_code = COAP_MSG_CODE_REQUEST_PUT,
    .msg_id = SL_WISUN_OTA_DFU_HOST_NOTIFY_COAP_MSG_ID,
    .msg_type = COAP_MSG_TYPE_NON_CONFIRMABLE,
    .content_format = COAP_CT_JSON,
  };

  (void) notify;

  // Do not prepare the packet if notify required state is false
  if (!_is_notify_required) {
    return &notify_pkt;
  }

  // Update URI path
  notify_pkt.uri_path_ptr = (uint8_t *)_settings.notify_coap_uri_path;
  notify_pkt.uri_path_len = sl_strnlen((char *) _settings.notify_coap_uri_path,
                                       SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN);

  // Update payload
  notify_pkt.payload_ptr = (uint8_t *)_get_status_json_string();
  notify_pkt.payload_len = sl_strnlen((char *) notify_pkt.payload_ptr,
                                      SL_WISUN_OTA_DFU_STATUS_JSON_STR_MAX_LEN);

  sl_wisun_ota_dfu_log("notify: coap://[%s]:%u%s\n",
                       _settings.notify_host_addr_str,
                       _settings.notify_host_port,
                       _settings.notify_coap_uri_path);
  return &notify_pkt;
}

bool _notify_condition_cb(const struct sl_wisun_coap_notify *notify)
{
  bool ret = true;

  (void) notify;

  ret = _is_notify_required;
  if (ret) {
    _is_notify_required = false;
  }
  return ret;
}

__STATIC_INLINE void _notify_host(void)
{
  _is_notify_required = true;
  if (sl_wisun_coap_notify_tick_evt_is_enabled()) {
    sl_wisun_coap_notify_tick();
  }
}
#endif

static void _change_status(const uint32_t status_mask)
{
  // Update fw update time
  (void) sl_sleeptimer_convert_time_to_date(_get_elapsed_time_s(), 0, &_stats.fw_update_time);
  sl_wisun_ota_dfu_log("OTA DFU status update: 0x%lx\n", status_mask);
  (void) osEventFlagsSet(_ota_dfu_evt, status_mask);
#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
  _notify_host();
#endif
}

__STATIC_INLINE sl_sleeptimer_timestamp_t _get_elapsed_time_s(void)
{
  return sl_sleeptimer_get_time() - _stats.start_time;
}

__STATIC_INLINE void _ota_dfu_mtx_acquire(void)
{
  EFM_ASSERT(osMutexAcquire(_ota_dfu_mtx, osWaitForever) == osOK);
}

__STATIC_INLINE void _ota_dfu_mtx_release(void)
{
  EFM_ASSERT(osMutexRelease(_ota_dfu_mtx) == osOK);
}
