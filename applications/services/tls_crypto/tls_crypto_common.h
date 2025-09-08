#pragma once

#include <furi.h>

#define TLS_CRYPTO_DATA_SIZE_MAX 128

typedef enum {
    TlsCryptoSignRequest,
    TlsCryptoSignResponse,
    TlsCryptoError = 0xFFFFFFFF, /**< Special value for error handling */
} TlsCryptoCmd;

typedef struct FURI_PACKED {
    TlsCryptoCmd cmd; /**< Command type */
    uint8_t key_slot;
    size_t data_size;
    uint8_t data[TLS_CRYPTO_DATA_SIZE_MAX];
} TlsCryptoSignMessage;

typedef struct FURI_PACKED {
    TlsCryptoCmd cmd; /**< Command type */
} TlsCryptoErrorMessage;
