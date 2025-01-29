#pragma once

#include <sl_si91x_socket_types.h>

#include "sockets_common.h"

typedef struct {
    union {
        struct sockaddr address;
        struct sockaddr_in address4;
        struct sockaddr_in6 address6;
    };
    socklen_t length;
} SocketSlAddress;

void sockets_connection_info_to_sl_address(
    const SocketConnectionInfo* connection_info,
    SocketSlAddress* sl_address);

void sockets_sockaddr_to_connection_info(
    const struct sockaddr* sock_addr,
    SocketConnectionInfo* connection_info);

int32_t sockets_get_parent(int32_t socket);
