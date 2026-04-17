/***************************************************************************/ /**
 * @file sl_wlan_logger.h
 * @brief 
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

#include <string.h>
#include <stdio.h>
#include "cmsis_compiler.h"

#ifdef PRINT_DEBUG_LOG
extern void sl_debug_log(const char *format, ...);
#define SL_DEBUG_LOG(format, ...)                                                              \
  do {                                                                                         \
    sl_debug_log("%s:%s:%d:" format "\r\n", __FILE_NAME__, __func__, __LINE__, ##__VA_ARGS__); \
  } while (0)
#else
#define SL_DEBUG_LOG(format, ...) \
  do {                            \
  } while (0)
#endif