#include "sockets_i.h"

static inline SocketEventType sockets_match_response_type(SocketResponseType response_type) {
    return (SocketEventType)(response_type - SocketResponseTypeAsyncReceive);
}

static inline void sockets_send_request(SocketSrv* instance, const SocketRequest* request) {
    const size_t request_size = sockets_get_request_size(request);
    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelSockets, request, request_size, FuriWaitForever);
    furi_check(tx_size == request_size);
}

static void sockets_process_request(SocketSrv* instance) {
    const SocketSrvMessage* message = instance->current_message;
    const SocketRequestType request_type = message->request_type;

    SocketRequest* request = &instance->request;
    request->type = request_type;

    if(request_type == SocketRequestTypeAlloc) {
        SocketAllocRequest* alloc_request = &request->alloc_request;
        const SocketSrvAllocMessage* alloc_message = &message->alloc_message;

        alloc_request->socket_info = *alloc_message->socket_info;

    } else if(request_type == SocketRequestTypeFree) {
        SocketFreeRequest* free_request = &request->free_request;
        const SocketSrvFreeMessage* free_message = &message->free_message;

        free_request->socket_id = free_message->socket_id;

    } else if(request_type == SocketRequestTypeBind) {
        SocketBindRequest* bind_request = &request->bind_request;
        const SocketSrvBindMessage* bind_message = &message->bind_message;

        bind_request->socket_id = bind_message->socket_id;
        bind_request->bind_info = *bind_message->bind_info;

    } else if(request_type == SocketRequestTypeListen) {
        SocketListenRequest* listen_request = &request->listen_request;
        const SocketSrvListenMessage* listen_message = &message->listen_message;

        listen_request->socket_id = listen_message->socket_id;
        listen_request->max_clients = listen_message->max_clients;

    } else if(request_type == SocketRequestTypeAccept) {
        SocketAcceptRequest* accept_request = &request->accept_request;
        const SocketSrvAcceptMessage* accept_message = &message->accept_message;

        accept_request->socket_id = accept_message->socket_id;

    } else if(request_type == SocketRequestTypeConnect) {
        SocketConnectRequest* connect_request = &request->connect_request;
        const SocketSrvConnectMessage* connect_message = &message->connect_message;

        connect_request->socket_id = connect_message->socket_id;
        connect_request->connection_info = *connect_message->connection_info;

    } else if(request_type == SocketRequestTypeSend) {
        SocketSendRequest* send_request = &request->send_request;
        const SocketSrvSendMessage* send_message = &message->send_message;

        const size_t chunk_size = MIN(send_message->data_size, SOCKET_SEND_DATA_SIZE);

        send_request->socket_id = send_message->socket_id;
        send_request->data_size = chunk_size;
        memcpy(send_request->data, send_message->data, chunk_size);

    } else if(request_type == SocketRequestTypeReceive) {
        SocketReceiveRequest* receive_request = &request->receive_request;
        const SocketSrvReceiveMessage* receive_message = &message->receive_message;

        // TODO: Receive more than one chunk in one request?
        const size_t chunk_size = MIN(receive_message->data_size, SOCKET_SEND_DATA_SIZE);

        receive_request->socket_id = receive_message->socket_id;
        receive_request->data_size = chunk_size;

    } else {
        furi_crash("Invalid request type");
    }

    sockets_send_request(instance, request);
}

static Socket* sockets_alloc_socket(
    SocketSrv* instance,
    uint8_t socket_id,
    SocketEventCallback callback,
    void* context) {
    furi_assert(socket_id < SOCKET_COUNT);

    Socket** socket_slot = &instance->sockets[socket_id];
    furi_check(*socket_slot == NULL);

    Socket* socket = malloc(sizeof(Socket));

    socket->id = socket_id;
    socket->owner = instance;
    socket->event_callback = callback;
    socket->callback_context = context;

    *socket_slot = socket;
    return socket;
}

static void sockets_free_socket(SocketSrv* instance, uint8_t socket_id) {
    furi_assert(socket_id < SOCKET_COUNT);

    Socket** socket_slot = &instance->sockets[socket_id];
    Socket* socket = *socket_slot;
    furi_check(socket);
    free(socket);

    *socket_slot = NULL;
}

static void sockets_process_response(SocketSrv* instance, const SocketResponse* response) {
    SocketSrvMessage* message = instance->current_message;
    furi_assert(message);

    const SocketResponseType response_type = response->type;
    message->status = response->status;

    if(message->status == SocketStatusOk) {
        if(response_type == SocketResponseTypeAlloc) {
            SocketSrvAllocMessage* alloc_message = &message->alloc_message;
            const SocketAllocResponse* alloc_response = &response->alloc_response;
            alloc_message->socket = sockets_alloc_socket(
                instance,
                alloc_response->socket_id,
                alloc_message->event_callback,
                alloc_message->callback_context);

        } else if(response_type == SocketResponseTypeFree) {
            SocketSrvFreeMessage* free_message = &message->free_message;
            sockets_free_socket(instance, free_message->socket_id);

        } else if(response_type == SocketResponseTypeSend) {
            SocketSrvSendMessage* send_message = &message->send_message;
            const SocketSendResponse* send_response = &response->send_response;

            if(send_message->sent_size) {
                *send_message->sent_size = send_response->sent_size;
            }

        } else if(response_type == SocketResponseTypeReceive) {
            SocketSrvReceiveMessage* receive_message = &message->receive_message;
            const SocketReceiveResponse* receive_response = &response->receive_response;

            memcpy(receive_message->data, receive_response->data, receive_response->data_size);

            if(receive_message->received_size) {
                *receive_message->received_size = receive_response->data_size;
            }

        } else {
            /* Do nothing */
        }
    }

    api_lock_unlock(message->lock);
    furi_check(furi_semaphore_release(instance->access_semaphore) == FuriStatusOk);
}

static void sockets_process_async_response(SocketSrv* instance, const SocketResponse* response) {
    const SocketResponseType response_type = response->type;

    const SocketAsyncResponse* async_response = &response->async_response;

    const uint8_t socket_id = async_response->socket_id;
    furi_assert(socket_id < SOCKET_COUNT);

    Socket* socket = instance->sockets[socket_id];
    furi_assert(socket);

    SocketEvent event = {
        .socket = socket,
        .type = sockets_match_response_type(response_type),
    };

    if(response_type == SocketResponseTypeAsyncAccept) {
        SocketAcceptEvent* accept_event = &event.accept;
        const SocketAcceptAsyncResponse* accept_async_response =
            &async_response->accept_async_response;

        accept_event->client_socket = sockets_alloc_socket(
            instance,
            accept_async_response->client_socket_id,
            socket->event_callback,
            socket->callback_context);
        accept_event->connection_info = accept_async_response->connection_info;

    } else {
        /* Do nothing*/
    }

    if(socket->event_callback) {
        socket->event_callback(&event, socket->callback_context);
    }
}

static void sockets_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    SocketSrv* instance = context;

    const SocketResponse* response = data;
    furi_assert(data_size == sockets_get_response_size(response));

    const SocketResponseType response_type = response->type;

    if(response_type < SocketResponseTypeAsyncReceive) {
        sockets_process_response(instance, response);
    } else if(response_type < SocketResponseTypeMax) {
        sockets_process_async_response(instance, response);
    } else {
        furi_crash("Invalid response type");
    }
}

static void sockets_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    SocketSrv* instance = context;

    if(events & SocketSrvEventRequest) {
        sockets_process_request(instance);
    }
}

SocketSrv* sockets_alloc(void) {
    SocketSrv* instance = malloc(sizeof(SocketSrv));

    instance->event_loop = furi_event_loop_alloc();
    instance->access_semaphore = furi_semaphore_alloc(1, 1);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sockets_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, sockets_intercom_rx_callback, instance);

    furi_record_create(RECORD_SOCKETS, instance);

    return instance;
}

int32_t sockets_srv(void* arg) {
    UNUSED(arg);

    SocketSrv* instance = sockets_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
