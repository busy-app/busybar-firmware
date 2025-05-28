#include "http_api.h"
#define TAG "WS_TEST"

typedef struct {
    struct mg_connection* conn;
    FuriTimer* test_tmr;
} WsClientCtx;

LIST_DEF(WsClientsList, WsClientCtx, M_POD_OPLIST);

typedef struct {
    WsClientsList_t clients;
} WsTestCtx;

// Async send from another thread
static void websocket_test_async_tmr_cb(void* context) {
    unsigned long client_id = (unsigned long)context;
    struct mg_str tick_str = mg_str("Tick");
    mg_wakeup(web_srv_get_mgr(), client_id, tick_str.buf, tick_str.len);
}

static void websocket_test_on_open(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    WsTestCtx* context = conn_ctx->context;
    furi_assert(context);

    // Add connection to WebSocket clients list
    WsClientCtx ws_client = {.conn = conn};
    ws_client.test_tmr =
        furi_timer_alloc(websocket_test_async_tmr_cb, FuriTimerTypePeriodic, (void*)conn->id);
    furi_check(furi_timer_start(ws_client.test_tmr, 1000) == FuriStatusOk);

    WsClientsList_push_back(context->clients, ws_client);

    FURI_LOG_I(TAG, "Open");
}

static void websocket_test_on_close(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    WsTestCtx* context = conn_ctx->context;
    furi_assert(context);

    // Remove connection from WebSocket clients list
    WsClientsList_it_t it;
    for(WsClientsList_it(it, context->clients); !WsClientsList_end_p(it); WsClientsList_next(it)) {
        if(WsClientsList_cref(it)->conn == conn) {
            furi_timer_free(WsClientsList_cref(it)->test_tmr);
            WsClientsList_remove(context->clients, it);
            break;
        }
    }

    // Clear connection callbacks
    conn_ctx->ws.on_open = NULL;
    conn_ctx->ws.on_message = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->on_wakeup = NULL;

    FURI_LOG_I(TAG, "Close");
}

static void websocket_test_on_message(struct mg_connection* conn, struct mg_ws_message* ws_msg) {
    furi_assert(conn);
    FURI_LOG_I(TAG, "Msg flags: %02X len: %u", ws_msg->flags, ws_msg->data.len);
    if(ws_msg->flags & WEBSOCKET_OP_TEXT) {
        FURI_LOG_I(TAG, "%.*s", ws_msg->data.len, ws_msg->data.buf);
        mg_ws_send(conn, ws_msg->data.buf, ws_msg->data.len, WEBSOCKET_OP_TEXT);
    }
}

static void websocket_test_on_wakeup(struct mg_connection* conn, void* data, size_t len) {
    furi_assert(conn);
    UNUSED(data);
    UNUSED(len);
    FURI_LOG_I(TAG, "Wakeup");
    mg_ws_send(conn, data, len, WEBSOCKET_OP_TEXT);
}

void* http_websocket_alloc(void) {
    WsTestCtx* context = malloc(sizeof(WsTestCtx));
    WsClientsList_init(context->clients);
    return context;
}

void http_websocket_free(void* ctx) {
    furi_assert(ctx);
    WsTestCtx* context = ctx;
    WsClientsList_clear(context->clients);
    free(context);
}

bool http_websocket_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    furi_assert(ctx);
    WsTestCtx* context = ctx;

    bool is_ws_upgrade = (mg_http_get_header(msg, "Sec-WebSocket-Key") != NULL);
    if(is_ws_upgrade) {
        // Assign connection callbacks
        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->ws.on_open = websocket_test_on_open;
        conn_ctx->ws.on_message = websocket_test_on_message;
        conn_ctx->on_close = websocket_test_on_close;
        conn_ctx->on_wakeup = websocket_test_on_wakeup;
        conn_ctx->context = context;

        // Upgrade connection to WebSocket
        mg_ws_upgrade(conn, msg, NULL);

        return true;
    }

    return false;
}
