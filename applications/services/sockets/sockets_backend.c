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

typedef enum {
    SocketsEventFlagReady = 1UL << 0,
} SocketsEventFlag;

struct Sockets {
    FuriEventLoop* event_loop;
    FuriEventFlag* event_flag;
    Intercom* intercom;
    SocketRequest request;
    SocketResponse response;
};

typedef void (*SocketsRequestHandler)(Sockets* instance);

static const SocketsRequestHandler sockets_request_handlers[SocketRequestTypeMax];

/* Global sockets instance, needed for socket callbacks */
static Sockets* sockets_instance;

static inline void sockets_wait_for_response_slot(Sockets* instance) {
    const uint32_t flags = furi_event_flag_wait(
        instance->event_flag, SocketsEventFlagReady, FuriFlagWaitAny, FuriWaitForever);
    furi_check((flags & FuriFlagError) == 0);
}

static inline void sockets_send_response(Sockets* instance) {
    const SocketResponse* response = &instance->response;

    const size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelSockets,
        response,
        sizeof(SocketResponse),
        FuriWaitForever);

    furi_check(tx_size == sizeof(SocketResponse));

    furi_event_flag_set(instance->event_flag, SocketsEventFlagReady);
}

static void sockets_closed_callback(int socket, uint16_t port, uint32_t bytes_sent) {
    sockets_wait_for_response_slot(sockets_instance);

    SocketResponse* response = &sockets_instance->response;
    response->type = SocketResponseTypeAsyncClose;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    async_response->socket_id = socket;

    SocketCloseAsyncResponse* close_async_response = &async_response->close_async_response;
    close_async_response->port = port;
    close_async_response->sent_size = bytes_sent;

    sockets_send_response(sockets_instance);
}

static void sockets_receive_callback(
    uint32_t socket,
    uint8_t* buffer,
    uint32_t length,
    const sl_si91x_socket_metadata_t* metadata) {
    UNUSED(metadata);

    SocketResponse* response = &sockets_instance->response;
    SocketAsyncResponse* async_response = &response->async_response;
    SocketReceiveAsyncResponse* receive_async_response = &async_response->receive_async_response;

    for(size_t total_size = length; total_size > 0;) {
        const size_t chunk_size = MIN(length, SOCKET_RECV_DATA_SIZE);
        total_size -= chunk_size;

        sockets_wait_for_response_slot(sockets_instance);

        response->type = SocketResponseTypeAsyncReceive;
        response->status = SocketStatusOk;

        async_response->socket_id = socket;

        receive_async_response->data_size = chunk_size;
        memcpy(receive_async_response->data, buffer, chunk_size);

        sockets_send_response(sockets_instance);
    }
}

static void sockets_send_callback(int32_t socket, uint16_t length) {
    sockets_wait_for_response_slot(sockets_instance);

    SocketResponse* response = &sockets_instance->response;
    response->type = SocketResponseTypeAsyncSend;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    async_response->socket_id = socket;

    SocketSendAsyncResponse* send_async_response = &async_response->send_async_response;
    send_async_response->sent_size = length;

    sockets_send_response(sockets_instance);
}

static void sockets_alloc_request_handler(Sockets* instance) {
    const SocketAllocRequest* alloc_request = &instance->request.alloc_request;
    const SocketInfo* socket_info = &alloc_request->socket_info;

    int ip_type, socket_type, socket_protocol;

    if(socket_info->ip_type == SocketIpTypeV4) {
        ip_type = AF_INET;
    } else if(socket_info->ip_type == SocketIpTypeV6) {
        ip_type = AF_INET6;
    } else {
        furi_crash("Invalid IP version");
    }

    if(socket_info->protocol == SocketProtocolTcp) {
        socket_type = SOCK_STREAM;
        socket_protocol = IPPROTO_TCP;

    } else if(socket_info->protocol == SocketProtocolUdp) {
        socket_type = SOCK_DGRAM;
        socket_protocol = IPPROTO_UDP;

    } else {
        furi_crash("Invalid protocol");
    }

    const int socket_id =
        sl_si91x_socket_async(ip_type, socket_type, socket_protocol, sockets_receive_callback);

    sockets_wait_for_response_slot(instance);

    SocketResponse* response = &instance->response;
    response->type = SocketResponseTypeAlloc;

    SocketAllocResponse* alloc_response = &response->alloc_response;

    if(socket_id < 0) {
        response->status = SocketStatusError;
    } else {
        response->status = SocketStatusOk;
        alloc_response->socket_id = socket_id;
    }
}

static void sockets_free_request_handler(Sockets* instance) {
    const SocketFreeRequest* request = &instance->request.free_request;

    const int status = sl_si91x_shutdown(request->socket_id, SHUTDOWN_BY_ID);

    sockets_wait_for_response_slot(instance);

    SocketResponse* response = &instance->response;
    response->type = SocketResponseTypeFree;

    if(status < 0) {
        response->status = SocketStatusError;
    } else {
        response->status = SocketStatusOk;
    }
}

static void sockets_connect_request_handler(Sockets* instance) {
    const SocketConnectRequest* request = &instance->request.connect_request;
    const SocketConnectionInfo* connection_info = &request->connection_info;

    const struct sockaddr* sock_addr;
    socklen_t sock_addr_len;

    if(connection_info->ip_type == SocketIpTypeV4) {
        struct sockaddr_in sock_addr_v4 = {0};

        sock_addr_v4.sin_family = AF_INET;
        sock_addr_v4.sin_port = connection_info->port;
        memcpy(&sock_addr_v4.sin_addr, connection_info->address.v4, sizeof(sock_addr_v4.sin_addr));

        sock_addr = (const struct sockaddr*)&sock_addr_v4;
        sock_addr_len = sizeof(sock_addr_v4);

    } else if(connection_info->ip_type == SocketIpTypeV6) {
        struct sockaddr_in6 sock_addr_v6 = {0};

        sock_addr_v6.sin6_family = AF_INET6;
        sock_addr_v6.sin6_port = connection_info->port;
        memcpy(
            &sock_addr_v6.sin6_addr, connection_info->address.v6, sizeof(sock_addr_v6.sin6_addr));

        sock_addr = (const struct sockaddr*)&sock_addr_v6;
        sock_addr_len = sizeof(sock_addr_v6);

    } else {
        furi_crash("Invalid IP version");
    }

    const int status = sl_si91x_connect(request->socket_id, sock_addr, sock_addr_len);

    sockets_wait_for_response_slot(instance);

    SocketResponse* response = &instance->response;
    response->type = SocketResponseTypeConnect;

    if(status < 0) {
        response->status = SocketStatusError;
    } else {
        response->status = SocketStatusOk;
    }
}

static void sockets_send_request_handler(Sockets* instance) {
    const SocketSendRequest* send_request = &instance->request.send_request;

    const int bytes_sent = sl_si91x_send_async(
        send_request->socket_id,
        send_request->data,
        send_request->data_size,
        0,
        sockets_send_callback);

    sockets_wait_for_response_slot(instance);

    SocketResponse* response = &instance->response;
    response->type = SocketResponseTypeSend;

    SocketSendResponse* send_response = &response->send_response;

    if(bytes_sent < 0) {
        response->status = SocketStatusError;
        send_response->sent_size = 0;
    } else {
        response->status = SocketStatusOk;
        send_response->sent_size = bytes_sent;
    }
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

        sockets_request_handlers[request_type](instance);
        sockets_send_response(instance);
    }
}

Sockets* sockets_alloc(void) {
    Sockets* instance = malloc(sizeof(Sockets));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_flag = furi_event_flag_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    const sl_status_t status = sl_si91x_config_socket(sockets_backend_config);
    furi_check(status == SL_STATUS_OK);

    sl_si91x_set_remote_termination_callback(sockets_closed_callback);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sockets_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, sockets_intercom_rx_callback, instance);

    furi_event_flag_set(instance->event_flag, SocketsEventFlagReady);

    return instance;
}

int32_t sockets_srv(void* arg) {
    UNUSED(arg);

    sockets_instance = sockets_alloc();
    furi_event_loop_run(sockets_instance->event_loop);

    return 0;
}

static const SocketsRequestHandler sockets_request_handlers[SocketRequestTypeMax] = {
    [SocketRequestTypeAlloc] = sockets_alloc_request_handler,
    [SocketRequestTypeFree] = sockets_free_request_handler,
    [SocketRequestTypeConnect] = sockets_connect_request_handler,
    [SocketRequestTypeSend] = sockets_send_request_handler,
};
