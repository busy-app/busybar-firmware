#include "http_api.h"
#include <toolbox/dyn_buffer.h>
#include <state_publisher/state_publisher.h>

#define TAG "StatusStream"

#ifdef STREAM_DEBUG
#define STREAM_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#define STREAM_LOG_W(...) FURI_LOG_W(TAG, __VA_ARGS__)
#else
#define STREAM_LOG_D(...)
#define STREAM_LOG_W(...)
#endif

#define MAX_CLIENTS_COUNT (4)

#define RAW_BUFFER_SIZE        (6400U)
#define COMPRESSED_BUFFER_SIZE (RAW_BUFFER_SIZE + 600U)

#define FRAME_MUTEX_TIMEOUT        (10)
#define FRAME_THREAD_PERIOD_MS     (100)
#define CLIENT_HEARTBEAT_PERIOD_MS (10000)

#define WEBSOCKET_FLAG_TEST(flags, test) ((flags & test) == test)
#define WEBSOCKET_PING(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_PING))
#define WEBSOCKET_PONG(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_PONG))
#define WEBSOCKET_TEXT(flags)            (WEBSOCKET_FLAG_TEST(flags, WEBSOCKET_OP_TEXT))

typedef enum {
    StreamClientStateIdle,
    StreamClientStateActive,
    StreamClientStateRequestingPing,
    StreamClientStateWaitingPong,
    StreamClientStateInvalid,
} StreamClientState;

typedef struct {
    struct mg_connection* conn;
    struct mg_timer* heartbeat_timer;
    StreamClientState state;
    void* context;
    StatePublisherTransportHandle transport_handle;
} StreamClientCtx;

LIST_DEF(StreamClientsList, StreamClientCtx*, M_POD_OPLIST);

typedef enum {
    ApiStreamingModeIdle,
    ApiStreamingModeSingleScreen,
    ApiStreamingModeDualScreen,
} ApiStreamingMode;

typedef struct {
    FuriMutex* clients_lock;
    StreamClientsList_t clients;
    uint8_t idle_clients_count;
    uint8_t front_clients_count;
    uint8_t back_clients_count;
    ApiStreamingMode mode;

    StatePublisher* state_publisher;

    FuriMutex* mutex;
    DynBuffer buffer;
} ApiStreamingCtx;

static StreamClientCtx* api_streaming_get_client_by_id(
    ApiStreamingCtx* instance,
    const unsigned long client_id,
    StreamClientsList_it_t out_iterator) {
    StreamClientsList_it_t it;
    StreamClientCtx* client = NULL;
    for(StreamClientsList_it(it, instance->clients); !StreamClientsList_end_p(it);
        StreamClientsList_next(it)) {
        StreamClientCtx* const* it_ptr = StreamClientsList_cref(it);
        client = *it_ptr;
        if(client->conn->id != client_id) continue;
        if(out_iterator) StreamClientsList_it_set(out_iterator, it);
        break;
    }
    return client;
}

static inline void
    api_streaming_client_set_state(StreamClientCtx* client, StreamClientState new_state) {
    STREAM_LOG_D("Client state %d -> %d", client->state, new_state);
    client->state = new_state;
}

// Async send from another thread
void api_streaming_client_heartbeat_timer_callback(void* ctx) {
    STREAM_LOG_D("Heartbeat timer");

    StreamClientCtx* client = ctx;

    StreamClientState new_state = StreamClientStateInvalid;
    if(client->state != StreamClientStateWaitingPong &&
       client->state != StreamClientStateInvalid) {
        new_state = StreamClientStateRequestingPing;
    } else if(client->state == StreamClientStateWaitingPong) {
        new_state = StreamClientStateInvalid;
    }

    api_streaming_client_set_state(client, new_state);
    mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
}

void publish_callback(const void* data, size_t data_size, void* context) {
    StreamClientCtx* client = context;
    UNUSED(client);
    UNUSED(data);
    UNUSED(data_size);
}

static inline void api_streaming_client_free(StreamClientCtx* client) {
    mg_timer_free(&client->conn->mgr->timers, client->heartbeat_timer);
    free(client->heartbeat_timer);
    free(client);
}

static void api_streaming_client_connection_open(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;
    furi_assert(instance);

    StreamClientCtx* client = malloc(sizeof(StreamClientCtx));
    client->conn = conn;
    client->state = StreamClientStateIdle;
    client->context = conn->data;
    client->heartbeat_timer = mg_timer_add(
        conn->mgr,
        CLIENT_HEARTBEAT_PERIOD_MS,
        MG_TIMER_REPEAT,
        api_streaming_client_heartbeat_timer_callback,
        client);
    client->transport_handle = state_publisher_add_transport(instance->state_publisher, StatePublisherTransportClassWebSocket, FRAME_THREAD_PERIOD_MS, publish_callback, client);

    // Add connection to WebSocket clients list
    furi_mutex_acquire(instance->clients_lock, FuriWaitForever);
    StreamClientsList_push_back(instance->clients, client);
    instance->idle_clients_count += 1;
    furi_mutex_release(instance->clients_lock);

    STREAM_LOG_D("Add client %ld", conn->id);
}

static void api_streaming_client_connection_close(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;
    furi_assert(instance);

    // Remove connection from WebSocket clients list
    StreamClientsList_it_t it;
    StreamClientCtx* client = api_streaming_get_client_by_id(instance, conn->id, it);
    if(client) {
        STREAM_LOG_D("Remove client: %ld", client->conn->id);
        furi_mutex_acquire(instance->clients_lock, FuriWaitForever);
        StreamClientsList_remove(instance->clients, it);
        furi_mutex_release(instance->clients_lock);
        api_streaming_client_free(client);
    }

    // Clear connection callbacks
    conn_ctx->ws.on_open = NULL;
    conn_ctx->ws.on_message = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->on_wakeup = NULL;
}

static void api_streaming_client_send_frame(struct mg_connection* conn, void* data, size_t len) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;

    StreamClientCtx* client = api_streaming_get_client_by_id(instance, conn->id, NULL);
    if(client->state == StreamClientStateActive) {
        do {
            if(client->display_id != instance->display_id) {
                STREAM_LOG_W("Display mismatch");
                break;
            }

            if(furi_mutex_acquire(instance->mutex, 10) != FuriStatusOk) {
                STREAM_LOG_W("Unable to lock frame");
                break;
            }
            mg_ws_send(
                conn, instance->compressed_buffer, instance->frame_size, WEBSOCKET_OP_BINARY);
            furi_mutex_release(instance->mutex);
        } while(false);
    } else if(client->state == StreamClientStateRequestingPing) {
        STREAM_LOG_D("Requesting ping");
        api_streaming_client_set_state(client, StreamClientStateWaitingPong);
        mg_ws_send(conn, data, len, WEBSOCKET_OP_PING);
    } else if(client->state == StreamClientStateInvalid) {
        mg_close_conn(conn);
    }
}

static void
    api_streaming_client_on_message(struct mg_connection* conn, struct mg_ws_message* ws_msg) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;
    StreamClientCtx* const client = api_streaming_get_client_by_id(instance, conn->id, NULL);

    if(WEBSOCKET_PING(ws_msg->flags)) {
        STREAM_LOG_D("PING");
        api_streaming_client_set_state(client, StreamClientStateActive);
        client->heartbeat_timer->expire = mg_now() + CLIENT_HEARTBEAT_PERIOD_MS;
    } else if(WEBSOCKET_PONG(ws_msg->flags)) {
        STREAM_LOG_D("PONG");
        api_streaming_client_set_state(client, StreamClientStateActive);
        client->heartbeat_timer->expire = mg_now() + CLIENT_HEARTBEAT_PERIOD_MS;
    } else if(WEBSOCKET_TEXT(ws_msg->flags)) {
        STREAM_LOG_D("MSG");
        const char* resp;
        do {
            GuiDisplayId display_id = mg_json_get_long(ws_msg->data, "$.display", GuiDisplayIdMax);
            if(display_id >= GuiDisplayIdMax) {
                resp = "Wrong display value";
                break;
            }

            if(client->state == StreamClientStateActive && client->display_id == display_id) {
                resp = "Same screen ignore";
                break;
            }

            if(client->state == StreamClientStateActive && client->display_id != display_id) {
                api_streaming_client_counter_move(instance, display_id);
                resp = "Change screen";
            } else if(client->state == StreamClientStateIdle) {
                api_streaming_client_counter_increment(instance, display_id);
                resp = "Start streaming...";
            } else {
                resp = "Unknown client state";
                break;
            }

            client->display_id = display_id;
            api_streaming_update_mode(instance);
            api_streaming_client_set_state(client, StreamClientStateActive);
        } while(false);
        mg_ws_send(conn, resp, strlen(resp), WEBSOCKET_OP_TEXT);
    }
}

bool http_api_streaming_ws_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    furi_assert(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    ApiStreamingCtx* instance = ctx;

    bool success = false;
    do {
        bool is_ws_upgrade = (mg_http_get_header(msg, "Sec-WebSocket-Key") != NULL);
        if(!is_ws_upgrade) break;

        if(StreamClientsList_size(instance->clients) >= MAX_CLIENTS_COUNT) {
            MG_REPLY_ERROR(conn, 400, "Exceed max clients count");
            break;
        }

        // Assign connection callbacks
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->ws.on_open = api_streaming_client_connection_open;
        conn_ctx->on_close = api_streaming_client_connection_close;
        conn_ctx->ws.on_message = api_streaming_client_on_message;
        conn_ctx->on_wakeup = api_streaming_client_send_frame;
        conn_ctx->context = instance;

        // Upgrade connection to WebSocket
        mg_ws_upgrade(conn, msg, NULL);

        success = true;
    } while(false);

    return success;
}

static int32_t api_streaming_frame_update_thread(void* context) {
    ApiStreamingCtx* instance = context;

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    while(!instance->stop) {
        if(furi_mutex_acquire(instance->mutex, FRAME_MUTEX_TIMEOUT) != FuriStatusOk) {
            STREAM_LOG_W("Unable to lock in thread");
            continue;
        }

        const size_t frame_size =
            instance->display_id == GuiDisplayIdFront ? FRONT_DISPLAY_BUF_SIZE : RAW_BUFFER_SIZE;

        with_gui(instance->gui, {
            const uint8_t* frame =
                gui_display_get_frame_buffer(instance->gui, instance->display_id);
            if(instance->display_id == GuiDisplayIdFront)
                memcpy(instance->raw_buffer, frame, frame_size);
            else
                color_buf_l8_to_l4(instance->raw_buffer, frame, BACK_DISPLAY_BUF_SIZE);
        });

        const uint8_t blk_size = instance->display_id == GuiDisplayIdFront ? 3 : 2;
        bool compress_result = rle_compress(
            instance->raw_buffer,
            frame_size,
            instance->compressed_buffer,
            COMPRESSED_BUFFER_SIZE,
            blk_size,
            &instance->frame_size);
        furi_mutex_release(instance->mutex);

        if(compress_result) {
            struct mg_mgr* mgr = web_srv_get_mgr();
            StreamClientsList_it_t it;
            furi_mutex_acquire(instance->clients_lock, FuriWaitForever);
            for(StreamClientsList_it(it, instance->clients); !StreamClientsList_end_p(it);
                StreamClientsList_next(it)) {
                StreamClientCtx* const* it_ptr = StreamClientsList_cref(it);
                StreamClientCtx* client = *it_ptr;
                if(client->display_id == instance->display_id)
                    mg_wakeup(mgr, client->conn->id, NULL, 0);
            }
            furi_mutex_release(instance->clients_lock);
        } else {
            STREAM_LOG_W("Compression failed");
        }

        furi_delay_ms(FRAME_THREAD_PERIOD_MS);
        api_streaming_update_display_id(instance);
        memset(instance->raw_buffer, 0, RAW_BUFFER_SIZE);
    }

    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    return 0;
}

void* http_api_streaming_ws_alloc(void) {
    ApiStreamingCtx* instance = malloc(sizeof(ApiStreamingCtx));
    instance->clients_lock = furi_mutex_alloc(FuriMutexTypeNormal);
    StreamClientsList_init(instance->clients);

    instance->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->buffer = dyn_buffer_init();
    instance->state_publisher = furi_record_open(RECORD_STATE_PUBLISHER);

    return instance;
}

void http_api_streaming_ws_free(void* ctx) {
    furi_assert(ctx);
    ApiStreamingCtx* instance = ctx;
    for(StreamClientsList_it(it, instance->clients); !StreamClientsList_end_p(it);
        StreamClientsList_next(it)) {
        StreamClientCtx* const* it_ptr = StreamClientsList_cref(it);
        StreamClientCtx* client = *it_ptr;
        state_publisher_del_transport(instance->state_publisher, client->transport_handle);
    }
    furi_record_close(RECORD_STATE_PUBLISHER)

    StreamClientsList_clear(instance->clients);
    dyn_buffer_destroy(&instance->buffer);
    furi_mutex_free(instance->clients_lock);
    furi_thread_free(instance->thread);
    furi_mutex_free(instance->mutex);
    free(instance);
}
