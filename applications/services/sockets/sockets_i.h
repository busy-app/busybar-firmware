#pragma once

#include "sockets.h"
#include "sockets_common_i.h"

#include <furi.h>
#include <api_lock.h>

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
} SocketSrvBindMessage;

typedef struct {
    const uint8_t socket_id;
    const uint8_t max_clients;
} SocketSrvListenMessage;

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
    const uint8_t socket_id;
    void* data;
    const size_t data_size;
    size_t* received_size;
} SocketSrvReceiveMessage;

typedef struct {
    SocketRequestType request_type;
    SocketStatus status;
    union {
        SocketSrvAllocMessage alloc_message;
        SocketSrvFreeMessage free_message;
        SocketSrvBindMessage bind_message;
        SocketSrvListenMessage listen_message;
        SocketSrvConnectMessage connect_message;
        SocketSrvSendMessage send_message;
        SocketSrvReceiveMessage receive_message;
    };
    FuriApiLock lock;
} SocketSrvMessage;

typedef enum {
    SocketSrvEventRequest = 1UL << 0,
} SocketSrvEvent;

struct Socket {
    uint8_t id;
    SocketSrv* owner;
    SocketEventCallback event_callback;
    void* callback_context;
};

struct SocketSrv {
    FuriEventLoop* event_loop;
    FuriSemaphore* access_semaphore;
    SocketSrvMessage* current_message;
    Intercom* intercom;
    SocketRequest request;
    Socket* sockets[SOCKET_COUNT];
};
