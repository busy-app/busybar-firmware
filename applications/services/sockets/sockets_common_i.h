#pragma once

#include "sockets.h"
#include "sockets_common.h"

#include <stddef.h>

#define SOCKET_REQUEST_SIZE_MAX  (1019UL) /* See intercom/intercom_frame.h */
#define SOCKET_RESPONSE_SIZE_MAX (SOCKET_REQUEST_SIZE_MAX)

#define SOCKET_SEND_DATA_SIZE (SOCKET_REQUEST_SIZE_MAX - 21UL)
#define SOCKET_RECV_DATA_SIZE (SOCKET_RESPONSE_SIZE_MAX - 21UL)

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
    SocketResponseTypeAsyncAccept,
    SocketResponseTypeAsyncSelect,
    /* Special value */
    SocketResponseTypeMax,
} SocketResponseType;

typedef struct {
    int32_t domain;
    int32_t type;
    int32_t protocol;
} SocketAllocRequest;

typedef struct {
    struct sockaddr name;
    socklen_t namelen;
} SocketBindRequest;

typedef struct {
    int32_t backlog;
} SocketListenRequest;

typedef struct {
    uint32_t addrlen;
} SocketAcceptRequest;

typedef struct {
    struct sockaddr name;
    uint32_t namelen;
} SocketConnectRequest;

typedef struct {
    struct sockaddr to;
    uint8_t tolen;
    uint16_t size;
    uint8_t data[SOCKET_SEND_DATA_SIZE];
} SocketSendRequest;

typedef struct {
    uint16_t len;
    uint8_t include_from;
} SocketReceiveRequest;

typedef struct {
    int32_t level;
    int32_t optname;
} SocketGetSockOptRequest;

typedef struct {
    int32_t level;
    int32_t optname;
    uint32_t optlen;
    uint32_t optval;
} SocketSetSockOptRequest;

typedef struct {
    uint8_t type;
    uint8_t socket_id;
    union {
        SocketAllocRequest alloc_request;
        SocketBindRequest bind_request;
        SocketListenRequest listen_request;
        SocketAcceptRequest accept_request;
        SocketConnectRequest connect_request;
        SocketSendRequest send_request;
        SocketReceiveRequest receive_request;
        SocketGetSockOptRequest getsockopt_request;
        SocketSetSockOptRequest setsockopt_request;
    };
} SocketRequest;

typedef struct {
    struct sockaddr from;
    uint8_t fromlen;
    uint8_t data[SOCKET_RECV_DATA_SIZE];
} SocketReceiveResponse;

typedef struct {
    struct sockaddr addr;
    uint32_t addrlen;
} SocketAcceptResponse;

typedef struct {
    struct sockaddr name;
    uint32_t namelen;
} SocketGetPeerNameResponse;

typedef struct {
    struct sockaddr name;
    uint32_t namelen;
} SocketGetSockNameResponse;

typedef struct {
    uint32_t optlen;
    uint32_t optval;
} SocketGetSockOptResponse;

typedef struct {
    struct sockaddr addr;
    uint32_t addrlen;
} SocketAcceptAsyncResponse;

typedef struct {
    uint8_t dummy;
} SocketSelectAsyncResponse;

typedef struct {
    uint8_t socket_id;
    union {
        SocketAcceptAsyncResponse accept_async_response;
        SocketSelectAsyncResponse select_async_response;
    };
} SocketAsyncResponse;

typedef struct {
    uint8_t type;
    int16_t status;
    int8_t _errno;
    union {
        SocketReceiveResponse receive_response;
        SocketAcceptResponse accept_response;
        SocketGetPeerNameResponse getpeername_response;
        SocketGetSockNameResponse getsockname_response;
        SocketGetSockOptResponse getsockopt_response;
        SocketAsyncResponse async_response;
    };
} SocketResponse;

_Static_assert(sizeof(SocketRequest) <= SOCKET_REQUEST_SIZE_MAX);
_Static_assert(sizeof(SocketResponse) <= SOCKET_RESPONSE_SIZE_MAX);

#pragma pack(pop)

size_t sockets_get_request_size(const SocketRequest* request);
size_t sockets_get_response_size(const SocketResponse* response);

const char* sockets_get_request_name(SocketRequestType request_type);
