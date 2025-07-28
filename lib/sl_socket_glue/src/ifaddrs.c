#include <ifaddrs.h>
#include <net/if.h>
#include <sl_net.h>
#include <sl_net_wifi_types.h>

static const uint8_t if_addrs_default_v6_netmask[16] = {
    // default is /64
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/**
 * @brief Swaps between network byte order (big endian) and host byte order
 * (little endian) 32-bit-word-wise
 */
static void if_addrs_swap_byteorder(void* v6_addr) {
    for(size_t i = 0; i < 16 / 4; i++) {
        uint32_t* word_ptr = (uint32_t*)v6_addr + i;
        *word_ptr = REVERSE_BYTES_U32(*word_ptr);
    }
}

static void if_addrs_sl_addr_to_sockaddr(const void* addr, size_t addr_len, struct sockaddr_storage* out) {
    if(addr_len == sizeof(sl_ipv4_address_t)) {
        struct sockaddr_in sockaddr = {
            .sin_len = sizeof(struct sockaddr_in),
            .sin_family = AF_INET,
        };
        memcpy(&sockaddr.sin_addr, addr, addr_len);
        memcpy(out, &sockaddr, sizeof(sockaddr));
    } else if(addr_len == sizeof(sl_ipv6_address_t)) {
        struct sockaddr_in6 sockaddr = {
            .sin6_len = sizeof(struct sockaddr_in),
            .sin6_family = AF_INET6,
            .sin6_scope_id = __IPV6_ADDR_SCOPE_LINKLOCAL,
        };
        memcpy(&sockaddr.sin6_addr, addr, addr_len);
        memcpy(out, &sockaddr, sizeof(sockaddr));
    }
}

static struct sockaddr* if_addrs_getifaddrs_sockaddr_copy(struct sockaddr_storage* output_sockaddrs, size_t* sockaddr_pos, const struct sockaddr_storage* source) {
    struct sockaddr_storage* dest = &output_sockaddrs[*sockaddr_pos];
    memcpy(dest, source, sizeof(*source));
    (*sockaddr_pos)++;
    return (struct sockaddr*)dest;
}

static void if_addrs_getifaddrs_link_addr(struct ifaddrs* output_buf, size_t* buf_pos) {
    if(*buf_pos) output_buf[*buf_pos].ifa_next = &output_buf[*buf_pos + 1];
    (*buf_pos)++;
}

int getifaddrs(struct ifaddrs** ifap) {
    // sl_net_ip_address_t sl_addresses = {
    //     .mode = SL_IP_MANAGEMENT_STATIC_IP,
    //     .type = SL_IPV6_LINK_LOCAL,
    //     .v6 = {
    //         .link_local_address = {.bytes = {
    //             0xfe, 0x80,  0x00, 0x00,
    //             0x00, 0x00,  0x00, 0x00,
    //             0x8e, 0x8b,  0x48, 0xff,
    //             0xfe, 0x33,  0xe7, 0x88,
    //         }},
    //         .global_address = {.bytes = {
    //             0xfe, 0x80,  0x00, 0x00,
    //             0x00, 0x00,  0x00, 0x00,
    //             0x8e, 0x8b,  0x48, 0xff,
    //             0xfe, 0x33,  0xe7, 0x88,
    //         }},
    //         .gateway = {.bytes = {
    //             0xfe, 0x80,  0x00, 0x00,
    //             0x00, 0x00,  0x00, 0x00,
    //             0x00, 0x00,  0x00, 0x00,
    //             0x00, 0x00,  0x00, 0x01,
    //         }},
    //     },
    // };
    // sl_status_t status = sl_net_get_ip_address(SL_NET_WIFI_CLIENT_INTERFACE, &sl_addresses, FuriWaitForever);

    sl_net_wifi_client_profile_t profile;
    sl_status_t status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);

    FURI_LOG_I("ifaddrs", "sl_net_get_profile status=%x", status);
    if(status != SL_STATUS_OK) {
        *ifap = NULL;
        return -1;
    }

    size_t addr_count = 0;
    // if(sl_addresses.type & SL_IPV4) addr_count++;
    // if(sl_addresses.type & SL_IPV6) addr_count++;
    addr_count = 1;
    if(!addr_count) {
        *ifap = NULL;
        return -1;
    }

    struct sockaddr_storage sockaddr_temp;

    // the entire linked list is contained in two allocations
    size_t sockaddr_count = addr_count * 2;
    struct sockaddr_storage* output_sockaddrs = malloc(sizeof(struct sockaddr_storage) * sockaddr_count);
    size_t sockaddr_pos = 0;
    struct ifaddrs* output_buf = malloc(sizeof(struct ifaddrs) * addr_count);
    size_t buf_pos = 0;

    // if(sl_addresses.type & SL_IPV4) {
    if(profile.ip.type & SL_IPV4) {
        output_buf[buf_pos].ifa_name = NET_IF_SIWX917_INTERFACE_NAME;
        output_buf[buf_pos].ifa_flags = 0;
        // if_addrs_sl_addr_to_sockaddr(&sl_addresses.v4, sizeof(sl_ipv4_address_t), &sockaddr_temp);
        if_addrs_sl_addr_to_sockaddr(&profile.ip.ip.v4, sizeof(sl_ipv4_address_t), &sockaddr_temp);
        output_buf[buf_pos].ifa_addr = if_addrs_getifaddrs_sockaddr_copy(output_sockaddrs, &sockaddr_pos, &sockaddr_temp);
        // if_addrs_sl_addr_to_sockaddr(&sl_addresses.v4, sizeof(sl_ipv4_address_t), &sockaddr_temp);
        // output_buf[buf_pos].ifa_netmask = if_addrs_getifaddrs_sockaddr_copy(output_sockaddrs, &sockaddr_pos, &sockaddr_temp);
        output_buf[buf_pos].ifa_netmask = NULL;
        if_addrs_getifaddrs_link_addr(output_buf, &buf_pos);
    }
    if(profile.ip.type & SL_IPV6) {
    // if(sl_addresses.type & SL_IPV6) {
        output_buf[buf_pos].ifa_name = NET_IF_SIWX917_INTERFACE_NAME;
        output_buf[buf_pos].ifa_flags = 0;
        // if_addrs_sl_addr_to_sockaddr(&sl_addresses.v6, sizeof(sl_ipv6_address_t), &sockaddr_temp);
        if_addrs_swap_byteorder(&profile.ip.ip.v6);
        if_addrs_sl_addr_to_sockaddr(&profile.ip.ip.v6, sizeof(sl_ipv6_address_t), &sockaddr_temp);
        output_buf[buf_pos].ifa_addr = if_addrs_getifaddrs_sockaddr_copy(output_sockaddrs, &sockaddr_pos, &sockaddr_temp);
        if_addrs_sl_addr_to_sockaddr(if_addrs_default_v6_netmask, sizeof(sl_ipv6_address_t), &sockaddr_temp);
        output_buf[buf_pos].ifa_netmask = if_addrs_getifaddrs_sockaddr_copy(output_sockaddrs, &sockaddr_pos, &sockaddr_temp);
        if_addrs_getifaddrs_link_addr(output_buf, &buf_pos);
    }

    furi_assert(buf_pos == addr_count);
    *ifap = output_buf;
    return 0;
}

void freeifaddrs(struct ifaddrs* ifp) {
    if(!ifp) return;
    // the entire linked list is contained in two allocations
    free(ifp->ifa_addr);
    free(ifp);
}
