#include "sockets_common_i.h"

#include <furi.h>
#include <wifi/wifi_common.h>
#include <intercom/intercom.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_utility.h>

#define TAG "Sockets"

#define TOTAL_SOCKETS                   (TOTAL_TCP_SOCKETS + TOTAL_UDP_SOCKETS)
#define TOTAL_TCP_SOCKETS               1
#define TOTAL_UDP_SOCKETS               0
#define TCP_TX_ONLY_SOCKETS             0
#define TCP_RX_ONLY_SOCKETS             0
#define UDP_TX_ONLY_SOCKETS             0
#define UDP_RX_ONLY_SOCKETS             0
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 0
#define TCP_RX_WINDOW_SIZE_CAP          44
#define TCP_RX_WINDOW_DIV_FACTOR        44

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
    SocketsEventWifiDeinit = 1UL << 1,
    SocketsEventWifiDown = 1UL << 2,
    SocketsEventWifiUp = 1UL << 3,
} SocketsEvent;

/* Assuming only 2 threads might want to send a response,
 * hence 2 response indexes (slots). Intercom will block
 * a particular thread until it is ready to send. */
typedef enum {
    SocketResponseIndexRequest,
    SocketResponseIndexAsync,
    SocketResponseIndexMax,
} SocketResponseIndex;

struct Sockets {
    FuriEventLoop* event_loop;
    Intercom* intercom;
    SocketRequest request;
    SocketResponse response[SocketResponseIndexMax];
};

typedef void (*SocketsRequestHandler)(const SocketRequest* request, SocketResponse* response);

static const SocketsRequestHandler sockets_request_handlers[SocketRequestTypeMax];

/* Global sockets instance, needed for socket callbacks */
static Sockets* sockets_instance;

static inline void sockets_send_response(Sockets* instance, const SocketResponse* response) {
    const size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelSockets,
        response,
        sizeof(SocketResponse),
        FuriWaitForever);

    furi_assert(tx_size == sizeof(SocketResponse));
}

static void sockets_closed_callback(int socket, uint16_t port, uint32_t bytes_sent) {
    FURI_LOG_D(TAG, "Close: %lu byte(s) on socket %d, port %hu", bytes_sent, socket, port);

    SocketResponse* response = &sockets_instance->response[SocketResponseIndexAsync];
    response->type = SocketResponseTypeAsyncClose;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    async_response->socket_id = socket;

    SocketCloseAsyncResponse* close_async_response = &async_response->close_async_response;
    close_async_response->port = port;
    close_async_response->sent_size = bytes_sent;

    sockets_send_response(sockets_instance, response);
}

static void sockets_receive_callback(
    uint32_t socket,
    uint8_t* buffer,
    uint32_t length,
    const sl_si91x_socket_metadata_t* metadata) {
    UNUSED(metadata);

    FURI_LOG_D(TAG, "Rx: %lu byte(s) on socket %lu", length, socket);

    SocketResponse* response = &sockets_instance->response[SocketResponseIndexAsync];
    SocketAsyncResponse* async_response = &response->async_response;
    SocketReceiveAsyncResponse* receive_async_response = &async_response->receive_async_response;

    for(size_t total_size = 0; total_size < length;) {
        const size_t chunk_size = MIN(length - total_size, SOCKET_RECV_DATA_SIZE);

        response->type = SocketResponseTypeAsyncReceive;
        response->status = SocketStatusOk;

        async_response->socket_id = socket;

        receive_async_response->data_size = chunk_size;
        memcpy(receive_async_response->data, buffer + total_size, chunk_size);
        total_size += chunk_size;

        sockets_send_response(sockets_instance, response);
    }
}

static void sockets_send_callback(int32_t socket, uint16_t length) {
    FURI_LOG_D(TAG, "Tx: %hu byte(s) on socket %ld", length, socket);

    SocketResponse* response = &sockets_instance->response[SocketResponseIndexAsync];
    response->type = SocketResponseTypeAsyncSend;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    async_response->socket_id = socket;

    SocketSendAsyncResponse* send_async_response = &async_response->send_async_response;
    send_async_response->sent_size = length;

    sockets_send_response(sockets_instance, response);
}

static void sockets_alloc_request_handler(const SocketRequest* request, SocketResponse* response) {
    FURI_LOG_D(TAG, "Alloc");

    const SocketAllocRequest* alloc_request = &request->alloc_request;
    const SocketInfo* socket_info = &alloc_request->socket_info;
    SocketAllocResponse* alloc_response = &response->alloc_response;

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

    if(socket_id < 0) {
        FURI_LOG_E(TAG, "Failed to allocate socket: %s", strerror(errno));
        response->status = SocketStatusError;

    } else {
        FURI_LOG_D(TAG, "Allocated socket with id: %d", socket_id);
        response->status = SocketStatusOk;
        alloc_response->socket_id = socket_id;
    }
}

static void sockets_free_request_handler(const SocketRequest* request, SocketResponse* response) {
    FURI_LOG_D(TAG, "Free");

    const SocketFreeRequest* free_request = &request->free_request;
    const int status = sl_si91x_shutdown(free_request->socket_id, SHUTDOWN_BY_ID);

    if(status < 0) {
        FURI_LOG_E(TAG, "Failed to free socket: %s", strerror(errno));
        response->status = SocketStatusError;

    } else {
        FURI_LOG_D(TAG, "Free'd socket with id: %hhu", free_request->socket_id);
        response->status = SocketStatusOk;
    }
}

static void
    sockets_connect_request_handler(const SocketRequest* request, SocketResponse* response) {
    FURI_LOG_D(TAG, "Connect");

    const SocketConnectRequest* connect_request = &request->connect_request;
    const SocketConnectionInfo* connection_info = &connect_request->connection_info;

    const struct sockaddr* sock_addr;
    socklen_t sock_addr_len;

    if(connection_info->ip_type == SocketIpTypeV4) {
        FURI_LOG_D(
            TAG,
            "Connecting to %hhu.%hhu.%hhu.%hhu:%hu ...",
            connection_info->address.v4[0],
            connection_info->address.v4[1],
            connection_info->address.v4[2],
            connection_info->address.v4[3],
            connection_info->port);

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

    const int status = sl_si91x_connect(connect_request->socket_id, sock_addr, sock_addr_len);

    if(status < 0) {
        FURI_LOG_E(TAG, "Failed to connect: %s", strerror(errno));
        response->status = SocketStatusError;

    } else {
        FURI_LOG_D(TAG, "Connection successful");
        response->status = SocketStatusOk;
    }
}

static void sockets_send_request_handler(const SocketRequest* request, SocketResponse* response) {
    FURI_LOG_D(TAG, "Send");

    const SocketSendRequest* send_request = &request->send_request;
    SocketSendResponse* send_response = &response->send_response;

    const int bytes_sent = sl_si91x_send_async(
        send_request->socket_id,
        send_request->data,
        send_request->data_size,
        0,
        sockets_send_callback);

    if(bytes_sent < 0) {
        FURI_LOG_E(TAG, "Failed to send: %s", strerror(errno));
        response->status = SocketStatusError;
        send_response->sent_size = 0;

    } else {
        FURI_LOG_D(TAG, "Successfully sent %d bytes", bytes_sent);
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
        const SocketRequest* request = &instance->request;
        const SocketRequestType request_type = request->type;
        furi_assert(request_type < SocketRequestTypeMax);

        SocketResponse* response = &instance->response[SocketResponseIndexRequest];
        response->type = (SocketResponseType)request->type;

        sockets_request_handlers[request_type](request, response);
        sockets_send_response(instance, response);

    } else if(events == SocketsEventWifiDeinit) {
        // Wifi has been deinitialised
    } else if(events == SocketsEventWifiDown) {
        // Wifi has been initialised or disconnected
    } else if(events == SocketsEventWifiUp) {
        const sl_status_t status = sl_si91x_config_socket(sockets_backend_config);
        furi_check(status == SL_STATUS_OK);
    }
}

static void sockets_wifi_state_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const WifiState state = *(WifiState*)message;
    Sockets* instance = context;

    uint32_t event;

    if(state == WifiStateDeinit) {
        event = SocketsEventWifiDeinit;
    } else if(state == WifiStateDown) {
        event = SocketsEventWifiDown;
    } else if(state == WifiStateUp) {
        event = SocketsEventWifiUp;
    } else {
        furi_crash("Invalid Wifi state");
    }

    furi_event_loop_set_custom_event(instance->event_loop, event);
}

Sockets* sockets_alloc(void) {
    Sockets* instance = malloc(sizeof(Sockets));

    instance->event_loop = furi_event_loop_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    FuriPubSub* wifi_pubsub = furi_record_open(RECORD_WIFI);
    FuriPubSubSubscription* sub =
        furi_pubsub_subscribe(wifi_pubsub, sockets_wifi_state_callback, instance);
    UNUSED(sub);

    sl_si91x_set_remote_termination_callback(sockets_closed_callback);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sockets_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, sockets_intercom_rx_callback, instance);

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
