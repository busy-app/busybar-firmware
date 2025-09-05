#pragma once
#include <furi.h>

#include <wifi/wifi.h>
#include <network/network.h>
#include <mongoose.h>
#include <mongoose_glue.h>

#define FETCH_CLIENT_CA_BUNDLE_PATH "/ext/apps_assets/ca/cacert.pem"
#define FETCH_CLIENT_USER_AGENT \
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36"
#define FETCH_CLIENT_THREAD_STACK_SIZE (1024 * 10) // 10 KB
#define MAX_UPLOAD_FILE_SIZE           (40 * 1024 * 1024) // User-set: 40MB

#define HEADER_CORS_ORIGIN        "Access-Control-Allow-Origin: *\r\n"
#define HEADER_CORS_METHODS       "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
#define HEADER_CORS_HEADERS       "Access-Control-Allow-Headers: *\r\n"
#define HEADER_CORS               HEADER_CORS_ORIGIN HEADER_CORS_METHODS HEADER_CORS_HEADERS
#define HEADER_CONTENT_TYPE_JSON  "Content-Type: application/json\r\n"
#define HEADER_CONTENT_TYPE_IMAGE "Content-Type: image/bmp\r\n"

#define DEFAULT_JSON_HEADERS  HEADER_CORS HEADER_CONTENT_TYPE_JSON
#define DEFAULT_IMAGE_HEADERS HEADER_CORS HEADER_CONTENT_TYPE_IMAGE

#define RESPONSE_BODY_OK "{\"result\":\"OK\"}\n"

#define _MG_OPTIONS_RESULT(conn, code) mg_http_reply(conn, code, HEADER_CORS, "")

#define _MG_JSON_RESULT(conn, code, body, ...) \
    mg_http_reply(conn, code, DEFAULT_JSON_HEADERS, body, ##__VA_ARGS__)

#define _MG_JSON_ERROR(conn, code, body, ...) \
    mg_http_reply(conn, code, DEFAULT_JSON_HEADERS, body, ##__VA_ARGS__)

#define MG_REPLY_IMAGE(conn, image, size) \
    mg_http_reply(conn, 200, DEFAULT_IMAGE_HEADERS, "%M\r\n", mg_print_base64, size, image)

#define MG_REPLY_OPTIONS(conn)                 _MG_OPTIONS_RESULT(conn, 200)
#define MG_REPLY_OK(conn)                      _MG_JSON_RESULT(conn, 200, RESPONSE_BODY_OK)
#define MG_REPLY_OK_BODY(conn, json_body, ...) _MG_JSON_RESULT(conn, 200, json_body, ##__VA_ARGS__)

#define MG_REPLY_ERROR(conn, code, ...) \
    _MG_JSON_ERROR(                     \
        conn, code, "{\"error\":\"%s\"}\n", M_IF_EMPTY(__VA_ARGS__)("failed", __VA_ARGS__))

#define MG_REPLY_BAD_REQUEST(conn)     MG_REPLY_ERROR(conn, 400, "Bad Request")
#define MG_REPLY_NOT_FOUND(conn)       MG_REPLY_ERROR(conn, 404, "Not Found")
#define MG_REPLY_FORBIDDEN(conn)       MG_REPLY_ERROR(conn, 403, "Forbidden")
#define MG_REPLY_INVALID_VERSION(conn) MG_REPLY_ERROR(conn, 405, "Incompatible API version")

#define MG_REPLY_METHOD_NOT_ALLOWED(conn, ...) \
    MG_REPLY_ERROR(conn, 405, M_IF_EMPTY(__VA_ARGS__)("Method Not Allowed", (__VA_ARGS__)))
#define MG_REPLY_PAYLOAD_TOO_LARGE(conn, ...) \
    MG_REPLY_ERROR(conn, 413, M_IF_EMPTY(__VA_ARGS__)("Payload Too Large", (__VA_ARGS__)))

#define _MG_REPLY_INTERNAL_ERROR(conn, msg) MG_REPLY_ERROR(conn, 500, msg)
#define MG_REPLY_INTERNAL_ERROR(conn, ...) \
    _MG_REPLY_INTERNAL_ERROR(conn, M_IF_EMPTY(__VA_ARGS__)("failed", (__VA_ARGS__)))

#define IS_HTTP_ENDPOINT(path) furi_string_empty(path)

typedef union {
    struct {
        union {
            struct {
                void (*on_open)(struct mg_connection* conn);
                void (*on_message)(struct mg_connection* conn, struct mg_ws_message* ws_msg);
            } ws;
            struct {
                void (*on_data)(struct mg_connection* conn, struct mg_iobuf* data);
            } raw;
        };
        void (*on_close)(struct mg_connection* conn);
        void (*on_wakeup)(struct mg_connection* conn, void* data, size_t len);
        void* context;
    };
    uint8_t data[MG_DATA_SIZE];
} ConnectionContext;
static_assert(sizeof(ConnectionContext) == MG_DATA_SIZE);
