#include "http_api.h"
#define TAG "WS_TEST"

typedef struct {
    ClientsList_t clients;
} WsTestCtx;

static void websocket_test_on_open(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    WsTestCtx* context = conn_ctx->ws.context;
    furi_assert(context);

    // Add connection to WebSocket clients list
    ClientsList_push_back(context->clients, conn);

    FURI_LOG_I(TAG, "Open");
}

static void websocket_test_on_close(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;
    WsTestCtx* context = conn_ctx->ws.context;
    furi_assert(context);

    // Remove connection from WebSocket clients list
    ClientsList_it_t it;
    for(ClientsList_it(it, context->clients); !ClientsList_end_p(it); ClientsList_next(it)) {
        if(*ClientsList_cref(it) == conn) {
            ClientsList_remove(context->clients, it);
            break;
        }
    }

    // Clear connection callbacks
    conn_ctx->ws.on_open = NULL;
    conn_ctx->ws.on_close = NULL;
    conn_ctx->ws.on_message = NULL;

    FURI_LOG_I(TAG, "Close");
}

static void websocket_test_on_message(struct mg_connection* conn, struct mg_ws_message* ws_msg) {
    UNUSED(conn); // TODO: flags?
    FURI_LOG_I(TAG, "Msg flags: %02X len: %u", ws_msg->flags, ws_msg->data.len);
    if(ws_msg->flags & WEBSOCKET_OP_TEXT) {
        FURI_LOG_I(TAG, "%.*s", ws_msg->data.len, ws_msg->data.buf);
        mg_ws_send(conn, ws_msg->data.buf, ws_msg->data.len, WEBSOCKET_OP_TEXT);
    }
}

void* http_websocket_alloc(void) {
    WsTestCtx* context = malloc(sizeof(WsTestCtx));
    ClientsList_init(context->clients);
    return context;
}

void http_websocket_free(void* ctx) {
    furi_assert(ctx);
    WsTestCtx* context = ctx;
    ClientsList_clear(context->clients);
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
        conn_ctx->ws.on_close = websocket_test_on_close;
        conn_ctx->ws.on_message = websocket_test_on_message;
        conn_ctx->ws.context = context;

        // Upgrade connection to WebSocket
        mg_ws_upgrade(conn, msg, NULL);

        return true;
    }

    return false;
}
