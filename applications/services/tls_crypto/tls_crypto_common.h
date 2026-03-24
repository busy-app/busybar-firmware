/**
 * @file tls_crypto_common.h
 *
 * @brief Common types and definitions for the TlsCrypto system.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum length of the message to be signed, in bytes. */
#define TLS_CRYPTO_DATA_LEN_MAX (800)

/** Maximum length of a cerificate, in bytes. */
#define TLS_CRYPTO_CERT_LEN_MAX (TLS_CRYPTO_DATA_LEN_MAX)

/** Maximum length of a signature, in bytes. */
#define TLS_CRYPTO_SIGN_LEN_MAX (80)

/**
 * @brief Enumeration of possible statuses returned by the TlsCrypto API functions.
 */
typedef enum {
    TlsCryptoStatusOk, /**< Operation was successful, no error occurred. */
    TlsCryptoStatusErrorTimeout, /**< Operation could not be completed within a reasonable time. */
    TlsCryptoStatusErrorInternal, /**< Operation failed due to a backend error. */
    TlsCryptoStatusMax, /** Special value for internal use. */
} TlsCryptoStatus;

/**
 * @brief Enumeration of available key identifiers for use with the TlsCrypto API functions.
 */
typedef enum {
    TlsCryptoKeyIdIntermediate, /**< Intermediate certificate. */
    TlsCryptoKeyIdDevice, /**< Device certificate + key pair. */
    TlsCryptoKeyIdMax, /**< Special value for internal use. */
} TlsCryptoKeyId;

/**
 * @brief Certificate container type for use with the TlsCrypto API functions.
 */
typedef struct {
    uint8_t bytes[TLS_CRYPTO_CERT_LEN_MAX]; /**< Certificate data */
    size_t length; /**< Certificate length, in bytes */
} TlsCryptoCertificate;

/**
 * @brief Signature container type for use with the TlsCrypto API functions.
 */
typedef struct {
    uint8_t bytes[TLS_CRYPTO_SIGN_LEN_MAX]; /**< Signature data */
    size_t length; /**< Signature length, in bytes */
} TlsCryptoSignature;

#ifdef __cplusplus
}
#endif
