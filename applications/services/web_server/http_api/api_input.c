#include "http_api.h"
#include <power/power_service/power.h>
#include <version.h>
#include <input/input.h>

#define TAG "HttpInput"

#define KEY_NAME_LEN_MAX 10

static const struct {
    char* name;
    InputKey key;
} input_keys[] = {
    {"up", InputKeyDown},
    {"down", InputKeyUp},
    {"ok", InputKeyOk},
    {"back", InputKeyBack},
    {"start", InputKeyStart},
    {"busy", InputKeyBusy},
    {"status", InputKeyStatus},
    {"off", InputKeyOff},
    {"apps", InputKeyApps},
    {"settings", InputKeySettings},
};

typedef struct {
    InputKey key[COUNT_OF(input_keys)];
    bool state[COUNT_OF(input_keys)];
} InputKeyState;

static void input_websocket_on_close(struct mg_connection* conn) {
    // Get handler context from connection data
    ConnectionContext* conn_ctx = (void*)conn->data;

    // Release all held keys
    Input* input = furi_record_open(RECORD_INPUT);
    InputKeyState* keys_context = conn_ctx->context;
    for(size_t i = 0; i < COUNT_OF(input_keys); i++) {
        if(keys_context->state[i]) {
            input_key_release(input, keys_context->key[i]);
            keys_context->state[i] = false;
        }
    }
    furi_record_close(RECORD_INPUT);

    // Clear connection callbacks
    conn_ctx->ws.on_open = NULL;
    conn_ctx->ws.on_message = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->on_wakeup = NULL;
    conn_ctx->context = NULL;

    FURI_LOG_I(TAG, "WS close");
}

static void input_websocket_on_message(struct mg_connection* conn, struct mg_ws_message* ws_msg) {
    furi_assert(conn);
    ConnectionContext* conn_ctx = (void*)conn->data;
    InputKeyState* keys_context = conn_ctx->context;

    int root_len = 0;
    int root_offset = mg_json_get(ws_msg->data, "$", &root_len);

    if((root_offset < 0) || (root_len <= 0)) {
        conn->is_draining = 1;
        return;
    }

    Input* input = furi_record_open(RECORD_INPUT);

    for(size_t i = 0; i < COUNT_OF(input_keys); i++) {
        char key_name_token[KEY_NAME_LEN_MAX + 2];
        snprintf(key_name_token, sizeof(key_name_token), "$.%s", input_keys[i].name);

        long key_val = mg_json_get_long(ws_msg->data, key_name_token, -1);
        if(key_val == 0) {
            if(keys_context->state[i] == true) {
                keys_context->state[i] = false;
                input_key_release(input, input_keys[i].key);
            }
        } else if(key_val == 1) {
            if(keys_context->state[i] == false) {
                keys_context->state[i] = true;
                input_key_press(input, input_keys[i].key);
            }
        }
    }

    furi_record_close(RECORD_INPUT);
}

bool http_api_input_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    ConnectionContext* conn_ctx = (void*)conn->data;

    if(mg_match(msg->method, mg_str("GET"), NULL) &&
       (mg_http_get_header(msg, "Sec-WebSocket-Key") != NULL)) {
        // Upgrade to WebSocket
        conn_ctx->ws.on_message = input_websocket_on_message;
        conn_ctx->on_close = input_websocket_on_close;
        conn_ctx->context = ctx;
        mg_ws_upgrade(conn, msg, NULL);
        return true;
    } else if(mg_match(msg->method, mg_str("POST"), NULL)) {
        char key_name[KEY_NAME_LEN_MAX];
        bool success = false;

        do {
            if(msg->query.len == 0) break;

            int var_len = mg_http_get_var(&msg->query, "key", key_name, sizeof(key_name));
            if(var_len <= 0) break;

            Input* input = furi_record_open(RECORD_INPUT);
            for(size_t i = 0; i < COUNT_OF(input_keys); i++) {
                if(strncmp(input_keys[i].name, key_name, var_len) == 0) {
                    input_key_toggle(input, input_keys[i].key);
                    success = true;
                    break;
                }
            }
            furi_record_close(RECORD_INPUT);
        } while(0);
        if(success) {
            MG_REPLY_OK(conn);
        } else {
            MG_REPLY_BAD_REQUEST(conn);
        }
    } else {
        MG_REPLY_METHOD_NOT_ALLOWED(conn);
    }

    return true;
}

void* http_api_input_alloc(void) {
    InputKeyState* keys_context = malloc(sizeof(InputKeyState));
    for(size_t i = 0; i < COUNT_OF(input_keys); i++) {
        keys_context->key[i] = input_keys[i].key;
        keys_context->state[i] = false;
    }
    return keys_context;
}

void http_api_input_free(void* ctx) {
    furi_assert(ctx);
    free(ctx);
}
