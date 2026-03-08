#pragma once

#include "tls_crypto_common.h"

#define TAG "TlsCrypto"

typedef enum {
    TlsCryptoRequestTypeGetCertificate,
    TlsCryptoRequestTypeSignMessage,
    TlsCryptoRequestTypeMax,
} TlsCryptoRequestType;

typedef struct {
    TlsCryptoKeyId key_id;
} TlsCryptoRequestGetCertificate;

typedef struct {
    TlsCryptoKeyId key_id;
    uint8_t message[TLS_CRYPTO_DATA_LEN_MAX];
    size_t message_length;
} TlsCryptoRequestSignMessage;

typedef struct {
    TlsCryptoRequestType type;
    union {
        TlsCryptoRequestGetCertificate get_cert;
        TlsCryptoRequestSignMessage sign_message;
    };
} TlsCryptoRequest;

typedef struct {
    TlsCryptoCertificate certificate;
} TlsCryptoResponseGetCertificate;

typedef struct {
    TlsCryptoSignature signature;
} TlsCryptoResponseSignMessage;

typedef struct {
    TlsCryptoRequestType type;
    TlsCryptoStatus status;
    union {
        TlsCryptoResponseGetCertificate get_cert;
        TlsCryptoResponseSignMessage sign_message;
    };
} TlsCryptoResponse;

void tls_crypto_log_response_status(const TlsCryptoResponse* response);
