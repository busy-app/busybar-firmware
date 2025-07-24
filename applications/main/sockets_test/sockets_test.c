#include <furi.h>

#include <wifi/wifi.h>
#include <sockets/sockets.h>

#define TAG "SocketsTestApp"

#define MAX_CHUNK_SIZE (1014) // Limited by intercom et al

#define CONNECT_PORT  (8080)
#define LISTEN_PORT   (8081)
#define UDP_BIND_PORT (8082)
#define UDP_SEND_PORT (8083)

#ifndef SOCKETS_TEST_ADDRESS
#define SOCKETS_TEST_ADDRESS 10, 46, 30, 143 // MUST be commas, not dots
#endif
#define BIND_ADDRESS 0, 0, 0, 0

#define MAX_TCP_CLIENTS 1

typedef enum {
    SocketIndexTcpClient,
    SocketIndexTcpServer,
    SocketIndexTcpRemoteClient,
    SocketIndexUdpServer,
    SocketIndexMax,
} SocketIndex;

typedef struct {
    Wifi* wifi;
    // FuriEventLoop* event_loop;
    // FuriMessageQueue* event_queue;
    // Socket* sockets[SocketIndexMax];
    char tmp_buf[MAX_CHUNK_SIZE];
} SocketsTestApp;

// static void socket_event_callback(const SocketEvent* event, void* context) {
//     furi_assert(event);
//     furi_assert(context);
//
//     SocketsTestApp* instance = context;
//
//     furi_check(
//         furi_message_queue_put(instance->event_queue, event, FuriWaitForever) == FuriStatusOk);
// }

// static void
//     sockets_test_app_print_connection_info(const char* message, const SocketConnectionInfo* info) {
//     FURI_LOG_I(
//         TAG,
//         "%s %hhu.%hhu.%hhu.%hhu:%hu",
//         message,
//         info->address.v4[0],
//         info->address.v4[1],
//         info->address.v4[2],
//         info->address.v4[3],
//         info->port);
// }

// static bool sockets_test_app_get_wifi_info(SocketsTestApp* instance) {
//     bool success = false;
//
//     do {
//         WifiInfo wifi_info;
//
//         if(wifi_get_info(instance->wifi, &wifi_info) != WifiStatusOk) {
//             FURI_LOG_E(TAG, "Failed to get Wifi info");
//             break;
//         }
//
//         if(wifi_info.state != WifiStateUp) {
//             FURI_LOG_E(TAG, "Wifi is DOWN, please connect to a network first");
//             break;
//         }
//
//         FURI_LOG_I(TAG, "Wifi is connected to SSID %s", wifi_info.ssid);
//
//         const WifiIpv4 address = wifi_info.ip_config.ip4.address;
//
//         FURI_LOG_I(
//             TAG,
//             "Device IP address: %hhu.%hhu.%hhu.%hhu",
//             address.bytes[0],
//             address.bytes[1],
//             address.bytes[2],
//             address.bytes[3]);
//
//         success = true;
//
//     } while(false);
//
//     return success;
// }
//
// static bool sockets_test_app_init_tcp_client(SocketsTestApp* instance) {
//     bool success = false;
//
//     do {
//         FURI_LOG_I(TAG, "TCP client initialisation start ...");
//
//         const SocketInfo socket_info = {
//             .ip_type = SocketIpTypeV4,
//             .protocol = SocketProtocolTcp,
//         };
//
//         Socket* socket;
//         Socket** socket_slot = &instance->sockets[SocketIndexTcpClient];
//
//         *socket_slot =
//             socket_alloc(instance->sockets_srv, &socket_info, socket_event_callback, instance);
//         socket = *socket_slot;
//
//         if(socket == NULL) {
//             FURI_LOG_E(TAG, "Failed to allocate client socket");
//             break;
//         }
//
//         const SocketConnectionInfo connection_info = {
//             .port = CONNECT_PORT,
//             .ip_type = SocketIpTypeV4,
//             .address.v4 = {SOCKETS_TEST_ADDRESS},
//         };
//
//         sockets_test_app_print_connection_info(
//             "Client socket allocated successfully, connecting to", &connection_info);
//
//         if(socket_connect(socket, &connection_info) != SocketStatusOk) {
//             FURI_LOG_E(TAG, "Connection failed");
//             break;
//         }
//
//         sockets_test_app_print_connection_info("Connected to", &connection_info);
//
//         success = true;
//
//     } while(false);
//
//     return success;
// }
//
// static bool sockets_test_app_init_tcp_server(SocketsTestApp* instance) {
//     bool success = false;
//
//     do {
//         FURI_LOG_I(TAG, "TCP server initialisation start ...");
//
//         const SocketInfo socket_info = {
//             .ip_type = SocketIpTypeV4,
//             .protocol = SocketProtocolTcp,
//         };
//
//         Socket* socket;
//         Socket** socket_slot = &instance->sockets[SocketIndexTcpServer];
//
//         *socket_slot =
//             socket_alloc(instance->sockets_srv, &socket_info, socket_event_callback, instance);
//         socket = *socket_slot;
//
//         if(socket == NULL) {
//             FURI_LOG_E(TAG, "Failed to allocate Server socket");
//             break;
//         }
//
//         FURI_LOG_I(TAG, "Server socket allocated successfully!");
//
//         const SocketConnectionInfo bind_info = {
//             .port = LISTEN_PORT,
//             .ip_type = SocketIpTypeV4,
//             .address.v4 = {BIND_ADDRESS},
//         };
//
//         if(socket_bind(socket, &bind_info) != SocketStatusOk) {
//             FURI_LOG_E(TAG, "Failed to bind socket");
//             break;
//         }
//
//         FURI_LOG_I(TAG, "Server socket bound successfully!");
//
//         if(socket_listen(socket, MAX_TCP_CLIENTS) != SocketStatusOk) {
//             FURI_LOG_E(TAG, "Failed to listen on socket");
//             break;
//         }
//
//         FURI_LOG_I(TAG, "Listening on port %hu", bind_info.port);
//
//         if(socket_accept(socket) != SocketStatusOk) {
//             FURI_LOG_E(TAG, "Failed to accept on socket");
//             break;
//         }
//
//         FURI_LOG_I(TAG, "Accepting new client connections");
//
//         success = true;
//
//     } while(false);
//
//     return success;
// }
//
// static bool sockets_test_app_init_udp_server(SocketsTestApp* instance) {
//     bool success = false;
//
//     do {
//         FURI_LOG_I(TAG, "UDP server initialisation start ...");
//
//         const SocketInfo socket_info = {
//             .ip_type = SocketIpTypeV4,
//             .protocol = SocketProtocolUdp,
//         };
//
//         Socket* socket;
//         Socket** socket_slot = &instance->sockets[SocketIndexUdpServer];
//
//         *socket_slot =
//             socket_alloc(instance->sockets_srv, &socket_info, socket_event_callback, instance);
//         socket = *socket_slot;
//
//         if(socket == NULL) {
//             FURI_LOG_E(TAG, "Failed to allocate UDP server socket");
//             break;
//         }
//
//         FURI_LOG_I(TAG, "UDP server socket allocated successfully!");
//
//         const SocketConnectionInfo bind_info = {
//             .port = UDP_BIND_PORT,
//             .ip_type = SocketIpTypeV4,
//             .address.v4 = {BIND_ADDRESS},
//         };
//
//         if(socket_bind(socket, &bind_info) != SocketStatusOk) {
//             FURI_LOG_E(TAG, "Failed to bind socket");
//             break;
//         }
//
//         FURI_LOG_I(TAG, "UDP server socket bound successfully!");
//
//         const SocketConnectionInfo connection_info = {
//             .port = UDP_SEND_PORT,
//             .ip_type = SocketIpTypeV4,
//             .address.v4 = {SOCKETS_TEST_ADDRESS},
//         };
//
//         // For UDP sockets, connect() sets the address used for send()
//         if(socket_connect(socket, &connection_info) != SocketStatusOk) {
//             FURI_LOG_E(TAG, "Connection failed");
//             break;
//         }
//
//         sockets_test_app_print_connection_info("UDP destination set to", &connection_info);
//
//         success = true;
//
//     } while(false);
//
//     return success;
// }
//
// static void sockets_test_app_deinit_sockets(SocketsTestApp* instance) {
//     for(uint32_t i = 0; i < SocketIndexMax; ++i) {
//         Socket* socket = instance->sockets[i];
//         if(socket) {
//             if(socket_free(socket) != SocketStatusOk) {
//                 FURI_LOG_E(TAG, "Failed to free socket");
//             }
//         }
//     }
//
//     furi_record_close(RECORD_SOCKETS);
// }
//
// static void sockets_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
//     SocketsTestApp* instance = context;
//     furi_assert(object == instance->event_queue);
//
//     SocketEvent event;
//     furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);
//
//     const SocketEventType event_type = event.type;
//
//     if(event_type == SocketEventTypeReceive) {
//         SocketStatus status;
//
//         do {
//             size_t data_size;
//             status = socket_receive(event.socket, instance->tmp_buf, MAX_CHUNK_SIZE, &data_size);
//
//             if(status != SocketStatusOk) {
//                 FURI_LOG_E(TAG, "Failed to receive the data");
//                 break;
//             }
//
//             status = socket_send(event.socket, instance->tmp_buf, data_size, &data_size);
//
//             if(status != SocketStatusOk) {
//                 FURI_LOG_E(TAG, "Failed to send %zu bytes", data_size);
//                 break;
//             }
//
//         } while(false);
//
//         if(status != SocketStatusOk) {
//             furi_event_loop_stop(instance->event_loop);
//         }
//
//     } else if(event_type == SocketEventTypeAccept) {
//         const SocketAcceptEvent* accept_event = &event.accept;
//         const SocketConnectionInfo* connection_info = &accept_event->connection_info;
//
//         sockets_test_app_print_connection_info("Accepting new connection from", connection_info);
//
//         Socket* client_socket = accept_event->client_socket;
//         instance->sockets[SocketIndexTcpRemoteClient] = client_socket;
//
//     } else if(event_type == SocketEventTypeClose) {
//         FURI_LOG_I(TAG, "Socket closed!");
//         furi_event_loop_stop(instance->event_loop);
//     }
// }

static void sockets_test_print_sockaddr(
    const struct sockaddr* name,
    socklen_t namelen,
    const char* message) {
    char buf[32] = {0};

    in_port_t port;

    if(name->sa_family == AF_INET) {
        const struct sockaddr_in* name4 = (struct sockaddr_in*)name;
        lwip_inet_ntop(AF_INET, &name4->sin_addr, buf, namelen);
        port = ntohs(name4->sin_port);

    } else if(name->sa_family == AF_INET6) {
        furi_crash("Ipv6 not implemented");
    } else {
        furi_crash();
    }

    FURI_LOG_I(TAG, "%s%s:%hu", message, buf, port);
}

static bool sockets_test_tcp_client(SocketsTestApp* instance) {
    UNUSED(instance);

    bool success = false;

    do {
        const int client_socket = sl_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if(client_socket < 0) {
            FURI_LOG_E(TAG, "Failed to create client socket");
            break;
        }

        int status;

        struct sockaddr_in connect_name;
        connect_name.sin_family = AF_INET;
        connect_name.sin_port = htons(CONNECT_PORT);
        lwip_inet_pton(AF_INET, "10.46.30.158", &connect_name.sin_addr);

        status = sl_connect(client_socket, (struct sockaddr*)&connect_name, sizeof(connect_name));

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to connect client socket");
            break;
        }

        const char* message = "Hello from TCP client!\r\n";

        status = sl_send(client_socket, message, strlen(message), 0);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to send data");
            break;
        }

        struct sockaddr name;
        socklen_t namelen;

        status = sl_recvfrom(
            client_socket, instance->tmp_buf, sizeof(instance->tmp_buf), 0, &name, &namelen);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to receive data");
            break;
        }

        FURI_LOG_I(TAG, "Received data (%zd bytes): %.*s", status, status, instance->tmp_buf);
        sockets_test_print_sockaddr(&name, namelen, "From: ");

        status = sl_getsockname(client_socket, &name, &namelen);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to get socket name");
            break;
        }

        sockets_test_print_sockaddr(&name, namelen, "getsockname(): ");

        status = sl_getpeername(client_socket, &name, &namelen);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to get peer name");
            break;
        }

        sockets_test_print_sockaddr(&name, namelen, "gepeername(): ");

        status = sl_close(client_socket);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to close client socket");
            break;
        }

        success = true;

    } while(false);

    return success;
}

static bool sockets_test_tcp_server(SocketsTestApp* instance) {
    UNUSED(instance);

    bool success = false;

    do {
        const int server_socket = sl_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if(server_socket < 0) {
            FURI_LOG_E(TAG, "Failed to create server socket");
            break;
        }

        int status;

        const struct sockaddr_in listen_name = {
            .sin_family = AF_INET,
            .sin_port = htons(LISTEN_PORT),
        };

        status = sl_bind(server_socket, (struct sockaddr*)&listen_name, sizeof(listen_name));

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to bind server socket");
            break;
        }

        status = sl_listen(server_socket, MAX_TCP_CLIENTS);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to listen on server socket");
            break;
        }

        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(server_socket, &readset);

        struct timeval timeout = {
            .tv_sec = 20,
        };

        status = sl_select(server_socket + 1, &readset, NULL, NULL, &timeout);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to select");
            break;
        }

        FURI_LOG_D(TAG, "Select OK");
        break;

        struct sockaddr remote_name;
        socklen_t remote_namelen = sizeof(remote_name);

        status = sl_accept(server_socket, &remote_name, &remote_namelen);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to accept");
            break;
        }

        sockets_test_print_sockaddr(&remote_name, remote_namelen, "Accepted connection from: ");

        const int remote_client_socket = status;

        const char* message = "Hello from TCP server!\r\n";

        status = sl_send(remote_client_socket, message, strlen(message), 0);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to send to remote client");
            break;
        }

        status = sl_close(remote_client_socket);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to close remote client socket");
            break;
        }

        status = sl_close(server_socket);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to close server socket");
            break;
        }

        success = true;

    } while(false);

    return success;
}

static SocketsTestApp* sockets_test_app_alloc(void) {
    SocketsTestApp* instance = malloc(sizeof(SocketsTestApp));

    instance->wifi = furi_record_open(RECORD_WIFI);

    return instance;
}

static void sockets_test_app_free(SocketsTestApp* instance) {
    furi_record_close(RECORD_WIFI);
    free(instance);
}

int32_t sockets_test_app(void* arg) {
    UNUSED(arg);

    SocketsTestApp* instance = sockets_test_app_alloc();

    do {
        UNUSED(sockets_test_tcp_client);
        // if(!sockets_test_tcp_client(instance)) {
        //     break;
        // }
        if(!sockets_test_tcp_server(instance)) {
            break;
        }

    } while(false);

    sockets_test_app_free(instance);
    return 0;
}
