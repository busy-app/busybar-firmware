#pragma once

#include <furi.h>
#include "sys/socket.h"

#ifdef __cplusplus
extern "C" {
#endif

// https://linux.die.net/man/3/getifaddrs
struct ifaddrs {
    struct ifaddrs  *ifa_next;
    char            *ifa_name;
    unsigned int     ifa_flags;
    struct sockaddr *ifa_addr;
    struct sockaddr *ifa_netmask;
    union {
        struct sockaddr *ifu_broadaddr;
        struct sockaddr *ifu_dstaddr;
    } ifa_ifu;
    void            *ifa_data;
};

#define ifa_broadaddr ifa_ifu.ifu_broadaddr
#define ifa_dstaddr   ifa_ifu.ifu_dstaddr

#define IFF_BROADCAST   (1 << 0)
#define IFF_POINTOPOINT (1 << 1)

int getifaddrs(struct ifaddrs** ifap);
void freeifaddrs(struct ifaddrs* ifp);

#ifdef __cplusplus
}
#endif
