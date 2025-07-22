#pragma once

#include "sockets_common_i.h"

#include <furi.h>

#include <intercom/intercom.h>

#define TAG "SocketSrv"

typedef struct {
    const int domain;
    const int type;
    const int protocol;
    int* fd;
} SocketSrvAllocMessage;

typedef struct {
    const struct sockaddr* const name;
    const socklen_t namelen;
} SocketSrvBindMessage;

typedef struct {
    const int backlog;
} SocketSrvListenMessage;

typedef struct {
    struct sockaddr* addr;
    socklen_t* addrlen;
} SocketSrvAcceptMessage;

typedef struct {
    const struct sockaddr* const name;
    const socklen_t namelen;
} SocketSrvConnectMessage;

typedef struct {
    const void* const dataptr;
    const size_t size;
    const int flags;
    const struct sockaddr* const to;
    const socklen_t tolen;
} SocketSrvSendMessage;

typedef struct {
    void* const mem;
    const size_t len;
    const int flags;
    struct sockaddr* const from;
    socklen_t* const fromlen;
} SocketSrvReceiveMessage;

typedef struct {
    struct sockaddr* const name;
    socklen_t* const namelen;
} SocketSrvGetSockNameMessage;

typedef struct {
    struct sockaddr* const name;
    socklen_t* const namelen;
} SocketSrvGetPeerNameMessage;

typedef struct {
    const int level;
    const int optname;
    void* const optval;
    socklen_t* const optlen;
} SocketSrvGetSockOptMessage;

typedef struct {
    const int level;
    const int optname;
    const void* const optval;
    const socklen_t optlen;
} SocketSrvSetSockOptMessage;

typedef struct {
    const SocketRequestType request_type;
    const int socket_fd;
    ssize_t status;
    union {
        SocketSrvAllocMessage alloc_message;
        SocketSrvBindMessage bind_message;
        SocketSrvListenMessage listen_message;
        SocketSrvAcceptMessage accept_message;
        SocketSrvConnectMessage connect_message;
        SocketSrvSendMessage send_message;
        SocketSrvReceiveMessage receive_message;
        SocketSrvGetSockNameMessage getsockname_message;
        SocketSrvGetPeerNameMessage getpeername_message;
        SocketSrvGetSockOptMessage getsockopt_message;
        SocketSrvSetSockOptMessage setsockopt_message;
    };
    // FuriApiLock lock;
} SocketSrvMessage;

// TODO: Concept testing

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
    SocketRequestType request_type;
    union {
        SocketAcceptParams accept_params;
        SocketRecvParams recv_params;
        SocketGetSockNameParams getsockname_params;
        SocketGetPeerNameParams getpeername_params;
        SocketGetSockOptParams getsockopt_params;
    };
} SocketReturnParams;

struct SocketSrv {
    FuriSemaphore* access_semaphore;
    FuriSemaphore* response_semaphore;
    Intercom* intercom;
    SocketSrvMessage* current_message;
    SocketRequest request;
    SocketReturnParams ret;
    ssize_t status;
};
