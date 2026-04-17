/********************************************************************************
 * @file  x_types.h
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

#ifndef X_TYPES_H
#define X_TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifndef APP_WIFI_TYPE_UINT8ARRARY
#define APP_WIFI_TYPE_UINT8ARRARY
typedef struct {
  uint8_t len;
  uint8_t data[];
} uint8array;
#endif

#ifndef APP_WIFI_TYPE_BYTE_ARRARY
#define APP_WIFI_TYPE_BYTE_ARRARY
typedef struct {
  uint16_t len;
  uint8_t data[];
} byte_array;
#endif

#ifndef APP_WIFI_TYPE_STRING
#define APP_WIFI_TYPE_STRING
typedef struct {
  uint8_t len;
  int8_t data[];
} string;
#endif

#ifndef APP_WIFI_TYPE_BDADDR
#define APP_WIFI_TYPE_BDADDR
typedef struct {
  uint8_t addr[6];
} bd_addr;
#endif

#ifndef APP_WIFI_TYPE_MACADDR
#define APP_WIFI_TYPE_MACADDR
typedef struct {
  uint8_t addr[6];
} mac_addr;
#endif

#ifndef APP_WIFI_TYPE_IPV4ADDR
#define APP_WIFI_TYPE_IPV4ADDR
typedef struct {
  uint8_t addr[4];
} ipv4_addr;
#endif

#ifndef APP_WIFI_TYPE_IPV6ADDR
#define APP_WIFI_TYPE_IPV6ADDR
typedef struct {
  uint8_t addr[16];
} ipv6_addr;
#endif

#ifndef APP_WIFI_TYPE_SERNAME
#define APP_WIFI_TYPE_SERNAME
typedef struct {
  uint8_t name[16];
} ser_name;
#endif

#ifndef APP_WIFI_TYPE_UUID128
#define APP_WIFI_TYPE_UUID128
typedef struct {
  uint8_t data[16];
} uuid_128;
#endif

#ifndef APP_WIFI_TYPE_AES_KEY128
#define APP_WIFI_TYPE_AES_KEY128
typedef struct {
  uint8_t data[16];
} aes_key_128;
#endif

#ifndef APP_WIFI_TYPE_UUID64
#define APP_WIFI_TYPE_UUID64
typedef struct {
  uint8_t data[8];
} app_wifi_uuid_64_t;
#endif

#define MAX_UUID_LEN 16

#ifndef APP_WIFI_TYPE_UUID
#define APP_WIFI_TYPE_UUID
typedef struct {
  uint8_t uuid[MAX_UUID_LEN];
  uint32_t len;
} app_wifi_uuid_t;
#endif

#if !defined _MSC_VER && !defined __APPLE__
/* Don't define under MSVC and OSX */

#ifndef memcpy
void *memcpy(void *OUT, const void *IN, size_t N);
#endif
#ifndef memcmp
int memcmp(const void *S1, const void *S2, size_t N);
#endif
#ifndef memset
void *memset(void *DST, int c, size_t LENGTH);
#endif
#ifndef memmove
void *memmove(void *DST, const void *src, size_t LENGTH);
#endif
#endif

#endif
