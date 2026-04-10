#include "http_api.h"
#include <state_publisher/state_publisher.h>
#include <furi/core/message_queue.h>

#define TAG "StatusStream"

#ifdef STREAM_DEBUG
#define STREAM_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#define STREAM_LOG_W(...) FURI_LOG_W(TAG, __VA_ARGS__)
#else
#define STREAM_LOG_D(...)
#define STREAM_LOG_W(...)
#endif

#define MAX_CLIENTS_COUNT                 (4)
#define MAX_PUBLISH_MESSAGES_COUNT        (8)
#define PUBLISH_MESSAGES_PAUSE_THRESHOLD  (6)
#define PUBLISH_MESSAGES_RESUME_THRESHOLD (2)

#define FRAME_QUEUE_TIMEOUT          (10)
#define FRAME_INTERVAL_MS            (100)
#define CLIENT_HEARTBEAT_INTERVAL_MS (10000)

#define WEBSOCKET_FLAG_TEST(flags, test) ((flags & test) == test)
#define WEBSOCKET_PING(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_PING))
#define WEBSOCKET_PONG(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_PONG))
#define WEBSOCKET_TEXT(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_TEXT))

#define RATE_LIMIT                                \
    (RateLimiterLimit) {                          \
        .max_packet_count = 11, .period_ms = 1000 \
    }

typedef enum {
    ClientStateHandshake,
    ClientStateActive,
    ClientStateRequestingPing,
    ClientStateWaitingPong,
    ClientStateInvalid,
} ClientState;

typedef struct DataMessage {
    SharedByteArray_t data;
} DataMessage;

typedef struct StatusStreaming StatusStreaming;

typedef struct {
    bool valid;
    ClientState state;
    struct mg_connection* conn;
    StatusStreaming* parent;

    struct {
        struct mg_timer* heartbeat_timer;
        StatePublisherTransportHandle transport_handle;
        FuriMessageQueue* queue; // Queue of DataMessage
    } active;
} Client;

typedef struct StatusStreaming {
    Client clients[MAX_CLIENTS_COUNT];
    FuriMutex* client_alloc_mutex; // protects clients[].valid

    StatePublisher* state_publisher;
} StatusStreaming;

static inline void client_set_state(Client* client, ClientState new_state) {
    STREAM_LOG_D("Client state %d -> %d", client->state, new_state);
    client->state = new_state;
}

static void client_heartbeat_timer_callback(void* ctx) {
    STREAM_LOG_D("Heartbeat timer");

    Client* client = ctx;

    switch(client->state) {
    case ClientStateActive:
        client_set_state(client, ClientStateRequestingPing);
        mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
        break;
    case ClientStateWaitingPong:
        STREAM_LOG_W("pong timeout");
        client_set_state(client, ClientStateInvalid);
        mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
        break;
    case ClientStateRequestingPing:
    case ClientStateInvalid:
    case ClientStateHandshake:
        // should never happen
        break;
    }
}

static void client_publish_callback(const SharedByteArray_t data, void* context) {
    Client* client = context;

    furi_assert(client->state > ClientStateHandshake);

    DataMessage msg;
    SharedByteArray_init_set(msg.data, data);
    FuriStatus error = furi_message_queue_put(client->active.queue, &msg, FRAME_QUEUE_TIMEOUT);
    size_t count = furi_message_queue_get_count(client->active.queue);
    if(count >= PUBLISH_MESSAGES_PAUSE_THRESHOLD) {
        STREAM_LOG_W("Queue full, throttling");
        state_publisher_set_paused(
            client->parent->state_publisher, client->active.transport_handle, true);
    }
    if(error != FuriStatusOk) {
        FURI_LOG_E(TAG, "Queue overflow, update skipped (%u)", error);
        SharedByteArray_clear(msg.data);
    }
    mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
}

static Client* client_alloc(StatusStreaming* instance, struct mg_connection* conn) {
    Client* result = NULL;
    furi_mutex_acquire(instance->client_alloc_mutex, FuriWaitForever);
    for(size_t i = 0; i != COUNT_OF(instance->clients); ++i) {
        Client* client = instance->clients + i;
        if(!client->valid) {
            client->valid = true;
            client->conn = conn;
            client->state = ClientStateHandshake;
            client->parent = instance;
            result = client;
            break;
        }
    }
    furi_mutex_release(instance->client_alloc_mutex);
    return result;
}

static inline void client_free(Client* client, StatusStreaming* streaming_ctx) {
    if(client->valid) {
        switch(client->state) {
        case ClientStateActive:
        case ClientStateRequestingPing:
        case ClientStateWaitingPong:
        case ClientStateInvalid: {
            if(client->active.transport_handle != STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID) {
                state_publisher_del_transport(
                    streaming_ctx->state_publisher, client->active.transport_handle);
            }
            mg_timer_free(&client->conn->mgr->timers, client->active.heartbeat_timer);
            free(client->active.heartbeat_timer);
            DataMessage msg;
            while(furi_message_queue_get(client->active.queue, &msg, 0) == FuriStatusOk) {
                SharedByteArray_clear(msg.data);
            }
            furi_message_queue_free(client->active.queue);
        } break;
        case ClientStateHandshake:
            break;
        }
        client->valid = false;
    }
}

static void client_connection_open(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    Client* client = conn_ctx->context;
    furi_assert(client);
    StatusStreaming* instance = client->parent;
    furi_assert(instance);

    furi_assert(client->state == ClientStateHandshake);
    client_set_state(client, ClientStateActive);
    client->active.heartbeat_timer = mg_timer_add(
        conn->mgr,
        CLIENT_HEARTBEAT_INTERVAL_MS,
        MG_TIMER_REPEAT,
        client_heartbeat_timer_callback,
        client);
    client->active.queue =
        furi_message_queue_alloc(MAX_PUBLISH_MESSAGES_COUNT, sizeof(DataMessage));
    client->active.transport_handle = STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;

    STREAM_LOG_D("Add client %ld", conn->id);
}

static void client_connection_close(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    Client* client = conn_ctx->context;
    furi_assert(client);
    StatusStreaming* instance = client->parent;
    furi_assert(instance);

    if(client) {
        STREAM_LOG_D("Remove client: %ld", client->conn->id);
        furi_mutex_acquire(instance->client_alloc_mutex, FuriWaitForever);
        client_free(client, instance);
        furi_mutex_release(instance->client_alloc_mutex);
    }

    conn_ctx->ws.on_open = NULL;
    conn_ctx->ws.on_message = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->on_wakeup = NULL;
}

static void client_send_frame(struct mg_connection* conn, void* data, size_t len) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    Client* client = conn_ctx->context;

    if(client->state == ClientStateActive) {
        DataMessage msg;
        while(furi_message_queue_get(client->active.queue, &msg, 0) == FuriStatusOk) {
            const ByteArray_t* array = SharedByteArray_cref(msg.data);
            mg_ws_send(
                conn, ByteArray_cget(*array, 0), ByteArray_size(*array), WEBSOCKET_OP_BINARY);
            SharedByteArray_clear(msg.data);
        }
        if(client->active.transport_handle != STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID &&
           furi_message_queue_get_count(client->active.queue) <=
               PUBLISH_MESSAGES_RESUME_THRESHOLD) {
            state_publisher_set_paused(
                client->parent->state_publisher, client->active.transport_handle, false);
        }
    } else if(client->state == ClientStateRequestingPing) {
        STREAM_LOG_D("Requesting ping");
        client_set_state(client, ClientStateWaitingPong);
        mg_ws_send(conn, data, len, WEBSOCKET_OP_PING);
    } else if(client->state == ClientStateInvalid) {
        conn->is_draining = 1;
    }
}

static void client_set_enabled(Client* client, bool enabled) {
    furi_assert(client);

    STREAM_LOG_D("set enabled = %u", enabled ? 1 : 0);

    bool was_enabled = client->active.transport_handle != STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;
    if(enabled && !was_enabled) {
        client->active.transport_handle = state_publisher_add_transport(
            client->parent->state_publisher,
            StatePublisherTransportClassWebSocket,
            FRAME_INTERVAL_MS,
            RATE_LIMIT,
            client_publish_callback,
            client);
    } else if(was_enabled && !enabled) {
        state_publisher_del_transport(
            client->parent->state_publisher, client->active.transport_handle);
        client->active.transport_handle = STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;
    }
}

static void client_on_message(struct mg_connection* conn, struct mg_ws_message* ws_msg) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    Client* client = conn_ctx->context;

    switch(client->state) {
    case ClientStateHandshake:
    case ClientStateInvalid:
        furi_assert(false);
        break;
    default:
        break;
    }

    if(WEBSOCKET_PONG(ws_msg->flags)) {
        STREAM_LOG_D("PONG");
        client_set_state(client, ClientStateActive);
        client->active.heartbeat_timer->expire = mg_now() + CLIENT_HEARTBEAT_INTERVAL_MS;
        // Make sure to wake up and send collected messages
        mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
    } else if(WEBSOCKET_TEXT(ws_msg->flags)) {
        STREAM_LOG_D("MSG");

        bool enabled = false;
        if(mg_json_get_bool(ws_msg->data, "$.enable", &enabled)) {
            client_set_enabled(client, enabled);
        } else {
            FURI_LOG_E(TAG, "bad request");
        }
    }
}

static void client_connection_on_open_rejected(struct mg_connection* conn) {
    ByteArray_t buf;
    ByteArray_init(buf);
    if(state_publisher_serialize_error_message(
           &buf, BSB_Error_Severity_FATAL, BSB_Error_Cause_RESOURCE_LIMIT)) {
        const void* data = ByteArray_cget(buf, 0);
        size_t len = ByteArray_size(buf);
        mg_ws_send(conn, data, len, WEBSOCKET_OP_BINARY);
    }
    ByteArray_clear(buf);
    conn->is_draining = 1;
}

bool http_api_status_ws_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    furi_assert(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    StatusStreaming* instance = ctx;

    Client* client = client_alloc(instance, conn);

    if(!client) {
        FURI_LOG_W(TAG, "No available websockets");
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->ws.on_open = client_connection_on_open_rejected;
    } else {
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->ws.on_open = client_connection_open;
        conn_ctx->on_close = client_connection_close;
        conn_ctx->ws.on_message = client_on_message;
        conn_ctx->on_wakeup = client_send_frame;
        conn_ctx->context = client;
    }

    mg_ws_upgrade(conn, msg, NULL);

    return true;
}

void* http_api_status_ws_alloc(void) {
    StatusStreaming* instance = malloc(sizeof(StatusStreaming));
    instance->client_alloc_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    for(size_t i = 0; i != COUNT_OF(instance->clients); ++i) {
        instance->clients[i].valid = false;
    }

    instance->state_publisher = furi_record_open(RECORD_STATE_PUBLISHER);

    return instance;
}

void http_api_status_ws_free(void* ctx) {
    furi_assert(ctx);
    StatusStreaming* instance = ctx;
    furi_mutex_acquire(instance->client_alloc_mutex, FuriWaitForever);
    for(size_t i = 0; i != COUNT_OF(instance->clients); ++i) {
        Client* client = instance->clients + i;
        client_free(client, instance);
    }
    furi_mutex_release(instance->client_alloc_mutex);
    furi_record_close(RECORD_STATE_PUBLISHER);

    furi_mutex_free(instance->client_alloc_mutex);
    free(instance);
}
