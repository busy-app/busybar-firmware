#pragma once

#include "sockets.h"
#include "sockets_common_i.h"

#include <furi.h>
#include <intercom/intercom.h>

#define SOCKET_COUNT (20)

#define TAG "SocketSrv"

typedef struct {
    const SocketInfo* socket_info;
    Socket* socket;
} SocketSrvAllocMessage;

typedef struct {
    const uint8_t socket_id;
} SocketSrvFreeMessage;

typedef struct {
    const uint8_t socket_id;
    const SocketConnectionInfo* bind_info;
} SocketSrvAcceptMessage;

typedef struct {
    const uint8_t socket_id;
    const SocketConnectionInfo* connection_info;
} SocketSrvConnectMessage;

typedef struct {
    const uint8_t socket_id;
    const void* data;
    const size_t data_size;
    size_t* sent_size;
} SocketSrvSendMessage;

typedef struct {
    SocketRequestType request_type;
    SocketStatus status;
    union {
        SocketSrvAllocMessage alloc_message;
        SocketSrvFreeMessage free_message;
        SocketSrvAcceptMessage accept_message;
        SocketSrvConnectMessage connect_message;
        SocketSrvSendMessage send_message;
    };
} SocketSrvMessage;

typedef enum {
    SocketSrvEventRequest = 1UL << 0,
    SocketSrvEventResponse = 1UL << 1,
    SocketSrvEventAsyncResponse = 1UL << 2,
} SocketSrvEvent;

typedef enum {
    SocketSrvFlagReady = 1UL << 0,
    SocketSrvFlagDone = 1UL << 1,
} SocketSrvFlag;

struct Socket {
    uint8_t id;
    SocketSrv* owner;
    SocketEventCallback event_callback;
    void* callback_context;
};

struct SocketSrv {
    FuriEventLoop* event_loop;
    FuriEventFlag* event_flag;
    Intercom* intercom;
    SocketSrvMessage* message;
    SocketRequest request;
    SocketResponse response[SocketChannelMax];
    Socket* sockets[SOCKET_COUNT];
};
