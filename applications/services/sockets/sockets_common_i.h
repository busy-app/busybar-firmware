#pragma once

#include "sockets_common.h"

#include <stddef.h>

#define SOCKET_REQUEST_SIZE_MAX  (1019UL) /* See intercom/intercom_frame.h */
#define SOCKET_RESPONSE_SIZE_MAX (SOCKET_REQUEST_SIZE_MAX)

#define SOCKET_SEND_DATA_SIZE (SOCKET_REQUEST_SIZE_MAX - 4UL)
#define SOCKET_RECV_DATA_SIZE (SOCKET_RESPONSE_SIZE_MAX - 5UL)

#pragma pack(push, 1)

typedef enum {
    SocketRequestTypeAlloc,
    SocketRequestTypeFree,
    SocketRequestTypeAccept,
    SocketRequestTypeConnect,
    SocketRequestTypeSend,
    /* Async Requests */
    SocketRequestTypeAsyncConfirm,
    /* Special value */
    SocketRequestTypeMax,
} SocketRequestType;

typedef enum {
    SocketResponseTypeAlloc = SocketRequestTypeAlloc,
    SocketResponseTypeFree = SocketRequestTypeFree,
    SocketResponseTypeAccept = SocketRequestTypeAccept,
    SocketResponseTypeConnect = SocketRequestTypeConnect,
    SocketResponseTypeSend = SocketRequestTypeSend,
    /* Async responses */
    SocketResponseTypeAsyncSend,
    SocketResponseTypeAsyncReceive,
    SocketResponseTypeAsyncAccept,
    SocketResponseTypeAsyncClose,
    /* Special value */
    SocketResponseTypeMax,
} SocketResponseType;

typedef enum {
    SocketResponseIndexSync,
    SocketResponseIndexAsync,
    SocketResponseIndexMax,
} SocketResponseIndex;

typedef struct {
    SocketInfo socket_info;
} SocketAllocRequest;

typedef struct {
    uint8_t socket_id;
} SocketFreeRequest;

typedef struct {
    uint8_t socket_id;
    SocketConnectionInfo bind_info;
} SocketAcceptRequest;

typedef struct {
    uint8_t socket_id;
    SocketConnectionInfo connection_info;
} SocketConnectRequest;

typedef struct {
    uint8_t socket_id;
    uint16_t data_size;
    uint8_t data[SOCKET_SEND_DATA_SIZE];
} SocketSendRequest;

typedef struct {
    uint8_t type;
    union {
        SocketAllocRequest alloc_request;
        SocketFreeRequest free_request;
        SocketAcceptRequest accept_request;
        SocketConnectRequest connect_request;
        SocketSendRequest send_request;
    };
} SocketRequest;

typedef struct {
    uint8_t socket_id;
} SocketAllocResponse;

typedef struct {
    uint16_t sent_size;
} SocketSendResponse;

typedef struct {
    uint16_t sent_size;
} SocketSendAsyncResponse;

typedef struct {
    uint16_t data_size;
    uint8_t data[SOCKET_RECV_DATA_SIZE];
} SocketReceiveAsyncResponse;

typedef struct {
    uint8_t client_socket_id;
    SocketConnectionInfo connection_info;
} SocketAcceptAsyncResponse;

typedef struct {
    uint16_t port;
    uint16_t sent_size;
} SocketCloseAsyncResponse;

typedef struct {
    uint8_t socket_id;
    union {
        SocketSendAsyncResponse send_async_response;
        SocketReceiveAsyncResponse receive_async_response;
        SocketAcceptAsyncResponse accept_async_response;
        SocketCloseAsyncResponse close_async_response;
    };
} SocketAsyncResponse;

typedef struct {
    uint8_t type;
    uint8_t status;
    union {
        SocketAllocResponse alloc_response;
        SocketSendResponse send_response;
        SocketAsyncResponse async_response;
    };
} SocketResponse;

_Static_assert(sizeof(SocketRequest) <= SOCKET_REQUEST_SIZE_MAX);
_Static_assert(sizeof(SocketResponse) <= SOCKET_RESPONSE_SIZE_MAX);

#pragma pack(pop)

size_t sockets_get_request_size(const SocketRequest* request);
size_t sockets_get_response_size(const SocketResponse* response);
