#include "http_api.h"
#include <version.h>
#include <gui/gui.h>
#include <gui/modules/image.h>

typedef struct {
    bool led_state;
} ApiLedCtx;

void* http_api_led_alloc(void) {
    return malloc(sizeof(ApiLedCtx));
}

void http_api_led_free(void* ctx) {
    free(ctx);
}

bool http_api_led_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiLedCtx* context = ctx;

    bool success = false;

    if(mg_match(msg->method, mg_str("GET"), NULL)) {
        if(msg->query.len == 0) {
            // Get current value
            success = true;
        } else {
            // Set by query string
            char led_state_str[2];
            do {
                int var_len =
                    mg_http_get_var(&msg->query, "state", led_state_str, sizeof(led_state_str));
                if(var_len != 1) {
                    break;
                }
                if(led_state_str[0] == '0') {
                    context->led_state = false;
                } else if(led_state_str[0] == '1') {
                    context->led_state = true;
                } else {
                    break;
                }
                success = true;
            } while(0);
        }
    } else {
        // Set by JSON post
        do {
            struct mg_str led_state_str = mg_json_get_tok(msg->body, "$.state");
            if(led_state_str.len != 1) {
                break;
            }
            if(led_state_str.buf[0] == '0') {
                context->led_state = false;
            } else if(led_state_str.buf[0] == '1') {
                context->led_state = true;
            } else {
                break;
            }
            success = true;
        } while(0);
    }

    if(success) {
        mg_http_reply(
            conn,
            200,
            "Content-Type: application/json\r\n",
            "{\"result\":\"OK\",\"state\":%u}\n",
            context->led_state);
    }
    return success;
}

bool http_api_version_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);
    FuriString* ver_str = furi_string_alloc();
    const Version* firmware_version = version_get();
    furi_string_printf(
        ver_str,
        "\"branch\":\"%s\",\"version\":\"%s\",\"build_date\":\"%s\",",
        version_get_gitbranch(firmware_version),
        version_get_version(firmware_version),
        version_get_builddate(firmware_version));
    furi_string_cat_printf(
        ver_str,
        "\"commit_hash\":\"%s%s\"",
        version_get_githash(firmware_version),
        version_get_dirty_flag(firmware_version) ? "-dirty" : "");

    mg_http_reply(
        conn, 200, "Content-Type: application/json\r\n", "{%s}\n", furi_string_get_cstr(ver_str));
    furi_string_free(ver_str);

    return true;
}

static bool
    http_api_display_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);
    char file_path[32];
    GuiDisplayId display_id;
    bool success = false;

    do {
        if(msg->query.len == 0) {
            break;
        }

        int var_len = mg_http_get_var(&msg->query, "path", file_path, sizeof(file_path));
        if(var_len <= 0) {
            break;
        }

        Storage* storage = furi_record_open(RECORD_STORAGE);
        bool file_exists = storage_file_exists(storage, file_path);
        furi_record_close(RECORD_STORAGE);
        if(!file_exists) {
            FURI_LOG_E("HTTP display", "Image file not exists");
            break;
        }

        char display_id_str[8];
        var_len = mg_http_get_var(&msg->query, "display", display_id_str, sizeof(display_id_str));
        if(var_len <= 0) {
            break;
        }

        if(strncmp(display_id_str, "front", 5) == 0) {
            display_id = GuiDisplayIdFront;
        } else if(strncmp(display_id_str, "back", 4) == 0) {
            display_id = GuiDisplayIdBack;
        } else {
            break;
        }
        success = true;
    } while(0);

    if(!success) {
        mg_http_reply(conn, 400, "", "Bad Request");
        return true;
    }

    FURI_LOG_I(
        "HTTP display",
        "Showing %s on %s display",
        file_path,
        (display_id == GuiDisplayIdBack) ? "back" : "front");

    Gui* gui = furi_record_open(RECORD_GUI);

    Image* image;

    bool result = false;
    with_gui(gui, {
        GuiLayer* system_layer = gui_get_layer(gui, GuiLayerIdSystem);
        Widget* root = gui_layer_get_root_widget(system_layer, display_id);
        image = image_alloc(root);
        result = image_set_source(image, file_path);
    });

    if(!result) {
        FURI_LOG_I("HTTP display", "Unable to load image");
        mg_http_reply(conn, 400, "", "Image load error");
    } else {
        mg_http_reply(conn, 200, "Content-Type: application/json\r\n", "{\"result\":\"OK\"}\n");
    }

    // TODO: with_gui(gui, { image_free(image); });

    furi_record_close(RECORD_GUI);

    return true;
}

static const HttpHandler handlers_api_root[] = {
    {
        .uri = "/*/led",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_led_alloc,
        .ctx_free = http_api_led_free,
        .callback = http_api_led_callback,
    },
    {
        .uri = "/*/version",
        .method = "GET",
        .type = HttpHandlerCustom,
        .callback = http_api_version_callback,
    },
    {
        .uri = "/*/display",
        .method = "GET",
        .type = HttpHandlerCustom,
        .callback = http_api_display_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiRootCtx;

void* http_api_root_alloc(void) {
    ApiRootCtx* context = malloc(sizeof(ApiRootCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_api_root); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_api_root[i - 1]);
    }
    return context;
}

void http_api_root_free(void* ctx) {
    furi_assert(ctx);
    ApiRootCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_root_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiRootCtx* context = ctx;
    FURI_LOG_I(
        "HTTP API", "%.*s %.*s", msg->method.len, msg->method.buf, msg->uri.len, msg->uri.buf);
    if(msg->query.len > 0) {
        FURI_LOG_I("HTTP API", "Query %.*s", msg->query.len, msg->query.buf);
    }
    return http_handle_request(context->handlers, conn, msg);
}
