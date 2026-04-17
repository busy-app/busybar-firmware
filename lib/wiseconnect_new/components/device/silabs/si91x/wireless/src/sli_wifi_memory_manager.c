/***************************************************************************/ /**
 * @file sli_wifi_memory_manager.c
 * @brief This file contains the implementation of the WiFi memory manager functions.
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
#include "sli_wifi_memory_manager.h"
#include "sl_rsi_utility.h"

sl_status_t sli_wifi_memory_manager_allocate_buffer(sl_wifi_buffer_t **buffer,
                                                    sl_wifi_buffer_type_t type,
                                                    uint32_t buffer_size,
                                                    uint32_t timeout_ms)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(buffer);
  if (type >= SL_WIFI_SCAN_RESULT_BUFFER) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (buffer_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  return sli_si91x_host_allocate_buffer(buffer, type, buffer_size, timeout_ms);
}

void sli_wifi_memory_manager_free_buffer(sl_wifi_buffer_t *buffer)
{
  sli_si91x_host_free_buffer(buffer);
}
