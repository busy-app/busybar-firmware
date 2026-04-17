/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

#include "sl_status.h"
#include "sl_ip_types.h"
#include "sl_ieee802_types.h"
#include "sl_wifi_types.h"

typedef struct {
  uint8_t common_log_level;
  uint8_t cm_pm_log_level;
  uint8_t wlan_lmac_log_level;
  uint8_t wlan_umac_log_level;
  uint8_t wlan_netstack_log_level;
  uint8_t bt_ble_ctrl_log_level;
  uint8_t bt_ble_stack_log_level;
  uint8_t reserved;
} sli_nwp_log_level_t;

typedef struct {
  uint8_t logging_enable;
  uint8_t tsf_granularity;
  uint16_t reserved_1;
  sli_nwp_log_level_t component_log_level; // log levels for the 7 firmware components and 1 byte reserved
  uint32_t reserved_2;
  uint32_t log_buffer_size;
} sli_nwp_log_t;

typedef struct {
  uint8_t log_config_level;
} sli_nwp_log_config_t;

/***************************************************************************/ /**
 * @brief 
 *   Convert a character string into a sl_ipv4_address_t
 * @param line  
 *   Argument string that is expected to be like 192.168.0.1
 * @param ip    
 *   Pointer to sl_ipv4_address_t.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t convert_string_to_sl_ipv4_address(char *line, sl_ipv4_address_t *ip);

/***************************************************************************/ /**
* @brief
*   Convert IPv6 binary address into presentation (printable) format
* @param[in] input
*   A pointer to the buffer containing the binary IPV6 address
* @param[in] dst
*   A pointer to the buffer where the resulting string will be stored
* @param[in] size
*   The size of the destination buffer in bytes
* @return
*   A pointer to a resulting string containing human readable representation of IPV6 address.
******************************************************************************/
char *sl_inet_ntop6(const unsigned char *input, char *dst, uint32_t size);

/***************************************************************************/ /**
 * @brief
 *   Convert a character string into a [sl_mac_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-types#sl-mac-address-t)
 * @param line
 *   Argument string that is expected to be like 00:11:22:33:44:55
 * @param mac
 *   Pointer to [sl_mac_address_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-types#sl-mac-address-t).
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t convert_string_to_mac_address(const char *line, sl_mac_address_t *mac);

void print_sl_ip_address(const sl_ip_address_t *sl_ip_address);
void print_sl_ipv4_address(const sl_ipv4_address_t *ip_address);
void print_sl_ipv6_address(const sl_ipv6_address_t *ip_address);
void print_mac_address(const sl_mac_address_t *mac_address);
void sli_convert_uint32_to_bytestream(uint16_t data, uint8_t *buffer);
void sli_little_to_big_endian(const unsigned int *source, unsigned char *result, unsigned int length);
void sli_big_to_little_endian(const unsigned int *source, unsigned char *result, unsigned int length);
int sl_inet_pton6(const char *src, const char *src_endp, unsigned char *dst, unsigned int *ptr_result);
void sli_reverse_digits(unsigned char *xx, int no_digits);
sl_status_t sli_nwp_log_configure(const sli_nwp_log_config_t *config);
void sli_handle_nwp_log_packet(const uint8_t *data, uint16_t length);

/***************************************************************************/ /**
 * @brief Print 802.11 packet
 *
 * @param[in] packet - pointer to start of MAC header
 * @param[in] packet_length - total packet length (MAC header + payload)
 * @param[in] max_payload_length - maximum number of payload bytes to print
 ******************************************************************************/
void print_80211_packet(const uint8_t *packet, uint32_t packet_length, uint16_t max_payload_length);
