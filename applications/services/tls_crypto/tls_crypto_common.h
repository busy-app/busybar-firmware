#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TLS_CRYPTO_DATA_LEN_MAX (800)

#define TLS_CRYPTO_CERT_LEN_MAX (TLS_CRYPTO_DATA_LEN_MAX)

#define TLS_CRYPTO_SIGN_LEN_MAX (80)

typedef enum {
    TlsCryptoStatusOk,
    TlsCryptoStatusErrorTimeout,
    TlsCryptoStatusErrorInternal,
    TlsCryptoStatusMax,
} TlsCryptoStatus;

typedef enum {
    TlsCryptoKeyIdIntermediate,
    TlsCryptoKeyIdDevice,
    TlsCryptoKeyIdMax,
} TlsCryptoKeyId;

typedef struct {
    uint8_t bytes[TLS_CRYPTO_CERT_LEN_MAX];
    size_t length;
} TlsCryptoCertificate;

typedef struct {
    uint8_t bytes[TLS_CRYPTO_SIGN_LEN_MAX];
    size_t length;
} TlsCryptoSignature;

#ifdef __cplusplus
}
#endif
