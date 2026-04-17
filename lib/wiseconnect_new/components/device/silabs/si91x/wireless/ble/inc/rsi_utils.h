/*******************************************************************************
 * @file  rsi_utils.h
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

#ifndef RSI_UTILS_H
#define RSI_UTILS_H

#include <stdint.h>
/******************************************************
 * *                      Macros
 * ******************************************************/
#ifndef BIT
#define BIT(a) ((uint32_t)1U << a)
#endif

//This statement is modified to avoid compilation warning from RSI_MIN(x, y) ((x) > (y) ? (y) : (x))
#define RSI_MIN(x, y) ((int32_t)(x) > (int32_t)(y) ? (int32_t)(y) : (int32_t)(x))

#ifndef NULL
#define NULL 0
#endif

/******************************************************
 * *                    Constants
 * ******************************************************/
/******************************************************
 * *                   Enumerations
 * ******************************************************/
/******************************************************
 * *                 Type Definitions
 * ******************************************************/
/******************************************************
 * *                    Structures
 * ******************************************************/
/******************************************************
 * *                 Global Variables
 * ******************************************************/
/******************************************************
 * *               Function Declarations
 * ******************************************************/
void rsi_uint32_to_4bytes(uint8_t *dBuf, uint32_t val);
void rsi_uint16_to_2bytes(uint8_t *dBuf, uint16_t val);
uint16_t rsi_bytes2R_to_uint16(const uint8_t *dBuf);
uint32_t rsi_bytes4R_to_uint32(const uint8_t *dBuf);
uint8_t *rsi_ascii_dev_address_to_6bytes_rev(uint8_t *hex_addr, int8_t *ascii_mac_address);
uint8_t *rsi_6byte_dev_address_to_ascii(uint8_t *ascii_mac_address, const uint8_t *hex_addr);
uint8_t convert_lower_case_to_upper_case(uint8_t lwrcase);
void string2array(uint8_t *dst, const uint8_t *src, uint32_t length);
int32_t rsi_atoi(const int8_t *str);
void rsi_ascii_dot_address_to_4bytes(uint8_t *hexAddr, int8_t *asciiDotAddress);
void rsi_ascii_mac_address_to_6bytes(uint8_t *hexAddr, int8_t *asciiMacAddress);
uint64_t ip_to_reverse_hex(const char *ip);

int8_t rsi_ascii_hex2num(int8_t ascii_hex_in);
int8_t rsi_char_hex2dec(int8_t *cBuf);
int8_t hex_to_ascii(uint8_t hex_num);
uint8_t *rsi_itoa(uint32_t val, uint8_t *str);
int8_t asciihex_2_num(int8_t ascii_hex_in);
int8_t rsi_charhex_2_dec(int8_t *cBuf);
uint32_t rsi_ntohl(uint32_t a);
#endif
