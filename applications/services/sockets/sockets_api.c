#include "sockets_i.h"

static void sockets_send_message(SocketSrvMessage* message) {
    extern SocketSrv* instance;
    furi_check(instance);

    message->lock = api_lock_alloc_locked();

    furi_check(
        furi_semaphore_acquire(instance->access_semaphore, FuriWaitForever) == FuriStatusOk);

    instance->current_message = message;
    furi_event_loop_set_custom_event(instance->event_loop, SocketSrvEventRequest);

    api_lock_wait_unlock_and_free(message->lock);
}

int sl_socket(int domain, int type, int protocol) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeAlloc,
        .alloc_message =
            {
                .domain = domain,
                .type = type,
                .protocol = protocol,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

int sl_bind(int s, const struct sockaddr* name, socklen_t namelen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeBind,
        .socket_fd = s,
        .bind_message = {
            .name = name,
            .namelen = namelen,
        }};

    sockets_send_message(&msg);
    return msg.status;
}

int sl_getsockname(int s, struct sockaddr* name, socklen_t* namelen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeGetSockName,
        .socket_fd = s,
        .getsockname_message =
            {
                .name = name,
                .namelen = namelen,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

int sl_connect(int s, const struct sockaddr* name, socklen_t namelen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeConnect,
        .socket_fd = s,
        .connect_message = {
            .name = name,
            .namelen = namelen,
        }};

    sockets_send_message(&msg);
    return msg.status;
}

int sl_getpeername(int s, struct sockaddr* name, socklen_t* namelen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeGetPeerName,
        .socket_fd = s,
        .getpeername_message =
            {
                .name = name,
                .namelen = namelen,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

ssize_t sl_send(int s, const void* dataptr, size_t size, int flags) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeSend,
        .socket_fd = s,
        .send_message =
            {
                .dataptr = dataptr,
                .size = size,
                .flags = flags,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

ssize_t sl_recv(int s, void* mem, size_t len, int flags) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeReceive,
        .socket_fd = s,
        .receive_message =
            {
                .mem = mem,
                .len = len,
                .flags = flags,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

ssize_t sl_sendto(
    int s,
    const void* dataptr,
    size_t size,
    int flags,
    const struct sockaddr* to,
    socklen_t tolen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeSend,
        .socket_fd = s,
        .send_message =
            {
                .dataptr = dataptr,
                .size = size,
                .flags = flags,
                .to = to,
                .tolen = tolen,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

ssize_t
    sl_recvfrom(int s, void* mem, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeReceive,
        .socket_fd = s,
        .receive_message =
            {
                .mem = mem,
                .len = len,
                .flags = flags,
                .from = from,
                .fromlen = fromlen,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

int sl_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeGetSockOpt,
        .socket_fd = s,
        .getsockopt_message =
            {
                .level = level,
                .optname = optname,
                .optval = optval,
                .optlen = optlen,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

int sl_setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeSetSockOpt,
        .socket_fd = s,
        .setsockopt_message =
            {
                .level = level,
                .optname = optname,
                .optval = optval,
                .optlen = optlen,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

int sl_listen(int s, int backlog) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeListen,
        .socket_fd = s,
        .listen_message =
            {
                .backlog = backlog,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

int sl_accept(int s, struct sockaddr* addr, socklen_t* addrlen) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeAccept,
        .socket_fd = s,
        .accept_message =
            {
                .addr = addr,
                .addrlen = addrlen,
            },
    };

    sockets_send_message(&msg);
    return msg.status;
}

int sl_close(int s) {
    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeFree,
        .socket_fd = s,
    };

    sockets_send_message(&msg);
    return msg.status;
}
