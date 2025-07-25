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
    char tmp_buf[MAX_CHUNK_SIZE];
} SocketsTestApp;

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
        lwip_inet_pton(AF_INET, "10.46.30.137", &connect_name.sin_addr);

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
        UNUSED(sockets_test_tcp_server);
        if(!sockets_test_tcp_server(instance)) {
            break;
        }

    } while(false);

    sockets_test_app_free(instance);
    return 0;
}
