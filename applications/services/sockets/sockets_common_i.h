#pragma once

#include "sockets_common.h"

typedef uint16_t SocketId;

typedef enum {
    SocketRequestTypeAlloc,
    SocketRequestTypeFree,
    SocketRequestTypeConnect,
    SocketRequestTypeSend,
    SocketRequestTypeReceive,
    SocketRequestTypeMax,
} SocketRequestType;

typedef struct {
    SocketInfo info;
} SocketAllocRequest;

typedef struct {
    SocketId socket_id;
} SocketFreeRequest;

typedef struct {
    SocketId socket_id;
    SocketConnectionInfo info;
} SocketConnectRequest;

typedef struct {
    SocketId id;
    // TODO: other fields
} SocketSendRequest;

typedef struct {
    SocketRequestType type;
    union {
        SocketAllocRequest alloc_request;
        SocketFreeRequest free_request;
        SocketConnectRequest connect_request;
        SocketSendRequest send_request;
    };
} SocketRequest;

typedef struct {
    SocketId socket_id;
} SocketAllocResponse;

typedef struct {
    SocketId socket_id;
    // TODO: other fields
} SocketSendResponse;

typedef struct {
    SocketId socket_id;
    // TODO: other fields
} SocketReceiveResponse;

typedef struct {
    SocketRequestType type;
    SocketStatus status;
    union {
        SocketAllocResponse alloc_response;
        SocketReceiveResponse receinve_response;
    };
} SocketResponse;
