/***************************************************************************//**
 * @file
 * @brief RTL log.
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

#include "rtl_log.h"
#include "cs_initiator_config.h"

#if CS_INITIATOR_RTL_LOG
#include "sl_rtl_clib_api.h"
#include "sl_bt_version.h"
#include "sli_bgapi_trace.h"
#include "app_assert.h"
#include <stdio.h>

static void rtl_log_callback(uint8_t *log_data, size_t log_data_len);

void rtl_log_init(void)
{
  int ret;
  enum sl_rtl_error_code ec;
  const char hash[] = SL_BT_VERSION_HASH;
  sl_rtl_log_params config_params = {
    .log_callback_function = rtl_log_callback,
    .sdk_version = { 0 },
    .command_line_options = { 0 }
  };

  ret = snprintf(config_params.sdk_version,
                 SL_RTL_LOG_SDK_VERSION_CHAR_ARRAY_MAX_SIZE,
                 "%u.%u.%u-%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                 SL_BT_VERSION_MAJOR,
                 SL_BT_VERSION_MINOR,
                 SL_BT_VERSION_PATCH,
                 hash[0], hash[1], hash[2], hash[3], hash[4],
                 hash[5], hash[6], hash[7], hash[8], hash[9],
                 hash[10], hash[11], hash[12], hash[13], hash[14],
                 hash[15], hash[16], hash[17], hash[18], hash[19]);
  app_assert(ret > 0 && ret < SL_RTL_LOG_SDK_VERSION_CHAR_ARRAY_MAX_SIZE,
             "failed to construct version string");

  ec = sl_rtl_log_init();
  app_assert(ec == SL_RTL_ERROR_SUCCESS, "sl_rtl_log_init failed");

  ec = sl_rtl_log_configure(&config_params);
  app_assert(ec == SL_RTL_ERROR_SUCCESS, "sl_rtl_log_configure failed");
}

void rtl_log_deinit(void)
{
  enum sl_rtl_error_code ec = sl_rtl_log_deinit();
  app_assert(ec == SL_RTL_ERROR_SUCCESS, "sl_rtl_log_deinit failed");
}

static void rtl_log_callback(uint8_t *log_data, size_t log_data_len)
{
  while (log_data_len > 0) {
    size_t log_written = sli_bgapi_trace_log_custom_message(log_data, log_data_len);
    log_data_len -= log_written;
    log_data += log_written;
  }
}

#else // CS_INITIATOR_RTL_LOG
void rtl_log_init(void)
{
}

void rtl_log_deinit(void)
{
}
#endif // CS_INITIATOR_RTL_LOG
