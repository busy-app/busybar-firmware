#include "wifi_async_socket_client_udp_tx.h"

#include <sl_net.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_constants.h>
#include <sl_si91x_socket_utility.h>

#include "errno.h"

#define TAG "WifiAsyncSocketClientUDPTx"

#define UDP_BUFFER_SIZE 1460
#define BUFFER_SIZE     UDP_BUFFER_SIZE
#define BYTES_TO_SEND   (1 << 29) //512MB
//#define TEST_TIMEOUT    (5 * 60000) //5 mins

typedef struct {
    FuriThread* thread;
    WifiTestApp* app;
    FuriString* msg;
    char* ip;
    uint16_t port;
    bool exit;
} WifiAsyncSocketClientUdpTx;

static WifiAsyncSocketClientUdpTx* wifi_async_socket_client_udp_tx_instance = NULL;

static int32_t wifi_async_socket_client_udp_tx_callback(void* context) {
    furi_check(context != NULL);
    WifiAsyncSocketClientUdpTx* instance = (WifiAsyncSocketClientUdpTx*)context;

    uint32_t now = 0;
    uint32_t start = 0;
    int client_socket = -1;
    uint32_t total_bytes_sent = 0;
    int socket_return_value = 0;
    int sent_bytes = 1;
    struct sockaddr_in server_address = {0};
    socklen_t socket_length = sizeof(struct sockaddr_in);

    uint8_t* data_buffer = NULL;

    server_address.sin_family = AF_INET;
    server_address.sin_port = instance->port;
    sl_net_inet_addr(instance->ip, &server_address.sin_addr.s_addr);

    // Create client socket
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(client_socket < 0) {
        furi_string_printf(instance->msg, "Socket create failed with BSD error: %d\r\n", errno);
        wifi_test_app_send_text(instance->app, instance->msg);
        return 0;
    }
    furi_string_printf(instance->msg, "Socket ID : %d\r\n", client_socket);
    wifi_test_app_send_text(instance->app, instance->msg);
    // Connect socket
    socket_return_value = connect(client_socket, (struct sockaddr*)&server_address, socket_length);
    if(socket_return_value < 0) {
        furi_string_printf(instance->msg, "Socket Connect failed with BSD error: %d\r\n", errno);
        wifi_test_app_send_text(instance->app, instance->msg);
        close(client_socket);
        return 0;
    }
    furi_string_printf(instance->msg, "Socket connected to UDP server\r\n");
    wifi_test_app_send_text(instance->app, instance->msg);

    // Send data

    furi_string_printf(instance->msg, "UDP_TX Throughput test start\r\n");
    wifi_test_app_send_text(instance->app, instance->msg);
    start = furi_get_tick();

    data_buffer = (uint8_t*)malloc(UDP_BUFFER_SIZE);
    while(instance->exit == false) {
        sent_bytes = send(client_socket, data_buffer, UDP_BUFFER_SIZE, 0);
        now = furi_get_tick();
        if(sent_bytes < 0) {
            furi_string_printf(instance->msg, "Socket send failed with bsd error: %d\r\n", errno);
            wifi_test_app_send_text(instance->app, instance->msg);
            close(client_socket);
            break;
        }
        total_bytes_sent = total_bytes_sent + sent_bytes;

        // if((now - start) > TEST_TIMEOUT) {
        //     furi_string_printf(msg, "Time Out: %ld\r\n", (now - start));
        //     wifi_test_app_send_text(instance->app, instance->msg);
        //     break;
        // }
    }
    free(data_buffer);
    furi_string_printf(instance->msg, "Time test: %ld\r\n", (now - start));
    wifi_test_app_send_text(instance->app, instance->msg);
    furi_string_printf(instance->msg, "UDP_TX Throughput test finished\r\n");
    wifi_test_app_send_text(instance->app, instance->msg);
    furi_string_printf(instance->msg, "Total bytes sent : %ld\r\n", total_bytes_sent);
    wifi_test_app_send_text(instance->app, instance->msg);
    // Close socket
    close(client_socket);
    return 0;
}

void wifi_async_socket_client_udp_tx_init(
    WifiTestApp* app,
    FuriString* msg,
    char* ip,
    uint16_t port) {
    if(wifi_async_socket_client_udp_tx_instance != NULL) {
        wifi_async_socket_client_udp_tx_stop();
    }
    wifi_async_socket_client_udp_tx_instance = malloc(sizeof(WifiAsyncSocketClientUdpTx));
    wifi_async_socket_client_udp_tx_instance->msg = msg;
    wifi_async_socket_client_udp_tx_instance->app = app;
    wifi_async_socket_client_udp_tx_instance->ip = ip;
    wifi_async_socket_client_udp_tx_instance->port = port;
    wifi_async_socket_client_udp_tx_instance->exit = false;
    wifi_async_socket_client_udp_tx_instance->thread = furi_thread_alloc_ex(
        "WifiAsyncSocketClientUdpTx",
        2048,
        wifi_async_socket_client_udp_tx_callback,
        wifi_async_socket_client_udp_tx_instance);
    furi_thread_start(wifi_async_socket_client_udp_tx_instance->thread);
}

void wifi_async_socket_client_udp_tx_stop() {
    if(wifi_async_socket_client_udp_tx_instance == NULL) {
        return;
    }
    wifi_async_socket_client_udp_tx_instance->exit = true;

    furi_thread_join(wifi_async_socket_client_udp_tx_instance->thread);
    furi_thread_free(wifi_async_socket_client_udp_tx_instance->thread);
    free(wifi_async_socket_client_udp_tx_instance);
    wifi_async_socket_client_udp_tx_instance = NULL;
}
