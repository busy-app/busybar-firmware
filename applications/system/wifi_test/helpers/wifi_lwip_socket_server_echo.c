
#include "wifi_lwip_socket_server_echo.h"
#include <lwip/sockets.h>

// #include <sl_net.h>

// #include <sl_si91x_socket.h>
// #include <sl_si91x_socket_constants.h>
// #include <sl_si91x_socket_utility.h>

// #include "errno.h"

#define TAG "WifiAsyncSocketServerEcho"

typedef struct {
    uint8_t exit;
    uint8_t first_data_frame;
    uint32_t start;
    uint32_t now;
    uint32_t bytes_read;
    FuriString* msg;
    WifiTestApp* app;
} WifiAsyncSocketServerEcho;

WifiAsyncSocketServerEcho* wifi_lwip_socket_echo_header = NULL;

// void wifi_lwip_socket_server_echo_data_callback(
//     uint32_t sock_no,
//     uint8_t* buffer,
//     uint32_t length,
//     const sl_si91x_socket_metadata_t* firmware_socket_response) {
//     UNUSED_PARAMETER(buffer);
//     UNUSED_PARAMETER(firmware_socket_response);

//     furi_string_printf(wifi_lwip_socket_echo_header->msg, "Client Socket ID : %ld\r\n", sock_no);
//     wifi_test_app_send_text(
//         wifi_lwip_socket_echo_header->app, wifi_lwip_socket_echo_header->msg);
//     furi_string_printf(wifi_lwip_socket_echo_header->msg, "Data received : %ld\r\n", length);
//     wifi_test_app_send_text(
//         wifi_lwip_socket_echo_header->app, wifi_lwip_socket_echo_header->msg);
//     furi_string_printf(wifi_lwip_socket_echo_header->msg, "Data : %s\r\n", buffer);
//     wifi_test_app_send_text(
//         wifi_lwip_socket_echo_header->app, wifi_lwip_socket_echo_header->msg);

//     int16_t sent_bytes = send(sock_no, buffer, length, 0);
//     if(sent_bytes < 0) {
//         furi_string_printf(
//             wifi_lwip_socket_echo_header->msg, "Send failed with BSD error:%d\r\n", errno);
//         wifi_test_app_send_text(
//             wifi_lwip_socket_echo_header->app, wifi_lwip_socket_echo_header->msg);
//         wifi_lwip_socket_echo_header->exit = 1;
//     } else {
//         wifi_lwip_socket_echo_header->bytes_read += length;
//     }
// }

#define buf_size 1024*4
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

void wifi_lwip_socket_server_echo_init(WifiTestApp* app, FuriString* msg, uint16_t port) {
    wifi_lwip_socket_echo_header =
        (WifiAsyncSocketServerEcho*)malloc(sizeof(WifiAsyncSocketServerEcho));

    wifi_lwip_socket_echo_header->exit = 0;
    wifi_lwip_socket_echo_header->first_data_frame = 1;
    wifi_lwip_socket_echo_header->start = 0;
    wifi_lwip_socket_echo_header->now = furi_get_tick();
    wifi_lwip_socket_echo_header->bytes_read = 0;
    wifi_lwip_socket_echo_header->msg = msg;
    wifi_lwip_socket_echo_header->app = app;

    int32_t listen_fd;
    struct sockaddr_in address;
    furi_delay_ms(1000);
    FURI_LOG_I(TAG, "Started");

    // Create a socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(listen_fd < 0) {
        furi_crash("socket() failed");
    }

    // Set up the address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
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
    while (1)
    {
        sent=recv(cli_socket.client_socket, &buffer, buf_size, 0);
        sent ++;
        //send(cli_socket.client_socket, &buffer, sent, 0);

        // furi_string_printf(msg, "DATA: %s\r\n", buffer);
        // wifi_test_app_send_text(app, msg);
    }
    
    



    free(wifi_lwip_socket_echo_header);
}
