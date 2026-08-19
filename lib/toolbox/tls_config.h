/**
 * @file tls_config.h
 * @brief TLS configuration data structure.
 *
 * Provides a common TLS configuration structure
 * across different modules that use it.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of possible TLS client certificate types.
 *
 * Non-default values indicate an mTLS connection or an invalid state.
 */
typedef enum {
    TlsClientCertTypeNone, /**< No client certificate shall be used (no mTLS). */
    TlsClientCertTypeDevice, /**< Device client certificate shall be used (HW crypto based) */
    TlsClientCertTypeCustom, /**< User-provided client certificate shall be used (file based) */
    TlsClientCertTypeInvalid, /**< The config shall not be used to establish a TLS connection */
} TlsClientCertType;

/**
 * @brief Structure containing file paths for the custom client certificate.
 */
typedef struct {
    const char* certificate; /**< Full file path to the client certificate file */
    const char* private_key; /**< Full file path to the matching private key file */
} TlsClientCertPaths;

/**
 * @brief Structure holding the information about a TLS client certificate.
 */
typedef struct {
    TlsClientCertType type; /**< Type of the client certificate */
    TlsClientCertPaths paths; /**< File paths (only if `type == TlsClientCertTypeCustom`) */
} TlsClientCertInfo;

/**
 * @brief TLS configuration structure.
 */
typedef struct {
    TlsClientCertInfo client_cert_info; /**< Information about the used client certificate */
    bool is_server_cert_ignored; /**< The server certificate check shall be skipped if @c true */
} TlsConfig;

/**
 * @brief Enumeration of possible statuses returned by @c tls_config_validate.
 */
typedef enum {
    TlsConfigValidationStatusOk, /**< TLS config is valid, no error detected */
    TlsConfigValidationStatusInvalidType, /**< Client certificate type is invalid */
    TlsConfigValidationStatusClientCertNotSpecified, /**< Client certificate is required, but not specified */
    TlsConfigValidationStatusPrivateKeyNotSpecified, /**< Private key is required, but not specified */
} TlsConfigValidationStatus;

/**
 * @brief Initialise TLS config structure with default values.
 *
 * @param[in,out] tls_config pointer to the config to be initialised
 */
void tls_config_init(TlsConfig* tls_config);

/**
 * @brief Check whether a TLS config structure is valid.
 *
 * @param[in] tls_config pointer to the config to be validated
 * @returns @c TlsConfigValidationStatusOk on success, any other @ref TlsConfigValidationStatus value on error
 */
TlsConfigValidationStatus tls_config_validate(const TlsConfig* tls_config);

#ifdef __cplusplus
}
#endif
