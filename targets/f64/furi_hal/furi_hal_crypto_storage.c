
#include <furi_hal_crypto_storage.h>
#include <furi_hal_crypto.h>
#include <sl_si91x_driver.h>
#include <toolbox_f64/crc32_calc.h>

#include <sl_si91x_trng.h>
#include <psa/crypto.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>

#define TAG "FuriHalCryptoStorage"

#define FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY (0x464c4950UL) // "FLIP"
#define FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MAX    (996UL) // Maximum data size for keys
#define FURI_HAL_CRYPTO_KEY_ADDRESS_INIT         (0xFFFFFFFFUL)
#define FURI_HAL_CRYPTO_CSR_BUFFER_SIZE_MAX      (2048UL)

FuriHalCryptoKey* furi_hal_crypto_storage_alloc(FuriHalCryptoPartition partition) {
    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    key->header.magic_number = FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY;
    key->header.reserved = 0xFFFF;
    key->header.reserved1 = 0xFFFFFFFF;
    key->partition = partition;
    key->address = FURI_HAL_CRYPTO_KEY_ADDRESS_INIT;
    key->length = FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MAX;
    key->data = malloc(key->length);
    return key;
}

void furi_hal_crypto_storage_free(FuriHalCryptoKey* key) {
    furi_check(key);
    if(key->data) {
        memset(key->data, 0, key->length);
        memset(&key->header, 0, sizeof(FuriHalCryptoKeyHeader));
        free(key->data);
        key->data = NULL;
    }
    free(key);
    key = NULL;
}

static FuriHalCryptoStatus furi_hal_crypto_storage_check_key_slot_is_free(
    FuriHalCryptoKey* key,
    uint32_t address_start,
    uint32_t address_end) {
    furi_assert(key);

    sl_status_t status = SL_STATUS_FAIL;
    FuriHalCryptoStatus ret = FuriHalCryptoStatusFail;
    // Calculate the size for writing the key
    if((address_start + key->header.size + sizeof(FuriHalCryptoKeyHeader)) > address_end) {
        FURI_LOG_E(TAG, "Key exceeds storage limits");
        return FuriHalCryptoStatusStorageFull;
    }

    uint16_t key_slot_size = key->header.size + sizeof(FuriHalCryptoKeyHeader);
    uint8_t* buf = malloc(key_slot_size);

    do {
        status = sl_si91x_command_to_read_common_flash(address_start, key_slot_size, buf);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
            break;
        }

        // Check if the slot is free (all bytes are 0xFF)
        uint32_t i = 0;
        while(i < key_slot_size && buf[i] == 0xFF) {
            i++;
        }
        if(i == key_slot_size) {
            // Slot is free
            ret = FuriHalCryptoStatusOk;
            break;
        }
    } while(false);
    memset(buf, 0, key_slot_size);
    free(buf);
    return ret;
}

static FuriHalCryptoStatus
    furi_hal_crypto_storage_search_clean_place(FuriHalCryptoKey* key, uint32_t* address) {
    furi_assert(key);

    FuriHalCryptoStatus ret = FuriHalCryptoStatusFail;
    uint32_t address_start = 0;
    uint32_t address_end = 0;
    switch(key->partition) {
    case FuriHalCryptoPartitionMain:
        address_start = FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS;
        address_end = FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_END_ADDRESS;
        break;
    case FuriHalCryptoPartitionUser:
        address_start = FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS;
        address_end = FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS;
        break;
    default:
        FURI_LOG_E(TAG, "Unsupported partition for key storage: %d", key->partition);
        furi_crash();
    }

    FuriHalCryptoKeyHeader* header_key = malloc(sizeof(FuriHalCryptoKeyHeader));
    sl_status_t status = SL_STATUS_FAIL;

    while((address_start + sizeof(FuriHalCryptoKeyHeader)) <= address_end) {
        // Read the header of the key at the current address
        status = sl_si91x_command_to_read_common_flash(
            address_start, sizeof(FuriHalCryptoKeyHeader), (uint8_t*)header_key);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
            ret = FuriHalCryptoStatusFail;
            break;
        }
        if(header_key->magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
            if(header_key->type == key->header.type && header_key->id == key->header.id) {
                // Duplicate key type+id found
                ret = FuriHalCryptoStatusDuplicate;
                break;
            }
            // Found a matching key, continue to the next slot
            address_start += sizeof(FuriHalCryptoKeyHeader) + header_key->size;
        } else if(header_key->magic_number == 0xFFFFFFFF) {
            // Found an empty slot, return the address
            ret = FuriHalCryptoStatusOk;
            break;
        } else {
            furi_crash();
        }
    }

    if((address_start + sizeof(FuriHalCryptoKeyHeader)) > address_end) {
        ret = FuriHalCryptoStatusStorageFull;
    }

    if(ret == FuriHalCryptoStatusOk) {
        ret = furi_hal_crypto_storage_check_key_slot_is_free(key, address_start, address_end);
        if(ret == FuriHalCryptoStatusOk) {
            *address = address_start;
        }
    }
    free(header_key);
    return ret;
}

static FuriHalCryptoStatus
    furi_hal_crypto_storage_read_address(FuriHalCryptoKey* key, uint32_t address_start) {
    furi_assert(key);

    sl_status_t status = sl_si91x_command_to_read_common_flash(
        address_start, sizeof(FuriHalCryptoKeyHeader), (uint8_t*)&key->header);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    // Read the key data
    status = sl_si91x_command_to_read_common_flash(
        address_start + sizeof(FuriHalCryptoKeyHeader), key->header.size, key->data);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    return FuriHalCryptoStatusOk;
}

FuriHalCryptoStatus furi_hal_crypto_storage_write(FuriHalCryptoKey* key) {
    furi_check(key);
    furi_check(key->header.size <= key->length);
    furi_check(key->header.magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY);

    uint32_t address_start = 0;
    FuriHalCryptoStatus ret = furi_hal_crypto_storage_search_clean_place(key, &address_start);
    if(ret == FuriHalCryptoStatusDuplicate) {
        FURI_LOG_E(
            TAG,
            "Key with type %d and id 0x%08lx already exists",
            key->header.type,
            key->header.id);
        return ret;
    } else if(ret == FuriHalCryptoStatusStorageFull) {
        FURI_LOG_E(TAG, "No free space for key storage");
        return ret;
    } else if(ret != FuriHalCryptoStatusOk) {
        FURI_LOG_E(TAG, "Failed to find a clean place for key storage: %d", ret);
        return ret;
    }

    //wrap key if needed
    if(key->header.flags & FuriHalCryptoKeyFlagWrap) {
        uint16_t key_size = (key->header.size % 16) != 0 ? ((key->header.size / 16) * 16) + 16 :
                                                           key->header.size; // Align to 16 bytes
        furi_check(key_size <= key->length);
        uint8_t* key_data_wrap = malloc(key_size);

        if(key->header.type != FuriHalCryptoKeyTypeHmacSha1 &&
           key->header.type != FuriHalCryptoKeyTypeHmacSha256 &&
           key->header.type != FuriHalCryptoKeyTypeHmacSha384 &&
           key->header.type != FuriHalCryptoKeyTypeHmacSha512) {
            furi_hal_crypto_wrap_key(key->header.size, key->data, key_data_wrap);
            key->header.size = key_size;
            memcpy(key->data, key_data_wrap, key->header.size);
            memset(key_data_wrap, 0, key->header.size);
        } else {
            FuriHalCryptoHmacShaMode hmac_sha_mode = 0xFF;
            if(key->header.type == FuriHalCryptoKeyTypeHmacSha1) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha1;
            } else if(key->header.type == FuriHalCryptoKeyTypeHmacSha256) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha256;
            } else if(key->header.type == FuriHalCryptoKeyTypeHmacSha384) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha384;
            } else if(key->header.type == FuriHalCryptoKeyTypeHmacSha512) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha512;
            }
            size_t key_hmac_data_wrap_size = 0;
            furi_hal_crypto_hmac_wrap_key(
                key->header.size,
                key->data,
                hmac_sha_mode,
                key_data_wrap,
                &key_hmac_data_wrap_size);
            furi_check(key_hmac_data_wrap_size <= key->length);
            memcpy(key->data, key_data_wrap, key_hmac_data_wrap_size);
            memset(key_data_wrap, 0, key_hmac_data_wrap_size);
            key->header.size = key_hmac_data_wrap_size;
        }
        free(key_data_wrap);
    }

    //get crc32 of key
    key->header.crc32 = 0;
    key->header.crc32 = crc32_calc_buffer(
        key->header.crc32,
        (uint8_t*)&key->header,
        sizeof(FuriHalCryptoKeyHeader) - sizeof(key->header.crc32));
    key->header.crc32 = crc32_calc_buffer(key->header.crc32, (uint8_t*)key->data, key->length);

    //write key to NWP flash
    sl_status_t status = SL_STATUS_FAIL;

    status = sl_si91x_command_to_write_common_flash(
        address_start, (uint8_t*)&key->header, sizeof(FuriHalCryptoKeyHeader), 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    status = sl_si91x_command_to_write_common_flash(
        address_start + sizeof(FuriHalCryptoKeyHeader), (uint8_t*)key->data, key->header.size, 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    //check if key is written correctly
    FuriHalCryptoKey* key_check = furi_hal_crypto_storage_alloc(key->partition);

    ret = furi_hal_crypto_storage_read_address(key_check, address_start);

    if(ret == FuriHalCryptoStatusOk) {
        if(!(memcmp(&key->header, &key_check->header, sizeof(FuriHalCryptoKeyHeader)) &&
             !(memcmp(key->data, key_check->data, key->length)))) {
        } else {
            FURI_LOG_E(TAG, "Failed to write key\r\n");
            ret = FuriHalCryptoStatusFailWrite;
        }
    }
    furi_hal_crypto_storage_free(key_check);

    key->address = address_start;
    return ret;
}

FuriHalCryptoStatus
    furi_hal_crypto_storage_read(FuriHalCryptoKey* key, FuriHalCryptoKeyType type, uint32_t id) {
    furi_check(key);

    FuriHalCryptoStatus ret = FuriHalCryptoStatusFail;
    uint32_t address_start = 0;
    uint32_t address_end = 0;

    switch(key->partition) {
    case FuriHalCryptoPartitionMain:
        address_start = FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS;
        address_end = FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_END_ADDRESS;
        break;
    case FuriHalCryptoPartitionUser:
        address_start = FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS;
        address_end = FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS;
        break;
    default:
        FURI_LOG_E(TAG, "Unsupported partition for key storage: %d", key->partition);
        furi_crash();
    }

    FuriHalCryptoKeyHeader* header_key = malloc(sizeof(FuriHalCryptoKeyHeader));
    sl_status_t status = SL_STATUS_FAIL;

    while((address_start + sizeof(FuriHalCryptoKeyHeader)) <= address_end) {
        // Read the header of the key at the current address
        status = sl_si91x_command_to_read_common_flash(
            address_start, sizeof(FuriHalCryptoKeyHeader), (uint8_t*)header_key);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
            ret = FuriHalCryptoStatusFail;
            break;
        }
        if(header_key->magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
            // Found a matching key, check if it matches the requested type and id
            if(header_key->type == type && header_key->id == id) {
                // Read the key data
                ret = furi_hal_crypto_storage_read_address(key, address_start);
                break;
            } else {
                // Continue to the next slot
                address_start += sizeof(FuriHalCryptoKeyHeader) + header_key->size;
            }
        } else if(header_key->magic_number == 0xFFFFFFFF) {
            // Found an empty slot
            ret = FuriHalCryptoStatusNotFound; // No matching key found
            break;
        } else {
            furi_crash();
        }
    }

    if((address_start + sizeof(FuriHalCryptoKeyHeader)) > address_end) {
        ret = FuriHalCryptoStatusNotFound; // No matching key found
    }

    free(header_key);

    if(ret == FuriHalCryptoStatusOk) {
        uint32_t crc32 = 0;
        crc32 = crc32_calc_buffer(
            crc32,
            (uint8_t*)&key->header,
            sizeof(FuriHalCryptoKeyHeader) - sizeof(key->header.crc32));
        crc32 = crc32_calc_buffer(crc32, (uint8_t*)key->data, key->length);

        if(crc32 != key->header.crc32) {
            FURI_LOG_E(TAG, "Error: Key CRC32 mismatch for read key\r\n");
            ret = FuriHalCryptoStatusErrorCrc;
        }
    }
    key->address = address_start;
    return ret;
}

FuriHalCryptoStatus furi_hal_crypto_storage_get_next_key(FuriHalCryptoKey* key) {
    furi_check(key);
    furi_check(key->header.magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY);

    FuriHalCryptoStatus ret = FuriHalCryptoStatusFail;
    uint32_t address_start = 0;
    uint32_t address_end = 0;

    switch(key->partition) {
    case FuriHalCryptoPartitionMain:
        if(key->address == FURI_HAL_CRYPTO_KEY_ADDRESS_INIT) {
            key->address = FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS;
        } else {
            address_start = key->address + sizeof(FuriHalCryptoKeyHeader) + key->header.size;
        }
        address_end = FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_END_ADDRESS;
        break;
    case FuriHalCryptoPartitionUser:
        if(key->address == FURI_HAL_CRYPTO_KEY_ADDRESS_INIT) {
            address_start = FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS;
        } else {
            address_start = key->address + sizeof(FuriHalCryptoKeyHeader) + key->header.size;
        }
        address_end = FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS;
        break;
    default:
        FURI_LOG_E(TAG, "Unsupported partition for key storage: %d", key->partition);
        furi_crash();
    }

    if((address_start + sizeof(FuriHalCryptoKeyHeader)) > address_end) {
        ret = FuriHalCryptoStatusStorageFull;
    }

    FuriHalCryptoKeyHeader* header_key = malloc(sizeof(FuriHalCryptoKeyHeader));
    do {
        sl_status_t status = sl_si91x_command_to_read_common_flash(
            address_start, sizeof(FuriHalCryptoKeyHeader), (uint8_t*)header_key);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
            break;
        }
        if(header_key->magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
            // Read the key data
            ret = furi_hal_crypto_storage_read_address(key, address_start);
            if(ret == FuriHalCryptoStatusOk) {
                key->address = address_start;
            }
        } else if(header_key->magic_number == 0xFFFFFFFF) {
            // No more keys found
            ret = FuriHalCryptoStatusNotFound;
        } else {
            furi_crash();
        }
    } while(false);

    free(header_key);

    return ret;
}

FuriHalCryptoStatus furi_hal_crypto_storage_gen_asimetric_pub_key(FuriHalCryptoKey* key) {
    furi_check(key);
    furi_check(key->header.magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY);
    furi_check(
        key->header.type == FuriHalCryptoKeyTypeEcdsaPriv224 ||
        key->header.type == FuriHalCryptoKeyTypeEcdsaPriv256);

    FuriHalCryptoKey* pub_key = furi_hal_crypto_storage_alloc(key->partition);

    psa_status_t psa_status;
    psa_key_id_t key_id;
    psa_key_attributes_t key_attr;
    size_t pubkey_len;
    FuriHalCryptoStatus status = FuriHalCryptoStatusFail;

    do {
        psa_status = psa_crypto_init();
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(
                TAG, "PSA crypto library initialization failed with error: %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "PSA crypto library initialization Success");
        }
        // Set up attributes for a volatile private key
        key_attr = psa_key_attributes_init();
        psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));

        if(key->header.type == FuriHalCryptoKeyTypeEcdsaPriv224) {
            psa_set_key_bits(&key_attr, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224_BITS);
            pub_key->header.size = FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224; // Get size in bytes
            pub_key->header.type = FuriHalCryptoKeyTypeEcdsaPub224;
        } else if(key->header.type == FuriHalCryptoKeyTypeEcdsaPriv256) {
            psa_set_key_bits(&key_attr, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS);
            pub_key->header.size = FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256; // Get size in bytes
            pub_key->header.type = FuriHalCryptoKeyTypeEcdsaPub256;
        }
        pub_key->header.id = key->header.id;
        pub_key->header.flags = key->header.flags &
                                ~FuriHalCryptoKeyFlagWrap; // Clear wrap flag for public key

        psa_set_key_usage_flags(
            &key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
        psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

        // Import a private key
        psa_status = psa_import_key(&key_attr, key->data, key->header.size, &key_id);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(TAG, "Import Key failed with error: status %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Import Key success");
        }

        // Export a public key from a volatile private key
        psa_status =
            psa_export_public_key(key_id, pub_key->data, pub_key->header.size, &pubkey_len);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(
                TAG, "Exporting a Public Key from Private key Failed with error: %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Export Public Key from Private Key Success");
        }

        furi_check(pubkey_len == pub_key->header.size);

        // Destroy the private key
        psa_status = psa_destroy_key(key_id);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(TAG, "Destroy Key failed with error : %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Destroy Key Success");
        }

        status = furi_hal_crypto_storage_write(pub_key);
    } while(false);
    free(pub_key);
    return status;
}

FuriHalCryptoStatus
    furi_hal_crypto_storage_gen_csr_der_ecdsa256(FuriHalCryptoKey* key, const char* subject_name) {
    furi_check(key);
    furi_check(key->header.magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY);
    furi_check(
        key->header.type == FuriHalCryptoKeyTypeEcdsaPriv256 &&
        key->header.size == FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256 &&
        (key->header.flags & FuriHalCryptoKeyFlagWrap) == 0);

    FuriHalCryptoKey* csr_der_key = furi_hal_crypto_storage_alloc(key->partition);
    psa_key_id_t key_id;
    mbedtls_pk_context key_ctx;
    mbedtls_x509write_csr csr_ctx;
    int csr_der_status = 0;

    size_t max_size = FURI_HAL_CRYPTO_CSR_BUFFER_SIZE_MAX;
    uint8_t* buffer = malloc(max_size);
    psa_key_attributes_t key_attr;
    FuriHalCryptoStatus status = FuriHalCryptoStatusFail;
    psa_status_t psa_status;

    do {
        psa_status = psa_crypto_init();
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(
                TAG, "PSA crypto library initialization failed with error: %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "PSA crypto library initialization Success");
        }

        // import key
        key_attr = psa_key_attributes_init();
        psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&key_attr, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS);
        psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
        psa_set_key_usage_flags(
            &key_attr,
            PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE |
                PSA_KEY_USAGE_VERIFY_MESSAGE);

        // Import a private key
        psa_status = psa_import_key(&key_attr, key->data, key->header.size, &key_id);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(TAG, "Import Key failed with error: status %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Import Key success");
        }

        // Generate CSR
        mbedtls_x509write_csr_init(&csr_ctx);
        csr_der_status = mbedtls_x509write_csr_set_subject_name(&csr_ctx, subject_name);
        if(csr_der_status != 0) {
            FURI_LOG_E(TAG, "Failed to set subject name for CSR: %d", csr_der_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Subject name set for CSR: %s", subject_name);
        }
        mbedtls_x509write_csr_set_md_alg(&csr_ctx, MBEDTLS_MD_SHA256);
        mbedtls_pk_init(&key_ctx);
        csr_der_status = mbedtls_pk_setup_opaque(&key_ctx, key_id);
        if(csr_der_status != 0) {
            FURI_LOG_E(TAG, "Failed to setup key context for CSR: %d", csr_der_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Key context setup for CSR");
        }
        mbedtls_x509write_csr_set_key(&csr_ctx, &key_ctx);
        int len_or_err =
            mbedtls_x509write_csr_der(&csr_ctx, (uint8_t*)buffer, max_size, NULL, NULL);
        if(len_or_err < 0) {
            FURI_LOG_E(TAG, "Failed to generate CSR DER: %d", len_or_err);
            break;
        } else {
            FURI_LOG_D(TAG, "CSR DER generated successfully");
        }

        csr_der_key->header.size = len_or_err;
        csr_der_key->header.type = FuriHalCryptoKeyTypeCsrDerEcdsa256;
        csr_der_key->header.id = key->header.id;

        memcpy(csr_der_key->data, buffer + (max_size - len_or_err), len_or_err);

        status = furi_hal_crypto_storage_write(csr_der_key);

    } while(false);

    mbedtls_x509write_csr_free(&csr_ctx);
    mbedtls_pk_free(&key_ctx);
    psa_destroy_key(key_id);
    free(buffer);
    furi_hal_crypto_storage_free(csr_der_key);

    return status;
}

FuriHalCryptoStatus furi_hal_crypto_storage_gen_random_buf(uint8_t* buf, size_t size) {
    furi_check(buf);
    furi_check(size > 0);
    furi_check(size <= 1024);

    uint32_t trng_key[TRNG_KEY_SIZE] = {0x16157E2B, 0xA6D2AE28, 0x8815F7AB, 0x3C4FCF09};
    sl_status_t status = SL_STATUS_FAIL;
    // This API checks the Entropy of TRNG i.e source for TRNG
    status = sl_si91x_trng_entropy();
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to check TRNG entropy: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }
    // This API Initializes key which needs to be programmed to TRNG hardware engine
    status = sl_si91x_trng_program_key(trng_key, TRNG_KEY_SIZE);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to program TRNG key: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }
    // Get Random dwords of desired length
    uint32_t reget_num = 10;
    do {
        status = sl_si91x_trng_get_random_num((uint32_t*)buf, size);
        --reget_num;
    } while((status == SL_STATUS_TRNG_DUPLICATE_ENTROPY) && reget_num);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to get random numbers: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }
    return FuriHalCryptoStatusOk;
}
