/**
 * @file ca_storage.h
 * @brief Certificate Authority (CA) storage API.
 */
#pragma once

#include <mbedtls/x509_crt.h>

/**
 * @brief The string key for CaStorage instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_CA_STORAGE);`
 */
#define RECORD_CA_STORAGE "ca_storage"

typedef struct CaStorage CaStorage;

const mbedtls_x509_crt* ca_storage_get_cert_chain(const CaStorage* instance);
