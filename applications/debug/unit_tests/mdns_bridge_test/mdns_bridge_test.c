#include "../unit_tests.h"

#include <toolbox/mdns_bridge.h>

#define MDNS_BRIDGE_TEST_FRAME_SIZE (256U)
#define MDNS_BRIDGE_TEST_PORT       (5353U)
#define MDNS_BRIDGE_TEST_PORT_OTHER (40000U)

#define MDNS_BRIDGE_TEST_ETH_SIZE  (14U)
#define MDNS_BRIDGE_TEST_IPV4_SIZE (20U)
#define MDNS_BRIDGE_TEST_IPV6_SIZE (40U)
#define MDNS_BRIDGE_TEST_UDP_SIZE  (8U)

#define MDNS_BRIDGE_TEST_IPV4_UDP (MDNS_BRIDGE_TEST_ETH_SIZE + MDNS_BRIDGE_TEST_IPV4_SIZE)
#define MDNS_BRIDGE_TEST_IPV4_DNS (MDNS_BRIDGE_TEST_IPV4_UDP + MDNS_BRIDGE_TEST_UDP_SIZE)
#define MDNS_BRIDGE_TEST_IPV6_UDP (MDNS_BRIDGE_TEST_ETH_SIZE + MDNS_BRIDGE_TEST_IPV6_SIZE)
#define MDNS_BRIDGE_TEST_IPV6_DNS (MDNS_BRIDGE_TEST_IPV6_UDP + MDNS_BRIDGE_TEST_UDP_SIZE)

#define MDNS_BRIDGE_TEST_QUERY_QCLASS_1 (35U)
#define MDNS_BRIDGE_TEST_QUERY_QCLASS_2 (45U)

static const uint8_t mdns_bridge_test_querier_mac[] = {0x58, 0xA8, 0xE8, 0x1F, 0xF3, 0xB1};
static const uint8_t mdns_bridge_test_querier_ipv4[] = {192, 168, 0, 101};
static const uint8_t mdns_bridge_test_ipv4_group[] = {224, 0, 0, 251};
static const uint8_t mdns_bridge_test_ipv4_group_mac[] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB};
static const uint8_t mdns_bridge_test_ipv6_group_mac[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0xFB};
static const uint8_t mdns_bridge_test_ipv6_group[] =
    {0xFF, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFB};
static const uint8_t mdns_bridge_test_querier_link_local[] =
    {0xFE, 0x80, 0, 0, 0, 0, 0, 0, 0x5A, 0xA8, 0xE8, 0xFF, 0xFE, 0x1F, 0xF3, 0xB1};
static const MdnsBridgeIpv4Address mdns_bridge_test_source = {.octets = {192, 168, 0, 100}};

// Two PTR questions with the unicast-response bit set, the second one using name compression
static const uint8_t mdns_bridge_test_query[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 8,    '_',  'm',  'a',
    't',  't',  'e',  'r',  'c',  4,    '_',  'u',  'd',  'p',  5,    'l',  'o',  'c',  'a',  'l',
    0,    0x00, 0x0C, 0x80, 0x01, 3,    '_',  'C',  'M',  0xC0, 0x0C, 0x00, 0x0C, 0x80, 0x01,
};

// One AAAA answer record
static const uint8_t mdns_bridge_test_response[] = {
    0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 4,
    'h',  'o',  's',  't',  5,    'l',  'o',  'c',  'a',  'l',  0,    0x00, 0x1C,
    0x80, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x10, 0xFE, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x8E, 0x8B, 0x48, 0xFF, 0xFE, 0x33, 0xDE, 0x0C,
};

static uint16_t mdns_bridge_test_read_u16(const uint8_t* data) {
    return (uint16_t)((data[0] << 8) | data[1]);
}

static void mdns_bridge_test_write_u16(uint8_t* data, uint16_t value) {
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

static uint32_t mdns_bridge_test_sum(uint32_t sum, const uint8_t* data, size_t data_size) {
    for(size_t i = 0; i < data_size; ++i) {
        sum += (i % 2) ? data[i] : ((uint32_t)data[i] << 8);
    }
    return sum;
}

static uint16_t mdns_bridge_test_fold(uint32_t sum) {
    while(sum >> 16) {
        sum = (sum & 0xFFFFU) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

static uint16_t mdns_bridge_test_udp_checksum(
    const uint8_t* addrs,
    size_t addrs_size,
    const uint8_t* udp,
    size_t udp_size) {
    uint32_t sum = mdns_bridge_test_sum(0, addrs, addrs_size);
    sum += 17U + (uint32_t)udp_size;
    return mdns_bridge_test_fold(mdns_bridge_test_sum(sum, udp, udp_size));
}

static void mdns_bridge_test_update_ipv4_checksums(uint8_t* frame) {
    uint8_t* ip = &frame[MDNS_BRIDGE_TEST_ETH_SIZE];
    uint8_t* udp = &frame[MDNS_BRIDGE_TEST_IPV4_UDP];
    const size_t udp_size = mdns_bridge_test_read_u16(&udp[4]);

    mdns_bridge_test_write_u16(&ip[10], 0);
    mdns_bridge_test_write_u16(
        &ip[10], mdns_bridge_test_fold(mdns_bridge_test_sum(0, ip, MDNS_BRIDGE_TEST_IPV4_SIZE)));

    mdns_bridge_test_write_u16(&udp[6], 0);
    mdns_bridge_test_write_u16(&udp[6], mdns_bridge_test_udp_checksum(&ip[12], 8, udp, udp_size));
}

static size_t mdns_bridge_test_build_ipv4_query(uint8_t* frame, uint16_t src_port) {
    const size_t dns_size = sizeof(mdns_bridge_test_query);
    const size_t udp_size = MDNS_BRIDGE_TEST_UDP_SIZE + dns_size;

    memset(frame, 0, MDNS_BRIDGE_TEST_FRAME_SIZE);
    memcpy(&frame[0], mdns_bridge_test_ipv4_group_mac, 6);
    memcpy(&frame[6], mdns_bridge_test_querier_mac, 6);
    mdns_bridge_test_write_u16(&frame[12], 0x0800);

    uint8_t* ip = &frame[MDNS_BRIDGE_TEST_ETH_SIZE];
    ip[0] = 0x45;
    mdns_bridge_test_write_u16(&ip[2], (uint16_t)(MDNS_BRIDGE_TEST_IPV4_SIZE + udp_size));
    ip[8] = 255;
    ip[9] = 17;
    memcpy(&ip[12], mdns_bridge_test_querier_ipv4, 4);
    memcpy(&ip[16], mdns_bridge_test_ipv4_group, 4);

    uint8_t* udp = &frame[MDNS_BRIDGE_TEST_IPV4_UDP];
    mdns_bridge_test_write_u16(&udp[0], src_port);
    mdns_bridge_test_write_u16(&udp[2], MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&udp[4], (uint16_t)udp_size);
    memcpy(&frame[MDNS_BRIDGE_TEST_IPV4_DNS], mdns_bridge_test_query, dns_size);

    mdns_bridge_test_update_ipv4_checksums(frame);
    return MDNS_BRIDGE_TEST_IPV4_DNS + dns_size;
}

static size_t mdns_bridge_test_build_ipv6_response(uint8_t* frame) {
    const size_t dns_size = sizeof(mdns_bridge_test_response);
    const size_t udp_size = MDNS_BRIDGE_TEST_UDP_SIZE + dns_size;

    memset(frame, 0, MDNS_BRIDGE_TEST_FRAME_SIZE);
    memcpy(&frame[0], mdns_bridge_test_ipv6_group_mac, 6);
    memcpy(&frame[6], mdns_bridge_test_querier_mac, 6);
    mdns_bridge_test_write_u16(&frame[12], 0x86DD);

    uint8_t* ip = &frame[MDNS_BRIDGE_TEST_ETH_SIZE];
    ip[0] = 0x60;
    mdns_bridge_test_write_u16(&ip[4], (uint16_t)udp_size);
    ip[6] = 17;
    ip[7] = 255;
    memcpy(&ip[8], mdns_bridge_test_querier_link_local, 16);
    memcpy(&ip[24], mdns_bridge_test_ipv6_group, 16);

    uint8_t* udp = &frame[MDNS_BRIDGE_TEST_IPV6_UDP];
    mdns_bridge_test_write_u16(&udp[0], MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&udp[2], MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&udp[4], (uint16_t)udp_size);
    memcpy(&frame[MDNS_BRIDGE_TEST_IPV6_DNS], mdns_bridge_test_response, dns_size);

    return MDNS_BRIDGE_TEST_IPV6_DNS + dns_size;
}

static size_t mdns_bridge_test_translate_query(const uint8_t* frame, size_t frame_size) {
    uint8_t out[MDNS_BRIDGE_TEST_FRAME_SIZE];
    return mdns_bridge_ipv4_query_to_ipv6(frame, frame_size, out, sizeof(out));
}

static size_t mdns_bridge_test_translate_response(const uint8_t* frame, size_t frame_size) {
    uint8_t out[MDNS_BRIDGE_TEST_FRAME_SIZE];
    return mdns_bridge_ipv6_response_to_ipv4(
        frame, frame_size, &mdns_bridge_test_source, out, sizeof(out));
}

MU_TEST(mdns_bridge_test_query_translated) {
    uint8_t frame[MDNS_BRIDGE_TEST_FRAME_SIZE];
    uint8_t out[MDNS_BRIDGE_TEST_FRAME_SIZE];
    const size_t frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    const size_t dns_size = sizeof(mdns_bridge_test_query);
    const size_t udp_size = MDNS_BRIDGE_TEST_UDP_SIZE + dns_size;

    const size_t out_size = mdns_bridge_ipv4_query_to_ipv6(frame, frame_size, out, sizeof(out));
    mu_assert_int_eq(MDNS_BRIDGE_TEST_IPV6_DNS + dns_size, out_size);
    mu_assert_int_eq(out_size, mdns_bridge_ipv4_query_to_ipv6(frame, frame_size, NULL, 0));

    mu_assert_mem_eq(mdns_bridge_test_ipv6_group_mac, &out[0], 6);
    mu_assert_mem_eq(mdns_bridge_test_querier_mac, &out[6], 6);
    mu_assert_int_eq(0x86DD, mdns_bridge_test_read_u16(&out[12]));

    const uint8_t* ip = &out[MDNS_BRIDGE_TEST_ETH_SIZE];
    mu_assert_int_eq(0x60, ip[0]);
    mu_assert_int_eq(udp_size, mdns_bridge_test_read_u16(&ip[4]));
    mu_assert_int_eq(17, ip[6]);
    mu_assert_int_eq(255, ip[7]);
    mu_assert_mem_eq(mdns_bridge_test_querier_link_local, &ip[8], 16);
    mu_assert_mem_eq(mdns_bridge_test_ipv6_group, &ip[24], 16);

    const uint8_t* udp = &out[MDNS_BRIDGE_TEST_IPV6_UDP];
    mu_assert_int_eq(MDNS_BRIDGE_TEST_PORT, mdns_bridge_test_read_u16(&udp[0]));
    mu_assert_int_eq(MDNS_BRIDGE_TEST_PORT, mdns_bridge_test_read_u16(&udp[2]));
    mu_assert_int_eq(udp_size, mdns_bridge_test_read_u16(&udp[4]));
    mu_assert_int_eq(0, mdns_bridge_test_udp_checksum(&ip[8], 32, udp, udp_size));

    uint8_t expected_dns[sizeof(mdns_bridge_test_query)];
    memcpy(expected_dns, mdns_bridge_test_query, sizeof(expected_dns));
    expected_dns[MDNS_BRIDGE_TEST_QUERY_QCLASS_1] = 0x00;
    expected_dns[MDNS_BRIDGE_TEST_QUERY_QCLASS_2] = 0x00;
    mu_assert_mem_eq(expected_dns, &out[MDNS_BRIDGE_TEST_IPV6_DNS], dns_size);
}

MU_TEST(mdns_bridge_test_query_variants_accepted) {
    uint8_t frame[MDNS_BRIDGE_TEST_FRAME_SIZE];
    uint8_t out[MDNS_BRIDGE_TEST_FRAME_SIZE];
    size_t frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT_OTHER);

    mu_check(mdns_bridge_ipv4_query_to_ipv6(frame, frame_size, out, sizeof(out)) != 0);
    mu_assert_int_eq(
        MDNS_BRIDGE_TEST_PORT, mdns_bridge_test_read_u16(&out[MDNS_BRIDGE_TEST_IPV6_UDP]));

    mu_check(mdns_bridge_test_translate_query(frame, frame_size + 6) != 0);

    mdns_bridge_test_write_u16(&frame[MDNS_BRIDGE_TEST_IPV4_UDP + 6], 0);
    mu_check(mdns_bridge_test_translate_query(frame, frame_size) != 0);
}

MU_TEST(mdns_bridge_test_query_rejected) {
    uint8_t frame[MDNS_BRIDGE_TEST_FRAME_SIZE];
    uint8_t* ip = &frame[MDNS_BRIDGE_TEST_ETH_SIZE];
    uint8_t* udp = &frame[MDNS_BRIDGE_TEST_IPV4_UDP];
    uint8_t* dns = &frame[MDNS_BRIDGE_TEST_IPV4_DNS];
    size_t frame_size = 0;

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size - 1));
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, MDNS_BRIDGE_TEST_IPV4_DNS + 11));

    uint8_t small_out[MDNS_BRIDGE_TEST_IPV6_DNS];
    mu_assert_int_eq(
        0, mdns_bridge_ipv4_query_to_ipv6(frame, frame_size, small_out, sizeof(small_out)));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&frame[12], 0x86DD);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    ip[16] = 239;
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&udp[2], MDNS_BRIDGE_TEST_PORT_OTHER);
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&ip[6], 0x2000);
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    ip[0] = 0x44;
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    ip[8] = 1;
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    dns[13] ^= 0x01;
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    dns[2] |= 0x80;
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    dns[12] = 0x3F;
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    dns[12] = 0x48;
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&dns[4], 3);
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    mdns_bridge_test_write_u16(&udp[4], (uint16_t)(frame_size - MDNS_BRIDGE_TEST_IPV4_UDP + 1));
    mdns_bridge_test_update_ipv4_checksums(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_query(frame, frame_size));
}

MU_TEST(mdns_bridge_test_response_translated) {
    uint8_t frame[MDNS_BRIDGE_TEST_FRAME_SIZE];
    uint8_t out[MDNS_BRIDGE_TEST_FRAME_SIZE];
    const size_t frame_size = mdns_bridge_test_build_ipv6_response(frame);
    const size_t dns_size = sizeof(mdns_bridge_test_response);
    const size_t udp_size = MDNS_BRIDGE_TEST_UDP_SIZE + dns_size;

    const size_t out_size = mdns_bridge_ipv6_response_to_ipv4(
        frame, frame_size, &mdns_bridge_test_source, out, sizeof(out));
    mu_assert_int_eq(MDNS_BRIDGE_TEST_IPV4_DNS + dns_size, out_size);
    mu_assert_int_eq(
        out_size,
        mdns_bridge_ipv6_response_to_ipv4(frame, frame_size, &mdns_bridge_test_source, NULL, 0));

    mu_assert_mem_eq(mdns_bridge_test_ipv4_group_mac, &out[0], 6);
    mu_assert_mem_eq(mdns_bridge_test_querier_mac, &out[6], 6);
    mu_assert_int_eq(0x0800, mdns_bridge_test_read_u16(&out[12]));

    const uint8_t* ip = &out[MDNS_BRIDGE_TEST_ETH_SIZE];
    mu_assert_int_eq(0x45, ip[0]);
    mu_assert_int_eq(MDNS_BRIDGE_TEST_IPV4_SIZE + udp_size, mdns_bridge_test_read_u16(&ip[2]));
    mu_assert_int_eq(0x4000, mdns_bridge_test_read_u16(&ip[6]));
    mu_assert_int_eq(255, ip[8]);
    mu_assert_int_eq(17, ip[9]);
    mu_assert_mem_eq(mdns_bridge_test_source.octets, &ip[12], 4);
    mu_assert_mem_eq(mdns_bridge_test_ipv4_group, &ip[16], 4);
    mu_assert_int_eq(0, mdns_bridge_test_fold(mdns_bridge_test_sum(0, ip, 20)));

    const uint8_t* udp = &out[MDNS_BRIDGE_TEST_IPV4_UDP];
    mu_assert_int_eq(MDNS_BRIDGE_TEST_PORT, mdns_bridge_test_read_u16(&udp[0]));
    mu_assert_int_eq(MDNS_BRIDGE_TEST_PORT, mdns_bridge_test_read_u16(&udp[2]));
    mu_assert_int_eq(udp_size, mdns_bridge_test_read_u16(&udp[4]));
    mu_assert_int_eq(0, mdns_bridge_test_udp_checksum(&ip[12], 8, udp, udp_size));
    mu_assert_mem_eq(mdns_bridge_test_response, &out[MDNS_BRIDGE_TEST_IPV4_DNS], dns_size);
}

MU_TEST(mdns_bridge_test_response_rejected) {
    uint8_t frame[MDNS_BRIDGE_TEST_FRAME_SIZE];
    uint8_t* ip = &frame[MDNS_BRIDGE_TEST_ETH_SIZE];
    uint8_t* udp = &frame[MDNS_BRIDGE_TEST_IPV6_UDP];
    size_t frame_size = 0;

    frame_size = mdns_bridge_test_build_ipv6_response(frame);
    mu_assert_int_eq(0, mdns_bridge_test_translate_response(frame, frame_size - 1));

    uint8_t small_out[MDNS_BRIDGE_TEST_IPV4_DNS];
    mu_assert_int_eq(
        0,
        mdns_bridge_ipv6_response_to_ipv4(
            frame, frame_size, &mdns_bridge_test_source, small_out, sizeof(small_out)));

    frame_size = mdns_bridge_test_build_ipv6_response(frame);
    frame[MDNS_BRIDGE_TEST_IPV6_DNS + 2] &= 0x7F;
    mu_assert_int_eq(0, mdns_bridge_test_translate_response(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv6_response(frame);
    memcpy(&ip[24], mdns_bridge_test_querier_link_local, 16);
    mu_assert_int_eq(0, mdns_bridge_test_translate_response(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv6_response(frame);
    mdns_bridge_test_write_u16(&udp[0], MDNS_BRIDGE_TEST_PORT_OTHER);
    mu_assert_int_eq(0, mdns_bridge_test_translate_response(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv6_response(frame);
    ip[6] = 0;
    mu_assert_int_eq(0, mdns_bridge_test_translate_response(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv6_response(frame);
    mdns_bridge_test_write_u16(&udp[4], (uint16_t)(mdns_bridge_test_read_u16(&udp[4]) - 1));
    mu_assert_int_eq(0, mdns_bridge_test_translate_response(frame, frame_size));

    frame_size = mdns_bridge_test_build_ipv6_response(frame);
    mdns_bridge_test_write_u16(&frame[12], 0x0800);
    mu_assert_int_eq(0, mdns_bridge_test_translate_response(frame, frame_size));
}

MU_TEST(mdns_bridge_test_ipv4_source) {
    uint8_t frame[MDNS_BRIDGE_TEST_FRAME_SIZE];
    const size_t frame_size = mdns_bridge_test_build_ipv4_query(frame, MDNS_BRIDGE_TEST_PORT);
    MdnsBridgeIpv4Address addr = {0};

    mu_check(mdns_bridge_get_ipv4_source(frame, frame_size, &addr));
    mu_assert_mem_eq(mdns_bridge_test_querier_ipv4, addr.octets, 4);

    const MdnsBridgeIpv4Address untouched = {.octets = {1, 2, 3, 4}};
    addr = untouched;
    mu_check(!mdns_bridge_get_ipv4_source(frame, MDNS_BRIDGE_TEST_IPV4_UDP - 1, &addr));
    mu_assert_mem_eq(untouched.octets, addr.octets, 4);

    memset(&frame[MDNS_BRIDGE_TEST_ETH_SIZE + 12], 0, 4);
    mu_check(!mdns_bridge_get_ipv4_source(frame, frame_size, &addr));

    const size_t ipv6_size = mdns_bridge_test_build_ipv6_response(frame);
    mu_check(!mdns_bridge_get_ipv4_source(frame, ipv6_size, &addr));
    mu_assert_mem_eq(untouched.octets, addr.octets, 4);
}

MU_TEST_SUITE(mdns_bridge_test_suite) {
    MU_RUN_TEST(mdns_bridge_test_query_translated);
    MU_RUN_TEST(mdns_bridge_test_query_variants_accepted);
    MU_RUN_TEST(mdns_bridge_test_query_rejected);
    MU_RUN_TEST(mdns_bridge_test_response_translated);
    MU_RUN_TEST(mdns_bridge_test_response_rejected);
    MU_RUN_TEST(mdns_bridge_test_ipv4_source);
}

int run_minunit_mdns_bridge_test(void) {
    MU_RUN_SUITE(mdns_bridge_test_suite);
    return MU_EXIT_CODE;
}
