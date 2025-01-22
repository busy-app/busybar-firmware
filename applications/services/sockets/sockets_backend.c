#include "sockets_common_i.h"

#include <furi.h>
#include <wifi/wifi_common.h>
#include <intercom/intercom.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_utility.h>

#include "sockets_backend_util.h"

#define TAG "Sockets"

#define TOTAL_SOCKETS                   (TOTAL_TCP_SOCKETS + TOTAL_UDP_SOCKETS)
#define TOTAL_TCP_SOCKETS               3
#define TOTAL_UDP_SOCKETS               0
#define TCP_TX_ONLY_SOCKETS             0
#define TCP_RX_ONLY_SOCKETS             0
#define UDP_TX_ONLY_SOCKETS             0
#define UDP_RX_ONLY_SOCKETS             0
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 0
#define TCP_RX_WINDOW_SIZE_CAP          1
#define TCP_RX_WINDOW_DIV_FACTOR        1

#define NUM_CLIENTS_PER_SOCKET 1
#define NUM_REQUEST_HANDLERS   (SocketRequestTypeMax - 1)

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
    SocketSrvEventRequest = 1UL << 0,
    SocketSrvEventAsyncRequest = 1UL << 1,
    SocketSrvEventAsyncResponse = 1UL << 2,
    SocketSrvEventWifiDeinit = 1UL << 3,
    SocketSrvEventWifiDown = 1UL << 4,
    SocketSrvEventWifiUp = 1UL << 5,
} SocketSrvEvent;

struct SocketSrv {
    FuriEventLoop* event_loop;
    FuriSemaphore* async_semaphore;
    Intercom* intercom;
    SocketRequest request;
    SocketResponse response[SocketChannelMax];
};

typedef void (*SocketRequestHandler)(const SocketRequest* request, SocketResponse* response);

static const SocketRequestHandler socket_request_handlers[NUM_REQUEST_HANDLERS];

/* Global sockets instance, needed for socket callbacks */
static SocketSrv* socket_srv;

static inline void sockets_send_response(SocketSrv* instance, const SocketResponse* response) {
    const size_t response_size = sockets_get_response_size(response);
    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelSockets, response, response_size, FuriWaitForever);
    furi_assert(tx_size == response_size);
}

static void sockets_closed_callback(int socket, uint16_t port, uint32_t bytes_sent) {
    FURI_LOG_D(TAG, "Close: %lu byte(s) on socket %d, port %hu", bytes_sent, socket, port);

    furi_check(
        furi_semaphore_acquire(socket_srv->async_semaphore, FuriWaitForever) == FuriStatusOk);

    SocketResponse* response = &socket_srv->response[SocketChannelAsync];
    response->type = SocketResponseTypeAsyncClose;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    async_response->socket_id = socket;

    SocketCloseAsyncResponse* close_async_response = &async_response->close_async_response;
    close_async_response->port = port;
    close_async_response->sent_size = bytes_sent;

    furi_event_loop_set_custom_event(socket_srv->event_loop, SocketSrvEventAsyncResponse);
}

static void sockets_receive_callback(
    uint32_t socket,
    uint8_t* buffer,
    uint32_t length,
    const sl_si91x_socket_metadata_t* metadata) {
    UNUSED(metadata);

    FURI_LOG_D(TAG, "Rx: %lu byte(s) on socket %lu", length, socket);

    SocketResponse* response = &socket_srv->response[SocketChannelAsync];
    SocketAsyncResponse* async_response = &response->async_response;
    SocketReceiveAsyncResponse* receive_async_response = &async_response->receive_async_response;

    for(size_t total_size = 0; total_size < length;) {
        const size_t chunk_size = MIN(length - total_size, SOCKET_RECV_DATA_SIZE);

        furi_check(
            furi_semaphore_acquire(socket_srv->async_semaphore, FuriWaitForever) == FuriStatusOk);

        response->type = SocketResponseTypeAsyncReceive;
        response->status = SocketStatusOk;

        async_response->socket_id = socket;

        receive_async_response->data_size = chunk_size;
        memcpy(receive_async_response->data, buffer + total_size, chunk_size);
        total_size += chunk_size;

        furi_event_loop_set_custom_event(socket_srv->event_loop, SocketSrvEventAsyncResponse);
    }
}

static void sockets_send_callback(int32_t socket, uint16_t length) {
    FURI_LOG_D(TAG, "Tx: %hu byte(s) on socket %ld", length, socket);

    furi_check(
        furi_semaphore_acquire(socket_srv->async_semaphore, FuriWaitForever) == FuriStatusOk);

    SocketResponse* response = &socket_srv->response[SocketChannelAsync];
    response->type = SocketResponseTypeAsyncSend;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    async_response->socket_id = socket;

    SocketSendAsyncResponse* send_async_response = &async_response->send_async_response;
    send_async_response->sent_size = length;

    furi_event_loop_set_custom_event(socket_srv->event_loop, SocketSrvEventAsyncResponse);
}

// BUG: params `addr` and `ip_version` do not contain correct data
static void sockets_accept_callback(int32_t socket, struct sockaddr* addr, uint8_t ip_version) {
    UNUSED(addr);
    UNUSED(ip_version);
    FURI_LOG_D(TAG, "Ac: client socket %ld", socket);

    furi_check(
        furi_semaphore_acquire(socket_srv->async_semaphore, FuriWaitForever) == FuriStatusOk);

    SocketResponse* response = &socket_srv->response[SocketChannelAsync];
    response->type = SocketResponseTypeAsyncAccept;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    async_response->socket_id = sockets_get_parent(socket);
    furi_check(async_response->socket_id >= 0);

    const sli_si91x_socket_t* client_socket = sli_si91x_sockets[socket];

    SocketAcceptAsyncResponse* accept_async_response = &async_response->accept_async_response;
    accept_async_response->client_socket_id = socket;
    sockets_sockaddr_to_connection_info(
        (const struct sockaddr*)&client_socket->remote_address,
        &accept_async_response->connection_info);

    furi_event_loop_set_custom_event(socket_srv->event_loop, SocketSrvEventAsyncResponse);
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

    if(status < 0 && errno != EBADF) {
        FURI_LOG_E(TAG, "Failed to free socket: %s", strerror(errno));
        response->status = SocketStatusError;

    } else {
        FURI_LOG_D(TAG, "Free'd socket with id: %hhu", free_request->socket_id);
        response->status = SocketStatusOk;
    }
}

static void
    sockets_accept_request_handler(const SocketRequest* request, SocketResponse* response) {
    FURI_LOG_D(TAG, "Accept");

    const SocketAcceptRequest* accept_request = &request->accept_request;
    const SocketConnectionInfo* bind_info = &accept_request->bind_info;
    const uint8_t socket_id = accept_request->socket_id;

    int status;

    do {
        SocketSlAddress sl_address = {0};
        sockets_connection_info_to_sl_address(bind_info, &sl_address);

        status = sl_si91x_bind(socket_id, &sl_address.address, sl_address.length);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to bind socket %hhu: %s", socket_id, strerror(errno));
            break;
        }

        status = sl_si91x_listen(socket_id, NUM_CLIENTS_PER_SOCKET);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to listen on socket %hhu: %s", socket_id, strerror(errno));
            break;
        }

        status = sl_si91x_accept_async(socket_id, sockets_accept_callback);

        if(status < 0) {
            FURI_LOG_E(TAG, "Failed to accept on socket %hhu: %s", socket_id, strerror(errno));
            break;
        }

    } while(false);

    if(status < 0) {
        response->status = SocketStatusError;

    } else {
        FURI_LOG_D(TAG, "Accepting client connections on socket %hhu", socket_id);
        response->status = SocketStatusOk;
    }
}

static void
    sockets_connect_request_handler(const SocketRequest* request, SocketResponse* response) {
    FURI_LOG_D(TAG, "Connect");

    const SocketConnectRequest* connect_request = &request->connect_request;
    const SocketConnectionInfo* connection_info = &connect_request->connection_info;

    SocketSlAddress sl_address = {0};
    sockets_connection_info_to_sl_address(connection_info, &sl_address);

    const int status =
        sl_si91x_connect(connect_request->socket_id, &sl_address.address, sl_address.length);

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
    SocketSrv* instance = context;

    const SocketRequest* request = data;
    furi_assert(data_size == sockets_get_request_size(request));

    if(request->type < SocketRequestTypeAsyncConfirm) {
        memcpy(&instance->request, data, data_size);
        furi_event_loop_set_custom_event(instance->event_loop, SocketSrvEventRequest);
    } else {
        furi_event_loop_set_custom_event(instance->event_loop, SocketSrvEventAsyncRequest);
    }
}

static void sockets_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);

    SocketSrv* instance = context;

    if(events & SocketSrvEventRequest) {
        const SocketRequest* request = &instance->request;
        const SocketRequestType request_type = request->type;
        furi_check(request_type < SocketRequestTypeAsyncConfirm);

        SocketResponse* response = &instance->response[SocketChannelSync];
        response->type = (SocketResponseType)request->type;

        socket_request_handlers[request_type](request, response);
        sockets_send_response(instance, response);
    }
    if(events & SocketSrvEventAsyncRequest) {
        furi_check(furi_semaphore_release(instance->async_semaphore) == FuriStatusOk);
    }
    if(events & SocketSrvEventAsyncResponse) {
        SocketResponse* response = &instance->response[SocketChannelAsync];
        sockets_send_response(instance, response);
    }
    if(events & SocketSrvEventWifiUp) {
        const sl_status_t status = sl_si91x_config_socket(sockets_backend_config);
        furi_check(status == SL_STATUS_OK);
    }
}

static void sockets_wifi_state_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const WifiState state = *(WifiState*)message;
    SocketSrv* instance = context;

    uint32_t event;

    if(state == WifiStateDeinit) {
        event = SocketSrvEventWifiDeinit;
    } else if(state == WifiStateDown) {
        event = SocketSrvEventWifiDown;
    } else if(state == WifiStateUp) {
        event = SocketSrvEventWifiUp;
    } else {
        furi_crash("Invalid Wifi state");
    }

    furi_event_loop_set_custom_event(instance->event_loop, event);
}

SocketSrv* sockets_alloc(void) {
    SocketSrv* instance = malloc(sizeof(SocketSrv));

    instance->event_loop = furi_event_loop_alloc();
    instance->async_semaphore = furi_semaphore_alloc(1, 1);
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

    socket_srv = sockets_alloc();
    furi_event_loop_run(socket_srv->event_loop);

    return 0;
}

static const SocketRequestHandler socket_request_handlers[NUM_REQUEST_HANDLERS] = {
    [SocketRequestTypeAlloc] = sockets_alloc_request_handler,
    [SocketRequestTypeFree] = sockets_free_request_handler,
    [SocketRequestTypeAccept] = sockets_accept_request_handler,
    [SocketRequestTypeConnect] = sockets_connect_request_handler,
    [SocketRequestTypeSend] = sockets_send_request_handler,
};
