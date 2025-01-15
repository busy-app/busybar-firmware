#include "sockets.h"
#include "sockets_common_i.h"

#include <furi.h>
#include <intercom/intercom.h>

#define SOCKET_COUNT           (20)
#define SOCKET_RX_BUFFER_LEN   (2048UL)
#define SOCKET_EVENT_QUEUE_LEN (16)

#define TAG "Sockets"

typedef struct {
    const SocketInfo* socket_info;
    Socket* socket;
} SocketsAllocMessage;

typedef struct {
    const uint8_t socket_id;
} SocketsFreeMessage;

typedef struct {
    const uint8_t socket_id;
    const SocketConnectionInfo* connection_info;
} SocketsConnectMessage;

typedef struct {
    const uint8_t socket_id;
    const void* data;
    const size_t data_size;
    size_t* sent_size;
} SocketsSendMessage;

typedef struct {
    SocketRequestType request_type;
    SocketStatus status;
    union {
        SocketsAllocMessage alloc_message;
        SocketsFreeMessage free_message;
        SocketsConnectMessage connect_message;
        SocketsSendMessage send_message;
    };
} SocketsMessage;

typedef enum {
    SocketsCustomEventRequest = 1UL << 0,
    SocketsCustomEventResponse = 1UL << 1,
} SocketsCustomEvent;

typedef enum {
    SocketsEventFlagReady = 1UL << 0,
    SocketsEventFlagDone = 1UL << 1,
} SocketsEventFlag;

struct Socket {
    uint8_t id;
    Sockets* owner;
    FuriStreamBuffer* rx_buffer;
    FuriMessageQueue* event_queue;
    SocketEventCallback event_callback;
    void* callback_context;
};

struct Sockets {
    FuriEventLoop* event_loop;
    FuriEventFlag* event_flag;
    Intercom* intercom;
    SocketsMessage* message;
    SocketRequest request;
    SocketResponse response;
    Socket* sockets[SOCKET_COUNT];
};

static void sockets_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(SocketResponse));
    furi_assert(context);

    Sockets* instance = context;

    const SocketResponse* response = data;
    const SocketResponseType response_type = response->type;

    if(response_type < SocketResponseTypeAsyncSend) {
        memcpy(&instance->response, response, sizeof(SocketResponse));
        furi_event_loop_set_custom_event(instance->event_loop, SocketsCustomEventResponse);

    } else if(response_type < SocketResponseTypeMax) {
        const SocketAsyncResponse* async_response = &response->async_response;
        const uint8_t socket_id = async_response->socket_id;
        furi_assert(socket_id < SOCKET_COUNT);
        Socket* socket = instance->sockets[socket_id];
        furi_assert(socket);

        SocketEvent event = {0};

        if(response_type == SocketResponseTypeAsyncSend) {
            const SocketSendAsyncResponse* send_async_response =
                &async_response->send_async_response;

            event.type = SocketEventTypeSendComplete;
            event.data_size = send_async_response->sent_size;

        } else if(response_type == SocketResponseTypeAsyncReceive) {
            const SocketReceiveAsyncResponse* receive_async_response =
                &async_response->receive_async_response;

            const size_t data_size = furi_stream_buffer_send(
                socket->rx_buffer,
                receive_async_response->data,
                receive_async_response->data_size,
                FuriWaitForever);
            furi_check(data_size == receive_async_response->data_size);

            event.type = SocketEventTypeReceiveReady;
            event.data_size = data_size;

        } else if(response_type == SocketResponseTypeAsyncClose) {
            const SocketCloseAsyncResponse* close_async_response =
                &async_response->close_async_response;

            event.type = SocketEventTypeClosed;
            event.data_size = close_async_response->sent_size;
        }

        furi_check(
            furi_message_queue_put(socket->event_queue, &event, FuriWaitForever) == FuriStatusOk);

    } else {
        furi_crash("Invalid response type");
    }
}

static void sockets_send_message(Sockets* instance, SocketsMessage* message) {
    uint32_t flags;
    // Wait until the Sockets system becomes ready for the next request
    flags = furi_event_flag_wait(
        instance->event_flag, SocketsEventFlagReady, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags & SocketsEventFlagReady);

    instance->message = message;
    furi_event_loop_set_custom_event(instance->event_loop, SocketsCustomEventRequest);

    // Wait until a response is received
    flags = furi_event_flag_wait(
        instance->event_flag, SocketsEventFlagDone, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags & SocketsEventFlagDone);
}

Socket* socket_alloc(Sockets* instance, const SocketInfo* socket_info) {
    furi_check(instance);
    furi_check(socket_info);

    SocketsMessage msg = {
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

    Sockets* instance = socket->owner;
    furi_assert(instance);

    SocketsMessage msg = {
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

SocketStatus socket_connect(Socket* socket, const SocketConnectionInfo* connection_info) {
    furi_check(socket);

    Sockets* instance = socket->owner;
    furi_assert(instance);

    SocketsMessage msg = {
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

    Sockets* instance = socket->owner;
    furi_assert(instance);

    SocketsMessage msg = {
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

// FIXME: Not thread safe...ish
SocketStatus socket_receive(Socket* socket, void* data, size_t data_size, size_t* received_size) {
    furi_check(socket);
    furi_check(data);

    const size_t rx_size = furi_stream_buffer_receive(socket->rx_buffer, data, data_size, 0);

    if(received_size) {
        *received_size = rx_size;
    }

    return SocketStatusOk;
}

static void sockets_process_request(Sockets* instance) {
    const SocketsMessage* message = instance->message;
    const SocketRequestType request_type = message->request_type;

    SocketRequest* request = &instance->request;
    request->type = request_type;

    if(request_type == SocketRequestTypeAlloc) {
        SocketAllocRequest* alloc_request = &request->alloc_request;
        const SocketsAllocMessage* alloc_message = &message->alloc_message;

        alloc_request->socket_info = *alloc_message->socket_info;

    } else if(request_type == SocketRequestTypeFree) {
        SocketFreeRequest* free_request = &request->free_request;
        const SocketsFreeMessage* free_message = &message->free_message;

        free_request->socket_id = free_message->socket_id;

    } else if(request_type == SocketRequestTypeConnect) {
        SocketConnectRequest* connect_request = &request->connect_request;
        const SocketsConnectMessage* connect_message = &message->connect_message;

        connect_request->socket_id = connect_message->socket_id;
        connect_request->connection_info = *connect_message->connection_info;

    } else if(request_type == SocketRequestTypeSend) {
        SocketSendRequest* send_request = &request->send_request;
        const SocketsSendMessage* send_message = &message->send_message;

        send_request->socket_id = send_message->socket_id;
        send_request->data_size = send_message->data_size;
        memcpy(send_request->data, send_message->data, send_message->data_size);

    } else if(request_type >= SocketRequestTypeMax) {
        furi_crash("Invalid request type");
    }
}

static void sockets_socket_event_callback(FuriEventLoopObject* object, void* context) {
    Socket* socket = context;
    furi_assert(object == socket->event_queue);

    SocketEvent event;

    while(furi_message_queue_get(socket->event_queue, &event, 0) == FuriStatusOk) {
        if(socket->event_callback) {
            socket->event_callback(socket, &event, socket->callback_context);
        }
    }
}

static Socket* sockets_alloc_socket(Sockets* instance, uint8_t socket_id) {
    furi_assert(socket_id < SOCKET_COUNT);

    Socket** socket_slot = &instance->sockets[socket_id];
    furi_check(*socket_slot == NULL);

    Socket* socket = malloc(sizeof(Socket));
    *socket_slot = socket;

    socket->id = socket_id;
    socket->owner = instance;
    socket->rx_buffer = furi_stream_buffer_alloc(SOCKET_RX_BUFFER_LEN, 1);
    socket->event_queue = furi_message_queue_alloc(SOCKET_EVENT_QUEUE_LEN, sizeof(SocketEvent));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        socket->event_queue,
        FuriEventLoopEventIn,
        sockets_socket_event_callback,
        socket);

    return socket;
}

static void sockets_free_socket(Sockets* instance, uint8_t socket_id) {
    furi_assert(socket_id < SOCKET_COUNT);

    Socket** socket_slot = &instance->sockets[socket_id];
    Socket* socket = *socket_slot;
    furi_check(socket);

    *socket_slot = NULL;

    furi_event_loop_unsubscribe(instance->event_loop, socket->event_queue);
    furi_message_queue_free(socket->event_queue);
    furi_stream_buffer_free(socket->rx_buffer);

    free(socket);
}

static void sockets_process_response(Sockets* instance) {
    SocketsMessage* message = instance->message;
    const SocketResponse* response = &instance->response;

    const SocketResponseType response_type = response->type;
    const SocketStatus status = response->status;

    if(status == SocketStatusOk) {
        if(response_type == SocketResponseTypeAlloc) {
            SocketsAllocMessage* alloc_message = &message->alloc_message;
            const SocketAllocResponse* alloc_response = &response->alloc_response;
            alloc_message->socket = sockets_alloc_socket(instance, alloc_response->socket_id);

        } else if(response_type == SocketResponseTypeFree) {
            SocketsFreeMessage* free_message = &message->free_message;
            sockets_free_socket(instance, free_message->socket_id);

        } else if(response_type == SocketResponseTypeSend) {
            SocketsSendMessage* send_message = &message->send_message;
            const SocketSendResponse* send_response = &response->send_response;

            if(send_message->sent_size) {
                *send_message->sent_size = send_response->sent_size;
            }
        }
    }

    message->status = status;
}

static void sockets_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Sockets* instance = context;

    if(events == SocketsCustomEventRequest) {
        sockets_process_request(instance);
        intercom_tx(
            instance->intercom,
            IntercomChannelSockets,
            &instance->request,
            sizeof(SocketRequest),
            FuriWaitForever);

    } else if(events == SocketsCustomEventResponse) {
        sockets_process_response(instance);
        furi_event_flag_set(instance->event_flag, SocketsEventFlagReady | SocketsEventFlagDone);

    } else {
        furi_crash("Multiple Socket events");
    }
}

Sockets* sockets_alloc(void) {
    Sockets* instance = malloc(sizeof(Sockets));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_flag = furi_event_flag_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sockets_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, sockets_intercom_rx_callback, instance);

    // Start receiving requests
    furi_event_flag_set(instance->event_flag, SocketsEventFlagReady);
    furi_record_create(RECORD_SOCKETS, instance);

    return instance;
}

int32_t sockets_srv(void* arg) {
    UNUSED(arg);

    Sockets* instance = sockets_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
