#include <furi.h>
#include <lwip/sockets.h>
#include <usb_network/usb_network.h>
#include <cli/cli_i.h>

#define CLI_SOCKET_PORT 23
#define TAG             "CliSocket"

#define CLI_SOCKET_DEBUG_ENABLE

#ifdef CLI_SOCKET_DEBUG_ENABLE
#define CLI_SOCKET_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define CLI_SOCKET_DEBUG(...)
#endif

#define FLAG_DISCONNECT (1 << 0)

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

int32_t cli_socket_srv(void* p) {
    UNUSED(p);

    UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
    usb_network_thread_init(usb_network);

    cli_socket.evt_flags = furi_event_flag_alloc();

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
    address.sin_port = htons(CLI_SOCKET_PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address
    if(bind(listen_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        furi_crash("bind() failed");
    }

    // Listen for _one_ incoming connection
    if(listen(listen_fd, 1) < 0) {
        furi_crash("listen() failed");
    }

    while(1) {
        cli_socket.client_socket = accept(listen_fd, NULL, NULL);

        if(cli_socket.client_socket < 0) {
            furi_crash("accept() failed");
        }

        cli_socket.soh_sent = false;
        cli_socket.connected = true;

        uint32_t flags = furi_event_flag_wait(
            cli_socket.evt_flags, FLAG_DISCONNECT, FuriFlagWaitAny, FuriWaitForever);
        furi_check(flags & FLAG_DISCONNECT);
        furi_check(!(flags & FuriFlagError));

        cli_socket.connected = false;
        close(cli_socket.client_socket);
    }

    usb_network_thread_cleanup(usb_network);
    furi_record_close(RECORD_USB_NETWORK);

    return 0;
}

void cli_socket_init(void) {
    UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
    usb_network_thread_init(usb_network);
}

void cli_socket_deinit(void) {
    usb_network_thread_cleanup(NULL);
    furi_record_close(RECORD_USB_NETWORK);
}

size_t cli_socket_rx(uint8_t* buffer, size_t size, uint32_t timeout) {
    if(cli_socket.connected == false) {
        return 0;
    }

    // Send SOH, since our cli is dumb
    if(cli_socket.soh_sent == false) {
        cli_socket.soh_sent = true;
        buffer[0] = 0x01;
        return 1;
    }

    struct pollfd poll_cfg = {
        .fd = cli_socket.client_socket,
        .events = POLLIN,
    };

    int32_t poll_ret = poll(&poll_cfg, 1, timeout);
    if(poll_ret < 0) {
        CLI_SOCKET_DEBUG("disconnected while polling, errno: %d", errno);
        furi_event_flag_set(cli_socket.evt_flags, FLAG_DISCONNECT);
        return 0;
    }

    size_t received = 0;
    while(received < size) {
        int32_t ret = recv(cli_socket.client_socket, buffer + received, size - received, 0);

        if(ret < 0) {
            CLI_SOCKET_DEBUG("disconnected while reading, errno: %d", errno);
            furi_event_flag_set(cli_socket.evt_flags, FLAG_DISCONNECT);
            return received;
        }

        received += ret;
    }

    return received;
}

void cli_socket_tx(const uint8_t* buffer, size_t size) {
    if(cli_socket.connected == false) {
        return;
    }

    size_t sent = 0;
    while(sent < size) {
        int32_t ret = send(cli_socket.client_socket, buffer + sent, size - sent, 0);

        if(ret < 0) {
            CLI_SOCKET_DEBUG("disconnected while writing, errno: %d", errno);
            furi_event_flag_set(cli_socket.evt_flags, FLAG_DISCONNECT);
            return;
        }

        sent += ret;
    }
}

void cli_socket_tx_stdout(const char* data, size_t size, void* context) {
    UNUSED(context);
    cli_socket_tx((const uint8_t*)data, size);
}

static bool cli_socket_is_connected(void) {
    return cli_socket.connected;
}

CliSession cli_session = {
    .init = cli_socket_init,
    .deinit = cli_socket_deinit,
    .rx = cli_socket_rx,
    .tx = cli_socket_tx,
    .tx_stdout = cli_socket_tx_stdout,
    .is_connected = cli_socket_is_connected,
};
