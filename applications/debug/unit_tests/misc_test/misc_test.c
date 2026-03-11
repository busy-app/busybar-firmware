#include "../unit_tests.h"

#include <wifi/wifi_util.h>

MU_TEST(misc_tests_ipv6_format) {
    char buf[40];

    // unspecified
    WifiIpv6 addr = {0};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("::", buf);

    // localhost
    addr = (WifiIpv6){.bytes = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("::1", buf);

    // zeros at the end
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337::", buf);

    // zeros in the middle
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337::1:2:3:4", buf);

    // one zero
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337:0:1:2:3:4:5", buf);

    // two equal zero runs
    addr = (WifiIpv6){.bytes = {0xca, 0xfe, 0x13, 0x37, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2}};
    wifi_format_ipv6(&addr, buf, sizeof(buf));
    mu_assert_string_eq("cafe:1337::1:0:0:2", buf);
}

MU_TEST(misc_tests_ipv4_format) {
    char buf[16];
    WifiIpv4 addr = {.bytes = {192, 168, 0, 1}};

    wifi_format_ipv4(&addr, buf, sizeof(buf));
    mu_assert_string_eq("192.168.0.1", buf);
}

MU_TEST_SUITE(misc_test_suite) {
    MU_RUN_TEST(misc_tests_ipv6_format);
    MU_RUN_TEST(misc_tests_ipv4_format);
}

int run_minunit_misc_test(void) {
    MU_RUN_SUITE(misc_test_suite);
    return MU_EXIT_CODE;
}
