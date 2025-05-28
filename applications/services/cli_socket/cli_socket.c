#include <furi.h>
#include <lwip/tcp.h>
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
    furi_delay_ms(1000);
    FURI_LOG_I(TAG, "Started");

    struct tcp_pcb* server_socket = tcp_new();
    furi_check(tcp_bind(server_socket, IP_ADDR_ANY, CLI_SOCKET_PORT) == ERR_OK);
    
    struct tcp_pcb* listen_socket = tcp_listen(server_socket);
    tcp_arg(listen_socket, listen_socket);
    tcp_accept(listen_socket, cli_socket_accept_callback);
}

// ================================= legacy

// void cli_socket_init(void) {
//     UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
//     usb_network_thread_init(usb_network);
// }

// void cli_socket_deinit(void) {
//     usb_network_thread_cleanup(NULL);
//     furi_record_close(RECORD_USB_NETWORK);
// }

// size_t cli_socket_rx(uint8_t* buffer, size_t size, uint32_t timeout) {
//     if(cli_socket.connected == false) {
//         return 0;
//     }

//     // Send SOH, since our cli is dumb
//     if(cli_socket.soh_sent == false) {
//         cli_socket.soh_sent = true;
//         buffer[0] = 0x01;
//         return 1;
//     }

//     struct pollfd poll_cfg = {
//         .fd = cli_socket.client_socket,
//         .events = POLLIN,
//     };

//     int32_t poll_ret = poll(&poll_cfg, 1, timeout);
//     if(poll_ret < 0) {
//         CLI_SOCKET_DEBUG("disconnected while polling, errno: %d", errno);
//         furi_event_flag_set(cli_socket.evt_flags, FLAG_DISCONNECT);
//         return 0;
//     }

//     size_t received = 0;
//     if(poll_ret > 0) {
//         while(received < size) {
//             // TODO exit if timeout
//             int32_t ret = recv(cli_socket.client_socket, buffer + received, size - received, 0);

//             if(ret < 0) {
//                 CLI_SOCKET_DEBUG("disconnected while reading, errno: %d", errno);
//                 furi_event_flag_set(cli_socket.evt_flags, FLAG_DISCONNECT);
//                 return received;
//             }

//             received += ret;
//         }
//     }

//     return received;
// }

// void cli_socket_tx(const uint8_t* buffer, size_t size) {
//     if(cli_socket.connected == false) {
//         return;
//     }

//     size_t sent = 0;
//     while(sent < size) {
//         int32_t ret = send(cli_socket.client_socket, buffer + sent, size - sent, 0);

//         if(ret < 0) {
//             CLI_SOCKET_DEBUG("disconnected while writing, errno: %d", errno);
//             furi_event_flag_set(cli_socket.evt_flags, FLAG_DISCONNECT);
//             return;
//         }

//         sent += ret;
//     }
// }

// void cli_socket_tx_stdout(const char* data, size_t size, void* context) {
//     UNUSED(context);
//     cli_socket_tx((const uint8_t*)data, size);
// }

// static bool cli_socket_is_connected(void) {
//     return cli_socket.connected;
// }

// CliSession cli_session = {
//     .init = cli_socket_init,
//     .deinit = cli_socket_deinit,
//     .rx = cli_socket_rx,
//     .tx = cli_socket_tx,
//     .tx_stdout = cli_socket_tx_stdout,
//     .is_connected = cli_socket_is_connected,
// };
