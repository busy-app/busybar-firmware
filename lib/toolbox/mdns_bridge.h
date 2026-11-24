/**
 * @file mdns_bridge.h
 * @brief Translation of mDNS packets between IPv4 and IPv6 Ethernet frames.
 *
 * Some Wi-Fi access points do not forward IPv6 multicast addressed to ff02::fb between
 * wireless stations, which hides services that are advertised over IPv6 only (e.g. Matter).
 * This module rewrites IPv4 mDNS queries into equivalent IPv6 queries that can be fed into
 * an IPv6-only responder, and rewrites that responder's IPv6 multicast responses into IPv4
 * multicast responses. The DNS payload of a response is carried unchanged, so the address
 * records stay exactly as the responder produced them.
 *
 * All functions operate on complete Ethernet II frames without FCS, keep no state and are
 * reentrant. Input frames are treated as untrusted and are fully validated.
 *
 * Limitations:
 * - IPv4 fragments and IPv6 extension headers are not supported, such frames are ignored.
 * - Translated queries request a multicast response (source port 5353, unicast-response bit
 *   cleared in every question). Legacy unicast queriers (RFC 6762, section 6.7) therefore
 *   receive no direct answer.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MDNS_BRIDGE_IPV4_ADDR_SIZE (4U) /**< Size of an IPv4 address in bytes. */

/** IPv4 address. */
typedef struct {
    uint8_t octets[MDNS_BRIDGE_IPV4_ADDR_SIZE]; /**< Address octets in network byte order. */
} MdnsBridgeIpv4Address;

/**
 * @brief Translate an IPv4 mDNS query frame into an IPv6 mDNS query frame.
 *
 * The frame is accepted only if it is a non-fragmented UDP datagram addressed to
 * 224.0.0.251:5353 with valid IPv4 header and UDP checksums, carrying a DNS query with a
 * well-formed question section. The resulting frame is sent from port 5353 to ff02::fb:5353,
 * has a link-local source address derived from the sender's MAC address (modified EUI-64),
 * and has the unicast-response bit cleared in every question.
 *
 * @param[in] frame pointer to the received Ethernet frame
 * @param[in] frame_size size of the frame in bytes, trailing padding is allowed
 * @param[out] out pointer to the buffer for the translated frame (can be @c NULL to only
 *                 compute the required size), must not overlap with @p frame
 * @param[in] out_size size of the @p out buffer in bytes, ignored if @p out is @c NULL
 * @returns size of the translated frame in bytes, or 0 if @p frame is not a valid IPv4 mDNS
 *          query or the translated frame does not fit into @p out
 */
size_t mdns_bridge_ipv4_query_to_ipv6(
    const uint8_t* frame,
    size_t frame_size,
    uint8_t* out,
    size_t out_size);

/**
 * @brief Translate an IPv6 mDNS multicast response frame into an IPv4 mDNS response frame.
 *
 * The frame is accepted only if it is a UDP datagram sent from port 5353 to ff02::fb:5353
 * without extension headers, carrying a DNS response. The resulting frame is sent from
 * @p source_addr to 224.0.0.251:5353 as an atomic datagram (Don't Fragment set, ID 0) with
 * TTL 255. The Ethernet source address and the DNS payload are copied unchanged.
 *
 * @param[in] frame pointer to the Ethernet frame to be translated
 * @param[in] frame_size size of the frame in bytes, trailing padding is allowed
 * @param[in] source_addr pointer to the IPv4 source address to be used
 * @param[out] out pointer to the buffer for the translated frame (can be @c NULL to only
 *                 compute the required size), must not overlap with @p frame
 * @param[in] out_size size of the @p out buffer in bytes, ignored if @p out is @c NULL
 * @returns size of the translated frame in bytes, or 0 if @p frame is not a valid IPv6 mDNS
 *          multicast response or the translated frame does not fit into @p out
 */
size_t mdns_bridge_ipv6_response_to_ipv4(
    const uint8_t* frame,
    size_t frame_size,
    const MdnsBridgeIpv4Address* source_addr,
    uint8_t* out,
    size_t out_size);

/**
 * @brief Get the source address of an Ethernet frame carrying an IPv4 packet.
 *
 * @param[in] frame pointer to the Ethernet frame
 * @param[in] frame_size size of the frame in bytes
 * @param[out] out_addr pointer to the address to be filled, left untouched on failure
 * @returns @c true if @p frame carries an IPv4 packet with a specified (non-zero) source
 *          address, @c false otherwise
 */
bool mdns_bridge_get_ipv4_source(
    const uint8_t* frame,
    size_t frame_size,
    MdnsBridgeIpv4Address* out_addr);

#ifdef __cplusplus
}
#endif
