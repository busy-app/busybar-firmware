#pragma once

#include <furi.h>
#include <net/if.h>

#ifdef __cplusplus
extern "C" {
#endif

// https://man7.org/linux/man-pages/man7/netdevice.7.html

#define IFNAMSZ IF_NAMESIZE

typedef enum {
    // Socket IOCtl - Get InterFace FLAGS?
    SIOCGIFFLAGS,
} SocketIoctlRequest;

typedef enum {
    IFF_UP = (1 << 0),
    IFF_LOOPBACK = (1 << 1),
    IFF_MULTICAST = (1 << 2),
} SocketIoctlFlag;

struct ifreq {
    char ifr_name[IFNAMSZ];
    union {
        short ifr_flags;
    };
};

int ioctl(int fd, unsigned long op, void* arg);

#ifdef __cplusplus
}
#endif
