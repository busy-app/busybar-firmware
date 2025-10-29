/**
 * @brief TLS crypto client
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TlsCryptoClient TlsCryptoClient;

#define RECORD_TLS_CRYPTO_CLIENT "tls_crypto_client"

bool tls_crypto_client_sign(
    TlsCryptoClient* client,
    const uint8_t* hash,
    size_t hash_len,
    uint8_t* sign_buf,
    size_t sign_buf_size,
    size_t* sign_len);

#ifdef __cplusplus
}
#endif

