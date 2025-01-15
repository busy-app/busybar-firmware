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

#pragma pack(push, 1)

typedef struct {
    uint8_t ip_type;
    uint8_t protocol;
} SocketInfo;

typedef struct {
    uint16_t port;
    uint8_t ip_type;
    union {
        uint8_t v4[4];
        uint8_t v6[16];
    } address;
} SocketConnectionInfo;

#pragma pack(pop)
