#pragma once

#include <furi.h>

#define TLS_CRYPTO_DATA_SIZE_MAX 800

typedef enum {
    TlsCryptoSignRequest,
    TlsCryptoSignResponse,
    TlsCryptoCertRequest,
    TlsCryptoCertResponse,
    TlsCryptoError = 0xFFFFFFFF,
} TlsCryptoCmd;

typedef struct {
    TlsCryptoCmd type;
    size_t data_size;
    uint8_t key_slot;
} TlsCryptoMessageHeader;

typedef struct {
    TlsCryptoMessageHeader header;
    uint8_t data[TLS_CRYPTO_DATA_SIZE_MAX];
} TlsCryptoMessageGeneric;

typedef struct {
    TlsCryptoMessageHeader header;
    uint8_t data[];
} TlsCryptoDataMessage;

typedef struct {
    TlsCryptoMessageHeader header;
    // TODO: error code?
} TlsCryptoErrorMessage;
