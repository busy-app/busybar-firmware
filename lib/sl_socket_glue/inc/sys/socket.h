#pragma once

#include <stdint.h>
#include "socket_byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

#include_next <socket.h> // hopefully includes lib/wiseconnect/components/service/bsd_socket/inc/socket.h

#undef IPV_PKTINFO
#undef IPV6_PKTINFO

#define PF_UNSPEC AF_UNSPEC
#define PF_INET6  AF_INET6
#define PF_INET   AF_INET

// https://linux.die.net/man/2/recvmsg
// https://linux.die.net/man/2/sendmsg

struct msghdr {
    void         *msg_name;
    socklen_t     msg_namelen;
    struct iovec *msg_iov;
    size_t        msg_iovlen;
    void         *msg_control;
    size_t        msg_controllen;
    int           msg_flags;
};

typedef enum {
    MSG_DONTWAIT = (1 << 0),
} RecvMsgFlag;

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);

// https://linux.die.net/man/3/cmsg

struct cmsghdr {
    socklen_t cmsg_len;
    int       cmsg_level;
    int       cmsg_type;
};

#define CMSG_FIRSTHDR(_a) NULL
#define CMSG_NXTHDR(_a, _b) NULL

#ifdef __cplusplus
}
#endif
