#include <furi.h>
#include <lwip/tcp.h>
#include <usb_network/usb_network.h>
#include "cli_socket_client.h"

#define CLI_SOCKET_PORT 23
#define TAG             "CliSocketServer"

static err_t cli_socket_accept_callback(void* context, struct tcp_pcb* client_socket, err_t err) {
    struct tcp_pcb* listen_socket = context;
    furi_check(err == ERR_OK);
    cli_socket_client_start(client_socket);
    tcp_accepted(listen_socket);
    return ERR_OK;
}

void cli_socket_on_system_start(void) {
    // furi_delay_ms(1000);
    furi_record_open(RECORD_USB_NETWORK);
    FURI_LOG_I(TAG, "Started");

    struct tcp_pcb* server_socket = tcp_new();
    furi_check(tcp_bind(server_socket, IP_ADDR_ANY, CLI_SOCKET_PORT) == ERR_OK);

    struct tcp_pcb* listen_socket = tcp_listen(server_socket);
    tcp_arg(listen_socket, listen_socket);
    tcp_accept(listen_socket, cli_socket_accept_callback);
}
