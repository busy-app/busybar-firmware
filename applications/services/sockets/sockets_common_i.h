#pragma once

#include "sockets_common.h"

#include <stddef.h>

#define SOCKET_REQUEST_SIZE_MAX  (1019UL) /* See intercom/intercom_frame.h */
#define SOCKET_RESPONSE_SIZE_MAX (SOCKET_REQUEST_SIZE_MAX)

#define SOCKET_SEND_DATA_SIZE (SOCKET_REQUEST_SIZE_MAX - 5UL)
#define SOCKET_RECV_DATA_SIZE (SOCKET_RESPONSE_SIZE_MAX - 5UL)

#pragma pack(push, 1)

typedef enum {
    SocketRequestTypeAlloc,
    SocketRequestTypeFree,
    SocketRequestTypeBind,
    SocketRequestTypeListen,
    SocketRequestTypeAccept,
    SocketRequestTypeConnect,
    SocketRequestTypeSend,
    SocketRequestTypeReceive,
    SocketRequestTypeGetSockName,
    SocketRequestTypeGetPeerName,
    SocketRequestTypeSetSockOpt,
    SocketRequestTypeGetSockOpt,
    /* Special value */
    SocketRequestTypeMax,
} SocketRequestType;

typedef enum {
    SocketResponseTypeAlloc = SocketRequestTypeAlloc,
    SocketResponseTypeFree = SocketRequestTypeFree,
    SocketResponseTypeBind = SocketRequestTypeBind,
    SocketResponseTypeListen = SocketRequestTypeListen,
    SocketResponseTypeAccept = SocketRequestTypeAccept,
    SocketResponseTypeConnect = SocketRequestTypeConnect,
    SocketResponseTypeSend = SocketRequestTypeSend,
    SocketResponseTypeReceive = SocketRequestTypeReceive,
    SocketResponseTypeGetSockName = SocketRequestTypeGetSockName,
    SocketResponseTypeGetPeerName = SocketRequestTypeGetPeerName,
    SocketResponseTypeSetSockOpt = SocketRequestTypeSetSockOpt,
    SocketResponseTypeGetSockOpt = SocketRequestTypeGetSockOpt,
    /* Async responses */
    SocketResponseTypeAsyncReceive,
    SocketResponseTypeAsyncAccept,
    SocketResponseTypeAsyncClose,
    /* Special value */
    SocketResponseTypeMax,
} SocketResponseType;

typedef enum {
    SocketChannelSync,
    SocketChannelAsync,
    SocketChannelMax,
} SocketChannel;

typedef struct {
    uint8_t socket_id;
} SocketFreeRequest;

typedef struct {
    uint8_t socket_id;
    // SocketConnectionInfo bind_info;
} SocketBindRequest;

typedef struct {
    uint8_t socket_id;
    uint8_t backlog;
} SocketListenRequest;

typedef struct {
    uint8_t socket_id;
} SocketAcceptRequest;

typedef struct {
    uint8_t socket_id;
    // SocketConnectionInfo connection_info;
} SocketConnectRequest;

typedef struct {
    uint8_t socket_id;
    uint16_t data_size;
    uint8_t data[SOCKET_SEND_DATA_SIZE];
} SocketSendRequest;

typedef struct {
    uint8_t socket_id;
    uint16_t data_size;
} SocketReceiveRequest;

typedef struct {
    uint8_t type;
    uint8_t socket_id;
    union {
        SocketFreeRequest free_request;
        SocketBindRequest bind_request;
        SocketListenRequest listen_request;
        SocketAcceptRequest accept_request;
        SocketConnectRequest connect_request;
        SocketSendRequest send_request;
        SocketReceiveRequest receive_request;
    };
} SocketRequest;

typedef struct {
    uint8_t socket_id;
} SocketAllocResponse;

typedef struct {
    uint16_t sent_size;
} SocketSendResponse;

typedef struct {
    uint16_t data_size;
    uint8_t data[SOCKET_RECV_DATA_SIZE];
} SocketReceiveResponse;

typedef struct {
    uint8_t client_socket_id;
} SocketAcceptAsyncResponse;

typedef struct {
    uint8_t socket_id;
    union {
        SocketAcceptAsyncResponse accept_async_response;
    };
} SocketAsyncResponse;

typedef struct {
    uint8_t type;
    uint8_t status;
    union {
        SocketAllocResponse alloc_response;
        SocketSendResponse send_response;
        SocketReceiveResponse receive_response;
        SocketAsyncResponse async_response;
    };
} SocketResponse;

_Static_assert(sizeof(SocketRequest) <= SOCKET_REQUEST_SIZE_MAX);
_Static_assert(sizeof(SocketResponse) <= SOCKET_RESPONSE_SIZE_MAX);

#pragma pack(pop)

size_t sockets_get_request_size(const SocketRequest* request);
size_t sockets_get_response_size(const SocketResponse* response);
