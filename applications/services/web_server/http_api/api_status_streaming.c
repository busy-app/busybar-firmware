#include "http_api.h"
#include <toolbox/dyn_buffer.h>
#include <state_publisher/state_publisher.h>
#include <furi/core/message_queue.h>
#define TAG "StatusStream"

#define STREAM_DEBUG

#ifdef STREAM_DEBUG
#define STREAM_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#define STREAM_LOG_W(...) FURI_LOG_W(TAG, __VA_ARGS__)
#else
#define STREAM_LOG_D(...)
#define STREAM_LOG_W(...)
#endif

#define MAX_CLIENTS_COUNT          (4)
#define MAX_PUBLISH_MESSAGES_COUNT (4)

#define FRAME_QUEUE_TIMEOUT          (10) // TODO rename
#define FRAME_INTERVAL_MS            (100)
#define CLIENT_HEARTBEAT_INTERVAL_MS (10000)

#define WEBSOCKET_FLAG_TEST(flags, test) ((flags & test) == test)
#define WEBSOCKET_PING(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_PING))
#define WEBSOCKET_PONG(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_PONG))
#define WEBSOCKET_TEXT(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_TEXT))

typedef enum {
    ClientStateIdle,
    ClientStateActive,
    ClientStateRequestingPing,
    ClientStateWaitingPong,
    ClientStateInvalid,
} ClientState;

typedef struct DataMessage {
    SharedPtr* data;
    size_t size;
} DataMessage;

typedef struct {
    struct mg_connection* conn;
    struct mg_timer* heartbeat_timer;
    ClientState state;
    void* context;
    StatePublisherTransportHandle transport_handle;
    FuriMessageQueue* queue; // Queue of DataMessage
} ClientCtx;

LIST_DEF(ClientList, ClientCtx*, M_POD_OPLIST);

typedef struct {
    FuriMutex* clients_lock;
    ClientList_t clients;

    StatePublisher* state_publisher;
} StatusStreamingCtx;

static ClientCtx* api_streaming_get_client_by_id(
    StatusStreamingCtx* instance,
    const unsigned long client_id,
    ClientList_it_t out_iterator) {
    ClientList_it_t it;
    ClientCtx* client = NULL;
    for(ClientList_it(it, instance->clients); !ClientList_end_p(it); ClientList_next(it)) {
        ClientCtx* const* it_ptr = ClientList_cref(it);
        client = *it_ptr;
        if(client->conn->id != client_id) continue;
        if(out_iterator) ClientList_it_set(out_iterator, it);
        break;
    }
    return client;
}

static inline void client_set_state(ClientCtx* client, ClientState new_state) {
    STREAM_LOG_D("Client state %d -> %d", client->state, new_state);
    client->state = new_state;
}

static void client_heartbeat_timer_callback(void* ctx) {
    STREAM_LOG_D("Heartbeat timer");

    ClientCtx* client = ctx;

    ClientState new_state = ClientStateInvalid;
    if(client->state != ClientStateWaitingPong && client->state != ClientStateInvalid) {
        new_state = ClientStateRequestingPing;
    } else if(client->state == ClientStateWaitingPong) {
        new_state = ClientStateInvalid;
    }

    client_set_state(client, new_state);
    mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
}

static void client_publish_callback(SharedPtr* data, size_t data_size, void* context) {
    ClientCtx* client = context;

    shared_ptr_acquire(data);
    FuriStatus error = furi_message_queue_put(
        client->queue, &(DataMessage){.data = data, .size = data_size}, FRAME_QUEUE_TIMEOUT);
    if(error != FuriStatusOk) {
        FURI_LOG_E(TAG, "Queue overflow, update skipped (%u)", error);
        shared_ptr_release(data);
    }
    mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
}

static inline void client_free(ClientCtx* client) {
    mg_timer_free(&client->conn->mgr->timers, client->heartbeat_timer);
    free(client->heartbeat_timer);
    DataMessage msg;
    while(furi_message_queue_get(client->queue, &msg, 0) == FuriStatusOk) {
        shared_ptr_release(msg.data);
    }
    furi_message_queue_free(client->queue);
    free(client);
}

static void client_connection_open(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    StatusStreamingCtx* instance = conn_ctx->context;
    furi_assert(instance);

    ClientCtx* client = malloc(sizeof(ClientCtx));
    client->conn = conn;
    client->state = ClientStateActive;
    client->context = conn->data;
    client->heartbeat_timer = mg_timer_add(
        conn->mgr,
        CLIENT_HEARTBEAT_INTERVAL_MS,
        MG_TIMER_REPEAT,
        client_heartbeat_timer_callback,
        client);
    client->queue = furi_message_queue_alloc(MAX_PUBLISH_MESSAGES_COUNT, sizeof(DataMessage));

    // Add connection to WebSocket clients list
    furi_mutex_acquire(instance->clients_lock, FuriWaitForever);
    ClientList_push_back(instance->clients, client);
    furi_mutex_release(instance->clients_lock);

    client->transport_handle = state_publisher_add_transport(
        instance->state_publisher,
        StatePublisherTransportClassWebSocket,
        FRAME_INTERVAL_MS,
        client_publish_callback,
        client);

    STREAM_LOG_D("Add client %ld", conn->id);
}

static void client_connection_close(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    StatusStreamingCtx* instance = conn_ctx->context;
    furi_assert(instance);

    // Remove connection from WebSocket clients list
    ClientList_it_t it;
    ClientCtx* client = api_streaming_get_client_by_id(instance, conn->id, it);
    if(client) {
        STREAM_LOG_D("Remove client: %ld", client->conn->id);
        furi_mutex_acquire(instance->clients_lock, FuriWaitForever);
        ClientList_remove(instance->clients, it);
        furi_mutex_release(instance->clients_lock);
        state_publisher_del_transport(instance->state_publisher, client->transport_handle);
        client_free(client);
    }

    // Clear connection callbacks
    conn_ctx->ws.on_open = NULL;
    conn_ctx->ws.on_message = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->on_wakeup = NULL;
}

static void client_send_frame(struct mg_connection* conn, void* data, size_t len) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    StatusStreamingCtx* instance = conn_ctx->context;

    ClientCtx* client = api_streaming_get_client_by_id(instance, conn->id, NULL);
    if(client->state == ClientStateActive) {
        DataMessage msg;
        if(furi_message_queue_get(client->queue, &msg, FRAME_QUEUE_TIMEOUT) == FuriStatusOk) {
            mg_ws_send(conn, msg.data->inner, msg.size, WEBSOCKET_OP_BINARY);
            shared_ptr_release(msg.data);
        } else {
            FURI_LOG_W(TAG, "Woke up for no message");
        }
    } else if(client->state == ClientStateRequestingPing) {
        STREAM_LOG_D("Requesting ping");
        client_set_state(client, ClientStateWaitingPong);
        mg_ws_send(conn, data, len, WEBSOCKET_OP_PING);
    } else if(client->state == ClientStateInvalid) {
        mg_close_conn(conn);
    }
}

static void client_on_message(struct mg_connection* conn, struct mg_ws_message* ws_msg) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    StatusStreamingCtx* instance = conn_ctx->context;
    ClientCtx* const client = api_streaming_get_client_by_id(instance, conn->id, NULL);

    if(WEBSOCKET_PING(ws_msg->flags)) {
        STREAM_LOG_D("PING");
        client_set_state(client, ClientStateActive);
        client->heartbeat_timer->expire = mg_now() + CLIENT_HEARTBEAT_INTERVAL_MS;
    } else if(WEBSOCKET_PONG(ws_msg->flags)) {
        STREAM_LOG_D("PONG");
        client_set_state(client, ClientStateActive);
        client->heartbeat_timer->expire = mg_now() + CLIENT_HEARTBEAT_INTERVAL_MS;
    } else if(WEBSOCKET_TEXT(ws_msg->flags)) {
        STREAM_LOG_D("MSG");
    }
}

bool http_api_status_ws_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    furi_assert(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    StatusStreamingCtx* instance = ctx;

    bool success = false;
    do {
        bool is_ws_upgrade = (mg_http_get_header(msg, "Sec-WebSocket-Key") != NULL);
        if(!is_ws_upgrade) break;

        if(ClientList_size(instance->clients) >= MAX_CLIENTS_COUNT) {
            MG_REPLY_ERROR(conn, 400, "Exceed max clients count");
            break;
        }

        // Assign connection callbacks
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->ws.on_open = client_connection_open;
        conn_ctx->on_close = client_connection_close;
        conn_ctx->ws.on_message = client_on_message;
        conn_ctx->on_wakeup = client_send_frame;
        conn_ctx->context = instance;

        // Upgrade connection to WebSocket
        mg_ws_upgrade(conn, msg, NULL);

        success = true;
    } while(false);

    return success;
}

void* http_api_status_ws_alloc(void) {
    StatusStreamingCtx* instance = malloc(sizeof(StatusStreamingCtx));
    instance->clients_lock = furi_mutex_alloc(FuriMutexTypeNormal);
    ClientList_init(instance->clients);

    instance->state_publisher = furi_record_open(RECORD_STATE_PUBLISHER);

    return instance;
}

void http_api_status_ws_free(void* ctx) {
    furi_assert(ctx);
    StatusStreamingCtx* instance = ctx;
    ClientList_it_t it;
    for(ClientList_it(it, instance->clients); !ClientList_end_p(it); ClientList_next(it)) {
        ClientCtx* const* it_ptr = ClientList_cref(it);
        ClientCtx* client = *it_ptr;
        state_publisher_del_transport(instance->state_publisher, client->transport_handle);
        client_free(client);
    }
    furi_record_close(RECORD_STATE_PUBLISHER);

    ClientList_clear(instance->clients);
    furi_mutex_free(instance->clients_lock);
    free(instance);
}
