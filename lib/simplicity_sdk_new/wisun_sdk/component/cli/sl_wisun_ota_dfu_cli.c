/***************************************************************************//**
 * @file sl_wisun_ota_dfu_cli.c
 * @brief Wi-SUN OTA DFU CLI handler
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_cli.h"
#include "sl_wisun_cli_core.h"
#include "sl_wisun_ota_dfu.h"
#include "sl_wisun_ota_dfu_config.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Error format string
#define SL_WISUN_OTA_DFU_CLI_ERROR_STR  "[Failed]\n"

/// Disabled Notification service error string
#define SL_WISUN_OTA_DFU_CLI_DISABLED_NOTIFICATION_ERROR_STR \
  "[Error: OTA DFU notification service is disabled]"

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void app_fw_update_start(const sl_cli_command_arg_t *arguments)
{
  sl_status_t stat = SL_STATUS_FAIL;
  (void) arguments;
  stat = sl_wisun_ota_dfu_start_fw_update();
  printf("[Firmware update %s]\n", stat == SL_STATUS_OK ? "started" : "cannot be started");
}

void app_fw_update_stop(const sl_cli_command_arg_t *arguments)
{
  sl_status_t stat = SL_STATUS_FAIL;
  (void) arguments;
  stat = sl_wisun_ota_dfu_stop_fw_update();
  printf("[Firmware update %s]\n", stat == SL_STATUS_OK ? "stopped" : "cannot be stopped");
}

void app_fw_reboot_and_install(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if SL_WISUN_OTA_DFU_AUTO_INSTALL_ENABLED
  printf("[Firmware reboot command is not available in auto-reboot mode]\n");
#else
  printf("[Installing firmware...]\n");
  (void) sl_wisun_ota_dfu_reboot_and_install();
  printf("[Firmware update reboot was unsuccessful]\n");
#endif
}

void app_fw_update_status(const sl_cli_command_arg_t *arguments)
{
  const char *str = NULL;
  (void) arguments;
  str = sl_wisun_ota_dfu_get_fw_update_status_json_str();

  if (str == NULL) {
    printf("[Error: firmware update status get]\n");
  } else {
    printf("%s\n", str);
    sl_wisun_ota_dfu_free_fw_update_status_json_str(str);
  }
}

void app_set_host_addr(const sl_cli_command_arg_t *arguments)
{
  const char *host = NULL;
  uint16_t port = 0U;
  sl_status_t status = SL_STATUS_FAIL;

  host = sl_cli_get_argument_string(arguments, 0);
  port = sl_cli_get_argument_uint16(arguments, 1);

  status = sl_wisun_ota_dfu_set_host_addr(host, port);
  if (status != SL_STATUS_OK) {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_get_host_addr(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  static char host_addr[SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN] = { 0 };
  uint16_t port = 0U;

  sl_status_t status = SL_STATUS_FAIL;

  status = sl_wisun_ota_dfu_get_host_addr(host_addr,
                                          SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN,
                                          &port);
  if (status == SL_STATUS_OK) {
    printf("[%s %u]\n", host_addr, port);
  } else {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_set_host_uri(const sl_cli_command_arg_t *arguments)
{
  const char *uri = NULL;
  sl_status_t status = SL_STATUS_FAIL;

  uri = sl_cli_get_argument_string(arguments, 0);

  status = sl_wisun_ota_dfu_set_host_uri(uri);
  if (status != SL_STATUS_OK) {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_get_host_uri(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  static char uri[SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN] = { 0 };
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_wisun_ota_dfu_get_host_uri(uri,
                                         SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN);
  if (status == SL_STATUS_OK) {
    printf("[%s]\n", uri);
  } else {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_set_gbl_path(const sl_cli_command_arg_t *arguments)
{
  const char *gbl_path = NULL;
  sl_status_t status = SL_STATUS_FAIL;

  gbl_path = sl_cli_get_argument_string(arguments, 0);

  status = sl_wisun_ota_dfu_set_gbl_path(gbl_path);
  if (status != SL_STATUS_OK) {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_get_gbl_path(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  static char gbl_path[SL_WISUN_OTA_DFU_GBL_PATH_STR_BUF_LEN] = { 0 };
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_wisun_ota_dfu_get_gbl_path(gbl_path,
                                         SL_WISUN_OTA_DFU_GBL_PATH_STR_BUF_LEN);
  if (status == SL_STATUS_OK) {
    printf("[%s]\n", gbl_path);
  } else {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

#if SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED
void app_set_notify_host_addr(const sl_cli_command_arg_t *arguments)
{
  const char *host = NULL;
  uint16_t port = 0U;
  sl_status_t status = SL_STATUS_FAIL;

  host = sl_cli_get_argument_string(arguments, 0);
  port = sl_cli_get_argument_uint16(arguments, 1);

  status = sl_wisun_ota_dfu_set_notify_host_addr(host, port);
  if (status != SL_STATUS_OK) {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_get_notify_host_addr(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  static char host_addr[SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN] = { 0 };
  uint16_t port = 0U;
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_wisun_ota_dfu_get_notify_host_addr(host_addr,
                                                 SL_WISUN_OTA_DFU_IPV6_STR_BUF_LEN,
                                                 &port);
  if (status == SL_STATUS_OK) {
    printf("[%s %u]\n", host_addr, port);
  } else {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_set_notify_host_uri(const sl_cli_command_arg_t *arguments)
{
  const char *uri = NULL;
  sl_status_t status = SL_STATUS_FAIL;

  uri = sl_cli_get_argument_string(arguments, 0);

  status = sl_wisun_ota_dfu_set_notify_host_uri(uri);
  if (status != SL_STATUS_OK) {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_get_notify_host_uri(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  static char uri[SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN] = { 0 };
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_wisun_ota_dfu_get_notify_host_uri(uri,
                                                SL_WISUN_OTA_DFU_COAP_URI_PATH_STR_BUF_LEN);
  if (status == SL_STATUS_OK) {
    printf("[%s]\n", uri);
  } else {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_set_notify_download_chunk(const sl_cli_command_arg_t *arguments)
{
  uint16_t chunk_cnt = 0U;
  sl_status_t status = SL_STATUS_FAIL;

  chunk_cnt = sl_cli_get_argument_uint16(arguments, 0);

  status = sl_wisun_ota_dfu_set_notify_download_chunk(chunk_cnt);
  if (status != SL_STATUS_OK) {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}

void app_get_notify_download_chunk(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  uint16_t chunk_cnt = 0U;
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_wisun_ota_dfu_get_notify_download_chunk(&chunk_cnt);
  if (status == SL_STATUS_OK) {
    printf("[%u]\n", chunk_cnt);
  } else {
    printf(SL_WISUN_OTA_DFU_CLI_ERROR_STR);
  }
}
#else
void app_set_notify_host_addr(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  printf(SL_WISUN_OTA_DFU_CLI_DISABLED_NOTIFICATION_ERROR_STR);
}

void app_get_notify_host_addr(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  printf(SL_WISUN_OTA_DFU_CLI_DISABLED_NOTIFICATION_ERROR_STR);
}

void app_set_notify_host_uri(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  printf(SL_WISUN_OTA_DFU_CLI_DISABLED_NOTIFICATION_ERROR_STR);
}

void app_get_notify_host_uri(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  printf(SL_WISUN_OTA_DFU_CLI_DISABLED_NOTIFICATION_ERROR_STR);
}

void app_set_notify_download_chunk(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  printf(SL_WISUN_OTA_DFU_CLI_DISABLED_NOTIFICATION_ERROR_STR);
}

void app_get_notify_download_chunk(const sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  printf(SL_WISUN_OTA_DFU_CLI_DISABLED_NOTIFICATION_ERROR_STR);
}
#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
