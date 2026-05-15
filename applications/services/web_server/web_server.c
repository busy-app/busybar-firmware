#include <furi.h>
#include <version.h>
#include "web_server_i.h"
#include "http_api/http_api.h"
#include <sysctl/sysctl.h>
#include <netstat/netstat.h>

#define TAG "HttpSrv"

typedef struct {
    HttpHandlersList_t handlers;
    struct mg_mgr mgr; // Event manager
} WebServer;

static WebServer srv = {0};

static const HttpHandler handlers_root[] = {
    {
        .uri = "/api",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .on_request = http_api_root_callback,
        .on_headers = http_api_root_hdr_callback,
        .ctx_alloc = http_api_root_alloc,
        .ctx_free = http_api_root_free,
    },
    {
        .uri = "",
        .method = HttpMethodGet,
        .type = HttpHandlerDir,
        .path = WEB_ROOT,
        .mime_types_custom = NULL,
        .extra_headers = HEADER_CORS,
    },
};

static const struct {
    const char* name;
    HttpMethod method;
} http_methods[] = {
    {"GET", HttpMethodGet},
    {"POST", HttpMethodPost},
    {"DELETE", HttpMethodDelete},
    {"PUT", HttpMethodPut},
    {"OPTIONS", HttpMethodOptions},
    {"HEAD", HttpMethodHead},
    {"CONNECT", HttpMethodConnect},
    {"PATCH", HttpMethodPatch},
    {"TRACE", HttpMethodTrace},
};

static HttpMethod http_method_from_str(struct mg_http_message* msg) {
    for(size_t i = 0; i < COUNT_OF(http_methods); i++) {
        if(mg_strcasecmp(msg->method, mg_str(http_methods[i].name)) == 0) {
            if(http_methods[i].method == HttpMethodGet) {
                return (IS_WEBSOCKET_UPGRADE(msg) ? HttpMethodWebSocket : HttpMethodGet);
            }
            return http_methods[i].method;
        }
    }
    return HttpMethodUnknown;
}

void http_reply_405_method_not_allowed(struct mg_connection* conn, HttpMethod allowed_methods) {
    if(allowed_methods & HttpMethodWebSocket) {
        allowed_methods = (allowed_methods & ~(HttpMethodWebSocket)) | HttpMethodGet;
    }
    FuriString* headers = furi_string_alloc_set(DEFAULT_JSON_HEADERS);
    furi_string_cat(headers, "Allow: ");
    bool is_first = true;
    for(size_t i = 0; i < COUNT_OF(http_methods); i++) {
        if(allowed_methods & http_methods[i].method) {
            furi_string_cat_printf(headers, "%s%s", is_first ? "" : ", ", http_methods[i].name);
            is_first = false;
        }
    }
    furi_string_cat(headers, "\r\n");
    mg_http_reply(
        conn, 405, furi_string_get_cstr(headers), "{\"error\":\"%s\"}\n", "Method Not Allowed");
    furi_string_free(headers);
}

static void http_event_handler(struct mg_connection* conn, int ev, void* ev_data) {
    if(ev == MG_EV_HTTP_MSG) {
        WebServer* context = conn->fn_data;
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        ConnectionContext* conn_ctx = (void*)conn->data;
        if(conn_ctx->raw.on_data == NULL) { // Skip raw connections
            FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);
            HttpMethod method = http_method_from_str(msg);
            bool result = http_handle_request(path, method, context->handlers, conn, msg);
            if(!result) {
                MG_REPLY_BAD_REQUEST(conn);
            }
            http_api_log_access(conn, msg, result ? http_api_extract_status(conn) : 400);
            furi_string_free(path);
        }

    } else if(ev == MG_EV_HTTP_HDRS) {
        WebServer* context = conn->fn_data;
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);
        HttpMethod method = http_method_from_str(msg);
        http_handle_headers(path, method, context->handlers, conn, msg);
        furi_string_free(path);
    } else if(ev == MG_EV_READ) {
        if(!conn->is_websocket) {
            ConnectionContext* conn_ctx = (void*)conn->data;
            if(conn_ctx->raw.on_data) {
                conn_ctx->raw.on_data(conn, &conn->recv);
            }
        }
    } else if(ev == MG_EV_WS_MSG || ev == MG_EV_WS_CTL) {
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
    HttpMethod method,
    HttpHandlersList_t handlers,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    bool handled = false;

    if(netstat_is_overloaded(NetstatLogOnOverload)) {
        MG_REPLY_ERROR(conn, 503, NETSTAT_RECOMMENDED_ERROR);
        handled = true;
        return handled;
    }

    HttpHandlersList_it_t it;
    for(HttpHandlersList_it(it, handlers); !HttpHandlersList_end_p(it);
        HttpHandlersList_next(it)) {
        const HttpHandlerInstance* inst = HttpHandlersList_cref(it);
        do {
            if(!furi_string_start_with(path, inst->handler->uri)) {
                break;
            }
            if(method == HttpMethodUnknown || !(method & inst->handler->method)) {
                http_reply_405_method_not_allowed(conn, inst->handler->method);
                handled = true;
                break;
            }
            if(inst->handler->type == HttpHandlerCustom) {
                furi_assert(inst->handler->on_request);
                FuriString* path_remain = furi_string_alloc_set(path);
                furi_string_right(path_remain, strlen(inst->handler->uri));
                if(furi_string_start_with(path_remain, "/")) {
                    furi_string_right(path_remain, 1);
                }
                handled = inst->handler->on_request(path_remain, method, conn, msg, inst->context);
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
    HttpMethod method,
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
            if(inst->handler->on_headers == NULL) {
                break;
            }
            if(!furi_string_start_with(path, inst->handler->uri)) {
                break;
            }
            if(method == HttpMethodUnknown || !(method & inst->handler->method)) {
                http_reply_405_method_not_allowed(conn, inst->handler->method);
                MG_CLOSE_AFTER_HEADERS(conn, msg);
                handled = true;
                break;
            }

            FuriString* path_remain = furi_string_alloc_set(path);
            furi_string_right(path_remain, strlen(inst->handler->uri));
            if(furi_string_start_with(path_remain, "/")) {
                furi_string_right(path_remain, 1);
            }
            handled = inst->handler->on_headers(path_remain, method, conn, msg, inst->context);
            furi_string_free(path_remain);

        } while(0);
        if(handled) break;
    }
    return handled;
}

int32_t web_srv_start(void* p) {
    UNUSED(p);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

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

    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    return 0;
}

struct mg_mgr* web_srv_get_mgr(void) {
    return (&srv.mgr);
}

void web_server_get_api_version(FuriString* version) {
    furi_assert(version);
    const uint8_t api_ver[] = API_VERSION;
    furi_string_printf(version, "%u.%u.%u", api_ver[0], api_ver[1], api_ver[2]);
}
