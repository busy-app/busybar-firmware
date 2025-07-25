/**
 * @file sockets.h
 * @brief Network communication API via BSD-style sockets.
 */
#pragma once

#include "sockets_common.h"

#ifdef __cplusplus
extern "C" {
#endif

int sl_socket(int domain, int type, int protocol);

int sl_bind(int s, const struct sockaddr* name, socklen_t namelen);

int sl_getsockname(int s, struct sockaddr* name, socklen_t* namelen);

int sl_connect(int s, const struct sockaddr* name, socklen_t namelen);

int sl_getpeername(int s, struct sockaddr* name, socklen_t* namelen);

ssize_t sl_send(int s, const void* dataptr, size_t size, int flags);

ssize_t sl_recv(int s, void* mem, size_t len, int flags);

ssize_t sl_sendto(
    int s,
    const void* dataptr,
    size_t size,
    int flags,
    const struct sockaddr* to,
    socklen_t tolen);

ssize_t
    sl_recvfrom(int s, void* mem, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen);

int sl_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen);

int sl_setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen);

int sl_listen(int s, int backlog);

int sl_accept(int s, struct sockaddr* addr, socklen_t* addrlen);

int sl_close(int s);

int sl_select(
    int maxfdp1,
    fd_set* readset,
    fd_set* writeset,
    fd_set* exceptset,
    struct timeval* timeout);

int sl_fcntl(int s, int cmd, int val);

#ifdef __cplusplus
}
#endif
