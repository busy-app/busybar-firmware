#include "sockets_common_i.h"

#include <furi.h>

#define REQUEST_SIZE(T)  (offsetof(SocketRequest, alloc_request) + sizeof(T))
#define RESPONSE_SIZE(T) (offsetof(SocketResponse, alloc_response) + sizeof(T))
#define ASYNC_RESPONSE_SIZE(T) \
    (offsetof(SocketResponse, async_response.receive_async_response) + sizeof(T))

typedef struct {
} SocketEmpty;

static const size_t sockets_request_size[SocketRequestTypeMax] = {
    [SocketRequestTypeAlloc] = REQUEST_SIZE(SocketAllocRequest),
    [SocketRequestTypeFree] = REQUEST_SIZE(SocketFreeRequest),
    [SocketRequestTypeAccept] = REQUEST_SIZE(SocketAcceptRequest),
    [SocketRequestTypeConnect] = REQUEST_SIZE(SocketConnectRequest),
    [SocketRequestTypeSend] = 0, // Special case, size computed dynamically
    [SocketRequestTypeAsyncConfirm] = REQUEST_SIZE(SocketEmpty),
};

static const size_t sockets_response_size[SocketResponseTypeMax] = {
    [SocketResponseTypeAlloc] = RESPONSE_SIZE(SocketAllocResponse),
    [SocketResponseTypeFree] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeAccept] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeConnect] = RESPONSE_SIZE(SocketEmpty),
    [SocketResponseTypeSend] = RESPONSE_SIZE(SocketSendResponse),
    [SocketResponseTypeAsyncReceive] = 0, // Special case, size computed dynamically
    [SocketResponseTypeAsyncAccept] = ASYNC_RESPONSE_SIZE(SocketAcceptAsyncResponse),
    [SocketResponseTypeAsyncClose] = ASYNC_RESPONSE_SIZE(SocketCloseAsyncResponse),
};

size_t sockets_get_request_size(const SocketRequest* request) {
    const uint8_t request_type = request->type;
    furi_assert(request_type < SocketRequestTypeMax);

    size_t request_size;

    if(request_type != SocketRequestTypeSend) {
        request_size = sockets_request_size[request_type];
    } else {
        const uint16_t data_size = request->send_request.data_size;
        furi_assert(data_size <= SOCKET_SEND_DATA_SIZE);
        request_size = offsetof(SocketRequest, send_request.data) + data_size;
    }

    return request_size;
}

size_t sockets_get_response_size(const SocketResponse* response) {
    const uint8_t response_type = response->type;
    furi_assert(response_type < SocketResponseTypeMax);

    size_t response_size;

    if(response_type != SocketResponseTypeAsyncReceive) {
        response_size = sockets_response_size[response_type];
    } else {
        const uint16_t data_size = response->async_response.receive_async_response.data_size;
        furi_assert(data_size <= SOCKET_RECV_DATA_SIZE);
        response_size =
            offsetof(SocketResponse, async_response.receive_async_response.data) + data_size;
    }

    return response_size;
}
