/***************************************************************************//**
 * @file sl_wisun_br_dhcpv6_server.c
 * @brief Components that implements a DHCPv6 server for Wi-SUN Border Router
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_assert.h"
#include <cmsis_os2.h>
#include <string.h>
#include "sl_memory_manager.h"
#include "sl_wisun_trace_api.h"
#include "sl_wisun_ip6string.h"
#include "sl_wisun_types.h"
#include "socket/socket.h"
#include "common/ns_list.h"
#include "common/endian.h"
#include "sl_wisun_common.h"
#include "sl_wisun_br_dhcpv6_server.h"
#include "common/pktbuf.h"

static int dhcpv6_server_socket = -1;
static uint8_t server_prefix[8], server_DUID[8];
static uint32_t valid_lifetime, preferred_lifetime;
dhcpv6_vendor_data_list_t vendorDataList;

// Messages types (RFC3315, section 5.3)
#define DHCPV6_MSG_SOLICIT      1
#define DHCPV6_MSG_ADVERT       2  /* Unused */
#define DHCPV6_MSG_REQUEST      3  /* Unused */
#define DHCPV6_MSG_CONFIRM      4  /* Unused */
#define DHCPV6_MSG_RENEW        5  /* Unused */
#define DHCPV6_MSG_REBIND       6  /* Unused */
#define DHCPV6_MSG_REPLY        7
#define DHCPV6_MSG_RELEASE      8  /* Unused */
#define DHCPV6_MSG_DECLINE      9  /* Unused */
#define DHCPV6_MSG_RECONFIGURE  10 /* Unused */
#define DHCPV6_MSG_INFO_REQUEST 11 /* Unused */
#define DHCPV6_MSG_RELAY_FWD    12
#define DHCPV6_MSG_RELAY_REPLY  13

// Options IDs (RFC3315, section 24.3)
#define DHCPV6_OPT_CLIENT_ID                  0x0001
#define DHCPV6_OPT_SERVER_ID                  0x0002
#define DHCPV6_OPT_IA_NA                      0x0003
#define DHCPV6_OPT_IA_TA                      0x0004 /* Unused */
#define DHCPV6_OPT_IA_ADDRESS                 0x0005
#define DHCPV6_OPT_ORO                        0x0006 /* Unused */
#define DHCPV6_OPT_PREFERENCE                 0x0007 /* Unused */
#define DHCPV6_OPT_ELAPSED_TIME               0x0008
#define DHCPV6_OPT_RELAY                      0x0009
#define DHCPV6_OPT_RESERVED1                  0x000a /* Unused */
#define DHCPV6_OPT_AUTH                       0x000b /* Unused */
#define DHCPV6_OPT_UNICAST                    0x000c /* Unused */
#define DHCPV6_OPT_STATUS_CODE                0x000d
#define DHCPV6_OPT_RAPID_COMMIT               0x000e
#define DHCPV6_OPT_USER_CLASS                 0x000f /* Unused */
#define DHCPV6_OPT_VENDOR_CLASS               0x0010 /* Unused */
#define DHCPV6_OPT_VENDOR_SPECIFIC            0x0011
#define DHCPV6_OPT_INTERFACE_ID               0x0012
#define DHCPV6_OPT_RECONF_MSG                 0x0013 /* Unused */
#define DHCPV6_OPT_RECONF_ACCEPT              0x0014 /* Unused */

#define DHCPV6_DUID_TYPE_LINK_LAYER_PLUS_TIME 0x0001 /* Unused */
#define DHCPV6_DUID_TYPE_EN                   0x0002 /* Unused */
#define DHCPV6_DUID_TYPE_LINK_LAYER           0x0003
#define DHCPV6_DUID_TYPE_UUID                 0x0004 /* Unused */

#define DHCPV6_DUID_HW_TYPE_EUI48             0x0001 /* Unused */
#define DHCPV6_DUID_HW_TYPE_IEEE802           0x0006
#define DHCPV6_DUID_HW_TYPE_EUI64             0x001b

static int dhcp_handle_request(uint8_t *req, int len, struct pktbuf *reply);
static int dhcp_handle_request_fwd(uint8_t *req, int len, struct pktbuf *reply);


dhcpv6_vendor_data_t *libdhcpv6_vendor_data_allocate(uint32_t enterprise_number)
{
    dhcpv6_vendor_data_t *entry = NULL;
    ns_list_foreach(dhcpv6_vendor_data_t, cur, &vendorDataList) {
        if (cur->enterprise_number == enterprise_number) {
            sl_free(cur->vendor_data);
            cur->vendor_data = NULL;
            cur->vendor_data_length = 0;
            return cur;
        }
    }
    entry = sl_malloc(sizeof(dhcpv6_vendor_data_t));
    if (!entry) {
        return NULL;
    }
    ns_list_add_to_end(&vendorDataList, entry);
    entry->enterprise_number = enterprise_number;
    entry->vendor_data = NULL;
    entry->vendor_data_length = 0;
    return entry;
}

static int dhcp_get_option(uint8_t *data, int len, uint16_t option, uint8_t **option_payload)
{
  uint16_t opt_type, opt_len;
  while (len >= 4) {
    opt_type = read_be16(data);
    opt_len = read_be16(data + 2);
    if (opt_type == option) {
      *option_payload = data + 4;
      return opt_len;
    }
    data += 4 + opt_len;
    len -= 4 + opt_len;
  }
  return -1;
}

static int dhcp_get_client_hwaddr(uint8_t *req, size_t req_len, uint8_t *hwaddr)
{
  uint8_t *opt;
  int opt_length;
  uint16_t duid_type, ll_type;

  opt_length = dhcp_get_option(req, req_len, DHCPV6_OPT_CLIENT_ID, &opt);
  if (opt_length != 12) {
    sl_wisun_trace_error("dhcp: missing or malformed client ID option");
    return -1;
  }
  duid_type = read_be16(opt);
  ll_type = read_be16(opt + 2);
  if (duid_type != DHCPV6_DUID_TYPE_LINK_LAYER ||
    (ll_type != DHCPV6_DUID_HW_TYPE_EUI64 && ll_type != DHCPV6_DUID_HW_TYPE_IEEE802)) {
    sl_wisun_trace_error("dhcp: unsupported client ID option %"PRIu16"", ll_type);
    return -1;
  }
  memcpy(hwaddr, opt + 4, 8);
  return ll_type;
}

static uint32_t dhcp_get_identity_association_id(uint8_t *req, size_t req_len)
{
  uint8_t *opt;
  int opt_length;
  uint32_t ia_id;

  opt_length = dhcp_get_option(req, req_len, DHCPV6_OPT_IA_NA, &opt);
  if (opt_length < 4) {
    sl_wisun_trace_error("dhcp: missing or malformed IA_NA option");
    ia_id = UINT32_MAX;
  }
  ia_id = read_be32(opt);
  return ia_id;
}

static int dhcp_check_rapid_commit(uint8_t *req, size_t req_len)
{
  uint8_t *opt;
  int opt_length;

  opt_length = dhcp_get_option(req, req_len, DHCPV6_OPT_RAPID_COMMIT, &opt);
  if (opt_length < -1) {
    sl_wisun_trace_error("dhcp: missing rapid commit option");
  }
  return opt_length;
}

static int dhcp_check_status_code(uint8_t *req, int req_len)
{
  uint8_t *opt;
  int opt_length;
  uint16_t status;

  opt_length = dhcp_get_option(req, req_len, DHCPV6_OPT_STATUS_CODE, &opt);
  if (opt_length < 1) {
    return 0;
  }
  status = read_be16(opt);
  if (status) {
    sl_wisun_trace_error("dhcp: status code: %d", status);
    return -1;
  }
  return 0;
}

static int dhcp_check_elapsed_time(uint8_t *req, int req_len)
{
  uint8_t *opt;
  int opt_length;

  opt_length = dhcp_get_option(req, req_len, DHCPV6_OPT_ELAPSED_TIME, &opt);
  if (opt_length < 0) {
    // Elapsed Time option is mandatory
    sl_wisun_trace_error("dhcp: missing elapsed time option");
  }
  return 0;
}

static void dhcp_fill_server_id(struct pktbuf *reply)
{
  pktbuf_push_tail_be16(reply, DHCPV6_OPT_SERVER_ID);
  pktbuf_push_tail_be16(reply, 2 + 2 + 8);
  pktbuf_push_tail_be16(reply, DHCPV6_DUID_TYPE_LINK_LAYER);
  pktbuf_push_tail_be16(reply, DHCPV6_DUID_HW_TYPE_IEEE802);
  pktbuf_push_tail(reply, server_DUID, 8);
}

static void dhcp_fill_client_id(struct pktbuf *reply,
                                uint16_t hwaddr_type, const uint8_t *hwaddr)
{
  pktbuf_push_tail_be16(reply, DHCPV6_OPT_CLIENT_ID);
  pktbuf_push_tail_be16(reply, 2 + 2 + 8);
  pktbuf_push_tail_be16(reply, DHCPV6_DUID_TYPE_LINK_LAYER);
  pktbuf_push_tail_be16(reply, hwaddr_type);
  pktbuf_push_tail(reply, hwaddr, 8);
  }

static void dhcp_fill_rapid_commit(struct pktbuf *reply)
{
  pktbuf_push_tail_be16(reply, DHCPV6_OPT_RAPID_COMMIT);
  pktbuf_push_tail_be16(reply, 0);
}

static void dhcp_fill_vendor_data(struct pktbuf *reply)
{
  ns_list_foreach(dhcpv6_vendor_data_t, cur, &vendorDataList) {
    if (cur->vendor_data_length == 0) {
      continue;
    }
    pktbuf_push_tail_be16(reply, DHCPV6_OPT_VENDOR_SPECIFIC);
    pktbuf_push_tail_be16(reply, cur->vendor_data_length + 4);
    pktbuf_push_tail_be32(reply, cur->enterprise_number);
    pktbuf_push_tail(reply, cur->vendor_data, cur->vendor_data_length);
  }
}

static void dhcp_fill_identity_association(struct pktbuf *reply,
                                           const uint8_t *hwaddr, uint32_t ia_id)
{
  uint8_t ipv6[16];
  memcpy(ipv6, server_prefix, 8);
  memcpy(ipv6 + 8, hwaddr, 8);
  ipv6[8] ^= 0x02;
  pktbuf_push_tail_be16(reply, DHCPV6_OPT_IA_NA);
  pktbuf_push_tail_be16(reply, 4 + 4 + 4 + 2 + 2 + 16 + 4 + 4);
  pktbuf_push_tail_be32(reply, ia_id);
  pktbuf_push_tail_be32(reply, 0); // T1
  pktbuf_push_tail_be32(reply, 0); // T2
  pktbuf_push_tail_be16(reply, DHCPV6_OPT_IA_ADDRESS);
  pktbuf_push_tail_be16(reply, 16 + 4 + 4);
  pktbuf_push_tail(reply, ipv6, 16);
  pktbuf_push_tail_be32(reply, preferred_lifetime);
  pktbuf_push_tail_be32(reply, valid_lifetime);
}

static int dhcp_send_reply(struct sockaddr_in6 *dest,
                           struct pktbuf *reply)
{
  char dst_addr_str[MAX_IPV6_STRING_LEN_WITH_TRAILING_NULL];
  if (reply->err) {
    sl_wisun_trace_error("dhcp: reply buffer error");
    return -1;
  }
  int32_t retval = sendto(dhcpv6_server_socket, pktbuf_head(reply), reply->buf_len, 0, (struct sockaddr *)dest, sizeof(struct sockaddr_in6));
  if (retval <= 0) {
    sl_wisun_trace_error("dhcp: sendto failed %d", retval);
    return -1;
  }
  ip6tos(dest->sin6_addr.address, dst_addr_str);
  sl_wisun_trace_info("dhcp: sending reply to %s", dst_addr_str);
  return 0;
}

static int dhcp_handle_request_fwd(uint8_t *req, int len, struct pktbuf *reply)
{
  struct pktbuf buf = { 0 };
  uint8_t *opt_interface_id, *opt_relay;
  int32_t opt_interface_id_len, opt_relay_len;
  uint8_t linkaddr[16], peeraddr[16];
  uint8_t hopcount;

if (len < 33) {
    sl_wisun_trace_error("dhcp-fwd: fwd message too short");
    return -1;
  }
  memcpy(&hopcount, req, 1);
  memcpy(linkaddr, req + 1, 16);
  memcpy(peeraddr, req + 17, 16);
  req += 33;
  len -= 33;
  pktbuf_push_tail_u8(reply, DHCPV6_MSG_RELAY_REPLY);
  pktbuf_push_tail_u8(reply, hopcount);
  pktbuf_push_tail(reply, linkaddr, 16);
  pktbuf_push_tail(reply, peeraddr, 16);
  opt_interface_id_len = dhcp_get_option(req, len, DHCPV6_OPT_INTERFACE_ID,
                                         &opt_interface_id);
  if (opt_interface_id_len > 0) {
    pktbuf_push_tail_be16(reply, DHCPV6_OPT_INTERFACE_ID);
    pktbuf_push_tail_be16(reply, opt_interface_id_len);
    pktbuf_push_tail(reply, opt_interface_id, opt_interface_id_len);
  }
  opt_relay_len = dhcp_get_option(req, len, DHCPV6_OPT_RELAY,
                                  &opt_relay);
  if (opt_relay_len < 0) {
    sl_wisun_trace_error("dhcp-fwd: missing relay option");
    return -1;
  }
  pktbuf_init(&buf, NULL, 0);
  if (dhcp_handle_request(opt_relay, opt_relay_len, &buf) < 0) {
    pktbuf_free(&buf);
    return -1;
  }

  pktbuf_push_tail_be16(reply, DHCPV6_OPT_RELAY);
  pktbuf_push_tail_be16(reply, pktbuf_len(&buf));
  pktbuf_push_tail(reply, pktbuf_head(&buf), pktbuf_len(&buf));

  pktbuf_free(&buf);

  if (reply->err) {
    sl_wisun_trace_error("dhcp-fwd: reply buffer error");
    return -1;
  }

  return 0;
}

static int dhcp_handle_request(uint8_t *req, int len, struct pktbuf *reply)
{
  uint24_t transaction;
  uint8_t msg_type;
  uint32_t iaid;
  uint8_t hwaddr[8];
  int hwaddr_type;

  memcpy (&msg_type, req, 1);
  req += 1;
  len -= 1;
  if (msg_type == DHCPV6_MSG_RELAY_FWD) {
    return dhcp_handle_request_fwd(req, len, reply);
  }
  if (msg_type != DHCPV6_MSG_SOLICIT) {
    sl_wisun_trace_error("dhcp: unsupported message type %d", msg_type);
    return -1;
  }

  transaction = read_be24(req);
  req +=3;
  len -= 3;
  if (dhcp_check_status_code(req, len)) {
    return -1;
  }
  if (dhcp_check_rapid_commit(req, len)) {
    return -1;
  }
  if (dhcp_check_elapsed_time(req, len)) {
    return -1;
  }
  iaid = dhcp_get_identity_association_id(req, len);
  if (iaid == UINT32_MAX) {
    return -1;
  }
  hwaddr_type = dhcp_get_client_hwaddr(req, len, hwaddr);
  if (hwaddr_type < 0) {
    return -1;
  }

  pktbuf_push_tail_u8(reply, DHCPV6_MSG_REPLY);
  pktbuf_push_tail_be24(reply, transaction);
  dhcp_fill_server_id(reply);
  dhcp_fill_client_id(reply, hwaddr_type, hwaddr);
  dhcp_fill_vendor_data(reply);
  dhcp_fill_identity_association(reply, hwaddr, iaid);
  dhcp_fill_rapid_commit(reply);

  if (reply->err) {
    sl_wisun_trace_error("dhcp: reply buffer error");
    return -1;
  }

  return 0;
}

void sl_wisun_br_dhcpv6_server_on_recv(uint8_t *buffer, ssize_t length, in6_addr_t peer_address, in_port_t remote_port)
{
  char src_addr_str[MAX_IPV6_STRING_LEN_WITH_TRAILING_NULL];
  struct pktbuf buf = { 0 };

  sockaddr_in6_t src_addr = {
    .sin6_family = AF_INET6,
    .sin6_port = remote_port,
    .sin6_flowinfo = 0,
    .sin6_addr = IN6ADDR_ANY_INIT,
    .sin6_scope_id = 0,
  };
  pktbuf_init(&buf, NULL, 0);

  ip6tos(peer_address.address, src_addr_str);
  sl_wisun_trace_info("dhcp: received msg from %s", src_addr_str);
  memcpy (src_addr.sin6_addr.address, peer_address.address, 16);
  if (!dhcp_handle_request(buffer, length, &buf)) {
    dhcp_send_reply(&src_addr, &buf);
  }
  pktbuf_free(&buf);
}

sl_status_t sl_wisun_br_dhcpv6_set_vendor_data(uint32_t enterprise_number, uint16_t vendor_data_length, const uint8_t *vendor_data)
{
  dhcpv6_vendor_data_t *vendor_data_entry;

  if (vendor_data_length && !vendor_data) {
    sl_wisun_trace_error("dhcp: invalid vendor data pointer");
    return SL_STATUS_INVALID_PARAMETER;
  }

  vendor_data_entry = libdhcpv6_vendor_data_allocate(enterprise_number);

  if (!vendor_data_entry) {
    sl_wisun_trace_error("dhcp: OOM vendor data entry");
    return SL_STATUS_ALLOCATION_FAILED;
  }

  if (vendor_data_length) {
    vendor_data_entry->vendor_data = sl_malloc(vendor_data_length);
    if (!vendor_data_entry->vendor_data) {
      sl_wisun_trace_error("dhcp: OOM vendor data");
      return SL_STATUS_ALLOCATION_FAILED;
    }
    vendor_data_entry->vendor_data_length = vendor_data_length;
    memcpy(vendor_data_entry->vendor_data, vendor_data, vendor_data_entry->vendor_data_length);
  }
  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_dhcpv6_server_init(void)
{
  ns_list_init(&vendorDataList);
  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_dhcpv6_server_start(int socket, uint8_t prefix[8], uint8_t DUID[8], uint32_t dhcp_address_lifetime)
{
  dhcpv6_server_socket = socket;
  valid_lifetime = dhcp_address_lifetime;
  if (dhcp_address_lifetime == LIFETIME_INFINITE) {
    preferred_lifetime = LIFETIME_INFINITE;
  } else {
    preferred_lifetime = dhcp_address_lifetime / 2;
  }
  memcpy(server_prefix, prefix, 8);
  memcpy(server_DUID, DUID, 8);
  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_dhcpv6_server_stop(void) {
  dhcpv6_server_socket = -1;
  return SL_STATUS_OK;
}
