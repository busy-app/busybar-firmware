#pragma once
#include <furi.h>
#include <mongoose.h>
#include <usb_network/usb_network.h>
#include <storage/storage.h>
#include <m-list.h>

#define WEB_ROOT EXT_PATH("www/")

typedef struct {
    char* uri;
    char* method;
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
            bool (*on_request)(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);
            bool (*on_headers)(struct mg_connection* conn, struct mg_http_message* msg, void* ctx);
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
            } raw;
        };
        void (*on_close)(struct mg_connection* conn);
        void (*on_wakeup)(struct mg_connection* conn, void* data, size_t len);
        void* context;
    };
    uint8_t data[MG_DATA_SIZE];
} ConnectionContext;
static_assert(sizeof(ConnectionContext) == MG_DATA_SIZE);

struct mg_fs* http_fs_get(void);

bool http_handle_request(
    HttpHandlersList_t handlers,
    struct mg_connection* conn,
    struct mg_http_message* msg);

bool http_handle_headers(
    HttpHandlersList_t handlers,
    struct mg_connection* conn,
    struct mg_http_message* msg);

void http_handler_add(HttpHandlersList_t list, const HttpHandler* handler);

void http_handler_remove(HttpHandlersList_t list, const HttpHandler* handler);

void http_handler_remove_all(HttpHandlersList_t list);

struct mg_mgr* web_srv_get_mgr(void);
