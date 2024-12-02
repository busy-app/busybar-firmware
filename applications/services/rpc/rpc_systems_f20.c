#include "rpc_i.h"

#include <furi.h>

#include "rpc_input.h"

const RpcSystem rpc_systems[] = {
    {
        .alloc = rpc_system_input_alloc,
        .free = rpc_system_input_free,
    },
};

const uint32_t rpc_system_count = COUNT_OF(rpc_systems);
