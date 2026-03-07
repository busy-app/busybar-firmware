/**
 * @brief TBD
 */
#pragma once

#include "tls_crypto_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_TLS_CRYPTO "tls_crypto"

typedef struct TlsCrypto TlsCrypto;

TlsCryptoStatus tls_crypto_get_certificate(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    TlsCryptoCertificate* certificate);

TlsCryptoStatus tls_crypto_sign(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    const void* message,
    size_t message_len,
    TlsCryptoSignature* signature);

#ifdef __cplusplus
}
#endif
