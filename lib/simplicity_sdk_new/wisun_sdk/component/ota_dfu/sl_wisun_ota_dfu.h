/***************************************************************************//**
 * @file sl_wisun_ota_dfu.h
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

#ifndef SL_WISUN_OTA_DFU_H
#define SL_WISUN_OTA_DFU_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "sl_wisun_ota_dfu_config.h"
#include "sl_status.h"
#include "em_common.h"
#include "sl_sleeptimer.h"

/**************************************************************************//**
 * @addtogroup SL_WISUN_OTA_DFU_API
 * @{
 *****************************************************************************/

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/// OTA DFU IPV6 string buffer length
#define SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN                   40U

/// OTA DFU CoAP URI Path string buffer length
#define SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN          128U

/// OTA DFU GBL File name string buffer length
#define SL_WISUN_OTA_DFU_GBL_PATH_STR_BUF_LEN               256U

/// OTA DFU Service loop
#ifndef SL_WISUN_OTA_DFU_SERVICE_LOOP
#define SL_WISUN_OTA_DFU_SERVICE_LOOP()                     while (1)
#endif

/**************************************************************************//**
 * @defgroup SL_WISUN_OTA_DFU_TYPES Type definitions
 * @ingroup SL_WISUN_OTA_DFU_API
 * @{
 *****************************************************************************/

/// Wi-SUN OTA DFU Status enumeration
typedef enum sl_wisun_ota_dfu_status {
  /// Firmware update started
  SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STARTED = 0,
  /// Firmware downloaded
  SL_WISUN_OTA_DFU_STATUS_FW_DOWNLOADED,
  /// Firmware verified
  SL_WISUN_OTA_DFU_STATUS_FW_VERIFIED,
  /// Firmware set
  SL_WISUN_OTA_DFU_STATUS_FW_SET,
  // Firmware update finished/stopped
  SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STOPPED,
  /// Firmware Download error
  SL_WISUN_OTA_DFU_STATUS_FW_DOWNLOAD_ERROR,
  /// Firmware Verify error
  SL_WISUN_OTA_DFU_STATUS_FW_VERIFY_ERROR,
  /// Firmware Set error
  SL_WISUN_OTA_DFU_STATUS_FW_SET_ERROR
} sl_wisun_ota_dfu_status_t;

/// Wi-SUN OTA DFU error code enumeration
typedef enum sl_wisun_ota_dfu_error_code {
  /// Firmware downloaded error code
  SL_WISUN_OTA_DFU_ERROR_FW_DOWNLOAD = 1001UL,
  /// Firmware verify error code
  SL_WISUN_OTA_DFU_ERROR_FW_VERIFY,
  /// Firmware set error code
  SL_WISUN_OTA_DFU_ERROR_FW_SET
} sl_wisun_ota_dfu_error_code_t;

/// Wi-SUN OTA DFU download error context definition
typedef struct sl_wisun_ota_dfu_error_ctx_fw_download {
  /// Return value of API call
  int32_t ret_val;
  /// Offset of GBL file
  uint32_t offset;
  /// Data size
  uint16_t data_size;
} sl_wisun_ota_dfu_error_ctx_fw_download_t;

/// Wi-SUN OTA DFU verify error context definition
typedef struct sl_wisun_ota_dfu_error_ctx_btl_fw_verify {
  /// Return value of API call
  int32_t ret_val;
} sl_wisun_ota_dfu_error_ctx_btl_fw_verify_t;

/// Wi-SUN OTA DFU set error context definition
typedef struct sl_wisun_ota_dfu_error_ctx_btl_fw_set {
  /// Return value of API call
  int32_t ret_val;
} sl_wisun_ota_dfu_error_ctx_btl_fw_set_t;

/// Wi-SUN OTA DFU error context definition
typedef union sl_wisun_ota_dfu_error_ctx {
  /// Download error context
  sl_wisun_ota_dfu_error_ctx_fw_download_t download;
  /// Verify error context
  sl_wisun_ota_dfu_error_ctx_btl_fw_verify_t verify;
  /// Set error context
  sl_wisun_ota_dfu_error_ctx_btl_fw_set_t set;
} sl_wisun_ota_dfu_error_ctx_t;

/// Wi-SUN OTA DFU statistics typedef
typedef struct sl_wisun_ota_dfu_statistics {
  /// Resent packet count
  uint32_t resent_cnt;
  /// Received packet count
  uint32_t received_cnt;
  /// Downloaded bytes
  uint32_t downl_bytes;
  /// Global IP String
  const char *global_ip_str;
  /// Start time reference in seconds
  sl_sleeptimer_timestamp_t start_time;
  /// Firmware download time
  sl_sleeptimer_date_t fw_downl_time;
  /// Firmware update time
  sl_sleeptimer_date_t fw_update_time;
  /// Elapsed time since reset
  sl_sleeptimer_date_t elapsed_time_since_rst;
} sl_wisun_ota_dfu_statistics_t;

/// Wi-SUN OTA DFU settings typedef
typedef struct sl_wisun_ota_dfu_settings {
  /// Host address string buffer
  char host_addr_str[SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN];
  /// Host port
  uint16_t host_port;
  /// GBL file path
  char gbl_path_str[SL_WISUN_OTA_DFU_GBL_PATH_STR_BUF_LEN];
  /// CoAP URI path
  char coap_uri_path[SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN];
#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
  /// Notify host addr str
  char notify_host_addr_str[SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN];
  /// Notify host port
  uint16_t notify_host_port;
  /// Notify coap uri path
  char notify_coap_uri_path[SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN];
  /// Download chunk count for notification
  uint32_t notify_dwnld_chunk_cnt;
#endif
} sl_wisun_ota_dfu_settings_t;

/** @} (end SL_WISUN_OTA_DFU_TYPES) */

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief Initialize the device firmware upgrade service.
 * @details Initialize Wi-SUN OTA Device Firmware Upgrade service.
 ******************************************************************************/
void sl_wisun_ota_dfu_init(void);

/**************************************************************************//**
 * @brief Start firmware update.
 * @details Start firmware update by setting
 *          SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STARTED flag
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 *****************************************************************************/
sl_status_t sl_wisun_ota_dfu_start_fw_update(void);

/**************************************************************************//**
 * @brief Stop firmware update.
 * @details Stop firmware update by setting
 *          SL_WISUN_OTA_DFU_STATUS_FW_UPDATE_STOPPED flag
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 *****************************************************************************/
sl_status_t sl_wisun_ota_dfu_stop_fw_update(void);

#if !SL_WISUN_OTA_DFU_AUTO_INSTALL_ENABLED
/**************************************************************************//**
 * @brief Reboot device.
 * @details Reboot device with calling corresponding gecko bootloader
 *          'bootloader_rebootAndInstall' API
 *          This functions is available if auto-reboot mode is disabled.
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 *****************************************************************************/
sl_status_t sl_wisun_ota_dfu_reboot_and_install(void);
#endif

/**************************************************************************//**
 * @brief Get the status value.
 * @details Returning the value of event flags
 * @return uint32_t Status
 *****************************************************************************/
uint32_t sl_wisun_ota_dfu_get_fw_update_status(void);

/**************************************************************************//**
 * @brief Get the status string in JSON format.
 * @details String buffer is allocated in heap by CoAP allocator
 * @return const char * Allocated string pointer on success, otherwise NULL
 *****************************************************************************/
const char* sl_wisun_ota_dfu_get_fw_update_status_json_str(void);

/**************************************************************************//**
* @brief Free the status string buffer.
* @details Call CoAP free to release allocated memory
* @param[in] str String ptr
******************************************************************************/
void sl_wisun_ota_dfu_free_fw_update_status_json_str(const char * str);

/**************************************************************************//**
* @brief Get the status flag value.
* @details Bool representation of status variable bit value
* @param[in] status_flag Status flag enum
* @return bool true if the flag is set, otherwise false
******************************************************************************/
bool sl_wisun_ota_dfu_get_fw_update_status_flag(const sl_wisun_ota_dfu_status_t status_flag);

/***************************************************************************//**
 * @brief OTA DFU error handler (weak implementation)
 * @details Catch error in different stages of boot load.
 * @param[in] error_code Error code
 * @param[in] ctx  Error context with error details
 ******************************************************************************/
void sl_wisun_ota_dfu_error_hnd(const sl_wisun_ota_dfu_error_code_t error_code,
                                sl_wisun_ota_dfu_error_ctx_t * const ctx);

/***************************************************************************//**
 * @brief Set host address and port.
 * @details Set host address and port
 * @param[in] host Host address
 * @param[in] port Host port
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_set_host_addr(const char *host, const uint16_t port);

/***************************************************************************//**
 * @brief Get host addres and port.
 * @details Get host address and port
 * @param[out] dst_host Host address
 * @param[in] dst_host_size Host address size
 * @param[out] dst_port Host port
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_get_host_addr(char * const dst_host,
                                           const uint16_t dst_host_size,
                                           uint16_t *dst_port);

/***************************************************************************//**
 * @brief Set host uri path.
 * @details Set host uri path
 * @param[in] uri Uri path
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_set_host_uri(const char *uri);

/***************************************************************************//**
 * @brief Get host uri path.
 * @details Get host uri path
 * @param[out] dst_uri Uri path
 * @param[in] dst_uri_size Uri path size
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_get_host_uri(char * const dst_uri,
                                          const uint16_t dst_uri_size);

/***************************************************************************//**
 * @brief Set GBL file path.
 * @details Set GBL file path
 * @param[in] gbl_path GBL file path
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_set_gbl_path(const char *gbl_path);

/***************************************************************************//**
 * @brief Get GBL file path.
 * @details Get GBL file path
 * @param[out] dst_gbl_path GBL file path
 * @param[in] dst_gbl_path_size GBL path size
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_get_gbl_path(char * const dst_gbl_path,
                                          const uint16_t dst_gbl_path_size);

/***************************************************************************//**
 * @brief Get firmware update statistics.
 * @details Get firmware update statistics
 * @param[out] stats Pointer to statistics structure
 * @return sl_status_t SL_STATUS_OK on success, otherwise failure
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_get_fw_update_statistics(sl_wisun_ota_dfu_statistics_t * const stats);

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
/***************************************************************************//**
 * @brief Set notify host address and port.
 * @details Set notify host address and port
 * @param[in] host Host address
 * @param[in] port Host port
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_set_notify_host_addr(const char *host,
                                                  const uint16_t port);

/***************************************************************************//**
 * @brief Get notify host address and port.
 * @details Get notify host address and port
 * @param[out] dst_host Host address
 * @param[in] dst_host_size Host address size
 * @param[out] dst_port Host port
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_get_notify_host_addr(char * const dst_host,
                                                  const uint16_t dst_host_size,
                                                  uint16_t *dst_port);

/***************************************************************************//**
 * @brief Set notify host uri path.
 * @details Set notify host uri path
 * @param[in] uri Uri path
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_set_notify_host_uri(const char *uri);

/***************************************************************************//**
 * @brief Get notify host uri path.
 * @details Get notify host uri path
 * @param[out] dst_uri Uri path
 * @param[in] dst_uri_size Uri path size
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_get_notify_host_uri(char * const dst_uri,
                                                 const uint16_t dst_uri_size);

/***************************************************************************//**
 * @brief Set number of download chunks.
 * @details Set number of download chunks
 * @param[in] chunk_cnt Number of download chunks
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_set_notify_download_chunk(const uint16_t chunk_cnt);

/***************************************************************************//**
 * @brief Get number of download chunks.
 * @details Get number of download chunks
 * @param[out] dst_chunk_cnt Number of download chunks
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_wisun_ota_dfu_get_notify_download_chunk(uint16_t * const dst_chunk_cnt);

#endif // SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED

/** @}*/

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_OTA_DFU_H
