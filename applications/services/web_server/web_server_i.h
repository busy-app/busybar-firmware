#pragma once
#include <furi.h>
#include <mongoose.h>
#include <mongoose_glue.h>
#include <network/network.h>
#include <storage/storage.h>
#include <m-list.h>

#define WEB_ROOT APP_ASSETS_PATH("www/")

#define HEADER_CORS_ORIGIN        "Access-Control-Allow-Origin: *\r\n"
#define HEADER_CORS_METHODS       "Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n"
#define HEADER_CORS_HEADERS       "Access-Control-Allow-Headers: *\r\n"
#define HEADER_CORS               HEADER_CORS_ORIGIN HEADER_CORS_HEADERS
#define HEADER_CONTENT_TYPE_JSON  "Content-Type: application/json\r\n"
#define HEADER_CONTENT_TYPE_IMAGE "Content-Type: image/bmp\r\n"

#define DEFAULT_JSON_HEADERS  HEADER_CORS HEADER_CONTENT_TYPE_JSON
#define DEFAULT_IMAGE_HEADERS HEADER_CORS HEADER_CONTENT_TYPE_IMAGE

#define RESPONSE_BODY_OK "{\"result\":\"OK\"}\n"

#define _MG_OPTIONS_RESULT(conn, code) \
    mg_http_reply(conn, code, HEADER_CORS HEADER_CORS_METHODS, "")

#define _MG_JSON_RESULT(conn, code, body, ...) \
    mg_http_reply(conn, code, DEFAULT_JSON_HEADERS, body, ##__VA_ARGS__)

#define MG_REPLY_IMAGE(conn, image, size) \
    mg_http_reply(conn, 200, DEFAULT_IMAGE_HEADERS, "%M", mg_print_base64, size, image)

#define MG_REPLY_OPTIONS(conn)                 _MG_OPTIONS_RESULT(conn, 200)
#define MG_REPLY_OK(conn)                      _MG_JSON_RESULT(conn, 200, RESPONSE_BODY_OK)
#define MG_REPLY_OK_BODY(conn, json_body, ...) _MG_JSON_RESULT(conn, 200, json_body, ##__VA_ARGS__)

// Force closing the connection - to prevent clients from reusing it
// (e.g. after a streaming upload) and racing with the server-side FIN
#define MG_REPLY_OK_CLOSE(conn) \
    mg_http_reply(conn, 200, DEFAULT_JSON_HEADERS "Connection: close\r\n", RESPONSE_BODY_OK)

#define MG_REPLY_ERROR_CLOSE(conn, code, ...)         \
    mg_http_reply(                                    \
        conn,                                         \
        code,                                         \
        DEFAULT_JSON_HEADERS "Connection: close\r\n", \
        "{\"error\":\"%s\"}\n",                       \
        M_IF_EMPTY(__VA_ARGS__)("failed", __VA_ARGS__))

#define MG_REPLY_ERROR(conn, code, ...) \
    _MG_JSON_RESULT(                    \
        conn,                           \
        code,                           \
        "{\"error\":\"%M\"}\n",         \
        M_IF_EMPTY(__VA_ARGS__)(MG_ESC("failed"), MG_ESC(__VA_ARGS__)))

#define MG_REPLY_BAD_REQUEST(conn)     MG_REPLY_ERROR(conn, 400, "Bad Request")
#define MG_REPLY_NOT_FOUND(conn)       MG_REPLY_ERROR(conn, 404, "Not Found")
#define MG_REPLY_FORBIDDEN(conn)       MG_REPLY_ERROR(conn, 403, "Forbidden")
#define MG_REPLY_INVALID_VERSION(conn) MG_REPLY_ERROR(conn, 405, "Incompatible API version")

#define MG_REPLY_TIMEOUT(conn, ...) \
    MG_REPLY_ERROR_CLOSE(conn, 408, M_IF_EMPTY(__VA_ARGS__)("Request Timeout", __VA_ARGS__))
#define MG_REPLY_CONFLICT(conn, ...) \
    MG_REPLY_ERROR(conn, 409, M_IF_EMPTY(__VA_ARGS__)("Conflict", __VA_ARGS__))

#define MG_REPLY_PAYLOAD_TOO_LARGE(conn, ...) \
    MG_REPLY_ERROR(conn, 413, M_IF_EMPTY(__VA_ARGS__)("Payload Too Large", __VA_ARGS__))

#define MG_REPLY_SERVICE_UNAVAILABLE(conn, ...) \
    MG_REPLY_ERROR(conn, 503, M_IF_EMPTY(__VA_ARGS__)("Service Unavailable", __VA_ARGS__))

#define MG_REPLY_OVERLOADED(conn) MG_REPLY_ERROR(conn, 508, "Resource Limit Reached")

#define MG_REPLY_METHOD_NOT_ALLOWED(conn, headers_cstr) \
    mg_http_reply(conn, 405, headers_cstr, "{\"error\":\"Method Not Allowed\"}\n")
#define MG_REPLY_CORS_OPTIONS(conn, headers_cstr) mg_http_reply(conn, 200, headers_cstr, "")

#define MG_CLOSE_AFTER_HEADERS(conn, msg)        \
    mg_iobuf_del(&conn->recv, 0, msg->head.len); \
    conn->pfn = NULL;                            \
    conn->is_draining = 1;

#define IS_HTTP_ENDPOINT(path) furi_string_empty(path)

#define IS_WEBSOCKET_UPGRADE(msg) mg_http_get_header(msg, "Sec-WebSocket-Key") != NULL

// HTTP method bitmask, can be combined to support multiple methods for the handler
typedef enum {
    HttpMethodUnknown = 0, // Special value, should not be used

    HttpMethodGet = (1 << 0),
    HttpMethodHead = (1 << 1),
    HttpMethodPost = (1 << 2),
    HttpMethodPut = (1 << 3),
    HttpMethodDelete = (1 << 4),
    HttpMethodConnect = (1 << 5),
    HttpMethodOptions = (1 << 6),
    HttpMethodTrace = (1 << 7),
    HttpMethodPatch = (1 << 8),

    HttpMethodWebSocket = (1 << 9), // WebSocket upgrade request

    HttpMethodAny = 0xFFFFFFFF,
} HttpMethod;

typedef struct {
    char* uri;
    HttpMethod method;
    enum {
        HttpHandlerCustom,
        HttpHandlerFile,
        HttpHandlerDir,
    } type;

    union {
        struct {
            char* path;
            char* mime_types_custom;
            char* extra_headers;
        };
        struct {
            void* (*ctx_alloc)(void);
            void (*ctx_free)(void*);
            bool (*on_request)(
                FuriString* path,
                HttpMethod method,
                struct mg_connection* conn,
                struct mg_http_message* msg,
                void* ctx);
            bool (*on_headers)(
                FuriString* path,
                HttpMethod method,
                struct mg_connection* conn,
                struct mg_http_message* msg,
                void* ctx);
        };
    };
} HttpHandler;

typedef struct {
    const HttpHandler* handler;
    void* context;
} HttpHandlerInstance;
LIST_DEF(HttpHandlersList, HttpHandlerInstance, M_POD_OPLIST);

typedef union {
    struct {
        union {
            struct {
                void (*on_open)(struct mg_connection* conn);
                void (*on_message)(struct mg_connection* conn, struct mg_ws_message* ws_msg);
            } ws;
            struct {
                void (*on_data)(struct mg_connection* conn, struct mg_iobuf* data);
                void (*on_poll)(struct mg_connection* conn);
            } raw;
        };
        void (*on_close)(struct mg_connection* conn);
        void (*on_wakeup)(struct mg_connection* conn, void* data, size_t len);
        void* context;
    };
    uint8_t data[MG_DATA_SIZE];
} ConnectionContext;
static_assert(sizeof(ConnectionContext) == MG_DATA_SIZE);

bool http_handle_request(
    FuriString* path,
    HttpMethod method,
    HttpHandlersList_t handlers,
    struct mg_connection* conn,
    struct mg_http_message* msg);

bool http_handle_headers(
    FuriString* path,
    HttpMethod method,
    HttpHandlersList_t handlers,
    struct mg_connection* conn,
    struct mg_http_message* msg);

void http_handler_add(HttpHandlersList_t list, const HttpHandler* handler);

void http_handler_remove(HttpHandlersList_t list, const HttpHandler* handler);

void http_handler_remove_all(HttpHandlersList_t list);

void http_reply_405_method_not_allowed(
    struct mg_connection* conn,
    HttpMethod allowed_methods,
    bool close);
void http_reply_cors_preflight(struct mg_connection* conn, HttpMethod allowed_methods);

void http_upload_start(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    const char* file_path,
    bool append);

struct mg_mgr* web_srv_get_mgr(void);
