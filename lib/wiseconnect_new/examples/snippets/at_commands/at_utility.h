/***************************************************************************/ /**
 * @file at_utility.h
 * @brief Top level application functions
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

#ifndef AT_UTILITY_H
#define AT_UTILITY_H

#include "sl_status.h"
#include "sl_net.h"
#include "cmsis_os2.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern osMutexId_t print_mutex;

#ifndef AT_PRINTF
#define AT_PRINTF(...)                         \
  {                                            \
    osMutexAcquire(print_mutex, 0xFFFFFFFFUL); \
    printf(__VA_ARGS__);                       \
    osMutexRelease(print_mutex);               \
  }
#endif

#define PRINT_AT_CMD_SUCCESS AT_PRINTF("OK ");
#define PRINT_OK_WITH_CRLF   AT_PRINTF("OK\r\n");

/**
 * @brief Checks if all mandatory arguments are passed for a specific AT command.
 *
 * This macro verifies that the required arguments, specified by the `_x` bitmap,
 * are present in the `arguments->bitmap`. If any mandatory argument is missing,
 * the macro returns `SL_STATUS_INVALID_PARAMETER`.
 *
 * @param[in] arguments Pointer to the structure containing the bitmap of passed arguments.
 * @param[in] _x Bitmap representing the mandatory arguments that must be set.
 *
 * @return Returns `SL_STATUS_INVALID_PARAMETER` if any mandatory argument is missing.
 */
#define CHECK_ARGUMENT_BITMAP(arguments, _x) \
  do {                                       \
    if ((arguments->bitmap & _x) != _x) {    \
      return SL_STATUS_INVALID_PARAMETER;    \
    }                                        \
  } while (0)

void at_print_char_buffer(char *buffer, uint32_t buffer_length);

sl_status_t sl_net_inet_addr_auto(const char *addr, sl_ip_address_t *ip_address);

#ifdef __cplusplus
}
#endif

#endif // AT_UTILITY_H
