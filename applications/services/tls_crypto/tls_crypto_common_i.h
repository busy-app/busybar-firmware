#pragma once

#include "tls_crypto_common.h"

typedef enum {
    TlsCryptoRequestTypeGetCertificate,
    TlsCryptoRequestTypeSign,
    TlsCryptoRequestTypeMax,
} TlsCryptoRequestType;

typedef struct {
    TlsCryptoKeyId key_id;
} TlsCryptoRequestGetCertificate;

typedef struct {
    TlsCryptoKeyId key_id;
    uint8_t data[TLS_CRYPTO_DATA_LEN_MAX];
    size_t length;
} TlsCryptoRequestSign;

typedef struct {
    TlsCryptoRequestType type;
    union {
        TlsCryptoRequestGetCertificate get_cert;
        TlsCryptoRequestSign sign;
    };
} TlsCryptoRequest;

typedef struct {
    TlsCryptoCertificate certificate;
} TlsCryptoResponseGetCertificate;

typedef struct {
    TlsCryptoSignature signature;
} TlsCryptoResponseSign;

typedef struct {
    TlsCryptoRequestType type;
    TlsCryptoStatus status;
    union {
        TlsCryptoResponseGetCertificate get_cert;
        TlsCryptoResponseSign sign;
    };
} TlsCryptoResponse;
