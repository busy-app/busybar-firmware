#pragma once

#include "tls_crypto_common.h"

#define TAG "TlsCrypto"

typedef struct {
    uint8_t bytes[TLS_CRYPTO_DATA_LEN_MAX];
    size_t length;
} TlsCryptoMessage;

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
    TlsCryptoMessage message;
} TlsCryptoRequestSignMessage;

typedef struct {
    TlsCryptoRequestType type;
    uint16_t id;
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
    uint16_t id;
    union {
        TlsCryptoResponseGetCertificate get_cert;
        TlsCryptoResponseSignMessage sign_message;
    };
} TlsCryptoResponse;

void tls_crypto_log_response_status(const TlsCryptoResponse* response);
