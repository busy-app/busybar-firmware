#include "sockets_common_i.h"

#include <furi.h>
#include <wifi/wifi_common.h>
#include <intercom/intercom.h>

#include <sl_si91x_socket.h>
#include <sl_si91x_socket_utility.h>

#include "sockets_backend_util.h"

#define TAG "SocketSrv"

#define TOTAL_SOCKETS                   (TOTAL_TCP_SOCKETS + TOTAL_UDP_SOCKETS)
#define TOTAL_TCP_SOCKETS               3
#define TOTAL_UDP_SOCKETS               0
#define TCP_TX_ONLY_SOCKETS             0
#define TCP_RX_ONLY_SOCKETS             0
#define UDP_TX_ONLY_SOCKETS             0
#define UDP_RX_ONLY_SOCKETS             0
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 0
#define TCP_RX_WINDOW_SIZE_CAP          44
#define TCP_RX_WINDOW_DIV_FACTOR        44

#define NUM_CLIENTS_PER_SOCKET 1
#define NUM_REQUEST_HANDLERS   (SocketRequestTypeMax - 1)

#define SOCKET_FLAGS_ALL (0x1FUL)

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
    SocketSrvEventWifiDeinit = 1UL << 1,
    SocketSrvEventWifiDown = 1UL << 2,
    SocketSrvEventWifiUp = 1UL << 3,
} SocketSrvEvent;

struct SocketSrv {
    FuriEventLoop* event_loop;
    FuriEventFlag* read_event_flag;
    FuriEventFlag* accept_event_flag;
    Intercom* intercom;
    SocketRequest request;
    SocketResponse response;
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

static void sockets_select_callback(
    fd_set* read_fds,
    fd_set* write_fds,
    fd_set* except_fds,
    int32_t status) {
    UNUSED(write_fds);
    UNUSED(except_fds);
    UNUSED(status);

    const uint32_t socket_bits = read_fds->__fds_bits[0];
    furi_check(socket_bits);

    furi_event_flag_set(socket_srv->read_event_flag, socket_bits);
}

// BUG: params `addr` and `ip_version` do not contain correct data
static void sockets_accept_callback(int32_t socket, struct sockaddr* addr, uint8_t ip_version) {
    UNUSED(addr);
    UNUSED(ip_version);

    const uint32_t socket_bits = (1UL << socket);
    furi_event_flag_set(socket_srv->accept_event_flag, socket_bits);
}

static void sockets_enable_read_events(uint32_t socket_mask) {
    const int nfds = fls(socket_mask);
    fd_set read_fds = {.__fds_bits = {socket_mask}};

    furi_check(
        sl_si91x_select(nfds, &read_fds, NULL, NULL, NULL, sockets_select_callback) ==
        SI91X_NO_ERROR);
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

    const int socket_id = sl_si91x_socket(ip_type, socket_type, socket_protocol);

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

    const int socket_id = connect_request->socket_id;

    const int status = sl_si91x_connect(socket_id, &sl_address.address, sl_address.length);

    if(status < 0) {
        FURI_LOG_E(TAG, "Failed to connect: %s", strerror(errno));
        response->status = SocketStatusError;

    } else {
        sockets_enable_read_events(1UL << socket_id);

        FURI_LOG_D(TAG, "Connection successful");
        response->status = SocketStatusOk;
    }
}

static void sockets_send_request_handler(const SocketRequest* request, SocketResponse* response) {
    FURI_LOG_D(TAG, "Send");

    const SocketSendRequest* send_request = &request->send_request;
    SocketSendResponse* send_response = &response->send_response;

    const int bytes_sent =
        sl_si91x_send(send_request->socket_id, send_request->data, send_request->data_size, 0);

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
    }
}

static void sockets_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);

    SocketSrv* instance = context;

    if(events & SocketSrvEventRequest) {
        const SocketRequest* request = &instance->request;
        const SocketRequestType request_type = request->type;
        furi_check(request_type < SocketRequestTypeAsyncConfirm);

        SocketResponse* response = &instance->response;
        response->type = (SocketResponseType)request->type;

        socket_request_handlers[request_type](request, response);
        sockets_send_response(instance, response);
    }
    if(events & SocketSrvEventWifiUp) {
        const sl_status_t status = sl_si91x_config_socket(sockets_backend_config);
        furi_check(status == SL_STATUS_OK);
    }
}

static void sockets_read_event_flag_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    SocketSrv* instance = context;
    furi_assert(object == instance->read_event_flag);

    uint32_t socket_bits =
        furi_event_flag_wait(instance->read_event_flag, SOCKET_FLAGS_ALL, FuriFlagWaitAny, 0);
    furi_check((socket_bits & FuriFlagError) == 0);

    SocketResponse* response = &socket_srv->response;
    SocketAsyncResponse* async_response = &response->async_response;
    SocketReceiveAsyncResponse* receive_async_response = &async_response->receive_async_response;

    for(int socket_id = 0; socket_id < NUMBER_OF_SOCKETS; ++socket_id) {
        const uint32_t socket_bit = (1UL << socket_id);

        if(socket_bits & socket_bit) {
            async_response->socket_id = socket_id;

            const int data_size = sl_si91x_recv(
                socket_id, receive_async_response->data, sizeof(receive_async_response->data), 0);

            if(data_size > 0) {
                FURI_LOG_D(TAG, "Received %d byte(s) on socket %d", data_size, socket_id);

                response->type = SocketResponseTypeAsyncReceive;
                response->status = SocketStatusOk;

                receive_async_response->data_size = data_size;

            } else {
                FURI_LOG_W(TAG, "Receive failed on socket %d", socket_id);

                response->type = SocketResponseTypeAsyncClose;
                response->status = SocketStatusOk;

                socket_bits &= ~socket_bit;
            }

            sockets_send_response(instance, response);
        }
    }

    if(socket_bits) {
        sockets_enable_read_events(socket_bits);
    }
}

static void sockets_accept_event_flag_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    SocketSrv* instance = context;
    furi_assert(object == instance->accept_event_flag);

    uint32_t socket_bits =
        furi_event_flag_wait(instance->accept_event_flag, SOCKET_FLAGS_ALL, FuriFlagWaitAny, 0);
    furi_check((socket_bits & FuriFlagError) == 0);

    SocketResponse* response = &socket_srv->response;

    response->type = SocketResponseTypeAsyncAccept;
    response->status = SocketStatusOk;

    SocketAsyncResponse* async_response = &response->async_response;
    SocketAcceptAsyncResponse* accept_async_response = &async_response->accept_async_response;

    for(int socket_id = 0; socket_id < NUMBER_OF_SOCKETS; ++socket_id) {
        const uint32_t socket_bit = (1UL << socket_id);

        if(socket_bits & socket_bit) {
            FURI_LOG_D(TAG, "Accepted client socket %d", socket_id);

            async_response->socket_id = sockets_get_parent(socket_id);
            furi_assert(async_response->socket_id >= 0);

            const sli_si91x_socket_t* client_socket = get_si91x_socket(socket_id);
            furi_assert(client_socket);

            sockets_sockaddr_to_connection_info(
                (const struct sockaddr*)&client_socket->remote_address,
                &accept_async_response->connection_info);

            sockets_send_response(instance, response);
        }
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
    instance->read_event_flag = furi_event_flag_alloc();
    instance->accept_event_flag = furi_event_flag_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    FuriPubSub* wifi_pubsub = furi_record_open(RECORD_WIFI);
    FuriPubSubSubscription* sub =
        furi_pubsub_subscribe(wifi_pubsub, sockets_wifi_state_callback, instance);
    UNUSED(sub);

    furi_event_loop_subscribe_event_flag(
        instance->event_loop,
        instance->read_event_flag,
        FuriEventLoopEventIn,
        sockets_read_event_flag_callback,
        instance);

    furi_event_loop_subscribe_event_flag(
        instance->event_loop,
        instance->accept_event_flag,
        FuriEventLoopEventIn,
        sockets_accept_event_flag_callback,
        instance);

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
