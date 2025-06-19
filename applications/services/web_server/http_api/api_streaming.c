#include "http_api.h"
#include <gui/gui.h>
#include <toolbox/rle_encode.h>

#define TAG                        "Stream"
#define BUFFER_SIZE                (1024U * 14)
#define MAX_CLIENTS_COUNT          (4)
#define CLIENT_HEARTBEAT_PERIOD_MS (10000)

typedef enum {
    WsClientStateIdle,
    WsClientStateActive,
    WsClientStateRequestingPing,
    WsClientStateWaitingPong,
    WsClientStateInvalid,
} WsClientState;

typedef struct {
    struct mg_connection* conn;
    GuiDisplayId display_id;
    FuriTimer* heartbeat_timer;
    WsClientState state;
    void* context;
} WsClientCtx;

// typedef struct {
//     GuiDisplayId display_id;
//     uint8_t width;
//     uint8_t height;

//     uint8_t bytes_per_pixel;
//     size_t buffer_size;
// } GuiFrameInfo;

LIST_DEF(WsClientsList, WsClientCtx*, M_POD_OPLIST);

typedef enum {
    ApiStreamingModeIdle,
    ApiStreamingModeSingleScreen,
    ApiStreamingModeDualScreen,
} ApiStreamingMode;

typedef struct {
    WsClientsList_t clients;
    uint8_t front_clients_count;
    uint8_t back_clients_count;
    ApiStreamingMode mode;

    FuriMutex* mutex;
    FuriThread* thread;
    bool stop;
    Gui* gui;
    GuiDisplayId display_id;
    ///TODO: think of creating separate buffers for front and back displays, or processing all through one buffer (but it is harder)
    size_t frame_size;
    uint8_t* buffer;
} ApiStreamingCtx;

static WsClientCtx* api_streaming_get_client_by_id(
    ApiStreamingCtx* instance,
    const unsigned long client_id,
    WsClientsList_it_t out_iterator) {
    WsClientsList_it_t it;
    WsClientCtx* client = NULL;
    for(WsClientsList_it(it, instance->clients); !WsClientsList_end_p(it);
        WsClientsList_next(it)) {
        WsClientCtx* const* it_ptr = WsClientsList_cref(it);
        client = *it_ptr;
        if(client->conn->id != client_id) continue;
        if(out_iterator) WsClientsList_it_set(out_iterator, it);
        break;
    }
    return client;
}

static inline void api_streaming_update_mode(ApiStreamingCtx* instance) {
    if(instance->front_clients_count > 0 && instance->back_clients_count > 0)
        instance->mode = ApiStreamingModeDualScreen;
    else {
        // instance->display_id = (instance->front_clients_count > 0) ? GuiDisplayIdFront :
        //                                                              GuiDisplayIdBack;
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
        WsClientsList_size(instance->clients));
}

static inline void
    api_streaming_client_counter_decrement(ApiStreamingCtx* instance, GuiDisplayId display_id) {
    if(display_id == GuiDisplayIdFront)
        instance->front_clients_count--;
    else if(display_id == GuiDisplayIdBack)
        instance->back_clients_count--;

    furi_assert(
        instance->front_clients_count + instance->back_clients_count ==
        WsClientsList_size(instance->clients));
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
        WsClientsList_size(instance->clients));
}

static inline void api_streaming_client_set_state(WsClientCtx* client, WsClientState new_state) {
    FURI_LOG_I(TAG, "Client state %d -> %d", client->state, new_state);
    client->state = new_state;
}

// Async send from another thread
void api_streaming_client_heartbeat_timer_callback(void* ctx) {
    FURI_LOG_I(TAG, "Heartbeat timer");

    WsClientCtx* client = ctx;

    WsClientState new_state = WsClientStateInvalid;
    if(client->state != WsClientStateWaitingPong && client->state != WsClientStateInvalid) {
        new_state = WsClientStateRequestingPing;
    } else if(client->state == WsClientStateWaitingPong) {
        new_state = WsClientStateInvalid;
    }

    api_streaming_client_set_state(client, new_state);
    mg_wakeup(web_srv_get_mgr(), client->conn->id, NULL, 0);
}

static WsClientCtx* api_streaming_client_alloc(struct mg_connection* conn) {
    WsClientCtx* client = malloc(sizeof(WsClientCtx));
    client->conn = conn;
    client->display_id = GuiDisplayIdMax;
    client->state = WsClientStateIdle;
    client->context = conn->data; //context;
    client->heartbeat_timer = furi_timer_alloc(
        api_streaming_client_heartbeat_timer_callback, FuriTimerTypePeriodic, client);
    return client;
}

static inline void api_streaming_client_free(WsClientCtx* client) {
    furi_timer_stop(client->heartbeat_timer);
    furi_timer_free(client->heartbeat_timer);
    free(client);
}

static void websocket_test_on_open(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;
    furi_assert(instance);

    WsClientCtx* ws_client = api_streaming_client_alloc(conn);

    if(WsClientsList_empty_p(instance->clients)) {
        FURI_LOG_I(TAG, "Start Thread");
        instance->gui = furi_record_open(RECORD_GUI);

        const size_t size = BUFFER_SIZE;
        instance->buffer = malloc(size);
        instance->stop = false;
        furi_thread_start(instance->thread);
    }

    // Add connection to WebSocket clients list
    WsClientsList_push_back(instance->clients, ws_client);
    furi_timer_start(ws_client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);

    FURI_LOG_I(TAG, "Add client");
}

static void websocket_test_on_close(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;
    furi_assert(instance);

    // Remove connection from WebSocket clients list
    WsClientsList_it_t it;
    WsClientCtx* client = api_streaming_get_client_by_id(instance, conn->id, it);
    if(client) {
        FURI_LOG_I(TAG, "Remove client: %ld", client->conn->id);
        WsClientsList_remove(instance->clients, it);
        api_streaming_client_counter_decrement(instance, client->display_id);
        api_streaming_update_mode(instance);
        api_streaming_client_free(client);
    }

    if(WsClientsList_empty_p(instance->clients)) {
        FURI_LOG_I(TAG, "stop thread");
        instance->stop = true;
        furi_thread_join(instance->thread);

        furi_record_close(RECORD_GUI);
        free(instance->buffer);
    }

    // Clear connection callbacks
    conn_ctx->ws.on_open = NULL;
    conn_ctx->ws.on_message = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->on_wakeup = NULL;

    FURI_LOG_I(TAG, "Close");
}

static void api_streaming_send_frame(struct mg_connection* conn, void* data, size_t len) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;

    WsClientCtx* client = api_streaming_get_client_by_id(instance, conn->id, NULL);
    if(client->state == WsClientStateActive) {
        ///TODO: simplify this
        if(client->display_id != instance->display_id) {
            FURI_LOG_W(TAG, "Display mismatch");
            return;
        }

        if(furi_mutex_acquire(instance->mutex, 10) != FuriStatusOk) {
            FURI_LOG_W(TAG, "Unable to lock frame");

        } else {
            mg_ws_send(conn, instance->buffer, instance->frame_size, WEBSOCKET_OP_BINARY);
            furi_mutex_release(instance->mutex);
        }
    } else if(client->state == WsClientStateRequestingPing) {
        FURI_LOG_I(TAG, "Requesting ping");
        api_streaming_client_set_state(client, WsClientStateWaitingPong);
        mg_ws_send(conn, data, len, WEBSOCKET_OP_PING);
    } else if(client->state == WsClientStateInvalid) {
        mg_close_conn(conn);
    }
}

static void websocket_test_on_message(struct mg_connection* conn, struct mg_ws_message* ws_msg) {
    furi_assert(conn);

    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* instance = conn_ctx->context;
    WsClientCtx* const client = api_streaming_get_client_by_id(instance, conn->id, NULL);

    ///TODO: make some MACRO to check FLAGS
    ///TODO: squash PING and PONG ifs to a single one
    if((ws_msg->flags & WEBSOCKET_OP_PING) == WEBSOCKET_OP_PING) {
        FURI_LOG_I(TAG, "PING");
        api_streaming_client_set_state(client, WsClientStateActive);
        furi_timer_restart(client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);
    } else if((ws_msg->flags & WEBSOCKET_OP_PONG) == WEBSOCKET_OP_PONG) {
        FURI_LOG_I(TAG, "PONG");
        api_streaming_client_set_state(client, WsClientStateActive);
        furi_timer_restart(client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);
    } else if((ws_msg->flags & WEBSOCKET_OP_TEXT) == WEBSOCKET_OP_TEXT) {
        FURI_LOG_I(TAG, "MSG");
        const char* resp;
        do {
            GuiDisplayId display_id = mg_json_get_long(ws_msg->data, "$.display", GuiDisplayIdMax);
            if(display_id >= GuiDisplayIdMax) {
                resp = "Wrong display value";
                break;
            }

            if(client->state == WsClientStateActive && client->display_id == display_id) {
                resp = "Same screen ignore";
                break;
            }

            if(client->state == WsClientStateActive && client->display_id != display_id) {
                api_streaming_client_counter_move(instance, display_id);
                resp = "Change screen";
            } else if(client->state == WsClientStateIdle) {
                api_streaming_client_counter_increment(instance, display_id);
                resp = "Start streaming...";
            } else {
                resp = "Unknown client state";
                break;
            }

            client->display_id = display_id;
            api_streaming_update_mode(instance);
            api_streaming_client_set_state(client, WsClientStateActive);
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

        if(WsClientsList_size(instance->clients) >= MAX_CLIENTS_COUNT) {
            MG_REPLY_ERROR(conn, 400, "Exceed max clients count");
            break;
        }

        // Assign connection callbacks
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->ws.on_open = websocket_test_on_open;
        conn_ctx->on_close = websocket_test_on_close;
        conn_ctx->ws.on_message = websocket_test_on_message;
        // conn_ctx->ws.on_ctrl = websocket_test_on_ctrl;
        conn_ctx->on_wakeup = api_streaming_send_frame;
        conn_ctx->context = instance;

        // Upgrade connection to WebSocket
        mg_ws_upgrade(conn, msg, NULL);

        success = true;
    } while(false);

    return success;
}

static void api_streaming_update_display_id(ApiStreamingCtx* instance) {
    do {
        if(furi_mutex_acquire(instance->mutex, 100) != FuriStatusOk) {
            FURI_LOG_W(TAG, "Unable to lock display_id");
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

static int32_t streaming_frame_update_callback(void* context) {
    ApiStreamingCtx* instance = context;

    while(!instance->stop) {
        if(furi_mutex_acquire(instance->mutex, 10) != FuriStatusOk) {
            FURI_LOG_W(TAG, "Unable to lock in thread");
            continue;
        }

        const size_t frame_size =
            gui_display_get_frame_buffer_size(instance->gui, instance->display_id);
        const uint8_t blk_size = instance->display_id == GuiDisplayIdFront ? 3 : 2;

        gui_lock(instance->gui);
        const uint8_t* frame = gui_display_get_frame_buffer(instance->gui, instance->display_id);
        instance->frame_size =
            rle_compress(frame, frame_size, instance->buffer, BUFFER_SIZE, blk_size);

        gui_unlock(instance->gui);
        furi_mutex_release(instance->mutex);

        struct mg_mgr* mgr = web_srv_get_mgr();
        WsClientsList_it_t it;
        for(WsClientsList_it(it, instance->clients); !WsClientsList_end_p(it);
            WsClientsList_next(it)) {
            WsClientCtx* const* it_ptr = WsClientsList_cref(it);
            WsClientCtx* client = *it_ptr;
            if(client->display_id == instance->display_id)
                mg_wakeup(mgr, client->conn->id, NULL, 0);
        }

        furi_delay_ms(200);
        api_streaming_update_display_id(instance);
    }
    return 0;
}

void* http_api_streaming_ws_alloc(void) {
    ApiStreamingCtx* context = malloc(sizeof(ApiStreamingCtx));
    WsClientsList_init(context->clients);

    context->thread =
        furi_thread_alloc_ex("WsFrameUpd", 1024U, streaming_frame_update_callback, context);
    context->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    return context;
}

void http_api_streaming_ws_free(void* ctx) {
    furi_assert(ctx);
    ApiStreamingCtx* context = ctx;
    WsClientsList_clear(context->clients);
    furi_thread_free(context->thread);
    free(context);
}

bool http_api_streaming_single_frame_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);
    Gui* gui = furi_record_open(RECORD_GUI);

    GuiDisplayId display_id = GuiDisplayIdBack;
    const size_t size = gui_display_get_frame_buffer_size(gui, display_id);

    uint8_t* frame = malloc(size);

    with_gui(gui, {
        const uint8_t* buf = gui_display_get_frame_buffer(gui, display_id);
        memcpy(frame, buf, size);
    });
    furi_record_close(RECORD_GUI);

    mg_http_reply(
        conn, 200, "Content-Type: image/bmp\r\n", "%M\r\n", mg_print_base64, size, frame);

    free(frame);

    return true;
}
