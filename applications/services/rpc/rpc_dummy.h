#pragma once

#include "rpc.h"

void* rpc_system_dummy_alloc(RpcSession* session);

void rpc_system_dummy_free(void* context);
