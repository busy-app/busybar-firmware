#include "rpc_handlers.h"

#include <core/core_defines.h>

#include "rpc_dummy.h"

const RpcSystem rpc_systems[] = {
    {
        .alloc = rpc_system_dummy_alloc,
        .free = rpc_system_dummy_free,
    }
};

const uint32_t rpc_system_count = COUNT_OF(rpc_systems);
