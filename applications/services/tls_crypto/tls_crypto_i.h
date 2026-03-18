#pragma once

#include "tls_crypto.h"

#include <intercom/intercom.h>

#include <api_lock.h>

#include "tls_crypto_common_i.h"

typedef enum {
    TlsCryptoApiMessageTypeGetCertificate,
    TlsCryptoApiMessageTypeSignMessage,
    TlsCryptoApiMessageTypeMax,
} TlsCryptoApiMessageType;

typedef struct {
    TlsCryptoKeyId key_id;
    TlsCryptoCertificate* certificate;
} TlsCryptoApiMessageGetCertificate;

typedef struct {
    TlsCryptoKeyId key_id;
    const void* data;
    size_t data_len;
    TlsCryptoSignature* signature;
} TlsCryptoApiMessageSignMessage;

typedef struct {
    TlsCryptoApiMessageType type;
    TlsCryptoStatus* status;
    FuriApiLock lock;
    union {
        TlsCryptoApiMessageGetCertificate get_cert;
        TlsCryptoApiMessageSignMessage sign_message;
    };
} TlsCryptoApiMessage;

struct TlsCrypto {
    FuriMessageQueue* api_queue;
    FuriMessageQueue* response_queue;
    IntercomChannel* intercom_ch;
    uint16_t current_request_id;
};
