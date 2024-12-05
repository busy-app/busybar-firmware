
#include "wifi_async_socket_server_tcp_rx.h"

#include <sl_net.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_constants.h>
#include <sl_si91x_socket_utility.h>

#include "errno.h"

#define TAG                        "WifiAsyncSocketServerTcpRx"
#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)
#define BACK_LOG                   1

#define TEST_TIMEOUT     (30000) //30sec
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
    uint8_t has_data_received;
    uint8_t first_data_frame;
    uint32_t start;
    uint32_t now;
    uint32_t bytes_read;
    FuriString* msg;
} WifiAsyncSocketServerTcpRx;

WifiAsyncSocketServerTcpRx* wifi_async_socket_header = NULL;

void wifi_async_socket_server_tcp_rx_data_callback(
    uint32_t sock_no,
    uint8_t* buffer,
    uint32_t length,
    const sl_si91x_socket_metadata_t* firmware_socket_response) {
    UNUSED_PARAMETER(buffer);
    UNUSED_PARAMETER(firmware_socket_response);

    if(wifi_async_socket_header->first_data_frame) {
        wifi_async_socket_header->start = furi_get_tick();
        furi_string_cat_printf(
            wifi_async_socket_header->msg, "Client Socket ID : %ld\r\n", sock_no);
        furi_string_cat_printf(wifi_async_socket_header->msg, "TCP_RX Throughput test start\r\n");

        wifi_async_socket_header->first_data_frame = 0;
    }

    wifi_async_socket_header->bytes_read += length;
    wifi_async_socket_header->now = furi_get_tick();
    if((wifi_async_socket_header->bytes_read > BYTES_TO_RECEIVE) ||
       ((wifi_async_socket_header->now - wifi_async_socket_header->start) > TEST_TIMEOUT)) {
        wifi_async_socket_header->has_data_received = 1;
    }
}

void wifi_async_socket_server_tcp_rx_init(FuriString* msg, uint16_t port) {
    wifi_async_socket_header =
        (WifiAsyncSocketServerTcpRx*)malloc(sizeof(WifiAsyncSocketServerTcpRx));

    wifi_async_socket_header->has_data_received = 0;
    wifi_async_socket_header->first_data_frame = 1;
    wifi_async_socket_header->start = 0;
    wifi_async_socket_header->now = furi_get_tick();
    wifi_async_socket_header->bytes_read = 0;
    wifi_async_socket_header->msg = msg;

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
            break;
        }
        furi_string_printf(msg, "Socket config Done\r\n");

        // Create Server socket
        server_socket = sl_si91x_socket_async(
            AF_INET, SOCK_STREAM, IPPROTO_TCP, &wifi_async_socket_server_tcp_rx_data_callback);
        if(server_socket < 0) {
            furi_string_cat_printf(msg, "Socket create failed with BSD error: %d\r\n", errno);
            break;
        }
        furi_string_cat_printf(msg, "Server Socket ID : %d\r\n", server_socket);

        //Set socket
        socket_return_value = sl_si91x_setsockopt_async(
            server_socket,
            SOL_SOCKET,
            SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET,
            &high_performance_socket,
            sizeof(high_performance_socket));
        if(socket_return_value < 0) {
            furi_string_cat_printf(msg, "Set Socket option failed with BSD error: %d\r\n", errno);
            close(server_socket);
            break;
        }
        server_address.sin_family = AF_INET;
        server_address.sin_port = port;

        // Bind socket
        socket_return_value =
            sl_si91x_bind(server_socket, (struct sockaddr*)&server_address, socket_length);
        if(socket_return_value < 0) {
            furi_string_cat_printf(msg, "Socket bind failed with BSD error: %d\r\n", errno);
            close(server_socket);
            break;
        }

        // Listen socket
        socket_return_value = sl_si91x_listen(server_socket, BACK_LOG);
        if(socket_return_value < 0) {
            furi_string_cat_printf(msg, "Socket listen failed with BSD error: %d\r\n", errno);
            close(server_socket);
            break;
        }
        furi_string_cat_printf(msg, "Listening on Local Port : %d\r\n", port);

        // Accept socket
        client_socket = sl_si91x_accept(server_socket, NULL, 0);
        if(client_socket < 0) {
            furi_string_cat_printf(msg, "Socket accept failed with BSD error: %d\r\n", errno);
            close(server_socket);
            break;
        }

        while(!wifi_async_socket_header->has_data_received) {
            osThreadYield();
        }

        furi_string_cat_printf(msg, "TCP_RX Throughput test finished\r\n");
        furi_string_cat_printf(
            msg, "Total bytes received : %ld\r\n", wifi_async_socket_header->bytes_read);

        // Close socket
        close(server_socket);
        close(client_socket);
    } while(0);

    free(wifi_async_socket_header);
}
