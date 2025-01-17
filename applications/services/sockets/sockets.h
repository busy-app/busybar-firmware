/**
 * @file sockets.h
 */
#pragma once

#include "sockets_common.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_SOCKETS "sockets"

typedef struct Socket Socket;

typedef enum {
    SocketEventTypeSend,
    SocketEventTypeReceive,
    SocketEventTypeAccept,
    SocketEventTypeClose,
} SocketEventType;

typedef struct {
    SocketEventType type;
    union {
        uint16_t data_size;
        struct {
            Socket* client_socket;
            SocketConnectionInfo connection_info;
        } accept;
    };
} SocketEvent;

typedef void (*SocketEventCallback)(Socket* socket, const SocketEvent* event, void* context);

Socket* socket_alloc(Sockets* instance, const SocketInfo* socket_info);

SocketStatus socket_free(Socket* socket);

SocketStatus
    socket_set_event_callback(Socket* socket, SocketEventCallback callback, void* context);

SocketStatus socket_accept(Socket* socket, const SocketConnectionInfo* bind_info);

SocketStatus socket_connect(Socket* socket, const SocketConnectionInfo* connection_info);

SocketStatus socket_send(Socket* socket, const void* data, size_t data_size, size_t* sent_size);

SocketStatus socket_receive(Socket* socket, void* data, size_t data_size, size_t* received_size);

#ifdef __cplusplus
}
#endif
