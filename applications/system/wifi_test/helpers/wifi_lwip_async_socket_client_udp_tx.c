#include "wifi_lwip_async_socket_client_udp_tx.h"

//#include <lwip/sockets.h>

#include <lwip/udp.h>
#include <lwip/ip_addr.h>

#define TAG "WifiAsyncSocketClientUdpTx"

#define TCP_BUFFER_SIZE 1460
// #define BUFFER_SIZE     TCP_BUFFER_SIZE
// #define BYTES_TO_SEND   (1 << 29) //512MB
#define TEST_TIMEOUT (15000) //30sec

struct udp_pcb* pcb = NULL;
#define UDP_BUFFER_SIZE 1460

void udp_receive_callback(
    void* arg,
    struct udp_pcb* upcb,
    struct pbuf* p,
    const ip_addr_t* addr,
    u16_t port) {
    //   /*increment message count */
    //   message_count++;

    //   /* Free receive pbuf */
    pbuf_free(p);
}

void wifi_lwip_async_socket_client_udp_tx_init(
    WifiTestApp* app,
    FuriString* msg,
    char* ip,
    uint16_t port) {
    uint32_t now = 0;
    uint32_t start = 0;
    //int32_t client_socket = -1;
    // uint32_t total_bytes_sent = 0;
    // int socket_return_value = 0;
    // int sent_bytes = 1;
    //struct sockaddr_in server_address = {0};
    // socklen_t socket_length = sizeof(struct sockaddr_in);

    //uint8_t* data_buffer = NULL;

    // client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // if(client_socket < 0) {
    //     furi_string_printf(msg, "Socket create failed with BSD error: %d\r\n", errno);
    //     wifi_test_app_send_text(app, msg);
    //     return;
    // }
    // furi_string_printf(msg, "Socket ID : %d\r\n", client_socket);
    // wifi_test_app_send_text(app, msg);
    
    // memset(&server_address, 0, sizeof(server_address));
    // server_address.sin_family = AF_INET;
    // server_address.sin_port = htons(port);
    // server_address.sin_addr.s_addr = inet_addr(ip);

    // int err = connect(
    //     client_socket, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in));
    // if(err < 0) {
    //     furi_string_printf(msg, "Socket Connect failed with BSD error: %d\r\n", errno);
    //     wifi_test_app_send_text(app, msg);
    //     close(client_socket);
    //     return;
    // }
    // furi_string_printf(msg, "Socket connected to UDP server\r\n");
    // wifi_test_app_send_text(app, msg);

    // start = furi_get_tick();
    // data_buffer = (uint8_t*)malloc(TCP_BUFFER_SIZE);

    // while(1) {
    //     int err = sendto(
    //         client_socket, data_buffer, TCP_BUFFER_SIZE, 0, (struct sockaddr*)&server_address,
    //         sizeof(struct sockaddr_in));
    //     //int err = send(client_socket, data_buffer, TCP_BUFFER_SIZE, 0);
    //     if(err < 0) {
    //         furi_string_printf(msg, "Socket send failed with bsd error: %d\r\n", errno);
    //         wifi_test_app_send_text(app, msg);
    //         close(client_socket);
    //         break;
    //     }
    //     furi_delay_ms(20);
    // }

    // furi_string_printf(msg, "Time Out: %ld\r\n", (furi_get_tick() - start));
    // wifi_test_app_send_text(app, msg);
    // close(client_socket);
    // furi_string_printf(msg, "Socket closed\r\n");
    // wifi_test_app_send_text(app, msg);
    // free(data_buffer);
    // data_buffer = NULL;

    ip_addr_t DestIPaddr;
    err_t err;

    uint8_t* data_buffer = NULL;

    pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    if(pcb == NULL) {
        furi_string_printf(msg, "PCB creation failed\r\n");
        wifi_test_app_send_text(app, msg);
        return;
    }

    furi_string_printf(msg, "PCB created\r\n");
    wifi_test_app_send_text(app, msg);

    IP4_ADDR(&DestIPaddr.u_addr.ip4, 192, 168, 10, 2);
    DestIPaddr.type = IPADDR_TYPE_V4;
    // 
    err = udp_connect(pcb, &DestIPaddr, port);
    if(err != ERR_OK) {
        furi_string_printf(msg, "UDP connect failed with error: %d\r\n", err);
        wifi_test_app_send_text(app, msg);
        udp_remove(pcb);
        return;
    }

    udp_recv(pcb, udp_receive_callback, app);
    furi_string_printf(msg, "UDP connected to server\r\n");
    wifi_test_app_send_text(app, msg);

    start = furi_get_tick();
    data_buffer = (uint8_t*)malloc(TCP_BUFFER_SIZE);
    while((now - start) > TEST_TIMEOUT) {
        struct pbuf* p;

        /* allocate pbuf from pool*/
        p = pbuf_alloc(PBUF_TRANSPORT, TCP_BUFFER_SIZE, PBUF_POOL);

        if(p != NULL) {
            /* copy data to pbuf */
            pbuf_take(p, data_buffer, TCP_BUFFER_SIZE);

            /* send udp data */
            udp_send(pcb, p);

            /* free pbuf */
            pbuf_free(p);
        }
        furi_delay_ms(20);
    }
    furi_string_printf(msg, "Time Out: %ld\r\n", (now - start));
    wifi_test_app_send_text(app, msg);
    udp_disconnect(pcb);
    udp_remove(pcb);
    pcb = NULL;
    furi_string_printf(msg, "PCB disconnected\r\n");
    wifi_test_app_send_text(app, msg);
    furi_string_printf(msg, "PCB removed\r\n");
    wifi_test_app_send_text(app, msg);
}
