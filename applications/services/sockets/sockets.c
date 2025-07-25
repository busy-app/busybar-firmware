#include "sockets_i.h"

static SocketSrv* instance;

static void sockets_process_response(const SocketResponse* response) {
    const SocketResponseType response_type = response->type;
    const ssize_t status = response->status;

    if(status >= 0) {
        const SocketReturnParams* ret = &instance->ret;

        if(response_type == SocketResponseTypeReceive) {
            const SocketReceiveResponse* recv_response = &response->receive_response;
            const SocketRecvParams* recv_params = &ret->recv_params;

            memcpy(recv_params->mem, recv_response->data, status);

            if(recv_params->from) {
                memcpy(recv_params->from, &recv_response->from, recv_response->fromlen);
                *recv_params->fromlen = recv_response->fromlen;
            }

        } else if(response_type == SocketResponseTypeAccept) {
            const SocketAcceptResponse* accept_response = &response->accept_response;
            const SocketAcceptParams* accept_params = &ret->accept_params;

            if(accept_params->addr) {
                memcpy(accept_params->addr, &accept_response->addr, accept_response->addrlen);
                *accept_params->addrlen = accept_response->addrlen;
            }

        } else if(response_type == SocketResponseTypeGetPeerName) {
            const SocketGetPeerNameResponse* getpeername_response =
                &response->getpeername_response;
            const SocketGetPeerNameParams* getpeername_params = &ret->getpeername_params;

            if(getpeername_params->name) {
                memcpy(
                    getpeername_params->name,
                    &getpeername_response->name,
                    getpeername_response->namelen);
                *getpeername_params->namelen = getpeername_response->namelen;
            }

        } else if(response_type == SocketResponseTypeGetSockName) {
            const SocketGetSockNameResponse* getsockname_response =
                &response->getsockname_response;
            const SocketGetSockNameParams* getsockname_params = &ret->getsockname_params;

            if(getsockname_params->name) {
                memcpy(
                    getsockname_params->name,
                    &getsockname_response->name,
                    getsockname_response->namelen);
                *getsockname_params->namelen = getsockname_response->namelen;
            }

        } else if(response_type == SocketResponseTypeGetSockOpt) {
            const SocketGetSockOptResponse* getsockopt_response = &response->getsockopt_response;
            const SocketGetSockOptParams* getsockopt_params = &ret->getsockopt_params;

            if(getsockopt_params->optval) {
                memcpy(
                    getsockopt_params->optval,
                    &getsockopt_response->optval,
                    getsockopt_response->optlen);
                *getsockopt_params->optlen = getsockopt_response->optlen;
            }

        } else if(response_type == SocketResponseTypeSelect) {
            const SocketSelectResponse* select_response = &response->select_response;
            const SocketSelectParams* select_params = &ret->select_params;

            if(select_params->readset) {
                memcpy(
                    select_params->readset,
                    &select_response->readset,
                    sizeof(select_response->readset));
            }

            if(select_params->writeset) {
                memcpy(
                    select_params->writeset,
                    &select_response->writeset,
                    sizeof(select_response->writeset));
            }

        } else {
            // Do nothing
        }
    }

    instance->status = status;
    instance->saved_errno = response->_errno;

    furi_check(furi_semaphore_release(instance->response_semaphore) == FuriStatusOk);
}

// static void sockets_process_async_response(SocketSrv* instance, const SocketResponse* response) {
//     const SocketResponseType response_type = response->type;
//
//     const SocketAsyncResponse* async_response = &response->async_response;
//
//     const uint8_t socket_id = async_response->socket_id;
//     furi_assert(socket_id < SOCKET_COUNT);
//
//     Socket* socket = instance->sockets[socket_id];
//     furi_assert(socket);
//
//     SocketEvent event = {
//         .socket = socket,
//         .type = sockets_match_response_type(response_type),
//     };
//
//     if(response_type == SocketResponseTypeAsyncAccept) {
//         SocketAcceptEvent* accept_event = &event.accept;
//         const SocketAcceptAsyncResponse* accept_async_response =
//             &async_response->accept_async_response;
//
//         accept_event->client_socket = sockets_alloc_socket(
//             instance,
//             accept_async_response->client_socket_id,
//             socket->event_callback,
//             socket->callback_context);
//         accept_event->connection_info = accept_async_response->connection_info;
//
//     } else {
//         /* Do nothing*/
//     }
//
//     if(socket->event_callback) {
//         socket->event_callback(&event, socket->callback_context);
//     }
// }
//
static void sockets_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    UNUSED(context);

    const SocketResponse* response = data;
    furi_assert(data_size == sockets_get_response_size(response));

    const SocketResponseType response_type = response->type;

    if(response_type < SocketResponseTypeAsyncAccept) {
        sockets_process_response(response);
    } else if(response_type < SocketResponseTypeMax) {
        furi_crash("Async responses not implemented");
        //         sockets_process_async_response(instance, response);
    } else {
        furi_crash("Invalid response type");
    }
}

static void sockets_lock(void) {
    furi_check(
        furi_semaphore_acquire(instance->access_semaphore, FuriWaitForever) == FuriStatusOk);
}

static void sockets_unlock(void) {
    furi_check(furi_semaphore_release(instance->access_semaphore) == FuriStatusOk);
}

static void sockets_send_request(void) {
    const SocketRequest* request = &instance->request;

    const size_t request_size = sockets_get_request_size(request);
    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelSockets, request, request_size, FuriWaitForever);
    furi_check(tx_size == request_size);
}

static ssize_t sockets_wait_for_response(void) {
    furi_semaphore_acquire(instance->response_semaphore, FuriWaitForever);

    // TODO: Better errno?
    errno = instance->saved_errno;

    return instance->status;
}

int sl_socket(int domain, int type, int protocol) {
    furi_check(instance);
    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeAlloc;

    SocketAllocRequest* alloc_request = &request->alloc_request;
    alloc_request->domain = domain;
    alloc_request->type = type;
    alloc_request->protocol = protocol;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_bind(int s, const struct sockaddr* name, socklen_t namelen) {
    furi_check(instance);
    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeBind;
    request->socket_id = s;

    SocketBindRequest* bind_request = &request->bind_request;
    memcpy(&bind_request->name, name, namelen);
    bind_request->namelen = namelen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_getsockname(int s, struct sockaddr* name, socklen_t* namelen) {
    furi_check(instance);
    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeGetSockName;
    request->socket_id = s;

    SocketReturnParams* ret = &instance->ret;
    // TODO: is type necessary here?

    SocketGetSockNameParams* getsockname_params = &ret->getsockname_params;
    getsockname_params->name = name;
    getsockname_params->namelen = namelen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_connect(int s, const struct sockaddr* name, socklen_t namelen) {
    furi_check(instance);
    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeConnect;
    request->socket_id = s;

    SocketConnectRequest* connect_request = &request->connect_request;
    memcpy(&connect_request->name, name, namelen);
    connect_request->namelen = namelen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_getpeername(int s, struct sockaddr* name, socklen_t* namelen) {
    furi_check(instance);
    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeGetPeerName;
    request->socket_id = s;

    SocketReturnParams* ret = &instance->ret;
    // TODO: is type necessary here?

    SocketGetPeerNameParams* getpeername_params = &ret->getpeername_params;
    getpeername_params->name = name;
    getpeername_params->namelen = namelen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

ssize_t sl_sendto(
    int s,
    const void* dataptr,
    size_t size,
    int flags,
    const struct sockaddr* to,
    socklen_t tolen) {
    UNUSED(flags);
    furi_check(instance);

    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeSend;
    request->socket_id = s;

    SocketSendRequest* send_request = &request->send_request;
    const uint16_t send_size = MIN(size, sizeof(send_request->data));

    memcpy(send_request->data, dataptr, send_size);
    send_request->size = send_size;

    if(to != NULL) {
        memcpy(&send_request->to, to, tolen);
    }

    send_request->tolen = tolen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

ssize_t
    sl_recvfrom(int s, void* mem, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen) {
    UNUSED(flags);
    furi_check(instance);
    furi_check(mem);

    sockets_lock();

    SocketRequest* request = &instance->request;
    // TODO: How to differentiate recv/recvfrom?
    request->type = SocketRequestTypeReceive;
    request->socket_id = s;

    SocketReceiveRequest* recv_request = &request->receive_request;
    recv_request->len = len;
    recv_request->include_from = (from != NULL);

    SocketReturnParams* ret = &instance->ret;
    // TODO: is type necessary here?

    SocketRecvParams* recv_params = &ret->recv_params;
    recv_params->mem = mem;
    recv_params->from = from;
    recv_params->fromlen = fromlen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

ssize_t sl_send(int s, const void* dataptr, size_t size, int flags) {
    return sl_sendto(s, dataptr, size, flags, NULL, 0);
}

ssize_t sl_recv(int s, void* mem, size_t len, int flags) {
    return sl_recvfrom(s, mem, len, flags, NULL, NULL);
}

int sl_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen) {
    furi_check(instance);
    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeGetSockOpt;
    request->socket_id = s;

    SocketGetSockOptRequest* getsockopt_request = &request->getsockopt_request;
    getsockopt_request->level = level;
    getsockopt_request->optname = optname;

    SocketReturnParams* ret = &instance->ret;
    // TODO: is type necessary here?

    SocketGetSockOptParams* getsockopt_params = &ret->getsockopt_params;
    getsockopt_params->optval = optval;
    getsockopt_params->optlen = optlen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen) {
    furi_check(instance);
    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeSetSockOpt;
    request->socket_id = s;

    SocketSetSockOptRequest* setsockopt_request = &request->setsockopt_request;
    setsockopt_request->level = level;
    setsockopt_request->optname = optname;

    if(optval != NULL) {
        furi_assert(optlen <= sizeof(setsockopt_request->optval));
        memcpy(&setsockopt_request->optval, optval, optlen);
    }

    setsockopt_request->optlen = optlen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_listen(int s, int backlog) {
    furi_check(instance);

    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeListen;
    request->socket_id = s;

    SocketListenRequest* listen_request = &request->listen_request;
    listen_request->backlog = backlog;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_accept(int s, struct sockaddr* addr, socklen_t* addrlen) {
    furi_check(instance);

    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeAccept;
    request->socket_id = s;

    SocketAcceptRequest* accept_request = &request->accept_request;
    accept_request->addrlen = *addrlen;

    SocketReturnParams* ret = &instance->ret;
    // TODO: is type necessary here?

    SocketAcceptParams* accept_params = &ret->accept_params;
    accept_params->addr = addr;
    accept_params->addrlen = addrlen;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_select(
    int maxfdp1,
    fd_set* readset,
    fd_set* writeset,
    fd_set* exceptset,
    struct timeval* timeout) {
    furi_check(instance);

    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeSelect;
    request->socket_id = 0;

    SocketSelectRequest* select_request = &request->select_request;
    select_request->maxfdp1 = maxfdp1;

    if(readset) {
        memcpy(&select_request->readset, readset, sizeof(select_request->readset));
    } else {
        memset(&select_request->readset, 0, sizeof(select_request->readset));
    }

    if(writeset) {
        memcpy(&select_request->writeset, writeset, sizeof(select_request->writeset));
    } else {
        memset(&select_request->writeset, 0, sizeof(select_request->writeset));
    }

    if(exceptset) {
        memcpy(&select_request->exceptset, exceptset, sizeof(select_request->exceptset));
    } else {
        memset(&select_request->exceptset, 0, sizeof(select_request->exceptset));
    }

    select_request->timeout.sec = timeout->tv_sec;
    select_request->timeout.usec = timeout->tv_usec;

    SocketReturnParams* ret = &instance->ret;
    // TODO: is type necessary here?

    SocketSelectParams* select_params = &ret->select_params;
    select_params->readset = readset;
    select_params->writeset = writeset;
    select_params->exceptset = exceptset;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

int sl_close(int s) {
    furi_check(instance);

    sockets_lock();

    SocketRequest* request = &instance->request;
    request->type = SocketRequestTypeFree;
    request->socket_id = s;

    sockets_send_request();
    const ssize_t status = sockets_wait_for_response();

    sockets_unlock();
    return status;
}

static SocketSrv* sockets_alloc(void) {
    SocketSrv* instance = malloc(sizeof(SocketSrv));
    instance->access_semaphore = furi_semaphore_alloc(1, 1);
    instance->response_semaphore = furi_semaphore_alloc(1, 0);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, sockets_intercom_rx_callback, instance);

    return instance;
}

int32_t sockets_on_system_start(void* arg) {
    UNUSED(arg);

    instance = sockets_alloc();

    return 0;
}
