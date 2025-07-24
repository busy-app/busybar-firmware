#include "sockets_common_i.h"

#include <furi.h>

#define REQUEST_SIZE(T)  (offsetof(SocketRequest, alloc_request) + sizeof(T))
#define RESPONSE_SIZE(T) (offsetof(SocketResponse, receive_response) + sizeof(T))
#define ASYNC_RESPONSE_SIZE(T) \
    (offsetof(SocketResponse, async_response.accept_async_response) + sizeof(T))

typedef struct {
} SocketEmpty;

static const size_t sockets_request_size[SocketRequestTypeMax] = {
    [SocketRequestTypeAlloc] = REQUEST_SIZE(SocketAllocRequest),
    [SocketRequestTypeFree] = REQUEST_SIZE(SocketEmpty),
    [SocketRequestTypeBind] = REQUEST_SIZE(SocketBindRequest),
    [SocketRequestTypeListen] = REQUEST_SIZE(SocketListenRequest),
    [SocketRequestTypeAccept] = REQUEST_SIZE(SocketAcceptRequest),
    [SocketRequestTypeConnect] = REQUEST_SIZE(SocketConnectRequest),
    [SocketRequestTypeSend] = 0, // Special case, size computed dynamically
    [SocketRequestTypeReceive] = REQUEST_SIZE(SocketReceiveRequest),
    [SocketRequestTypeGetSockName] = REQUEST_SIZE(SocketEmpty),
    [SocketRequestTypeGetPeerName] = REQUEST_SIZE(SocketEmpty),
    [SocketRequestTypeSetSockOpt] = REQUEST_SIZE(SocketSetSockOptRequest),
    [SocketRequestTypeGetSockOpt] = REQUEST_SIZE(SocketGetSockOptRequest),
    [SocketRequestTypeSelect] = REQUEST_SIZE(SocketSelectRequest),
};

static const size_t sockets_response_size[SocketResponseTypeMax] = {
    [SocketResponseTypeAlloc] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeFree] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeBind] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeListen] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeAccept] = RESPONSE_SIZE(SocketAcceptResponse),
    [SocketResponseTypeConnect] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeSend] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeReceive] = 0, // Special case, size computed dynamically
    [SocketResponseTypeGetSockName] = RESPONSE_SIZE(SocketGetSockNameResponse),
    [SocketResponseTypeGetPeerName] = RESPONSE_SIZE(SocketGetPeerNameResponse),
    [SocketResponseTypeSetSockOpt] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeGetSockOpt] = RESPONSE_SIZE(SocketGetSockOptResponse),
    [SocketResponseTypeSelect] = RESPONSE_SIZE(SocketSelectResponse),
    [SocketResponseTypeAsyncAccept] = ASYNC_RESPONSE_SIZE(SocketAcceptAsyncResponse),
    [SocketResponseTypeAsyncSelect] = ASYNC_RESPONSE_SIZE(SocketSelectAsyncResponse),
};

static const char* sockets_request_names[SocketRequestTypeMax] = {
    [SocketRequestTypeAlloc] = "socket",
    [SocketRequestTypeFree] = "close",
    [SocketRequestTypeBind] = "bind",
    [SocketRequestTypeListen] = "listen",
    [SocketRequestTypeAccept] = "accept",
    [SocketRequestTypeConnect] = "connect",
    [SocketRequestTypeSend] = "send",
    [SocketRequestTypeReceive] = "recv",
    [SocketRequestTypeGetSockName] = "getsockname",
    [SocketRequestTypeGetPeerName] = "getpeername",
    [SocketRequestTypeSetSockOpt] = "setsockopt",
    [SocketRequestTypeGetSockOpt] = "getsockopt",
    [SocketRequestTypeSelect] = "select",
};

size_t sockets_get_request_size(const SocketRequest* request) {
    const uint8_t request_type = request->type;
    furi_assert(request_type < SocketRequestTypeMax);

    size_t request_size;

    if(request_type != SocketRequestTypeSend) {
        request_size = sockets_request_size[request_type];

    } else {
        const uint16_t data_size = request->send_request.size;
        furi_assert(data_size <= SOCKET_SEND_DATA_SIZE);

        request_size = offsetof(SocketRequest, send_request.data) + data_size;
    }

    return request_size;
}

size_t sockets_get_response_size(const SocketResponse* response) {
    const uint8_t response_type = response->type;
    furi_assert(response_type < SocketResponseTypeMax);

    size_t response_size;

    if(response_type != SocketResponseTypeReceive) {
        response_size = sockets_response_size[response_type];

    } else {
        const uint16_t data_size = response->status < 0 ? 0 : response->status;
        furi_assert(data_size <= SOCKET_RECV_DATA_SIZE);

        response_size = offsetof(SocketResponse, receive_response.data) + data_size;
    }

    return response_size;
}

const char* sockets_get_request_name(SocketRequestType request_type) {
    furi_assert(request_type < SocketRequestTypeMax);
    return sockets_request_names[request_type];
}
