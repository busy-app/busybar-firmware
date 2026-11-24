#include "mdns_bridge.h"

#include <string.h>

#include <core/check.h>

#define MDNS_BRIDGE_PORT         (5353U)
#define MDNS_BRIDGE_IP_PROTO_UDP (17U)
#define MDNS_BRIDGE_HOP_LIMIT    (255U)

#define MDNS_BRIDGE_ETH_MAC_SIZE    (6U)
#define MDNS_BRIDGE_ETH_DST_OFFSET  (0U)
#define MDNS_BRIDGE_ETH_SRC_OFFSET  (6U)
#define MDNS_BRIDGE_ETH_TYPE_OFFSET (12U)
#define MDNS_BRIDGE_ETH_HEADER_SIZE (14U)
#define MDNS_BRIDGE_ETH_TYPE_IPV4   (0x0800U)
#define MDNS_BRIDGE_ETH_TYPE_IPV6   (0x86DDU)

#define MDNS_BRIDGE_IP_VERSION_SHIFT (4U)

#define MDNS_BRIDGE_IPV4_VERSION_IHL_OFFSET (0U)
#define MDNS_BRIDGE_IPV4_TOTAL_SIZE_OFFSET  (2U)
#define MDNS_BRIDGE_IPV4_FRAGMENT_OFFSET    (6U)
#define MDNS_BRIDGE_IPV4_TTL_OFFSET         (8U)
#define MDNS_BRIDGE_IPV4_PROTOCOL_OFFSET    (9U)
#define MDNS_BRIDGE_IPV4_CHECKSUM_OFFSET    (10U)
#define MDNS_BRIDGE_IPV4_SRC_OFFSET         (12U)
#define MDNS_BRIDGE_IPV4_DST_OFFSET         (16U)
#define MDNS_BRIDGE_IPV4_HEADER_SIZE        (20U)
#define MDNS_BRIDGE_IPV4_VERSION            (4U)
#define MDNS_BRIDGE_IPV4_IHL_MASK           (0x0FU)
#define MDNS_BRIDGE_IPV4_IHL_UNIT_SIZE      (4U)
#define MDNS_BRIDGE_IPV4_FLAG_DF            (0x4000U)
#define MDNS_BRIDGE_IPV4_FRAGMENT_MASK      (0x3FFFU) // More Fragments flag and fragment offset

#define MDNS_BRIDGE_IPV6_PAYLOAD_SIZE_OFFSET (4U)
#define MDNS_BRIDGE_IPV6_NEXT_HEADER_OFFSET  (6U)
#define MDNS_BRIDGE_IPV6_HOP_LIMIT_OFFSET    (7U)
#define MDNS_BRIDGE_IPV6_SRC_OFFSET          (8U)
#define MDNS_BRIDGE_IPV6_DST_OFFSET          (24U)
#define MDNS_BRIDGE_IPV6_HEADER_SIZE         (40U)
#define MDNS_BRIDGE_IPV6_ADDR_SIZE           (16U)
#define MDNS_BRIDGE_IPV6_VERSION             (6U)
#define MDNS_BRIDGE_IPV6_PREFIX_SIZE         (8U)
#define MDNS_BRIDGE_IPV6_EUI64_UL_BIT        (0x02U)

#define MDNS_BRIDGE_UDP_SRC_PORT_OFFSET (0U)
#define MDNS_BRIDGE_UDP_DST_PORT_OFFSET (2U)
#define MDNS_BRIDGE_UDP_SIZE_OFFSET     (4U)
#define MDNS_BRIDGE_UDP_CHECKSUM_OFFSET (6U)
#define MDNS_BRIDGE_UDP_HEADER_SIZE     (8U)
#define MDNS_BRIDGE_UDP_CHECKSUM_NONE   (0x0000U)
#define MDNS_BRIDGE_UDP_CHECKSUM_ZERO   (0xFFFFU) // RFC 768: a computed zero is sent as all ones

#define MDNS_BRIDGE_DNS_FLAGS_OFFSET       (2U)
#define MDNS_BRIDGE_DNS_QDCOUNT_OFFSET     (4U)
#define MDNS_BRIDGE_DNS_HEADER_SIZE        (12U)
#define MDNS_BRIDGE_DNS_FLAG_RESPONSE      (0x80U)
#define MDNS_BRIDGE_DNS_QTYPE_SIZE         (2U)
#define MDNS_BRIDGE_DNS_QCLASS_SIZE        (2U)
#define MDNS_BRIDGE_DNS_QCLASS_QU_BIT      (0x80U)
#define MDNS_BRIDGE_DNS_LABEL_TYPE_MASK    (0xC0U)
#define MDNS_BRIDGE_DNS_LABEL_POINTER      (0xC0U)
#define MDNS_BRIDGE_DNS_LABEL_POINTER_SIZE (2U)

#define MDNS_BRIDGE_IPV4_MIN_FRAME_SIZE                                                         \
    (MDNS_BRIDGE_ETH_HEADER_SIZE + MDNS_BRIDGE_IPV4_HEADER_SIZE + MDNS_BRIDGE_UDP_HEADER_SIZE + \
     MDNS_BRIDGE_DNS_HEADER_SIZE)
#define MDNS_BRIDGE_IPV6_MIN_FRAME_SIZE                                                         \
    (MDNS_BRIDGE_ETH_HEADER_SIZE + MDNS_BRIDGE_IPV6_HEADER_SIZE + MDNS_BRIDGE_UDP_HEADER_SIZE + \
     MDNS_BRIDGE_DNS_HEADER_SIZE)

typedef struct {
    const uint8_t* src_mac;
    const uint8_t* dns;
    size_t dns_size;
} MdnsBridgeDnsPacket;

static const uint8_t mdns_bridge_ipv4_group_addr[MDNS_BRIDGE_IPV4_ADDR_SIZE] = {224, 0, 0, 251};

static const uint8_t mdns_bridge_ipv6_group_addr[MDNS_BRIDGE_IPV6_ADDR_SIZE] =
    {0xFF, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFB};

static const uint8_t mdns_bridge_ipv6_link_local_prefix[MDNS_BRIDGE_IPV6_PREFIX_SIZE] =
    {0xFE, 0x80, 0, 0, 0, 0, 0, 0};

static const uint8_t mdns_bridge_ipv4_group_mac[MDNS_BRIDGE_ETH_MAC_SIZE] =
    {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB};

static const uint8_t mdns_bridge_ipv6_group_mac[MDNS_BRIDGE_ETH_MAC_SIZE] =
    {0x33, 0x33, 0x00, 0x00, 0x00, 0xFB};

static uint16_t mdns_bridge_read_u16(const uint8_t* data) {
    return (uint16_t)((data[0] << 8) | data[1]);
}

static void mdns_bridge_write_u16(uint8_t* data, uint16_t value) {
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

static uint32_t mdns_bridge_checksum_add(uint32_t sum, const uint8_t* data, size_t data_size) {
    for(size_t i = 0; i + 1 < data_size; i += 2) {
        sum += mdns_bridge_read_u16(&data[i]);
    }

    if(data_size % 2) {
        sum += (uint32_t)data[data_size - 1] << 8;
    }

    return sum;
}

static uint16_t mdns_bridge_checksum_finish(uint32_t sum) {
    while(sum >> 16) {
        sum = (sum & 0xFFFFU) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

static uint16_t mdns_bridge_udp_checksum(
    const uint8_t* src_addr,
    const uint8_t* dst_addr,
    size_t addr_size,
    const uint8_t* udp,
    size_t udp_size) {
    uint32_t sum = 0;
    sum = mdns_bridge_checksum_add(sum, src_addr, addr_size);
    sum = mdns_bridge_checksum_add(sum, dst_addr, addr_size);
    sum += MDNS_BRIDGE_IP_PROTO_UDP;
    sum += (uint32_t)udp_size;
    sum = mdns_bridge_checksum_add(sum, udp, udp_size);

    const uint16_t checksum = mdns_bridge_checksum_finish(sum);
    return checksum ? checksum : MDNS_BRIDGE_UDP_CHECKSUM_ZERO;
}

static bool mdns_bridge_skip_name(const uint8_t* dns, size_t dns_size, size_t* offset) {
    bool is_valid = false;
    size_t position = *offset;

    while(position < dns_size) {
        const uint8_t label_size = dns[position];
        const uint8_t label_type = label_size & MDNS_BRIDGE_DNS_LABEL_TYPE_MASK;

        if(label_size == 0) {
            *offset = position + 1;
            is_valid = true;
            break;
        } else if(label_type == MDNS_BRIDGE_DNS_LABEL_POINTER) {
            *offset = position + MDNS_BRIDGE_DNS_LABEL_POINTER_SIZE;
            is_valid = (*offset <= dns_size);
            break;
        } else if(label_type != 0) {
            break;
        }

        position += label_size + 1U;
    }

    return is_valid;
}

static bool
    mdns_bridge_clear_unicast_response_bits(const uint8_t* dns, size_t dns_size, uint8_t* out_dns) {
    const uint16_t questions_count = mdns_bridge_read_u16(&dns[MDNS_BRIDGE_DNS_QDCOUNT_OFFSET]);

    size_t offset = MDNS_BRIDGE_DNS_HEADER_SIZE;
    bool is_valid = true;

    for(uint16_t i = 0; i < questions_count; ++i) {
        if(!mdns_bridge_skip_name(dns, dns_size, &offset) ||
           (offset + MDNS_BRIDGE_DNS_QTYPE_SIZE + MDNS_BRIDGE_DNS_QCLASS_SIZE > dns_size)) {
            is_valid = false;
            break;
        }

        if(out_dns) {
            out_dns[offset + MDNS_BRIDGE_DNS_QTYPE_SIZE] &=
                (uint8_t)~MDNS_BRIDGE_DNS_QCLASS_QU_BIT;
        }

        offset += MDNS_BRIDGE_DNS_QTYPE_SIZE + MDNS_BRIDGE_DNS_QCLASS_SIZE;
    }

    return is_valid;
}

static bool mdns_bridge_is_ipv4_checksum_valid(const uint8_t* ip, size_t ip_header_size) {
    return mdns_bridge_checksum_finish(mdns_bridge_checksum_add(0, ip, ip_header_size)) == 0;
}

static bool
    mdns_bridge_is_udp_checksum_valid(const uint8_t* ip, const uint8_t* udp, size_t udp_size) {
    const uint16_t checksum = mdns_bridge_read_u16(&udp[MDNS_BRIDGE_UDP_CHECKSUM_OFFSET]);
    bool is_valid = (checksum == MDNS_BRIDGE_UDP_CHECKSUM_NONE);

    if(!is_valid) {
        uint32_t sum = 0;
        sum = mdns_bridge_checksum_add(
            sum, &ip[MDNS_BRIDGE_IPV4_SRC_OFFSET], MDNS_BRIDGE_IPV4_ADDR_SIZE * 2);
        sum += MDNS_BRIDGE_IP_PROTO_UDP;
        sum += (uint32_t)udp_size;
        sum = mdns_bridge_checksum_add(sum, udp, udp_size);
        is_valid = (mdns_bridge_checksum_finish(sum) == 0);
    }

    return is_valid;
}

static bool mdns_bridge_parse_ipv4_query(
    const uint8_t* frame,
    size_t frame_size,
    MdnsBridgeDnsPacket* out_packet) {
    bool is_valid = false;

    do {
        if(frame_size < MDNS_BRIDGE_IPV4_MIN_FRAME_SIZE) {
            break;
        }
        if(mdns_bridge_read_u16(&frame[MDNS_BRIDGE_ETH_TYPE_OFFSET]) !=
           MDNS_BRIDGE_ETH_TYPE_IPV4) {
            break;
        }

        const uint8_t* ip = &frame[MDNS_BRIDGE_ETH_HEADER_SIZE];
        const uint8_t version_ihl = ip[MDNS_BRIDGE_IPV4_VERSION_IHL_OFFSET];
        const size_t ip_header_size =
            (version_ihl & MDNS_BRIDGE_IPV4_IHL_MASK) * MDNS_BRIDGE_IPV4_IHL_UNIT_SIZE;
        const size_t ip_total_size = mdns_bridge_read_u16(&ip[MDNS_BRIDGE_IPV4_TOTAL_SIZE_OFFSET]);

        if((version_ihl >> MDNS_BRIDGE_IP_VERSION_SHIFT) != MDNS_BRIDGE_IPV4_VERSION) {
            break;
        }
        if(ip_header_size < MDNS_BRIDGE_IPV4_HEADER_SIZE) {
            break;
        }
        if(ip_total_size > frame_size - MDNS_BRIDGE_ETH_HEADER_SIZE) {
            break;
        }
        if(ip_total_size <
           ip_header_size + MDNS_BRIDGE_UDP_HEADER_SIZE + MDNS_BRIDGE_DNS_HEADER_SIZE) {
            break;
        }
        if(mdns_bridge_read_u16(&ip[MDNS_BRIDGE_IPV4_FRAGMENT_OFFSET]) &
           MDNS_BRIDGE_IPV4_FRAGMENT_MASK) {
            break;
        }
        if(ip[MDNS_BRIDGE_IPV4_PROTOCOL_OFFSET] != MDNS_BRIDGE_IP_PROTO_UDP) {
            break;
        }
        if(memcmp(
               &ip[MDNS_BRIDGE_IPV4_DST_OFFSET],
               mdns_bridge_ipv4_group_addr,
               MDNS_BRIDGE_IPV4_ADDR_SIZE) != 0) {
            break;
        }
        if(!mdns_bridge_is_ipv4_checksum_valid(ip, ip_header_size)) {
            break;
        }

        const uint8_t* udp = &ip[ip_header_size];
        const size_t udp_size = mdns_bridge_read_u16(&udp[MDNS_BRIDGE_UDP_SIZE_OFFSET]);

        if(mdns_bridge_read_u16(&udp[MDNS_BRIDGE_UDP_DST_PORT_OFFSET]) != MDNS_BRIDGE_PORT) {
            break;
        }
        if(udp_size < MDNS_BRIDGE_UDP_HEADER_SIZE + MDNS_BRIDGE_DNS_HEADER_SIZE) {
            break;
        }
        if(udp_size > ip_total_size - ip_header_size) {
            break;
        }
        if(!mdns_bridge_is_udp_checksum_valid(ip, udp, udp_size)) {
            break;
        }

        const uint8_t* dns = &udp[MDNS_BRIDGE_UDP_HEADER_SIZE];
        const size_t dns_size = udp_size - MDNS_BRIDGE_UDP_HEADER_SIZE;

        if(dns[MDNS_BRIDGE_DNS_FLAGS_OFFSET] & MDNS_BRIDGE_DNS_FLAG_RESPONSE) {
            break;
        }
        if(!mdns_bridge_clear_unicast_response_bits(dns, dns_size, NULL)) {
            break;
        }

        out_packet->src_mac = &frame[MDNS_BRIDGE_ETH_SRC_OFFSET];
        out_packet->dns = dns;
        out_packet->dns_size = dns_size;
        is_valid = true;
    } while(false);

    return is_valid;
}

static bool mdns_bridge_parse_ipv6_response(
    const uint8_t* frame,
    size_t frame_size,
    MdnsBridgeDnsPacket* out_packet) {
    bool is_valid = false;

    do {
        if(frame_size < MDNS_BRIDGE_IPV6_MIN_FRAME_SIZE) {
            break;
        }
        if(mdns_bridge_read_u16(&frame[MDNS_BRIDGE_ETH_TYPE_OFFSET]) !=
           MDNS_BRIDGE_ETH_TYPE_IPV6) {
            break;
        }

        const uint8_t* ip = &frame[MDNS_BRIDGE_ETH_HEADER_SIZE];
        const size_t payload_size =
            mdns_bridge_read_u16(&ip[MDNS_BRIDGE_IPV6_PAYLOAD_SIZE_OFFSET]);

        if((ip[0] >> MDNS_BRIDGE_IP_VERSION_SHIFT) != MDNS_BRIDGE_IPV6_VERSION) {
            break;
        }
        if(ip[MDNS_BRIDGE_IPV6_NEXT_HEADER_OFFSET] != MDNS_BRIDGE_IP_PROTO_UDP) {
            break;
        }
        if(payload_size >
           frame_size - MDNS_BRIDGE_ETH_HEADER_SIZE - MDNS_BRIDGE_IPV6_HEADER_SIZE) {
            break;
        }
        if(memcmp(
               &ip[MDNS_BRIDGE_IPV6_DST_OFFSET],
               mdns_bridge_ipv6_group_addr,
               MDNS_BRIDGE_IPV6_ADDR_SIZE) != 0) {
            break;
        }

        const uint8_t* udp = &ip[MDNS_BRIDGE_IPV6_HEADER_SIZE];
        const size_t udp_size = mdns_bridge_read_u16(&udp[MDNS_BRIDGE_UDP_SIZE_OFFSET]);

        if(mdns_bridge_read_u16(&udp[MDNS_BRIDGE_UDP_SRC_PORT_OFFSET]) != MDNS_BRIDGE_PORT) {
            break;
        }
        if(mdns_bridge_read_u16(&udp[MDNS_BRIDGE_UDP_DST_PORT_OFFSET]) != MDNS_BRIDGE_PORT) {
            break;
        }
        if(udp_size != payload_size) {
            break;
        }
        if(udp_size < MDNS_BRIDGE_UDP_HEADER_SIZE + MDNS_BRIDGE_DNS_HEADER_SIZE) {
            break;
        }

        const uint8_t* dns = &udp[MDNS_BRIDGE_UDP_HEADER_SIZE];
        if(!(dns[MDNS_BRIDGE_DNS_FLAGS_OFFSET] & MDNS_BRIDGE_DNS_FLAG_RESPONSE)) {
            break;
        }

        out_packet->src_mac = &frame[MDNS_BRIDGE_ETH_SRC_OFFSET];
        out_packet->dns = dns;
        out_packet->dns_size = udp_size - MDNS_BRIDGE_UDP_HEADER_SIZE;
        is_valid = true;
    } while(false);

    return is_valid;
}

static void mdns_bridge_write_link_local_addr(const uint8_t* mac, uint8_t* out_addr) {
    memcpy(out_addr, mdns_bridge_ipv6_link_local_prefix, MDNS_BRIDGE_IPV6_PREFIX_SIZE);

    uint8_t* interface_id = &out_addr[MDNS_BRIDGE_IPV6_PREFIX_SIZE];
    interface_id[0] = mac[0] ^ MDNS_BRIDGE_IPV6_EUI64_UL_BIT;
    interface_id[1] = mac[1];
    interface_id[2] = mac[2];
    interface_id[3] = 0xFF;
    interface_id[4] = 0xFE;
    interface_id[5] = mac[3];
    interface_id[6] = mac[4];
    interface_id[7] = mac[5];
}

static void mdns_bridge_write_udp(
    uint8_t* udp,
    const MdnsBridgeDnsPacket* packet,
    const uint8_t* src_addr,
    const uint8_t* dst_addr,
    size_t addr_size) {
    const size_t udp_size = MDNS_BRIDGE_UDP_HEADER_SIZE + packet->dns_size;

    mdns_bridge_write_u16(&udp[MDNS_BRIDGE_UDP_SRC_PORT_OFFSET], MDNS_BRIDGE_PORT);
    mdns_bridge_write_u16(&udp[MDNS_BRIDGE_UDP_DST_PORT_OFFSET], MDNS_BRIDGE_PORT);
    mdns_bridge_write_u16(&udp[MDNS_BRIDGE_UDP_SIZE_OFFSET], (uint16_t)udp_size);
    mdns_bridge_write_u16(&udp[MDNS_BRIDGE_UDP_CHECKSUM_OFFSET], MDNS_BRIDGE_UDP_CHECKSUM_NONE);

    uint8_t* dns = &udp[MDNS_BRIDGE_UDP_HEADER_SIZE];
    memcpy(dns, packet->dns, packet->dns_size);

    if(!(dns[MDNS_BRIDGE_DNS_FLAGS_OFFSET] & MDNS_BRIDGE_DNS_FLAG_RESPONSE)) {
        mdns_bridge_clear_unicast_response_bits(packet->dns, packet->dns_size, dns);
    }

    mdns_bridge_write_u16(
        &udp[MDNS_BRIDGE_UDP_CHECKSUM_OFFSET],
        mdns_bridge_udp_checksum(src_addr, dst_addr, addr_size, udp, udp_size));
}

static void mdns_bridge_write_ipv6_query(const MdnsBridgeDnsPacket* packet, uint8_t* out) {
    const size_t udp_size = MDNS_BRIDGE_UDP_HEADER_SIZE + packet->dns_size;

    memcpy(&out[MDNS_BRIDGE_ETH_DST_OFFSET], mdns_bridge_ipv6_group_mac, MDNS_BRIDGE_ETH_MAC_SIZE);
    memcpy(&out[MDNS_BRIDGE_ETH_SRC_OFFSET], packet->src_mac, MDNS_BRIDGE_ETH_MAC_SIZE);
    mdns_bridge_write_u16(&out[MDNS_BRIDGE_ETH_TYPE_OFFSET], MDNS_BRIDGE_ETH_TYPE_IPV6);

    uint8_t* ip = &out[MDNS_BRIDGE_ETH_HEADER_SIZE];
    memset(ip, 0, MDNS_BRIDGE_IPV6_HEADER_SIZE);
    ip[0] = MDNS_BRIDGE_IPV6_VERSION << MDNS_BRIDGE_IP_VERSION_SHIFT;
    mdns_bridge_write_u16(&ip[MDNS_BRIDGE_IPV6_PAYLOAD_SIZE_OFFSET], (uint16_t)udp_size);
    ip[MDNS_BRIDGE_IPV6_NEXT_HEADER_OFFSET] = MDNS_BRIDGE_IP_PROTO_UDP;
    ip[MDNS_BRIDGE_IPV6_HOP_LIMIT_OFFSET] = MDNS_BRIDGE_HOP_LIMIT;
    mdns_bridge_write_link_local_addr(packet->src_mac, &ip[MDNS_BRIDGE_IPV6_SRC_OFFSET]);
    memcpy(
        &ip[MDNS_BRIDGE_IPV6_DST_OFFSET], mdns_bridge_ipv6_group_addr, MDNS_BRIDGE_IPV6_ADDR_SIZE);

    mdns_bridge_write_udp(
        &ip[MDNS_BRIDGE_IPV6_HEADER_SIZE],
        packet,
        &ip[MDNS_BRIDGE_IPV6_SRC_OFFSET],
        &ip[MDNS_BRIDGE_IPV6_DST_OFFSET],
        MDNS_BRIDGE_IPV6_ADDR_SIZE);
}

static void mdns_bridge_write_ipv4_response(
    const MdnsBridgeDnsPacket* packet,
    const MdnsBridgeIpv4Address* source_addr,
    uint8_t* out) {
    const size_t ip_total_size =
        MDNS_BRIDGE_IPV4_HEADER_SIZE + MDNS_BRIDGE_UDP_HEADER_SIZE + packet->dns_size;

    memcpy(&out[MDNS_BRIDGE_ETH_DST_OFFSET], mdns_bridge_ipv4_group_mac, MDNS_BRIDGE_ETH_MAC_SIZE);
    memcpy(&out[MDNS_BRIDGE_ETH_SRC_OFFSET], packet->src_mac, MDNS_BRIDGE_ETH_MAC_SIZE);
    mdns_bridge_write_u16(&out[MDNS_BRIDGE_ETH_TYPE_OFFSET], MDNS_BRIDGE_ETH_TYPE_IPV4);

    uint8_t* ip = &out[MDNS_BRIDGE_ETH_HEADER_SIZE];
    memset(ip, 0, MDNS_BRIDGE_IPV4_HEADER_SIZE);
    ip[MDNS_BRIDGE_IPV4_VERSION_IHL_OFFSET] =
        (MDNS_BRIDGE_IPV4_VERSION << MDNS_BRIDGE_IP_VERSION_SHIFT) |
        (MDNS_BRIDGE_IPV4_HEADER_SIZE / MDNS_BRIDGE_IPV4_IHL_UNIT_SIZE);
    mdns_bridge_write_u16(&ip[MDNS_BRIDGE_IPV4_TOTAL_SIZE_OFFSET], (uint16_t)ip_total_size);
    // RFC 6864: atomic datagrams may carry any ID, so no per-datagram state is needed
    mdns_bridge_write_u16(&ip[MDNS_BRIDGE_IPV4_FRAGMENT_OFFSET], MDNS_BRIDGE_IPV4_FLAG_DF);
    ip[MDNS_BRIDGE_IPV4_TTL_OFFSET] = MDNS_BRIDGE_HOP_LIMIT;
    ip[MDNS_BRIDGE_IPV4_PROTOCOL_OFFSET] = MDNS_BRIDGE_IP_PROTO_UDP;
    memcpy(&ip[MDNS_BRIDGE_IPV4_SRC_OFFSET], source_addr->octets, MDNS_BRIDGE_IPV4_ADDR_SIZE);
    memcpy(
        &ip[MDNS_BRIDGE_IPV4_DST_OFFSET], mdns_bridge_ipv4_group_addr, MDNS_BRIDGE_IPV4_ADDR_SIZE);
    mdns_bridge_write_u16(
        &ip[MDNS_BRIDGE_IPV4_CHECKSUM_OFFSET],
        mdns_bridge_checksum_finish(
            mdns_bridge_checksum_add(0, ip, MDNS_BRIDGE_IPV4_HEADER_SIZE)));

    mdns_bridge_write_udp(
        &ip[MDNS_BRIDGE_IPV4_HEADER_SIZE],
        packet,
        &ip[MDNS_BRIDGE_IPV4_SRC_OFFSET],
        &ip[MDNS_BRIDGE_IPV4_DST_OFFSET],
        MDNS_BRIDGE_IPV4_ADDR_SIZE);
}

size_t mdns_bridge_ipv4_query_to_ipv6(
    const uint8_t* frame,
    size_t frame_size,
    uint8_t* out,
    size_t out_size) {
    furi_check(frame);

    size_t translated_size = 0;
    MdnsBridgeDnsPacket packet;

    if(mdns_bridge_parse_ipv4_query(frame, frame_size, &packet)) {
        const size_t required_size = MDNS_BRIDGE_ETH_HEADER_SIZE + MDNS_BRIDGE_IPV6_HEADER_SIZE +
                                     MDNS_BRIDGE_UDP_HEADER_SIZE + packet.dns_size;

        if(!out) {
            translated_size = required_size;
        } else if(out_size >= required_size) {
            mdns_bridge_write_ipv6_query(&packet, out);
            translated_size = required_size;
        }
    }

    return translated_size;
}

size_t mdns_bridge_ipv6_response_to_ipv4(
    const uint8_t* frame,
    size_t frame_size,
    const MdnsBridgeIpv4Address* source_addr,
    uint8_t* out,
    size_t out_size) {
    furi_check(frame);
    furi_check(source_addr);

    size_t translated_size = 0;
    MdnsBridgeDnsPacket packet;

    if(mdns_bridge_parse_ipv6_response(frame, frame_size, &packet)) {
        const size_t required_size = MDNS_BRIDGE_ETH_HEADER_SIZE + MDNS_BRIDGE_IPV4_HEADER_SIZE +
                                     MDNS_BRIDGE_UDP_HEADER_SIZE + packet.dns_size;

        if(!out) {
            translated_size = required_size;
        } else if(out_size >= required_size) {
            mdns_bridge_write_ipv4_response(&packet, source_addr, out);
            translated_size = required_size;
        }
    }

    return translated_size;
}

bool mdns_bridge_get_ipv4_source(
    const uint8_t* frame,
    size_t frame_size,
    MdnsBridgeIpv4Address* out_addr) {
    furi_check(frame);
    furi_check(out_addr);

    static const MdnsBridgeIpv4Address unspecified_addr = {0};
    bool has_source = false;

    do {
        if(frame_size < MDNS_BRIDGE_ETH_HEADER_SIZE + MDNS_BRIDGE_IPV4_HEADER_SIZE) {
            break;
        }
        if(mdns_bridge_read_u16(&frame[MDNS_BRIDGE_ETH_TYPE_OFFSET]) !=
           MDNS_BRIDGE_ETH_TYPE_IPV4) {
            break;
        }

        const uint8_t* ip = &frame[MDNS_BRIDGE_ETH_HEADER_SIZE];
        if((ip[MDNS_BRIDGE_IPV4_VERSION_IHL_OFFSET] >> MDNS_BRIDGE_IP_VERSION_SHIFT) !=
           MDNS_BRIDGE_IPV4_VERSION) {
            break;
        }

        const uint8_t* src_addr = &ip[MDNS_BRIDGE_IPV4_SRC_OFFSET];
        if(memcmp(src_addr, unspecified_addr.octets, MDNS_BRIDGE_IPV4_ADDR_SIZE) == 0) {
            break;
        }

        memcpy(out_addr->octets, src_addr, MDNS_BRIDGE_IPV4_ADDR_SIZE);
        has_source = true;
    } while(false);

    return has_source;
}
