/***************************************************************************/ /**
 * @file  sl_utility.c
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

#include <stdbool.h>
#include <stdlib.h>
#include "sl_utility.h"
#include "sl_constants.h"
#include "sl_string.h"
#include "cmsis_compiler.h"
#include <string.h>
#include <stdio.h>
#include "sli_wifi_types.h"
#include "sli_wifi_constants.h"
#include "sli_wifi.h"

extern bool device_initialized;

#define NWP_LOGGING_ENABLE  1
#define NWP_TSF_GRANULARITY 10
#define NWP_MAX_LOG_BUFFER  1024

#ifdef SPRINTF_CHAR
#define SPRINTF(x) strlen(sprintf /**/ x)
#else
#define SPRINTF(x) ((size_t)sprintf x)
#endif

#define DIGIT_VAL(c)                             \
  (((c >= '0') && (c <= '9'))   ? (c - '0')      \
   : ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 10) \
   : ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 10) \
                                : -1)
typedef struct {
  int base;
  int len;
} data_t;

extern char *strtok_r(char *, const char *, char **);

void sli_convert_uint32_to_bytestream(uint16_t data, uint8_t *buffer)
{
  buffer[0] = (uint8_t)(data & 0xFF);
  buffer[1] = (uint8_t)((data >> 8) & 0xFF);
  buffer[2] = (uint8_t)((data >> 16) & 0xFF);
  buffer[3] = (uint8_t)((data >> 24) & 0xFF);
  return;
}

sl_status_t convert_string_to_sl_ipv4_address(char *line, sl_ipv4_address_t *ip)
{
  char *lasts       = NULL;
  const char *token = strtok_r(line, ".", &lasts);

  for (uint8_t i = 0; i < 4; i++, token = strtok_r(NULL, ".", &lasts)) {
    if (token == NULL) {
      return SL_STATUS_COMMAND_IS_INVALID;
    }
    ip->bytes[i] = (uint8_t)strtoul(token, 0, 0);
  }
  return SL_STATUS_OK;
}

void print_sl_ip_address(const sl_ip_address_t *sl_ip_address)
{
  if (sl_ip_address == NULL) {
    return;
  }

  if (sl_ip_address->type == SL_IPV4) {
    print_sl_ipv4_address(&sl_ip_address->ip.v4);
  } else if (sl_ip_address->type == SL_IPV6) {
    print_sl_ipv6_address(&sl_ip_address->ip.v6);
  }
}

void print_sl_ipv4_address(const sl_ipv4_address_t *ip_address)
{
  printf("%d.%d.%d.%d", ip_address->bytes[0], ip_address->bytes[1], ip_address->bytes[2], ip_address->bytes[3]);
}

void print_sl_ipv6_address(const sl_ipv6_address_t *ip_address)
{
  char temp_buffer[46] = { 0 };
  sl_inet_ntop6((const unsigned char *)(ip_address), (char *)temp_buffer, sizeof(temp_buffer));
  printf("%s\r\n", temp_buffer);
}

void print_mac_address(const sl_mac_address_t *mac_address)
{
  if (mac_address == NULL) {
    return;
  }
  printf("%2X:%2X:%2X:%2X:%2X:%2X",
         mac_address->octet[0],
         mac_address->octet[1],
         mac_address->octet[2],
         mac_address->octet[3],
         mac_address->octet[4],
         mac_address->octet[5]);
}

// Helper function to convert IPv6 bytes to words
static void sli_convert_ipv6_bytes_to_words(const unsigned char *src, unsigned int *words)
{
  for (int i = 0; i < SL_IPV6_ADDRESS_LENGTH; i += 2) {
    int temp     = src[i];
    words[i / 2] = (temp << 8) | src[i + 1];
  }
}

// Helper function to find longest run of zeros in IPv6 address
static data_t sli_find_longest_zero_run(const unsigned int *words)
{
  data_t best = { -1, 0 };
  data_t cur  = { -1, 0 };

  for (int i = 0; i < (SL_IPV6_ADDRESS_LENGTH / 2); i++) {
    if (words[i] == 0) {
      if (cur.base == -1) {
        cur.base = i;
        cur.len  = 1;
      } else {
        cur.len++;
      }
      continue;
    }

    // Non-zero word: finalize current run if exists
    if (cur.base != -1) {
      if (best.base == -1 || cur.len > best.len) {
        best = cur;
      }
      cur.base = -1;
    }
  }

  // Check final run
  if ((cur.base != -1) && (best.base == -1 || cur.len > best.len)) {
    best = cur;
  }

  // Ignore runs of length 1
  if (best.base != -1 && best.len < 2) {
    best.base = -1;
  }

  return best;
}

// Helper function to format IPv6 address with :: compression
static char *sli_format_ipv6_address(const unsigned int *words, data_t best, char *tmp)
{
  char *tp = tmp;

  for (int i = 0; i < (SL_IPV6_ADDRESS_LENGTH / 2); i++) {
    // Skip compressed zero run
    if (best.base != -1 && i >= best.base && i < (best.base + best.len)) {
      if (i == best.base) {
        *tp++ = ':';
      }
      continue;
    }

    // Add separator after first word
    if (i != 0) {
      *tp++ = ':';
    }

    tp += SPRINTF((tp, "%x", words[i]));
  }

  // Add trailing colon for trailing zero run
  if (best.base != -1 && (best.base + best.len) == (SL_IPV6_ADDRESS_LENGTH / 2)) {
    *tp++ = ':';
  }

  *tp++ = '\0';
  return tp;
}

char *sl_inet_ntop6(const unsigned char *input, char *dst, uint32_t size)
{
  char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
  unsigned int words[SL_IPV6_ADDRESS_LENGTH / 2];
  unsigned int ip_big_endian[4];

  // Convert to big endian
  sli_little_to_big_endian((const unsigned int *)input, (unsigned char *)ip_big_endian, SL_IPV6_ADDRESS_LENGTH);

  // Convert bytes to 16-bit words
  memset(words, '\0', sizeof words);
  sli_convert_ipv6_bytes_to_words((unsigned char *)ip_big_endian, words);

  // Find longest run of zeros for :: compression
  data_t best = sli_find_longest_zero_run(words);

  // Format the address
  const char *tp = sli_format_ipv6_address(words, best, tmp);

  // Check for overflow and copy result
  if ((uint32_t)(tp - tmp) > size) {
    printf("\r\n Error \r\n");
    return NULL;
  }

  return memcpy(dst, tmp, size);
}

static int sli_hex_digit_value(char ch)
{
  if ('0' <= ch && ch <= '9')
    return ch - '0';
  if ('a' <= ch && ch <= 'f')
    return ch - 'a' + 10;
  if ('A' <= ch && ch <= 'F')
    return ch - 'A' + 10;
  return -1;
}

void sli_little_to_big_endian(const unsigned int *source, unsigned char *result, unsigned int length)
{
  unsigned char *temp;
  unsigned int curr = 0;
  length /= 4;

  for (unsigned int i = 0; i < length; i++) {
    curr    = source[i];
    temp    = &result[i * 4];
    temp[3] = (curr & 0xFF);
    temp[2] = ((curr >> 8) & 0xFF);
    temp[1] = ((curr >> 16) & 0xFF);
    temp[0] = ((curr >> 24) & 0xFF);
  }
}

// Helper function to validate and process hex digit in IPv6 address
static int sli_process_ipv6_hex_digit(int digit, size_t xdigits_seen, unsigned int *val)
{
  if (xdigits_seen == 4) {
    return 0; // Too many hex digits
  }
  *val = (*val << 4) | digit;
  if (*val > 0xffff) {
    return 0; // Value exceeds 16-bit limit
  }
  return 1; // Success
}

// Helper function to handle double colon in IPv6 address
static int sli_handle_ipv6_double_colon(unsigned char **colonp, unsigned char *tp)
{
  if (*colonp != NULL) {
    return 0; // Multiple :: not allowed
  }
  *colonp = tp;
  return 1; // Success
}

// Helper function to store IPv6 segment value
static int sli_store_ipv6_segment(unsigned char **tp, const unsigned char *endp, unsigned int val)
{
  if (*tp + 2 > endp) {
    return 0; // Buffer overflow
  }
  *(*tp)++ = (unsigned char)(val >> 8) & 0xff;
  *(*tp)++ = (unsigned char)val & 0xff;
  return 1; // Success
}

// Helper function to expand :: in IPv6 address
static int sli_expand_ipv6_double_colon(unsigned char *colonp, unsigned char **tp, unsigned char *endp)
{
  if (*tp == endp) {
    return 0; // :: would expand to zero-width field
  }
  size_t n = *tp - colonp;
  memmove(endp - n, colonp, n);
  memset(colonp, 0, endp - n - colonp);
  *tp = endp;
  return 1; // Success
}

// Helper function to parse IPv6 address segments
static int sli_parse_ipv6_segments(const char **src,
                                   const char *src_endp,
                                   unsigned char **tp,
                                   const unsigned char *endp,
                                   unsigned char **colonp,
                                   size_t *xdigits_seen,
                                   unsigned int *val)
{
  while (*src < src_endp) {
    int ch    = *(*src)++;
    int digit = sli_hex_digit_value((char)ch);

    // Process hex digit - early return on error
    if (digit >= 0) {
      if (!sli_process_ipv6_hex_digit(digit, *xdigits_seen, val)) {
        return 0;
      }
      ++(*xdigits_seen);
      continue;
    }

    // Skip non-colon characters
    if (ch != ':') {
      continue;
    }

    // Process colon separator

    // Handle :: (double colon) - early return on error
    if (*xdigits_seen == 0) {
      if (!sli_handle_ipv6_double_colon(colonp, *tp)) {
        return 0;
      }
      continue;
    }

    // Validate and store current segment - early return on error
    if (*src == src_endp || !sli_store_ipv6_segment(tp, endp, *val)) {
      return 0;
    }

    *xdigits_seen = 0;
    *val          = 0;
  }
  return 1; // Success
}

void sli_big_to_little_endian(const unsigned int *source, unsigned char *result, unsigned int length)
{
  const unsigned char *src = (const unsigned char *)source;

  for (unsigned int i = 0; i < length; i += 4) {
    result[i]     = src[i + 3];
    result[i + 1] = src[i + 2];
    result[i + 2] = src[i + 1];
    result[i + 3] = src[i];
  }
}

int sl_inet_pton6(const char *src, const char *src_endp, unsigned char *dst, unsigned int *ptr_result)
{
  unsigned char tmp[SL_IPV6_ADDRESS_LENGTH];
  unsigned char *tp     = memset(tmp, '\0', SL_IPV6_ADDRESS_LENGTH);
  unsigned char *endp   = tp + SL_IPV6_ADDRESS_LENGTH;
  unsigned char *colonp = NULL;
  size_t xdigits_seen   = 0;
  unsigned int val      = 0;

  // Validate input and handle leading ::
  if (src == src_endp) {
    return 0;
  }
  if (*src == ':') {
    ++src;
    if (src == src_endp || *src != ':') {
      return 0;
    }
  }

  // Parse IPv6 address segments
  if (!sli_parse_ipv6_segments(&src, src_endp, &tp, endp, &colonp, &xdigits_seen, &val)) {
    return 0;
  }

  // Store final segment if present
  if (xdigits_seen > 0 && !sli_store_ipv6_segment(&tp, endp, val)) {
    return 0;
  }

  // Expand :: if present
  if (colonp != NULL && !sli_expand_ipv6_double_colon(colonp, &tp, endp)) {
    return 0;
  }

  // Validate complete address
  if (tp != endp) {
    return 0;
  }

  // Convert and store result
  memcpy(dst, tmp, SL_IPV6_ADDRESS_LENGTH);
  sli_little_to_big_endian((const unsigned int *)dst, (unsigned char *)ptr_result, SL_IPV6_ADDRESS_LENGTH);

  return 1;
}

sl_status_t convert_string_to_mac_address(const char *line, sl_mac_address_t *mac)
{
  // Verify we have the exact number of characters. Basic argument verification
  if (sl_strnlen(line, 18) != 17) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint8_t index = 0;
  while (index < 6) {
    // Read all the data and verify validity
    int char1 = DIGIT_VAL(line[0]);
    int char2 = DIGIT_VAL(line[1]);
    if (char1 == -1 || char2 == -1 || (line[2] != '\0' && line[2] != ':')) {
      return SL_STATUS_INVALID_PARAMETER;
    }

    // Store value
    mac->octet[index++] = (uint8_t)((uint8_t)char1 << 4) + (uint8_t)char2;
    line += 3;
  }

  return SL_STATUS_OK;
}

void sli_reverse_digits(unsigned char *xx, int no_digits)
{
  uint8_t temp;
  for (int count = 0; count < (no_digits / 2); count++) {
    temp                      = xx[count];
    xx[count]                 = xx[no_digits - count - 1];
    xx[no_digits - count - 1] = temp;
  }
}

__WEAK void sl_debug_log(const char *format, ...)
{
  UNUSED_PARAMETER(format);
}

void sl_redirect_log(const char *format, ...)
{
  UNUSED_PARAMETER(format);
}

sl_status_t sli_nwp_log_configure(const sli_nwp_log_config_t *config)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (config == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (config->log_config_level > 4) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  sli_nwp_log_t log_config = {
    .logging_enable = NWP_LOGGING_ENABLE,
    .tsf_granularity = NWP_TSF_GRANULARITY,
    .reserved_1 = 0,
    .component_log_level = {
      .common_log_level = config->log_config_level,
      .cm_pm_log_level = config->log_config_level,
      .wlan_lmac_log_level = config->log_config_level,
      .wlan_umac_log_level = config->log_config_level,
      .wlan_netstack_log_level = config->log_config_level,
      .bt_ble_ctrl_log_level = config->log_config_level,
      .bt_ble_stack_log_level = config->log_config_level,
    },
    .reserved_2 = 0,
    .log_buffer_size = NWP_MAX_LOG_BUFFER
  };

  if (config->log_config_level == 0) {
    log_config.logging_enable = 0;
  }

  status = sli_wifi_send_command(SLI_COMMON_REQ_NWP_LOGGING,
                                 SLI_WIFI_COMMON_CMD,
                                 &log_config,
                                 sizeof(sli_nwp_log_t),
                                 SLI_WIFI_RETURN_IMMEDIATELY,
                                 NULL,
                                 NULL);
  return status;
}

// Weak implementation of the function to handle log packets received from the NWP.
//The user can override this function to handle the log packets as needed. But it should be non-blocking.
__WEAK void sli_handle_nwp_log_packet(const uint8_t *data, uint16_t length)
{
  SL_DEBUG_LOG(
    "\r\n&^$ "); // This print statement is required for the decoder script to identify it as a firmware print.
  for (int i = 0; i < length; i++) {
    SL_DEBUG_LOG("%02x ", data[i]);
  }
}