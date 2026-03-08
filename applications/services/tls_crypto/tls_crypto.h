/**
 * @brief APIs enabling TLS via the cryptographic backend on the Si917 chip.
 *
 * @see tls_crypto_common.h for further information about the types.
 */
#pragma once

#include "tls_crypto_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for TlsCrypto instance access.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_TLS_CRYPTO)`
 */
#define RECORD_TLS_CRYPTO "tls_crypto"

/** Opaque TlsCrypto type declaration. */
typedef struct TlsCrypto TlsCrypto;

/**
 * @brief Retrieve the certificate data under the designated id from the backend.
 *
 * @param[in,out] instance pointer to the TlsCrypto instance
 * @param[in] key_id numeric identifier of the certificate to be retrieved
 * @param[out] certificate pointer to the certificate container (must be allocated)
 * @returns @c TlsCryptoStatusOk on success, any other value from @ref TlsCryptoStatus on failure
 */
TlsCryptoStatus tls_crypto_get_certificate(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    TlsCryptoCertificate* certificate);

/**
 * @brief Sign the provided message with the designated key and certificate id.
 *
 * @param[in,out] instance pointer to the TlsCrypto instance
 * @param[in] key_id numeric identifier of the key and certificate to sign with
 * @param[in] message pointer to the data to be signed
 * @param[in] message_len length of the message to be signed, in bytes
 * @param[out] signature pointer to the signature container (must be allocated)
 * @returns @c TlsCryptoStatusOk on success, any other value from @ref TlsCryptoStatus on failure
 */
TlsCryptoStatus tls_crypto_sign(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    const void* message,
    size_t message_len,
    TlsCryptoSignature* signature);

#ifdef __cplusplus
}
#endif
