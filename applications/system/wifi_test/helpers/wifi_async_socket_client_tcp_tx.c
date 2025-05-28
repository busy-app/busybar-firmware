#include "wifi_async_socket_client_tcp_tx.h"

#include <sl_net.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_constants.h>
#include <sl_si91x_socket_utility.h>

#include "errno.h"

#define TAG "WifiAsyncSocketClientTcpTx"

#define TCP_BUFFER_SIZE 1460
#define BUFFER_SIZE     TCP_BUFFER_SIZE
#define BYTES_TO_SEND   (1 << 29) //512MB
#define TEST_TIMEOUT    (30000) //30sec

void wifi_async_socket_client_tcp_tx_init(
    char* ip,
    uint16_t port) {
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
    server_address.sin_port = port;
    sl_net_inet_addr(ip, &server_address.sin_addr.s_addr);

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(client_socket < 0) {
        printf("Socket create failed with BSD error: %d\r\n", errno);
        return;
    }
    printf("Socket ID : %d\r\n", client_socket);
    
    // Connect socket
    socket_return_value = connect(client_socket, (struct sockaddr*)&server_address, socket_length);
    if(socket_return_value < 0) {
        printf("Socket Connect failed with BSD error: %d\r\n", errno);
        close(client_socket);
        return;
    }
    printf("Socket connected to TCP server\r\n");

    // Send data
    printf("TCP_TX Throughput test start\r\n");
    start = furi_get_tick();

    data_buffer = (uint8_t*)malloc(TCP_BUFFER_SIZE);
    while(total_bytes_sent < BYTES_TO_SEND) {
        sent_bytes = send(client_socket, data_buffer, TCP_BUFFER_SIZE, 0);
        now = furi_get_tick();
        if(sent_bytes < 0) {
            printf("Socket send failed with bsd error: %d\r\n", errno); 
            close(client_socket);
            break;
        }
        total_bytes_sent = total_bytes_sent + sent_bytes;

        if((now - start) > TEST_TIMEOUT) {
            printf("Time Out: %ld\r\n", (now - start));   
            break;
        }
    }
    free(data_buffer);
    printf("TCP_TX Throughput test finished\r\n");
    printf("Total bytes sent : %ld\r\n", total_bytes_sent);
    
    // Close socket
    close(client_socket);
}
