#pragma once

#include "sockets_common_i.h"

#include <furi.h>

#include <intercom/intercom.h>

#define TAG "SocketSrv"

typedef struct {
    struct sockaddr* addr;
    socklen_t* addrlen;
} SocketAcceptParams;

typedef struct {
    void* mem;
    struct sockaddr* from;
    socklen_t* fromlen;
} SocketRecvParams;

typedef struct {
    struct sockaddr* name;
    socklen_t* namelen;
} SocketGetSockNameParams;

typedef struct {
    struct sockaddr* name;
    socklen_t* namelen;
} SocketGetPeerNameParams;

typedef struct {
    void* optval;
    socklen_t* optlen;
} SocketGetSockOptParams;

typedef struct {
    fd_set* readset;
    fd_set* writeset;
    fd_set* exceptset;
} SocketSelectParams;

typedef struct {
    SocketRequestType request_type;
    union {
        SocketAcceptParams accept_params;
        SocketRecvParams recv_params;
        SocketGetSockNameParams getsockname_params;
        SocketGetPeerNameParams getpeername_params;
        SocketGetSockOptParams getsockopt_params;
        SocketSelectParams select_params;
    };
} SocketReturnParams;

struct SocketSrv {
    FuriSemaphore* access_semaphore;
    FuriSemaphore* response_semaphore;
    Intercom* intercom;
    SocketRequest request;
    SocketReturnParams ret;
    ssize_t status;
    int saved_errno;
};
