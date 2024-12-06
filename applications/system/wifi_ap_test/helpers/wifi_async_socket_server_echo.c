
#include "wifi_async_socket_server_echo.h"

#include <sl_net.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_constants.h>
#include <sl_si91x_socket_utility.h>

#include "errno.h"

#define TAG                        "WifiAsyncSocketServerEcho"
#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)
#define BACK_LOG                   1

#define TEST_TIMEOUT     (90000) //90sec
#define BYTES_TO_RECEIVE (1 << 28) //256MB

#define TOTAL_SOCKETS                   1 //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS               1 //@ Total TCP sockets. TCP TX + TCP RX
#define TOTAL_UDP_SOCKETS               0 //@ Total UDP sockets. UDP TX + UDP RX
#define TCP_TX_ONLY_SOCKETS             0 //@ Total TCP TX only sockets. TCP TX
#define TCP_RX_ONLY_SOCKETS             1 //@ Total TCP RX only sockets. TCP RX
#define UDP_TX_ONLY_SOCKETS             0 //@ Total UDP TX only sockets. UDP TX
#define UDP_RX_ONLY_SOCKETS             0 //@ Total UDP RX only sockets. UDP RX
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 1 //@ Total TCP RX High Performance sockets
#define TCP_RX_WINDOW_SIZE_CAP          44 //@ TCP RX Window size
#define TCP_RX_WINDOW_DIV_FACTOR        44 //@ TCP RX Window division factor

static const sl_si91x_socket_config_t socket_config = {
    TOTAL_SOCKETS, // Total sockets
    TOTAL_TCP_SOCKETS, // Total TCP sockets
    TOTAL_UDP_SOCKETS, // Total UDP sockets
    TCP_TX_ONLY_SOCKETS, // TCP TX only sockets
    TCP_RX_ONLY_SOCKETS, // TCP RX only sockets
    UDP_TX_ONLY_SOCKETS, // UDP TX only sockets
    UDP_RX_ONLY_SOCKETS, // UDP RX only sockets
    TCP_RX_HIGH_PERFORMANCE_SOCKETS, // TCP RX high performance sockets
    TCP_RX_WINDOW_SIZE_CAP, // TCP RX window size
    TCP_RX_WINDOW_DIV_FACTOR // TCP RX window division factor
};

typedef struct {
    uint8_t exit;
    uint8_t first_data_frame;
    uint32_t start;
    uint32_t now;
    uint32_t bytes_read;
    FuriString* msg;
    WifiTestApp* app;
} WifiAsyncSocketServerEcho;

WifiAsyncSocketServerEcho* wifi_async_socket_echo_header = NULL;

void wifi_async_socket_server_echo_data_callback(
    uint32_t sock_no,
    uint8_t* buffer,
    uint32_t length,
    const sl_si91x_socket_metadata_t* firmware_socket_response) {
    UNUSED_PARAMETER(buffer);
    UNUSED_PARAMETER(firmware_socket_response);

    furi_string_printf(wifi_async_socket_echo_header->msg, "Client Socket ID : %ld\r\n", sock_no);
    wifi_test_app_send_text(
        wifi_async_socket_echo_header->app, wifi_async_socket_echo_header->msg);
    furi_string_printf(wifi_async_socket_echo_header->msg, "Data received : %ld\r\n", length);
    wifi_test_app_send_text(
        wifi_async_socket_echo_header->app, wifi_async_socket_echo_header->msg);
    furi_string_printf(wifi_async_socket_echo_header->msg, "Data : %s\r\n", buffer);
    wifi_test_app_send_text(
        wifi_async_socket_echo_header->app, wifi_async_socket_echo_header->msg);

    int16_t sent_bytes = send(sock_no, buffer, length, 0);
    if(sent_bytes < 0) {
        furi_string_printf(
            wifi_async_socket_echo_header->msg, "Send failed with BSD error:%d\r\n", errno);
        wifi_test_app_send_text(
            wifi_async_socket_echo_header->app, wifi_async_socket_echo_header->msg);
        wifi_async_socket_echo_header->exit = 1;
    } else {
        wifi_async_socket_echo_header->bytes_read += length;
    }
}

void wifi_async_socket_server_echo_init(WifiTestApp* app, FuriString* msg, uint16_t port) {
    wifi_async_socket_echo_header =
        (WifiAsyncSocketServerEcho*)malloc(sizeof(WifiAsyncSocketServerEcho));

    wifi_async_socket_echo_header->exit = 0;
    wifi_async_socket_echo_header->first_data_frame = 1;
    wifi_async_socket_echo_header->start = 0;
    wifi_async_socket_echo_header->now = furi_get_tick();
    wifi_async_socket_echo_header->bytes_read = 0;
    wifi_async_socket_echo_header->msg = msg;
    wifi_async_socket_echo_header->app = app;

    int server_socket = -1;
    int client_socket = -1;
    int socket_return_value = 0;
    struct sockaddr_in server_address = {0};
    socklen_t socket_length = sizeof(struct sockaddr_in);
    uint8_t high_performance_socket = SL_HIGH_PERFORMANCE_SOCKET;

    do {
        sl_status_t status = sl_si91x_config_socket(socket_config);
        if(status != SL_STATUS_OK) {
            furi_string_printf(msg, "Socket config failed: %ld\r\n", status);
            wifi_test_app_send_text(app, msg);
            break;
        }
        furi_string_printf(msg, "Socket config Done\r\n");
        wifi_test_app_send_text(app, msg);

        // Create Server socket
        server_socket = sl_si91x_socket_async(
            AF_INET, SOCK_STREAM, IPPROTO_TCP, &wifi_async_socket_server_echo_data_callback);
        if(server_socket < 0) {
            furi_string_printf(msg, "Socket create failed with BSD error: %d\r\n", errno);
            wifi_test_app_send_text(app, msg);
            break;
        }
        furi_string_printf(msg, "Server Socket ID : %d\r\n", server_socket);
        wifi_test_app_send_text(app, msg);
        //Set socket
        socket_return_value = sl_si91x_setsockopt_async(
            server_socket,
            SOL_SOCKET,
            SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET,
            &high_performance_socket,
            sizeof(high_performance_socket));
        if(socket_return_value < 0) {
            furi_string_printf(msg, "Set Socket option failed with BSD error: %d\r\n", errno);
            wifi_test_app_send_text(app, msg);
            close(server_socket);
            break;
        }
        server_address.sin_family = AF_INET;
        server_address.sin_port = port;

        // Bind socket
        socket_return_value =
            sl_si91x_bind(server_socket, (struct sockaddr*)&server_address, socket_length);
        if(socket_return_value < 0) {
            furi_string_printf(msg, "Socket bind failed with BSD error: %d\r\n", errno);
            wifi_test_app_send_text(app, msg);
            close(server_socket);
            break;
        }

        // Listen socket
        socket_return_value = sl_si91x_listen(server_socket, BACK_LOG);
        if(socket_return_value < 0) {
            furi_string_printf(msg, "Socket listen failed with BSD error: %d\r\n", errno);
            wifi_test_app_send_text(app, msg);
            close(server_socket);
            break;
        }
        furi_string_printf(msg, "Listening on Local Port : %d\r\n", port);
        wifi_test_app_send_text(app, msg);
        // Accept socket
        client_socket = sl_si91x_accept(server_socket, NULL, 0);
        if(client_socket < 0) {
            furi_string_printf(msg, "Socket accept failed with BSD error: %d\r\n", errno);
            wifi_test_app_send_text(app, msg);
            close(server_socket);
            break;
        }

        // Todo example async socket accept
        //     socket_return_value = sl_si91x_accept_async(server_socket, client_accept_callback);
        // if (socket_return_value != SI91X_NO_ERROR) {
        //   SL_DEBUG_LOG("\r\nSocket accept failed with bsd error: %d\r\n", errno);
        // }
        // static void client_accept_callback(
        //     int32_t sock_id, struct sockaddr * addr, uint8_t ip_version) {
        //     UNUSED_PARAMETER(addr);
        //     UNUSED_PARAMETER(ip_version);
        //     SL_DEBUG_LOG("\r\nAccepted socket ID : %ld\r\n", sock_id);
        //     client_socket = sock_id;
        //     osEventFlagsSet(server_handle->http_server_id, HTTP_SERVER_CONNECT_SUCCESS);
        // }
        uint32_t start = furi_get_tick();
        while(!wifi_async_socket_echo_header->exit && (furi_get_tick() - start) < TEST_TIMEOUT) {
            osThreadYield();
        }

        furi_string_printf(msg, "TCP_RX Throughput test finished\r\n");
        wifi_test_app_send_text(app, msg);
        furi_string_printf(
            msg, "Total bytes received : %ld\r\n", wifi_async_socket_echo_header->bytes_read);
        wifi_test_app_send_text(app, msg);
        // Close socket
        close(server_socket);
        close(client_socket);
    } while(0);

    free(wifi_async_socket_echo_header);
}
