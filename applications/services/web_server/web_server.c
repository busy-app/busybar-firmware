#include <furi.h>
#include <version.h>
#include "web_server_i.h"
#include "http_api/http_api.h"
#include <sysctl/sysctl.h>
#include <netstat/netstat.h>
#include <toolbox/path.h>

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

#define HTTP_API_METHODS \
    ((HttpMethod)(HttpMethodGet | HttpMethodPost | HttpMethodPut | HttpMethodDelete))

void http_reply_405_method_not_allowed(struct mg_connection* conn, HttpMethod allowed_methods) {
    if(allowed_methods & HttpMethodWebSocket) {
        allowed_methods = (HttpMethod)((allowed_methods & ~HttpMethodWebSocket) | HttpMethodGet);
    }
    allowed_methods = (HttpMethod)(allowed_methods & HTTP_API_METHODS);
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

void http_reply_cors_preflight(struct mg_connection* conn, HttpMethod allowed_methods) {
    if(allowed_methods & HttpMethodWebSocket) {
        allowed_methods = (HttpMethod)((allowed_methods & ~HttpMethodWebSocket) | HttpMethodGet);
    }
    allowed_methods = (HttpMethod)(allowed_methods & HTTP_API_METHODS);
    FuriString* headers = furi_string_alloc_set(HEADER_CORS_ORIGIN HEADER_CORS_HEADERS
                                                "Access-Control-Allow-Methods: ");
    bool is_first = true;
    for(size_t i = 0; i < COUNT_OF(http_methods); i++) {
        if(allowed_methods & http_methods[i].method) {
            furi_string_cat_printf(headers, "%s%s", is_first ? "" : ", ", http_methods[i].name);
            is_first = false;
        }
    }
    furi_string_cat(headers, "\r\n");
    mg_http_reply(conn, 200, furi_string_get_cstr(headers), "");
    furi_string_free(headers);
}

#define HTTP_UPLOAD_IDLE_TIMEOUT_MS 3000

typedef struct {
    size_t len_remain;
    void* file;
    uint64_t timeout_stamp;
} HttpUploadCtx;

static void http_upload_data_callback(struct mg_connection* conn, struct mg_iobuf* data) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    HttpUploadCtx* upload_ctx = conn_ctx->context;

    bool do_close_file = false;

    if(upload_ctx == NULL) return;
    if(upload_ctx->file == NULL) {
        data->len = 0; // drain any lingering data arriving on a completed/failed upload
        return;
    }

    upload_ctx->timeout_stamp = mg_millis() + HTTP_UPLOAD_IDLE_TIMEOUT_MS;

    if((data->len > 0) && (upload_ctx->file)) {
        // Write file chunk
        size_t write_len = MIN(data->len, upload_ctx->len_remain);
        if(http_fs_get()->wr(upload_ctx->file, data->buf, write_len) != write_len) {
            FURI_LOG_E(TAG, "Failed to write file chunk");
            MG_REPLY_INTERNAL_ERROR(conn, "Failed to write file chunk");
            do_close_file = true;
        } else {
            upload_ctx->len_remain -= write_len;
            FURI_LOG_T(
                TAG,
                "Wrote %zu bytes to file, remaining %zu bytes",
                write_len,
                upload_ctx->len_remain);
        }
    }

    if(upload_ctx->len_remain == 0) {
        MG_REPLY_OK_CLOSE(conn);
        do_close_file = true; // End of write
    }

    if(do_close_file) { // error or end of write
        FURI_LOG_I(TAG, "Closing file after write data");
        if(upload_ctx->file) {
            http_fs_get()->cl(upload_ctx->file);
            upload_ctx->file = NULL;
        }
        conn->is_draining = 1; // Drain connection
    }
    data->len = 0;
}

static void http_upload_close_callback(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    HttpUploadCtx* upload_ctx = conn_ctx->context;
    if(upload_ctx->file) {
        http_fs_get()->cl(upload_ctx->file);
        upload_ctx->file = NULL;
    }
    free(upload_ctx);
    conn_ctx->on_close = NULL;
    conn_ctx->raw.on_data = NULL;
    conn_ctx->raw.on_poll = NULL;
    conn_ctx->context = NULL;
}

static void http_upload_poll_callback(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    HttpUploadCtx* upload_ctx = conn_ctx->context;
    furi_assert(upload_ctx);

    if(mg_timer_expired(&upload_ctx->timeout_stamp, HTTP_UPLOAD_IDLE_TIMEOUT_MS, mg_millis())) {
        FURI_LOG_E(TAG, "Connection data timeout (%lu)", conn->id);
        conn->is_draining = 1; // Force close hanging connection
    }
}

void http_upload_start(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    const char* file_path) {
    // Create upload context
    HttpUploadCtx* upload_ctx = malloc(sizeof(HttpUploadCtx));
    upload_ctx->len_remain = msg->body.len;
    upload_ctx->file = NULL;
    upload_ctx->timeout_stamp = mg_millis() + HTTP_UPLOAD_IDLE_TIMEOUT_MS;

    // Assign callbacks
    ConnectionContext* conn_ctx = (void*)conn->data;
    conn_ctx->on_close = http_upload_close_callback;
    conn_ctx->raw.on_data = http_upload_data_callback;
    conn_ctx->raw.on_poll = http_upload_poll_callback;
    conn_ctx->context = upload_ctx;

    http_fs_get()->rm(file_path); // Delete file if it exists
    FuriString* dir_path = furi_string_alloc();
    path_extract_dirname(file_path, dir_path);
    http_fs_get()->mkd(furi_string_get_cstr(dir_path));
    furi_string_free(dir_path);
    upload_ctx->file = http_fs_get()->op(file_path, MG_FS_WRITE); // Open file for writing

    if(upload_ctx->file == NULL) {
        MG_REPLY_INTERNAL_ERROR(conn, "Failed to open file for writing");
        conn->is_draining = 1;
    }

    mg_iobuf_del(&conn->recv, 0, msg->head.len); // Delete HTTP headers
    conn->pfn = NULL; // Silence HTTP protocol handler, we'll use MG_EV_READ

    // Also handle possible data in the buffer
    http_upload_data_callback(conn, &conn->recv);
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
        if(netstat_is_overloaded(NetstatLogOnOverload)) {
            http_api_log_access(conn, msg, 508);
            MG_REPLY_OVERLOADED(conn);
            MG_CLOSE_AFTER_HEADERS(conn, msg);
        } else {
            FuriString* path = furi_string_alloc_printf("%.*s", msg->uri.len, msg->uri.buf);
            HttpMethod method = http_method_from_str(msg);
            http_handle_headers(path, method, context->handlers, conn, msg);
            furi_string_free(path);
        }
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
    } else if(ev == MG_EV_POLL) {
        ConnectionContext* conn_ctx = (void*)conn->data;
        if((conn_ctx->raw.on_poll) && (!conn->is_websocket)) {
            conn_ctx->raw.on_poll(conn);
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

    HttpHandlersList_it_t it;
    for(HttpHandlersList_it(it, handlers); !HttpHandlersList_end_p(it);
        HttpHandlersList_next(it)) {
        const HttpHandlerInstance* inst = HttpHandlersList_cref(it);
        do {
            if(!furi_string_start_with(path, inst->handler->uri)) {
                break;
            }
            if(method == HttpMethodUnknown) {
                http_reply_405_method_not_allowed(conn, inst->handler->method);
                handled = true;
                break;
            }
            if(!(method & inst->handler->method)) {
                if(method == HttpMethodOptions) {
                    http_reply_cors_preflight(conn, inst->handler->method);
                } else {
                    http_reply_405_method_not_allowed(conn, inst->handler->method);
                }
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
            if(method == HttpMethodUnknown) {
                http_reply_405_method_not_allowed(conn, inst->handler->method);
                MG_CLOSE_AFTER_HEADERS(conn, msg);
                handled = true;
                break;
            }
            if(!(method & inst->handler->method)) {
                // OPTIONS preflight: let http_handle_request (MG_EV_HTTP_MSG) respond
                if(method != HttpMethodOptions) {
                    http_reply_405_method_not_allowed(conn, inst->handler->method);
                    MG_CLOSE_AFTER_HEADERS(conn, msg);
                    handled = true;
                }
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
