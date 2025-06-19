#include "http_api.h"
#include <gui/gui.h>
#include <toolbox/rle_encode.h>

#define TAG                        "Stream"
#define BUFFER_SIZE                (1024U * 14)
#define MAX_CLIENTS_COUNT          (4)
#define CLIENT_HEARTBEAT_PERIOD_MS (10000)

typedef enum {
    WsClientStateStopped,
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

static void api_streaming_ws_client_set_state(WsClientCtx* client, WsClientState new_state) {
    furi_assert(client);
    FURI_LOG_I(TAG, "Client state %d -> %d", client->state, new_state);
    client->state = new_state;
}

// Async send from another thread
void websocket_test_async_tmr_cb(void* ctx) {
    FURI_LOG_I(TAG, "Heartbeat timer");

    WsClientCtx* client = ctx;

    struct mg_mgr* mgr = web_srv_get_mgr();

    if(client->state != WsClientStateWaitingPong && client->state != WsClientStateInvalid) {
        // client->state = WsClientStateRequestingPing;
        api_streaming_ws_client_set_state(client, WsClientStateRequestingPing);
        mg_wakeup(mgr, client->conn->id, NULL, 0);
    } else if(client->state == WsClientStateWaitingPong) {
        //client->state = WsClientStateInvalid;
        api_streaming_ws_client_set_state(client, WsClientStateInvalid);
        mg_wakeup(mgr, client->conn->id, NULL, 0);
    }
}

static void websocket_test_on_open(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* context = conn_ctx->context;
    furi_assert(context);

    // Add connection to WebSocket clients list
    WsClientCtx* ws_client = malloc(sizeof(WsClientCtx));
    ws_client->conn = conn;
    ws_client->display_id = GuiDisplayIdMax;
    ws_client->state = WsClientStateStopped;
    ws_client->context = context;
    ws_client->heartbeat_timer =
        furi_timer_alloc(websocket_test_async_tmr_cb, FuriTimerTypePeriodic, ws_client);

    if(WsClientsList_empty_p(context->clients)) {
        FURI_LOG_I(TAG, "Start Thread");
        context->gui = furi_record_open(RECORD_GUI);

        const size_t size = BUFFER_SIZE;
        context->buffer = malloc(size);
        context->stop = false;
        furi_thread_start(context->thread);
    }

    WsClientsList_push_back(context->clients, ws_client);
    furi_timer_start(ws_client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);

    FURI_LOG_I(TAG, "Add client");
}

static void websocket_test_on_close(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    ApiStreamingCtx* context = conn_ctx->context;
    furi_assert(context);

    // Remove connection from WebSocket clients list
    WsClientsList_it_t it;
    for(WsClientsList_it(it, context->clients); !WsClientsList_end_p(it); WsClientsList_next(it)) {
        WsClientCtx* const* it_ptr = WsClientsList_cref(it);
        WsClientCtx* client = *it_ptr;
        if(client->conn == conn) {
            //  furi_timer_free(WsClientsList_cref(it)->test_tmr);
            WsClientsList_remove(context->clients, it);
            FURI_LOG_I(TAG, "Free client");

            if(client->display_id == GuiDisplayIdFront)
                context->front_clients_count--;
            else if(client->display_id == GuiDisplayIdBack)
                context->back_clients_count--;

            if(context->front_clients_count > 0 && context->back_clients_count > 0)
                context->mode = ApiStreamingModeDualScreen;
            else {
                context->display_id = (context->front_clients_count > 0) ? GuiDisplayIdFront :
                                                                           GuiDisplayIdBack;
                context->mode = ApiStreamingModeSingleScreen;
            }

            furi_timer_stop(client->heartbeat_timer);
            furi_timer_free(client->heartbeat_timer);
            free(client);
            break;
        }
    }

    if(WsClientsList_empty_p(context->clients)) {
        FURI_LOG_I(TAG, "stop thread");
        // furi_check(furi_timer_stop(context->timer) == FuriStatusOk);
        context->stop = true;
        furi_thread_join(context->thread);

        furi_record_close(RECORD_GUI);
        free(context->buffer);
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
    ApiStreamingCtx* context = conn_ctx->context;

    WsClientCtx* client = api_streaming_get_client_by_id(context, conn->id, NULL);
    if(client->state == WsClientStateActive) {
        // const size_t size = gui_display_get_frame_buffer_size(context->gui, context->display_id);
        if(furi_mutex_acquire(context->mutex, 10) != FuriStatusOk) {
            FURI_LOG_W(TAG, "Unable to lock frame");

        } else {
            mg_ws_send(conn, context->buffer, context->frame_size, WEBSOCKET_OP_BINARY);
            furi_mutex_release(context->mutex);
        }
    } else if(client->state == WsClientStateRequestingPing) {
        // client->state = WsClientStateWaitingPong;
        FURI_LOG_I(TAG, "Requesting ping");
        api_streaming_ws_client_set_state(client, WsClientStateWaitingPong);
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

    if((ws_msg->flags & WEBSOCKET_OP_PING) == WEBSOCKET_OP_PING) {
        FURI_LOG_I(TAG, "PING");
        api_streaming_ws_client_set_state(client, WsClientStateActive);
        furi_timer_restart(client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);
    } else if((ws_msg->flags & WEBSOCKET_OP_PONG) == WEBSOCKET_OP_PONG) {
        FURI_LOG_I(TAG, "PONG");
        api_streaming_ws_client_set_state(client, WsClientStateActive);
        furi_timer_restart(client->heartbeat_timer, CLIENT_HEARTBEAT_PERIOD_MS);
    } else if((ws_msg->flags & WEBSOCKET_OP_TEXT) == WEBSOCKET_OP_TEXT) {
        FURI_LOG_I(TAG, "MSG");
        GuiDisplayId display_id = mg_json_get_long(ws_msg->data, "$.display", GuiDisplayIdMax);
        if(display_id < GuiDisplayIdMax) {
            const char* m = "Start streaming...";
            FURI_LOG_I(TAG, m);

            client->display_id = display_id;
            // if(instance->mode == ApiStreamingModeIdle) {
            //     instance->mode = ApiStreamingModeSingleScreen;
            //     instance->display_id = display_id;
            // } else if(
            //     instance->mode == ApiStreamingModeSingleScreen &&
            //     instance->display_id != display_id) {
            //     instance->mode = ApiStreamingModeDualScreen;
            // }

            if(display_id == GuiDisplayIdFront)
                instance->front_clients_count++;
            else if(display_id == GuiDisplayIdBack)
                instance->back_clients_count++;

            if(instance->front_clients_count > 0 && instance->back_clients_count > 0)
                instance->mode = ApiStreamingModeDualScreen;
            else {
                instance->display_id = (instance->front_clients_count > 0) ? GuiDisplayIdFront :
                                                                             GuiDisplayIdBack;
                instance->mode = ApiStreamingModeSingleScreen;
            }

            // client->state = WsClientStateActive;
            api_streaming_ws_client_set_state(client, WsClientStateActive);
            mg_ws_send(conn, m, strlen(m), WEBSOCKET_OP_TEXT);
        } else {
            const char* m = "Invalid screen skip";
            FURI_LOG_W(TAG, m);
            mg_ws_send(conn, m, strlen(m), WEBSOCKET_OP_TEXT);
        }
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

        // FURI_LOG_I(TAG, "Size: %d", instance->frame_size);
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
        if(instance->mode == ApiStreamingModeDualScreen) instance->display_id ^= 1;
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
