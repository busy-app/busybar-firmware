#include <furi.h>
#include <lwip/tcp.h>
#include <network/network.h>
#include "cli_socket_client.h"

#include <lwip/tcpip.h>

#define CLI_SOCKET_PORT 23
#define TAG             "CliSocketServer"

static err_t cli_socket_accept_callback(void* context, struct tcp_pcb* client_socket, err_t err) {
    struct tcp_pcb* listen_socket = context;
    if(err != ERR_OK) {
        FURI_LOG_E(TAG, "Error accepting connection: %d", err);
        return err;
    }

    FURI_LOG_I(
        TAG, "Accepted %s:%d", ipaddr_ntoa(&client_socket->remote_ip), client_socket->remote_port);
    cli_socket_client_start(client_socket);
    tcp_accepted(listen_socket);
    return ERR_OK;
}

static void cli_socket_init_callback(void* context) {
    UNUSED(context);

    struct tcp_pcb* server_socket = tcp_new();
    furi_check(tcp_bind(server_socket, IP_ADDR_ANY, CLI_SOCKET_PORT) == ERR_OK);

    struct tcp_pcb* listen_socket = tcp_listen(server_socket);
    tcp_arg(listen_socket, listen_socket);
    tcp_accept(listen_socket, cli_socket_accept_callback);

    FURI_LOG_I(TAG, "Started");
}

void cli_socket_on_system_start(void) {
    furi_record_open(RECORD_NETWORK);
    tcpip_callback(cli_socket_init_callback, NULL);
}
