#include "sockets_i.h"

static void sockets_send_message(SocketSrv* instance, SocketSrvMessage* message) {
    message->lock = api_lock_alloc_locked();

    furi_check(
        furi_semaphore_acquire(instance->access_semaphore, FuriWaitForever) == FuriStatusOk);

    instance->current_message = message;
    furi_event_loop_set_custom_event(instance->event_loop, SocketSrvEventRequest);

    api_lock_wait_unlock_and_free(message->lock);
}

Socket* socket_alloc(SocketSrv* instance, const SocketInfo* socket_info) {
    furi_check(instance);
    furi_check(socket_info);

    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeAlloc,
        .alloc_message =
            {
                .socket_info = socket_info,
            },
    };

    sockets_send_message(instance, &msg);
    return msg.alloc_message.socket;
}

SocketStatus socket_free(Socket* socket) {
    furi_check(socket);

    SocketSrv* instance = socket->owner;
    furi_assert(instance);

    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeFree,
        .free_message =
            {
                .socket_id = socket->id,
            },
    };

    sockets_send_message(instance, &msg);
    return msg.status;
}

// FIXME: Not thread safe...ish
SocketStatus
    socket_set_event_callback(Socket* socket, SocketEventCallback callback, void* context) {
    furi_check(socket);

    socket->event_callback = callback;
    socket->callback_context = context;

    return SocketStatusOk;
}

SocketStatus socket_bind(Socket* socket, const SocketConnectionInfo* bind_info) {
    furi_check(socket);
    furi_check(bind_info);

    SocketSrv* instance = socket->owner;
    furi_assert(instance);

    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeBind,
        .bind_message =
            {
                .socket_id = socket->id,
                .bind_info = bind_info,
            },
    };

    sockets_send_message(instance, &msg);
    return msg.status;
}

SocketStatus socket_listen(Socket* socket, uint8_t max_clients) {
    furi_check(socket);

    SocketSrv* instance = socket->owner;
    furi_assert(instance);

    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeListen,
        .listen_message =
            {
                .socket_id = socket->id,
                .max_clients = max_clients,
            },
    };

    sockets_send_message(instance, &msg);
    return msg.status;
}

SocketStatus socket_connect(Socket* socket, const SocketConnectionInfo* connection_info) {
    furi_check(socket);
    furi_check(connection_info);

    SocketSrv* instance = socket->owner;
    furi_assert(instance);

    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeConnect,
        .connect_message =
            {
                .socket_id = socket->id,
                .connection_info = connection_info,
            },
    };

    sockets_send_message(instance, &msg);
    return msg.status;
}

SocketStatus socket_send(Socket* socket, const void* data, size_t data_size, size_t* sent_size) {
    furi_check(socket);
    furi_check(data);

    SocketSrv* instance = socket->owner;
    furi_assert(instance);

    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeSend,
        .send_message =
            {
                .socket_id = socket->id,
                .data = data,
                .data_size = data_size,
                .sent_size = sent_size,
            },
    };

    sockets_send_message(instance, &msg);
    return msg.status;
}

SocketStatus socket_receive(Socket* socket, void* data, size_t data_size, size_t* received_size) {
    furi_check(socket);
    furi_check(data);

    SocketSrv* instance = socket->owner;
    furi_assert(instance);

    SocketSrvMessage msg = {
        .request_type = SocketRequestTypeReceive,
        .receive_message =
            {
                .socket_id = socket->id,
                .data = data,
                .data_size = data_size,
                .received_size = received_size,
            },
    };

    sockets_send_message(instance, &msg);
    return msg.status;
}
