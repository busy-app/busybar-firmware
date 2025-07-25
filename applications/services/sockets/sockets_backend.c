#include "sockets_common_i.h"

#include <furi.h>
#include <intercom/intercom.h>

#define TAG "SocketSrv"

#define SOCKET_FLAGS_ALL (0x1FUL)

typedef enum {
    SocketSrvEventRequest = 1UL << 0,
    SocketSrvEventWifiDeinit = 1UL << 1,
    SocketSrvEventWifiDown = 1UL << 2,
    SocketSrvEventWifiUp = 1UL << 3,
} SocketSrvEvent;

struct SocketSrv {
    FuriEventLoop* event_loop;
    Intercom* intercom;
    SocketRequest request;
    SocketResponse response;
};

typedef ssize_t (*SocketRequestHandler)(const SocketRequest* request, SocketResponse* response);

static const SocketRequestHandler socket_request_handlers[SocketRequestTypeMax];

/* Global sockets instance, needed for socket callbacks */
static SocketSrv* socket_srv;

static inline void sockets_send_response(SocketSrv* instance, const SocketResponse* response) {
    const size_t response_size = sockets_get_response_size(response);
    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelSockets, response, response_size, FuriWaitForever);
    furi_assert(tx_size == response_size);
}

static ssize_t
    sockets_socket_request_handler(const SocketRequest* request, SocketResponse* response) {
    UNUSED(response);

    const SocketAllocRequest* alloc_request = &request->alloc_request;
    int socket_id, status = -1;

    do {
        socket_id = socket(alloc_request->domain, alloc_request->type, alloc_request->protocol);

        if(socket_id < 0) {
            break;
        }

        status = socket_id;

    } while(false);

    if(status >= 0) {
        FURI_LOG_D(TAG, "new socket with fd: %d", socket_id);
    }

    return status;
}

static ssize_t
    sockets_close_request_handler(const SocketRequest* request, SocketResponse* response) {
    UNUSED(response);

    return close(request->socket_id);
}

static ssize_t
    sockets_bind_request_handler(const SocketRequest* request, SocketResponse* response) {
    UNUSED(response);

    const SocketBindRequest* bind_request = &request->bind_request;
    // Needed to align the &name pointer to 4 bytes
    struct sockaddr name;
    memcpy(&name, &bind_request->name, bind_request->namelen);

    return bind(request->socket_id, &name, bind_request->namelen);
}

static ssize_t
    sockets_listen_request_handler(const SocketRequest* request, SocketResponse* response) {
    UNUSED(response);

    const SocketListenRequest* listen_request = &request->listen_request;
    return listen(request->socket_id, listen_request->backlog);
}

static ssize_t
    sockets_accept_request_handler(const SocketRequest* request, SocketResponse* response) {
    const SocketAcceptRequest* accept_request = &request->accept_request;
    SocketAcceptResponse* accept_response = &response->accept_response;

    accept_response->addrlen = accept_request->addrlen;
    return accept(request->socket_id, &accept_response->addr, &accept_response->addrlen);
}

static ssize_t
    sockets_connect_request_handler(const SocketRequest* request, SocketResponse* response) {
    UNUSED(response);

    const SocketConnectRequest* connect_request = &request->connect_request;

    // Needed to align the &name pointer to 4 bytes
    struct sockaddr name;
    memcpy(&name, &connect_request->name, connect_request->namelen);

    return connect(request->socket_id, &name, connect_request->namelen);
}

static ssize_t
    sockets_send_request_handler(const SocketRequest* request, SocketResponse* response) {
    UNUSED(response);

    const SocketSendRequest* send_request = &request->send_request;
    ssize_t bytes_sent;

    if(send_request->tolen) {
        bytes_sent = sendto(
            request->socket_id,
            send_request->data,
            send_request->size,
            0,
            &send_request->to,
            send_request->tolen);
    } else {
        bytes_sent = send(request->socket_id, send_request->data, send_request->size, 0);
    }
#ifdef SOCKETS_SLOW_LOGS
    if(bytes_sent > 0) {
        FURI_LOG_D(TAG, "bytes sent: %d", bytes_sent);
    }
#endif
    return bytes_sent;
}

static ssize_t
    sockets_recv_request_handler(const SocketRequest* request, SocketResponse* response) {
    const SocketReceiveRequest* receive_request = &request->receive_request;
    SocketReceiveResponse* receive_response = &response->receive_response;

    ssize_t bytes_received;

    if(receive_request->include_from) {
        socklen_t fromlen;
        bytes_received = recvfrom(
            request->socket_id,
            receive_response->data,
            receive_request->len,
            0,
            &receive_response->from,
            &fromlen);
        receive_response->fromlen = fromlen;

    } else {
        bytes_received = recv(request->socket_id, receive_response->data, receive_request->len, 0);
    }
#ifdef SOCKETS_SLOW_LOGS
    if(bytes_received > 0) {
        FURI_LOG_D(TAG, "bytes received: %d", bytes_received);
    }
#endif
    return bytes_received;
}

static ssize_t
    sockets_getsockname_request_handler(const SocketRequest* request, SocketResponse* response) {
    SocketGetSockNameResponse* getsockname_response = &response->getsockname_response;
    return getsockname(
        request->socket_id, &getsockname_response->name, &getsockname_response->namelen);
}

static ssize_t
    sockets_getpeername_request_handler(const SocketRequest* request, SocketResponse* response) {
    SocketGetPeerNameResponse* getpeername_response = &response->getpeername_response;
    return getpeername(
        request->socket_id, &getpeername_response->name, &getpeername_response->namelen);
}

static ssize_t
    sockets_setsockopt_request_handler(const SocketRequest* request, SocketResponse* response) {
    UNUSED(response);

    const SocketSetSockOptRequest* setsockopt_request = &request->setsockopt_request;
    return setsockopt(
        request->socket_id,
        setsockopt_request->level,
        setsockopt_request->optname,
        &setsockopt_request->optval,
        setsockopt_request->optlen);
}

static ssize_t
    sockets_getsockopt_request_handler(const SocketRequest* request, SocketResponse* response) {
    const SocketGetSockOptRequest* getsockopt_request = &request->getsockopt_request;
    SocketGetSockOptResponse* getsockopt_response = &response->getsockopt_response;

    socklen_t optlen;
    const int status = getsockopt(
        request->socket_id,
        getsockopt_request->level,
        getsockopt_request->optname,
        &getsockopt_response->optval,
        &optlen);

    getsockopt_response->optlen = optlen;
    return status;
}

static ssize_t
    sockets_select_request_handler(const SocketRequest* request, SocketResponse* response) {
    const SocketSelectRequest* select_request = &request->select_request;
    SocketSelectResponse* select_response = &response->select_response;

    // Sets must be aligned to 4 bytes
    fd_set readset, writeset, exceptset;

    memcpy(&readset, &select_request->readset, sizeof(readset));
    memcpy(&writeset, &select_request->writeset, sizeof(writeset));
    memcpy(&exceptset, &select_request->exceptset, sizeof(exceptset));

    struct timeval timeout = {
        .tv_sec = select_request->timeout.sec,
        .tv_usec = select_request->timeout.usec,
    };

    const ssize_t status =
        select(select_request->maxfdp1, &readset, &writeset, &exceptset, &timeout);

    memcpy(&select_response->readset, &readset, sizeof(select_request->readset));
    memcpy(&select_response->writeset, &writeset, sizeof(select_request->writeset));
    memcpy(&select_response->exceptset, &exceptset, sizeof(select_request->exceptset));

    return status;
}

static void sockets_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    SocketSrv* instance = context;

    const SocketRequest* request = data;
    furi_assert(data_size == sockets_get_request_size(request));

    memcpy(&instance->request, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, SocketSrvEventRequest);
}

static void sockets_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);

    SocketSrv* instance = context;

    if(events & SocketSrvEventRequest) {
        const SocketRequest* request = &instance->request;
        const SocketRequestType request_type = request->type;

        SocketResponse* response = &instance->response;
        response->type = (SocketResponseType)request_type;

        FURI_LOG_D(TAG, "%s fd: %d", sockets_get_request_name(request_type), request->socket_id);

        response->status = socket_request_handlers[request_type](request, response);
        response->_errno = errno;

        if(response->status < 0) {
            FURI_LOG_E(
                TAG, "%s failed: %s", sockets_get_request_name(request_type), strerror(errno));
        }

        sockets_send_response(instance, response);
    }
}

SocketSrv* sockets_alloc(void) {
    SocketSrv* instance = malloc(sizeof(SocketSrv));

    instance->event_loop = furi_event_loop_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

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

static const SocketRequestHandler socket_request_handlers[SocketRequestTypeMax] = {
    [SocketRequestTypeAlloc] = sockets_socket_request_handler,
    [SocketRequestTypeFree] = sockets_close_request_handler,
    [SocketRequestTypeBind] = sockets_bind_request_handler,
    [SocketRequestTypeListen] = sockets_listen_request_handler,
    [SocketRequestTypeAccept] = sockets_accept_request_handler,
    [SocketRequestTypeConnect] = sockets_connect_request_handler,
    [SocketRequestTypeSend] = sockets_send_request_handler,
    [SocketRequestTypeReceive] = sockets_recv_request_handler,
    [SocketRequestTypeGetSockName] = sockets_getsockname_request_handler,
    [SocketRequestTypeGetPeerName] = sockets_getpeername_request_handler,
    [SocketRequestTypeSetSockOpt] = sockets_setsockopt_request_handler,
    [SocketRequestTypeGetSockOpt] = sockets_getsockopt_request_handler,
    [SocketRequestTypeSelect] = sockets_select_request_handler,
};
