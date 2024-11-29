#include "rpc_i.h"

typedef struct {
    RpcSession* session;
} RpcSystemDummy;

void* rpc_system_dummy_alloc(RpcSession* session) {
    RpcSystemDummy* instance = malloc(sizeof(RpcSystemDummy));
    instance->session = session;

    return instance;
}

void rpc_system_dummy_free(void* context) {
    free(context);
}
