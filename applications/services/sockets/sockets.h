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

typedef void (*SocketTxCallback)(Socket* socket, void* context);
typedef void (*SocketRxCallback)(Socket* socket, const void* data, size_t data_size, void* context);
// TODO: Close callback, error callback, etc

Socket* socket_alloc(Sockets* instance, const SocketInfo* info);

SocketStatus socket_free(Socket* socket);

SocketStatus socket_set_callback(
    Socket* socket,
    SocketTxCallback tx_callback,
    SocketRxCallback rx_callback,
    void* context);

SocketStatus socket_connect(Socket* socket, const SocketConnectionInfo* info);

SocketStatus socket_send(Socket* socket, const void* data, size_t max_size, size_t* size);

#ifdef __cplusplus
}
#endif
