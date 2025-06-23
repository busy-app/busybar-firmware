#include <furi.h>
#include <version.h>
#include "web_server_i.h"
#include "http_api/http_api.h"

#define TAG "HttpSrv"

// TODO: timers

typedef struct {
    HttpHandlersList_t handlers;
    struct mg_mgr mgr; // Event manager
} WebServer;

static WebServer srv = {0};

static const HttpHandler handlers_root[] = {
    {
        .uri = "/api/v0",
        .method = "*",
        .type = HttpHandlerCustom,
        .on_request = http_api_root_callback,
        .on_headers = http_api_root_hdr_callback,
        .ctx_alloc = http_api_root_alloc,
        .ctx_free = http_api_root_free,
    },
    {
        .uri = "/ws_test",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_websocket_callback,
        .ctx_alloc = http_websocket_alloc,
        .ctx_free = http_websocket_free,
    },
    {
        .uri = "",
        .method = "GET",
        .type = HttpHandlerDir,
        .path = WEB_ROOT,
        .mime_types_custom = NULL,
        .extra_headers = HEADER_CORS,
    },
};

static void http_event_handler(struct mg_connection* conn, int ev, void* ev_data) {
    if(ev == MG_EV_HTTP_MSG) {
        WebServer* context = conn->fn_data;
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->raw.on_data == NULL) { // Skip raw connections
            FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);
            bool result = http_handle_request(path, context->handlers, conn, msg);
            furi_string_free(path);
            if(!result) {
                MG_REPLY_BAD_REQUEST(conn);
            }
        }

    } else if(ev == MG_EV_HTTP_HDRS) {
        WebServer* context = conn->fn_data;
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);
        http_handle_headers(path, context->handlers, conn, msg);
        furi_string_free(path);
    } else if(ev == MG_EV_READ) {
        if(!conn->is_websocket) {
            ConnectionContext* conn_ctx = (void*)conn->data;
            if(conn_ctx->raw.on_data) {
                conn_ctx->raw.on_data(conn, &conn->recv);
            }
        }
    } else if(ev == MG_EV_WS_MSG) {
        struct mg_ws_message* ws_msg = (struct mg_ws_message*)ev_data;
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->ws.on_message) {
            conn_ctx->ws.on_message(conn, ws_msg);
        }
    } else if(ev == MG_EV_WS_OPEN) {
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->ws.on_open) {
            conn_ctx->ws.on_open(conn);
        }
    } else if(ev == MG_EV_CLOSE) {
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->on_close) {
            conn_ctx->on_close(conn);
        }
    } else if(ev == MG_EV_WAKEUP) {
        struct mg_str* wakeup_data = (struct mg_str*)ev_data;

        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->on_wakeup) {
            conn_ctx->on_wakeup(conn, wakeup_data->buf, wakeup_data->len);
        }
    }
}

void http_handler_add(HttpHandlersList_t list, const HttpHandler* handler) {
    HttpHandlerInstance inst = {.handler = handler, .context = NULL};
    if(inst.handler->type == HttpHandlerCustom) {
        if(inst.handler->ctx_alloc) {
            inst.context = inst.handler->ctx_alloc();
        }
    }
    HttpHandlersList_push_back(list, inst);
}

void http_handler_remove(HttpHandlersList_t list, const HttpHandler* handler) {
    HttpHandlersList_it_t it;
    for(HttpHandlersList_it(it, list); !HttpHandlersList_end_p(it); HttpHandlersList_next(it)) {
        const HttpHandlerInstance* inst = HttpHandlersList_cref(it);
        if(inst->handler == handler) {
            if((inst->handler) && (inst->context)) {
                inst->handler->ctx_free(inst->context);
            }
            HttpHandlersList_remove(list, it);
            break;
        }
    }
}

void http_handler_remove_all(HttpHandlersList_t list) {
    HttpHandlersList_it_t it;
    for(HttpHandlersList_it(it, list); !HttpHandlersList_end_p(it); HttpHandlersList_next(it)) {
        const HttpHandlerInstance* inst = HttpHandlersList_cref(it);
        if((inst->handler) && (inst->context)) {
            inst->handler->ctx_free(inst->context);
        }
    }
    HttpHandlersList_reset(list);
}

bool http_handle_request(
    FuriString* path,
    HttpHandlersList_t handlers,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    bool handled = false;
    HttpHandlersList_it_t it;
    for(HttpHandlersList_it(it, handlers); !HttpHandlersList_end_p(it);
        HttpHandlersList_next(it)) {
        const HttpHandlerInstance* inst = HttpHandlersList_cref(it);
        do {
            if(!furi_string_start_with(path, inst->handler->uri)) {
                break;
            }
            if(!mg_match(msg->method, mg_str(inst->handler->method), NULL)) {
                break;
            }
            if(inst->handler->type == HttpHandlerCustom) {
                furi_assert(inst->handler->on_request);
                FuriString* path_remain = furi_string_alloc_set(path);
                furi_string_right(path_remain, strlen(inst->handler->uri));
                if(furi_string_start_with(path_remain, "/")) {
                    furi_string_right(path_remain, 1);
                }
                handled = inst->handler->on_request(path_remain, conn, msg, inst->context);
                furi_string_free(path_remain);
            } else if(inst->handler->type == HttpHandlerFile) {
                struct mg_http_serve_opts opts = {
                    .ssi_pattern = NULL,
                    .extra_headers = inst->handler->extra_headers,
                    .mime_types = inst->handler->mime_types_custom,
                    .page404 = NULL, // TODO: WEB_ROOT "404.html",
                    .fs = http_fs_get(),
                };
                mg_http_serve_file(conn, msg, inst->handler->path, &opts);
                handled = true;
            } else if(inst->handler->type == HttpHandlerDir) {
                struct mg_http_serve_opts opts = {
                    .root_dir = inst->handler->path,
                    .ssi_pattern = NULL,
                    .extra_headers = inst->handler->extra_headers,
                    .mime_types = inst->handler->mime_types_custom,
                    .page404 = NULL, // TODO: WEB_ROOT "404.html",
                    .fs = http_fs_get(),
                };
                mg_http_serve_dir(conn, msg, &opts);
                handled = true;
            } else {
                furi_crash();
            }
        } while(0);
        if(handled) break;
    }
    return handled;
}

bool http_handle_headers(
    FuriString* path,
    HttpHandlersList_t handlers,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    bool handled = false;
    HttpHandlersList_it_t it;
    for(HttpHandlersList_it(it, handlers); !HttpHandlersList_end_p(it);
        HttpHandlersList_next(it)) {
        const HttpHandlerInstance* inst = HttpHandlersList_cref(it);
        do {
            if(inst->handler->type != HttpHandlerCustom) {
                break;
            }
            if(!furi_string_start_with(path, inst->handler->uri)) {
                break;
            }
            if(!mg_match(msg->method, mg_str(inst->handler->method), NULL)) {
                break;
            }
            if(inst->handler->on_headers) {
                FuriString* path_remain = furi_string_alloc_set(path);
                furi_string_right(path_remain, strlen(inst->handler->uri));
                if(furi_string_start_with(path_remain, "/")) {
                    furi_string_right(path_remain, 1);
                }
                handled = inst->handler->on_headers(path_remain, conn, msg, inst->context);
                furi_string_free(path_remain);
            }
        } while(0);
        if(handled) break;
    }
    return handled;
}

int32_t web_srv_start(void* p) {
    UNUSED(p);
    UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
    usb_network_thread_init(usb_network);

    // mg_log_set(MG_LL_VERBOSE);
    mg_log_set(MG_LL_INFO);

    mg_mgr_init(&srv.mgr); // Initialise event manager
    mg_wakeup_init(&srv.mgr);

    HttpHandlersList_init(srv.handlers);

    for(size_t i = COUNT_OF(handlers_root); i > 0; i--) {
        http_handler_add(srv.handlers, &handlers_root[i - 1]);
    }

    // Setup listener
    mg_http_listen(&srv.mgr, "http://0.0.0.0", http_event_handler, &srv);

    // Event loop
    while(1) {
        mg_mgr_poll(&srv.mgr, 1000);
    }

    http_handler_remove_all(srv.handlers);

    // Cleanup
    mg_mgr_free(&srv.mgr);

    usb_network_thread_cleanup(usb_network);
    furi_record_close(RECORD_USB_NETWORK);

    return 0;
}

struct mg_mgr* web_srv_get_mgr(void) {
    return (&srv.mgr);
}

uint64_t mg_millis(void) {
    return furi_get_tick();
}

void mg_log_prefix(int level, const char* file, int line, const char* fname) {
    UNUSED(file);
    FuriString* string = furi_string_alloc();

    const char* color = _FURI_LOG_CLR_RESET;
    const char* log_letter = " ";
    switch(level) {
    case MG_LL_ERROR:
        color = _FURI_LOG_CLR_E;
        log_letter = "E";
        break;
    case MG_LL_INFO:
        color = _FURI_LOG_CLR_I;
        log_letter = "I";
        break;
    case MG_LL_DEBUG:
        color = _FURI_LOG_CLR_D;
        log_letter = "D";
        break;
    case MG_LL_VERBOSE:
        color = _FURI_LOG_CLR_T;
        log_letter = "T";
        break;
    default:
        break;
    }

    furi_string_printf(
        string,
        "%lu %s[%s][%s] " _FURI_LOG_CLR_RESET,
        furi_get_tick(),
        color,
        log_letter,
        "Mongoose");

    furi_string_cat_printf(string, "%s:%u ", fname, line);
    furi_log_puts(furi_string_get_cstr(string));

    furi_string_free(string);
}

void mg_log(const char* fmt, ...) {
    FuriString* string = furi_string_alloc();

    va_list args;
    va_start(args, fmt);
    furi_string_vprintf(string, fmt, args);
    va_end(args);

    furi_string_cat_str(string, "\r\n");
    furi_log_puts(furi_string_get_cstr(string));
    furi_string_free(string);
}

int _gettimeofday(struct timeval* tv, void* tz) {
    uint64_t now = mg_now();
    (void)tz;
    tv->tv_sec = (time_t)(now / 1000);
    tv->tv_usec = (unsigned long)((now % 1000) * 1000);
    return 0;
}
