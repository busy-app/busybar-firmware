
#include "wifi_lwip_socket_server_echo.h"
#include <lwip/sockets.h>

#define TAG "WifiAsyncSocketServerEcho"

typedef struct {
    bool exit;
    uint32_t port;
    FuriString* msg;
    WifiTestApp* app;
    FuriThread* thread;
} WifiAsyncSocketServerEcho;

WifiAsyncSocketServerEcho* wifi_lwip_socket_echo_header = NULL;

#define buf_size 1024 * 2
uint8_t buffer[buf_size];

typedef struct {
    int32_t client_socket;
    volatile bool connected;
    FuriEventFlag* evt_flags;

    bool soh_sent;
} CliSocket;

static CliSocket cli_socket = {
    .client_socket = -1,
    .connected = false,
    .evt_flags = NULL,
};

static int32_t wifi_lwip_socket_server_echo_thread_callback(void* context) {
    WifiAsyncSocketServerEcho* instance = (WifiAsyncSocketServerEcho*)context;
    int32_t listen_fd;
    struct sockaddr_in address;
    FURI_LOG_I(TAG, "Started");

    // Create a socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(listen_fd < 0) {
        furi_crash("socket() failed");
    }

    // Set up the address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(instance->port);
    address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address
    if(bind(listen_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        furi_crash("bind() failed");
    }

    // Listen for _one_ incoming connection
    if(listen(listen_fd, 1) < 0) {
        furi_crash("listen() failed");
    }

    cli_socket.client_socket = accept(listen_fd, NULL, NULL);
    if(cli_socket.client_socket < 0) {
        furi_crash("accept() failed");
    }
    size_t sent = 0;
    //Todo need to send 1 more package to exit
    while(!instance->exit) {
        sent = recv(cli_socket.client_socket, &buffer, buf_size, 0);
        send(cli_socket.client_socket, &buffer, sent, 0);

        furi_string_printf(instance->msg, "DATA: %s\r\n", buffer);
        wifi_test_app_send_text(instance->app, instance->msg);
    }
    close(cli_socket.client_socket);
    return 0;
}

void wifi_lwip_socket_server_echo_init(WifiTestApp* app, FuriString* msg, uint16_t port) {
    wifi_lwip_socket_echo_header =
        (WifiAsyncSocketServerEcho*)malloc(sizeof(WifiAsyncSocketServerEcho));

    wifi_lwip_socket_echo_header->exit = 0;
    wifi_lwip_socket_echo_header->port = port;
    wifi_lwip_socket_echo_header->msg = msg;
    wifi_lwip_socket_echo_header->app = app;

    wifi_lwip_socket_echo_header->thread = furi_thread_alloc_ex(
        "BLEPerTestShowStatus",
        2048,
        wifi_lwip_socket_server_echo_thread_callback,
        wifi_lwip_socket_echo_header);
    furi_thread_start(wifi_lwip_socket_echo_header->thread);
}

void wifi_lwip_socket_server_echo_deinit(void) {
    FURI_LOG_I(TAG, "Stopping");
    furi_check(wifi_lwip_socket_echo_header);
    WifiAsyncSocketServerEcho* instance = (WifiAsyncSocketServerEcho*)wifi_lwip_socket_echo_header;
    wifi_lwip_socket_echo_header->exit = true;
    if(instance->thread) {
        instance->exit = true;
        furi_thread_join(instance->thread);
        furi_thread_free(instance->thread);
        instance->thread = NULL;
    }
    free(wifi_lwip_socket_echo_header);
    wifi_lwip_socket_echo_header = NULL;
}
