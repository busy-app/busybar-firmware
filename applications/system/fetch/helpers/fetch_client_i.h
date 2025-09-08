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
