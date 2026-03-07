/**
 * @brief TLS crypto client
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_TLS_CRYPTO "tls_crypto"

typedef struct TlsCrypto TlsCrypto;

bool tls_crypto_sign(
    TlsCrypto* instance,
    uint8_t key_slot,
    const void* data,
    size_t data_size,
    void* signature_buf,
    size_t signature_buf_size,
    size_t* signature_len);

uint8_t* tls_crypto_get_certificate(TlsCrypto* instance, uint8_t key_slot, size_t* cert_len);

#ifdef __cplusplus
}
#endif
