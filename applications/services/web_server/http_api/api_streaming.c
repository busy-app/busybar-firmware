#include "http_api.h"
#include <gui/gui.h>
#include <front_display/front_display.h>
#include <toolbox/rle_encode.h>

#define TAG "Stream"

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
    GuiDisplayId display_id;
    FuriTimer* heartbeat_timer;
    StreamClientState state;
    void* context;
} StreamClientCtx;

LIST_DEF(StreamClientsList, StreamClientCtx*, M_POD_OPLIST);

typedef enum {
    ApiStreamingModeIdle,
    ApiStreamingModeSingleScreen,
    ApiStreamingModeDualScreen,
} ApiStreamingMode;

typedef struct {
    StreamClientsList_t clients;
    uint8_t front_clients_count;
    uint8_t back_clients_count;
    ApiStreamingMode mode;

    FuriMutex* mutex;
    FuriThread* thread;
    bool stop;
    Gui* gui;
    GuiDisplayId display_id;
    size_t frame_size;
    uint8_t* compressed_buffer;
    uint8_t* raw_buffer;
} ApiStreamingCtx;

static void api_streaming_frame_update_thread_start(ApiStreamingCtx* instance) {
    STREAM_LOG_D("Start thread");
    instance->gui = furi_record_open(RECORD_GUI);

    instance->raw_buffer = malloc(RAW_BUFFER_SIZE);
    instance->compressed_buffer = malloc(COMPRESSED_BUFFER_SIZE);
    instance->stop = false;
    furi_thread_start(instance->thread);
}

static void api_streaming_frame_update_thread_stop(ApiStreamingCtx* instance) {
    STREAM_LOG_D("Stop thread");

    instance->stop = true;
    furi_thread_join(instance->thread);

    furi_record_close(RECORD_GUI);
    free(instance->compressed_buffer);
    free(instance->raw_buffer);
}

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

static inline void api_streaming_update_mode(ApiStreamingCtx* instance) {
    if(instance->front_clients_count > 0 && instance->back_clients_count > 0)
        instance->mode = ApiStreamingModeDualScreen;
    else {
        instance->mode = ApiStreamingModeSingleScreen;
    }
}

static inline void
    api_streaming_client_counter_increment(ApiStreamingCtx* instance, GuiDisplayId display_id) {
    if(display_id == GuiDisplayIdFront)
        instance->front_clients_count++;
    else if(display_id == GuiDisplayIdBack)
        instance->back_clients_count++;

    furi_assert(
        instance->front_clients_count + instance->back_clients_count ==
        StreamClientsList_size(instance->clients));
}

static inline void
    api_streaming_client_counter_decrement(ApiStreamingCtx* instance, GuiDisplayId display_id) {
    if(display_id == GuiDisplayIdFront)
        instance->front_clients_count--;
    else if(display_id == GuiDisplayIdBack)
        instance->back_clients_count--;

    furi_assert(
        instance->front_clients_count + instance->back_clients_count ==
        StreamClientsList_size(instance->clients));
}

static inline void
    api_streaming_client_counter_move(ApiStreamingCtx* instance, GuiDisplayId display_id) {
    if(display_id == GuiDisplayIdFront) {
        instance->front_clients_count++;
        instance->back_clients_count--;
    } else if(display_id == GuiDisplayIdBack) {
        instance->back_clients_count++;
        instance->front_clients_count--;
    }

    furi_assert(
        instance->front_clients_count + instance->back_clients_count ==
        StreamClientsList_size(instance->clients));
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

static StreamClientCtx* api_streaming_client_alloc(struct mg_connection* conn) {
    StreamClientCtx* client = malloc(sizeof(StreamClientCtx));
    client->conn = conn;
    client->display_id = GuiDisplayIdMax;
    client->state = StreamClientStateIdle;
    client->context = conn->data;
    client->heartbeat_timer = furi_timer_alloc(
        api_streaming_client_heartbeat_timer_callback, FuriTimerTypePeriodic, client);
    return client;
}

static inline void api_streaming_client_free(StreamClientCtx* client) {
    furi_timer_stop(client->heartbeat_timer);
    furi_timer_free(client->heartbeat_timer);
    free(client);
}

static void api_streaming_client_connection_open(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;
    furi_assert(instance);

    StreamClientCtx* client = api_streaming_client_alloc(conn);

    if(StreamClientsList_empty_p(instance->clients)) {
        api_streaming_frame_update_thread_start(instance);
    }

    // Add connection to WebSocket clients list
    StreamClientsList_push_back(instance->clients, client);
    furi_timer_start(client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);

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
        StreamClientsList_remove(instance->clients, it);
        api_streaming_client_counter_decrement(instance, client->display_id);
        api_streaming_update_mode(instance);
        api_streaming_client_free(client);
    }

    if(StreamClientsList_empty_p(instance->clients)) {
        api_streaming_frame_update_thread_stop(instance);
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
        furi_timer_restart(client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);
    } else if(WEBSOCKET_PONG(ws_msg->flags)) {
        STREAM_LOG_D("PONG");
        api_streaming_client_set_state(client, StreamClientStateActive);
        furi_timer_restart(client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);
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
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    furi_assert(ctx);
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

static void api_streaming_update_display_id(ApiStreamingCtx* instance) {
    do {
        if(furi_mutex_acquire(instance->mutex, FRAME_MUTEX_TIMEOUT) != FuriStatusOk) {
            STREAM_LOG_W("Unable to lock display_id");
            break;
        }

        if(instance->mode == ApiStreamingModeDualScreen) {
            instance->display_id ^= 1;
        } else if(instance->mode == ApiStreamingModeSingleScreen) {
            instance->display_id = (instance->front_clients_count > 0) ? GuiDisplayIdFront :
                                                                         GuiDisplayIdBack;
        }
        furi_mutex_release(instance->mutex);
    } while(false);
}

static void back_buffer_l8_to_l4(uint8_t* dst_l4, const uint8_t* src_l8) {
    for(uint32_t i = 0; i < RAW_BUFFER_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst_l4[i] = (src_l8[draw_idx] >> 4) | (src_l8[draw_idx + 1] & 0xF0);
    }
}

static int32_t api_streaming_frame_update_thread(void* context) {
    ApiStreamingCtx* instance = context;

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
                back_buffer_l8_to_l4(instance->raw_buffer, frame);
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
            for(StreamClientsList_it(it, instance->clients); !StreamClientsList_end_p(it);
                StreamClientsList_next(it)) {
                StreamClientCtx* const* it_ptr = StreamClientsList_cref(it);
                StreamClientCtx* client = *it_ptr;
                if(client->display_id == instance->display_id)
                    mg_wakeup(mgr, client->conn->id, NULL, 0);
            }
        } else {
            STREAM_LOG_W("Compression failed");
        }

        furi_delay_ms(FRAME_THREAD_PERIOD_MS);
        api_streaming_update_display_id(instance);
        memset(instance->raw_buffer, 0, RAW_BUFFER_SIZE);
    }
    return 0;
}

void* http_api_streaming_ws_alloc(void) {
    ApiStreamingCtx* instance = malloc(sizeof(ApiStreamingCtx));
    StreamClientsList_init(instance->clients);

    instance->thread =
        furi_thread_alloc_ex("FrameUpd", 1024U, api_streaming_frame_update_thread, instance);
    instance->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    return instance;
}

void http_api_streaming_ws_free(void* ctx) {
    furi_assert(ctx);
    ApiStreamingCtx* instance = ctx;
    StreamClientsList_clear(instance->clients);
    furi_thread_free(instance->thread);
    furi_mutex_free(instance->mutex);
    free(instance);
}

bool http_api_streaming_single_frame_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    char display_str[2];
    int var_len = mg_http_get_var(&msg->query, "display", display_str, sizeof(display_str));

    if(var_len == 1 && (display_str[0] == '0' || display_str[0] == '1')) {
        GuiDisplayId display_id = display_str[0] == '0' ? GuiDisplayIdFront : GuiDisplayIdBack;

        const size_t frame_size = display_id == GuiDisplayIdFront ? FRONT_DISPLAY_BUF_SIZE :
                                                                    RAW_BUFFER_SIZE;

        Gui* gui = furi_record_open(RECORD_GUI);
        uint8_t* frame = malloc(frame_size);

        with_gui(gui, {
            const uint8_t* buf = gui_display_get_frame_buffer(gui, display_id);
            if(display_id == GuiDisplayIdFront)
                memcpy(frame, buf, frame_size);
            else {
                back_buffer_l8_to_l4(frame, buf);
            }
        });
        furi_record_close(RECORD_GUI);

        MG_REPLY_IMAGE(conn, frame, frame_size);
        free(frame);
    } else
        MG_REPLY_ERROR(conn, 400, "Wrong display");
    return true;
}
