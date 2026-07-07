#pragma once

#include <mongoose/mongoose.h>

#define FETCH_CLIENT_CA_BUNDLE_PATH EXT_PATH("apps_assets/shared/ca/cacert.pem")

#define FETCH_CLIENT_USER_AGENT \
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36"

#define FETCH_CLIENT_THREAD_STACK_SIZE (1024 * 8) // 8 KB

typedef struct {
    void (*on_data)(struct mg_connection* conn, struct mg_iobuf* data);
    void (*on_close)(struct mg_connection* conn);
    void* context;
} FetchConnectionContext;

static_assert(sizeof(FetchConnectionContext) <= MG_DATA_SIZE);
