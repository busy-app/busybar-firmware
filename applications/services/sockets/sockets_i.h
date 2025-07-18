#pragma once

#include "sockets.h"
#include "sockets_common_i.h"

#include <furi.h>
#include <api_lock.h>

#include <intercom/intercom.h>

#define TAG "SocketSrv"

typedef struct {
    const int domain;
    const int type;
    const int protocol;
    int* fd;
} SocketSrvAllocMessage;

typedef struct {
} SocketSrvCloseMessage;

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
        SocketSrvCloseMessage close_message;
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
    FuriApiLock lock;
} SocketSrvMessage;

typedef enum {
    SocketSrvEventRequest = 1UL << 0,
} SocketSrvEvent;

struct SocketSrv {
    FuriEventLoop* event_loop;
    FuriSemaphore* access_semaphore;
    SocketSrvMessage* current_message;
    Intercom* intercom;
    SocketRequest request;
};
