#include "sockets_common_i.h"

#include <furi.h>
#include <intercom/intercom.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_utility.h>

#define TOTAL_SOCKETS                   (TOTAL_TCP_SOCKETS + TOTAL_UDP_SOCKETS)
#define TOTAL_TCP_SOCKETS               2
#define TOTAL_UDP_SOCKETS               2
#define TCP_TX_ONLY_SOCKETS             1
#define TCP_RX_ONLY_SOCKETS             1
#define UDP_TX_ONLY_SOCKETS             1
#define UDP_RX_ONLY_SOCKETS             1
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 0
#define TCP_RX_WINDOW_SIZE_CAP          44
#define TCP_RX_WINDOW_DIV_FACTOR        44

#define TAG "Sockets"

static const sl_si91x_socket_config_t sockets_backend_config = {
    .total_sockets = TOTAL_SOCKETS,
    .total_tcp_sockets = TOTAL_TCP_SOCKETS,
    .total_udp_sockets = TOTAL_UDP_SOCKETS,
    .tcp_tx_only_sockets = TCP_TX_ONLY_SOCKETS,
    .tcp_rx_only_sockets = TCP_RX_ONLY_SOCKETS,
    .udp_tx_only_sockets = UDP_TX_ONLY_SOCKETS,
    .udp_rx_only_sockets = UDP_RX_ONLY_SOCKETS,
    .tcp_rx_high_performance_sockets = TCP_RX_HIGH_PERFORMANCE_SOCKETS,
    .tcp_rx_window_size_cap = TCP_RX_WINDOW_SIZE_CAP,
    .tcp_rx_window_div_factor = TCP_RX_WINDOW_DIV_FACTOR,
};

typedef enum {
    SocketsEventRequest = 1UL << 0,
} SocketsEvent;

struct Sockets {
    FuriEventLoop* event_loop;
    Intercom* intercom;
    SocketRequest request;
    SocketResponse response;
};

typedef void (*SocketsRequestHandler)(Sockets* instance);

static const SocketsRequestHandler sockets_request_handlers[SocketRequestTypeMax];

static inline void sockets_send_response(Sockets* instance) {
    const size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelSockets,
        &instance->response,
        sizeof(SocketResponse),
        FuriWaitForever);
    furi_check(tx_size == sizeof(SocketResponse));
}

static void sockets_socket_termination_callback(int socket, uint16_t port, uint32_t bytes_sent) {
    UNUSED(socket);
    UNUSED(port);
    UNUSED(bytes_sent);
}

static void sockets_async_socket_callback(
    uint32_t socket,
    uint8_t* buffer,
    uint32_t length,
    const sl_si91x_socket_metadata_t* metadata) {
    UNUSED(socket);
    UNUSED(buffer);
    UNUSED(length);
    UNUSED(metadata);
}

static void sockets_alloc_request_handler(Sockets* instance) {
    const SocketInfo* info = &instance->request.alloc_request.info;

    int ip_type, socket_type, socket_protocol;

    if(info->ip_type == SocketIpTypeV4) {
        ip_type = AF_INET;
    } else if(info->ip_type == SocketIpTypeV6) {
        ip_type = AF_INET6;
    } else {
        furi_crash("Invalid IP version");
    }

    if(info->protocol == SocketProtocolTcp) {
        socket_type = SOCK_STREAM;
        socket_protocol = IPPROTO_TCP;

    } else if(info->protocol == SocketProtocolUdp) {
        socket_type = SOCK_DGRAM;
        socket_protocol = IPPROTO_UDP;

    } else {
        furi_crash("Invalid protocol");
    }

    const int socket_id = sl_si91x_socket_async(
        ip_type, socket_type, socket_protocol, sockets_async_socket_callback);

    SocketResponse* response = &instance->response;

    if(socket_id < 0) {
        response->status = SocketStatusError;
    } else {
        response->status = SocketStatusOk;
        response->alloc_response.socket_id = socket_id;
    }

    sockets_send_response(instance);
}

static void sockets_free_request_handler(Sockets* instance) {
    const SocketFreeRequest* request = &instance->request.free_request;

    const int status = sl_si91x_shutdown(request->socket_id, SHUTDOWN_BY_ID);

    SocketResponse* response = &instance->response;

    if(status < 0) {
        response->status = SocketStatusError;
    } else {
        response->status = SocketStatusOk;
    }

    sockets_send_response(instance);
}

static void sockets_connect_request_handler(Sockets* instance) {
    const SocketConnectRequest* request = &instance->request.connect_request;
    const SocketConnectionInfo* info = &request->info;

    const struct sockaddr* sock_addr;
    socklen_t sock_addr_len;

    if(info->ip_type == SocketIpTypeV4) {
        struct sockaddr_in sock_addr_v4 = {0};

        sock_addr_v4.sin_family = AF_INET;
        sock_addr_v4.sin_port = info->port;
        memcpy(&sock_addr_v4.sin_addr, info->address.v4, sizeof(sock_addr_v4.sin_addr));

        sock_addr = (const struct sockaddr*)&sock_addr_v4;
        sock_addr_len = sizeof(sock_addr_v4);

    } else if(info->ip_type == SocketIpTypeV6) {
        struct sockaddr_in6 sock_addr_v6 = {0};

        sock_addr_v6.sin6_family = AF_INET6;
        sock_addr_v6.sin6_port = info->port;
        memcpy(&sock_addr_v6.sin6_addr, info->address.v6, sizeof(sock_addr_v6.sin6_addr));

        sock_addr = (const struct sockaddr*)&sock_addr_v6;
        sock_addr_len = sizeof(sock_addr_v6);

    } else {
        furi_crash("Invalid IP version");
    }

    const int status = sl_si91x_connect(request->socket_id, sock_addr, sock_addr_len);

    SocketResponse* response = &instance->response;

    if(status < 0) {
        response->status = SocketStatusError;
    } else {
        response->status = SocketStatusOk;
    }

    sockets_send_response(instance);
}

static void sockets_send_request_handler(Sockets* instance) {
    UNUSED(instance);
}

static void sockets_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size == sizeof(SocketRequest));

    Sockets* instance = context;
    memcpy(&instance->request, data, data_size);

    furi_event_loop_set_custom_event(instance->event_loop, SocketsEventRequest);
}

static void sockets_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);

    Sockets* instance = context;

    if(events == SocketsEventRequest) {
        const SocketRequestType request_type = instance->request.type;
        furi_assert(request_type < SocketRequestTypeMax);

        instance->response.type = request_type;
        sockets_request_handlers[request_type](instance);

    } else {
        furi_crash("Multiple Socket events");
    }
}

Sockets* sockets_alloc(void) {
    Sockets* instance = malloc(sizeof(Sockets));

    instance->event_loop = furi_event_loop_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    const sl_status_t status = sl_si91x_config_socket(sockets_backend_config);
    furi_check(status == SL_STATUS_OK);

    sl_si91x_set_remote_termination_callback(sockets_socket_termination_callback);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sockets_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, sockets_intercom_rx_callback, instance);

    return instance;
}

int32_t sockets_srv(void* arg) {
    UNUSED(arg);

    Sockets* instance = sockets_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const SocketsRequestHandler sockets_request_handlers[SocketRequestTypeMax] = {
    [SocketRequestTypeAlloc] = sockets_alloc_request_handler,
    [SocketRequestTypeFree] = sockets_free_request_handler,
    [SocketRequestTypeConnect] = sockets_connect_request_handler,
    [SocketRequestTypeSend] = sockets_send_request_handler,
    [SocketRequestTypeReceive] = NULL,
};
