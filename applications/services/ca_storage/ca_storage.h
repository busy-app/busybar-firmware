/**
 * @file ca_storage.h
 * @brief Certificate Authority (CA) storage API.
 *
 * On startup, the CA certificate bundle is loaded and converted to the
 * internal MbedTLS representation. After that, it can be reused for all
 * TLS connections, thus reducing the memory and CPU overhead.
 */
#pragma once

#include <mbedtls/x509_crt.h>

/**
 * @brief The string key for CaStorage instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_CA_STORAGE);`
 */
#define RECORD_CA_STORAGE "ca_storage"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CaStorage CaStorage;

/**
 * @brief Get the parsed CA certificate chain in MbedTLS format.
 *
 * The return value is guaranteed to always be valid and non-NULL.
 *
 * @param[in] instance pointer to the CA Storage instance
 * @returns pointer to the head of the CA certificate chain
 */
const mbedtls_x509_crt* ca_storage_get_cert_chain(const CaStorage* instance);

#ifdef __cplusplus
}
#endif
