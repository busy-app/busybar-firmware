#pragma once

#include "rpc.h"

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include <main.pb.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*RpcSystemAlloc)(RpcSession* session);
typedef void (*RpcSystemFree)(void* context);
typedef void (*PBMessageHandler)(const PB_Main* msg_request, void* context);

struct RpcSystem {
    RpcSystemAlloc alloc;
    RpcSystemFree free;
};

typedef struct {
    bool (*decode_submessage)(pb_istream_t* stream, const pb_field_t* field, void** arg);
    PBMessageHandler message_handler;
    void* context;
} RpcHandler;

void rpc_send(RpcSession* session, PB_Main* main_message);

void rpc_send_and_release(RpcSession* session, PB_Main* main_message);

void rpc_send_and_release_empty(RpcSession* session, uint32_t command_id, PB_Status status);

void rpc_add_handler(RpcSession* session, pb_size_t message_tag, RpcHandler* handler);

#ifdef __cplusplus
}
#endif
