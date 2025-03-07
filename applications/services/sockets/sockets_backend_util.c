#include "sockets_backend_util.h"

#include <sl_si91x_socket_utility.h>

#include <furi.h>

void sockets_connection_info_to_sl_address(
    const SocketConnectionInfo* connection_info,
    SocketSlAddress* sl_address) {
    if(connection_info->ip_type == SocketIpTypeV4) {
        struct sockaddr_in* address4 = &sl_address->address4;

        address4->sin_family = AF_INET;
        address4->sin_port = connection_info->port;

        memcpy(&address4->sin_addr, connection_info->address.v4, sizeof(address4->sin_addr));
        sl_address->length = sizeof(struct sockaddr_in);

    } else if(connection_info->ip_type == SocketIpTypeV6) {
        struct sockaddr_in6* address6 = &sl_address->address6;

        address6->sin6_family = AF_INET6;
        address6->sin6_port = connection_info->port;

        memcpy(&address6->sin6_addr, connection_info->address.v6, sizeof(address6->sin6_addr));
        sl_address->length = sizeof(struct sockaddr_in6);

    } else {
        furi_crash("Invalid IP version");
    }
}

void sockets_sockaddr_to_connection_info(
    const struct sockaddr* sock_addr,
    SocketConnectionInfo* connection_info) {
    memset(connection_info, 0, sizeof(SocketConnectionInfo));

    if(sock_addr->sa_family == AF_INET) {
        const struct sockaddr_in* address4 = (struct sockaddr_in*)sock_addr;

        connection_info->port = address4->sin_port;
        connection_info->ip_type = SocketIpTypeV4;
        memcpy(connection_info->address.v4, &address4->sin_addr, sizeof(address4->sin_addr));

    } else if(sock_addr->sa_family == AF_INET6) {
        const struct sockaddr_in6* address6 = (struct sockaddr_in6*)sock_addr;

        connection_info->port = address6->sin6_port;
        connection_info->ip_type = SocketIpTypeV6;

    } else {
        furi_crash("Invalid IP version");
    }
}

int32_t sockets_get_parent(int32_t socket) {
    int32_t parent = -1;

    sli_si91x_socket_t* socket_p = sli_si91x_sockets[socket];
    furi_assert(socket_p);

    for(uint32_t i = 0; i < NUMBER_OF_SOCKETS; ++i) {
        sli_si91x_socket_t* it = sli_si91x_sockets[i];

        if(!it || it->role != SI91X_SOCKET_TCP_SERVER) {
            continue;
        }

        const struct sockaddr_in6* addr1 = &socket_p->local_address;
        const struct sockaddr_in6* addr2 = &it->local_address;

        if(addr1->sin6_family != addr2->sin6_family) {
            continue;
        }

        socklen_t addr_len;

        if(addr1->sin6_family == AF_INET) {
            addr_len = sizeof(struct sockaddr_in);
        } else if(addr1->sin6_family == AF_INET6) {
            addr_len = sizeof(struct sockaddr_in6);
        } else {
            furi_crash("Invalid IP version");
        }

        if(memcmp(addr1, addr2, addr_len) == 0) {
            parent = it->index;
            break;
        }
    }

    return parent;
}
