#pragma once

#include <stdint.h>

typedef struct Sockets Sockets;

typedef enum {
    SocketIpTypeV4,
    SocketIpTypeV6,
} SocketIpType;

typedef enum {
    SocketProtocolTcp,
    SocketProtocolUdp,
    SocketProtocolMax,
} SocketProtocol;

typedef enum {
    SocketStatusOk,
    SocketStatusError,
    // TODO: Add more errors
} SocketStatus;

typedef struct {
    SocketIpType ip_type;
    SocketProtocol protocol;
} SocketInfo;

typedef struct {
    uint16_t port;
    SocketIpType ip_type;
    union {
        uint8_t v4[4];
        uint8_t v6[16];
    } address;
} SocketConnectionInfo;
