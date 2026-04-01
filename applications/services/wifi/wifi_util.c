#include "wifi_util.h"
#include "wifi_common.h"
#include <string.h>
#include <stdio.h>
#include <furi/core/core_defines.h>

#if defined(__builtin_bswap16)
#define htons(x) __builtin_bswap16(x)
#else
#define htons(x) ((((x) >> 8) | ((x) << 8)) & 0xffff)
#endif

void wifi_format_bssid(const uint8_t* bssid, char* str_out, size_t str_out_size) {
    snprintf(str_out, str_out_size, "%02hhX", bssid[0]);
    for(size_t i = 1; i < HW_ADDRESS_LEN; i++) {
        char part[4];
        snprintf(part, sizeof(part), ":%02hhX", bssid[i]);
        strlcat(str_out, part, str_out_size);
    }
}

void wifi_format_ipv4(const WifiIpv4* ipv4, char* str_out, size_t str_out_size) {
    snprintf(
        str_out,
        str_out_size,
        "%hhu.%hhu.%hhu.%hhu",
        ipv4->bytes[0],
        ipv4->bytes[1],
        ipv4->bytes[2],
        ipv4->bytes[3]);
}

typedef struct ZeroRun {
    bool valid;
    size_t start;
    size_t length;
} ZeroRun;

static ZeroRun find_longest_zero_run(const WifiIpv6* ipv6) {
    ZeroRun result = {
        .valid = false,
        .start = 0,
        .length = 0,
    };
    size_t run_length = 0;
    for(size_t i = 0; i != COUNT_OF(ipv6->words); ++i) {
        if(ipv6->words[i] == 0) {
            run_length += 1;
            result.valid = true;
            if(run_length > result.length) {
                result.length = run_length;
                result.start = i + 1 - run_length;
            }
        } else {
            run_length = 0;
        }
    }
    return result;
}

static void append_ipv6_part(
    const WifiIpv6* ipv6,
    char* str_out,
    size_t str_out_size,
    size_t start,
    size_t end) {
    if(start == end) {
        return;
    }
    char buf[6];
    snprintf(buf, sizeof(buf), "%hx", htons(ipv6->words[start]));
    strlcat(str_out, buf, str_out_size);
    for(size_t i = start + 1; i != end; ++i) {
        snprintf(buf, sizeof(buf), ":%hx", htons(ipv6->words[i]));
        strlcat(str_out, buf, str_out_size);
    }
}

void wifi_format_ipv6(const WifiIpv6* ipv6, char* str_out, size_t str_out_size) {
    ZeroRun zero_run = find_longest_zero_run(ipv6);
    str_out[0] = 0;
    if(zero_run.valid && zero_run.length > 1) {
        // two halves separated by ::
        append_ipv6_part(ipv6, str_out, str_out_size, 0, zero_run.start);
        strlcat(str_out, "::", str_out_size);
        append_ipv6_part(
            ipv6, str_out, str_out_size, zero_run.start + zero_run.length, COUNT_OF(ipv6->words));
    } else {
        // one piece
        append_ipv6_part(ipv6, str_out, str_out_size, 0, COUNT_OF(ipv6->words));
    }
}

bool wifi_ipv6_is_specified(const WifiIpv6* ipv6) {
    uint32_t r = 0;
    for(size_t i = 0; i != COUNT_OF(ipv6->value); ++i) {
        r |= ipv6->value[i];
    }
    return r != 0;
}
