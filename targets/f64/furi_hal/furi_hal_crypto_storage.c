
#include <furi_hal_crypto_storage.h>
#include <furi_hal_crypto.h>
#include <sl_si91x_driver.h>
#include <toolbox_f64/crc32_calc.h>

#define TAG "FuriHalCryptoStorage"

#define FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY (0x464c4950UL) // "FLIP"
#define FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MAX    (996UL) // Maximum data size for keys
#define FURI_HAL_CRYPTO_STORAGE_BAD_ADDRESS      (0xFFFFFFFFUL)

FuriHalCryptoKey* furi_hal_crypto_storage_alloc(FuriHalCryptoPartition partition) {
    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    key->header.magic_number = FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY;
    key->partition = partition;
    key->length = FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MAX;
    key->data = malloc(key->length);
    return key;
}

void furi_hal_crypto_storage_free(FuriHalCryptoKey* key) {
    furi_check(key);
    if(key->data) {
        free(key->data);
    }
    free(key);
}

static bool furi_hal_crypto_storage_check_key_slot_is_free(
    FuriHalCryptoKey* key,
    uint32_t address_start,
    uint32_t address_end) {
    furi_assert(key);

    sl_status_t status = SL_STATUS_FAIL;
    bool ret = false;
    // Calculate the size for writing the key
    if((address_start + key->header.size + sizeof(FuriHalCryptoKeyHeader)) > address_end) {
        FURI_LOG_E(TAG, "Key exceeds storage limits");
        return ret;
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
        ret = (i == key_slot_size);
    } while(false);
    memset(buf, 0, key_slot_size);
    free(buf);
    return ret;
}

static uint32_t furi_hal_crypto_storage_search_clean_place(FuriHalCryptoKey* key) {
    furi_assert(key);

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
            address_start = FURI_HAL_CRYPTO_STORAGE_BAD_ADDRESS; // Exit the loop if read fails
            break;
        }
        if(header_key->magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
            // Found a matching key, continue to the next slot
            address_start += sizeof(FuriHalCryptoKeyHeader) + header_key->size;
        } else if(header_key->magic_number == 0xFFFFFFFF) {
            // Found an empty slot, return the address
            break;
        } else {
            furi_crash();
        }
    }

    if(!furi_hal_crypto_storage_check_key_slot_is_free(key, address_start, address_end)) {
        address_start = FURI_HAL_CRYPTO_STORAGE_BAD_ADDRESS;
    }

    free(header_key);
    return address_start;
}

static bool furi_hal_crypto_storage_read_address(FuriHalCryptoKey* key, uint32_t address_start) {
    furi_assert(key);

    sl_status_t status = sl_si91x_command_to_read_common_flash(
        address_start, sizeof(FuriHalCryptoKeyHeader), (uint8_t*)&key->header);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return false;
    }

    // Read the key data
    status = sl_si91x_command_to_read_common_flash(
        address_start + sizeof(FuriHalCryptoKeyHeader), key->header.size, key->data);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return false;
    }

    return true;
}

bool furi_hal_crypto_storage_write(FuriHalCryptoKey* key) {
    furi_check(key);
    furi_check(key->header.size <= key->length);
    furi_check(key->header.magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY);

    uint32_t address_start = furi_hal_crypto_storage_search_clean_place(key);
    if(address_start == FURI_HAL_CRYPTO_STORAGE_BAD_ADDRESS) {
        FURI_LOG_E(TAG, "Failed to find a clean place for key storage");
        return false;
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
        return false;
    }

    status = sl_si91x_command_to_write_common_flash(
        address_start + sizeof(FuriHalCryptoKeyHeader), (uint8_t*)key->data, key->header.size, 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx\r\n", status);
        return false;
    }

    //check if key is written correctly
    FuriHalCryptoKey* key_check = furi_hal_crypto_storage_alloc(key->partition);

    bool ret = furi_hal_crypto_storage_read_address(key_check, address_start);

    if(ret) {
        if(!(memcmp(&key->header, &key_check->header, sizeof(FuriHalCryptoKeyHeader)) &&
             !(memcmp(key->data, key_check->data, key->length)))) {
            ret = true;
        } else {
            FURI_LOG_E(TAG, "Failed to write key\r\n");
        }
    }
    memset(key_check->data, 0, key_check->length);
    memset(&key_check->header, 0, sizeof(FuriHalCryptoKeyHeader));
    furi_hal_crypto_storage_free(key_check);
    key_check = NULL;

    return ret;
}

bool furi_hal_crypto_storage_read(FuriHalCryptoKey* key, FuriHalCryptoKeyType type, uint32_t id) {
    furi_check(key);

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
    bool ret = false;

    while((address_start + sizeof(FuriHalCryptoKeyHeader)) <= address_end) {
        // Read the header of the key at the current address
        status = sl_si91x_command_to_read_common_flash(
            address_start, sizeof(FuriHalCryptoKeyHeader), (uint8_t*)header_key);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
            ret = false; // Exit the loop if read fails
            break;
        }
        if(header_key->magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
            // Found a matching key, check if it matches the requested type and id
            if(header_key->type == type && header_key->id == id) {
                // Read the key data
                if(!furi_hal_crypto_storage_read_address(key, address_start)) {
                    FURI_LOG_E(TAG, "Failed to read key from NWP flash");
                    ret = false;
                    break;
                } else {
                    ret = true;
                    break;
                }
            } else {
                // Continue to the next slot
                address_start += sizeof(FuriHalCryptoKeyHeader) + header_key->size;
            }
        } else if(header_key->magic_number == 0xFFFFFFFF) {
            // Found an empty slot
            ret = false; // No matching key found
            break;
        } else {
            furi_crash();
        }
    }
    free(header_key);

    if(ret) {
        uint32_t crc32 = 0;
        crc32 = crc32_calc_buffer(
            crc32,
            (uint8_t*)&key->header,
            sizeof(FuriHalCryptoKeyHeader) - sizeof(key->header.crc32));
        crc32 = crc32_calc_buffer(crc32, (uint8_t*)key->data, key->length);

        if(crc32 != key->header.crc32) {
            FURI_LOG_E(TAG, "Error: Key CRC32 mismatch for read key\r\n");
            ret = false;
        }
    }

    return ret;
}
