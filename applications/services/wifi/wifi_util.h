#pragma once
#include <stddef.h>
#include <stdint.h>
#include "wifi_common.h"

/**
 * @brief Format binary representation of a BSSID into a string
 *
 *
 * @param[in] bssid - binary BSSID (see struct WifiInfo)
 * @param[in] str_out output string buffer
 * @param[in] str_out_size maximum number of bytes str_out can hold
 */
void wifi_format_bssid(const uint8_t* bssid, char* str_out, size_t str_out_size);

/**
 * @brief Format binary representation of an IPv4 address into a string
 *
 *
 * @param[in] ipv4 IPv4 address
 * @param[in] str_out output string buffer
 * @param[in] str_out_size maximum number of bytes str_out can hold
 */
void wifi_format_ipv4(const WifiIpv4* ipv4, char* str_out, size_t str_out_size);

/**
 * @brief Format binary representation of an IPv6 address into a string
 *
 *
 * @param[in] ipv6 IPv6 address
 * @param[in] str_out output string buffer
 * @param[in] str_out_size maximum number of bytes str_out can hold
 */
void wifi_format_ipv6(const WifiIpv6* ipv6, char* str_out, size_t str_out_size);

/**
 * @brief Check if an IPv6 address is a specified one (not all zeroes).
 */
bool wifi_ipv6_is_specified(const WifiIpv6* ipv6);
