#include <furi_hal_crypto_storage.h>
#include <furi_hal_crypto.h>
#include <sl_si91x_driver.h>
#include <toolbox_f64/crc32_calc.h>

#define TAG "FuriHalCryptoStorage"

bool furi_hal_crypto_storage_check_key_slot_is_free(uint16_t key_slot) {
    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + key_slot * sizeof(FuriHalCryptoKey);
    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    bool ret = false;
    uint32_t i = 0;
    do {
        status = sl_si91x_command_to_read_common_flash(
            address, sizeof(FuriHalCryptoKey), (uint8_t*)key);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%lx\r\n", status);
            break;
        }
        uint8_t* p_key = (uint8_t*)key;
        while(i < sizeof(FuriHalCryptoKey)) {
            if(p_key[i] != 0xFF) {
                break;
            }
            i++;
        }

        if(i == sizeof(FuriHalCryptoKey)) {
            ret = true;
        }

    } while(false);
    free(key);
    return ret;
}

bool furi_hal_crypto_storage_write_key(FuriHalCryptoKey* key) {
    furi_check(key);
    furi_check(key->size <= sizeof(key->data));
    furi_check(key->magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY);

    if(key->slot >= FURI_HAL_CRYPTO_STORAGE_MAX_KEY_SLOT) {
        FURI_LOG_E(TAG, "Key slot %d is out of range\r\n", key->slot);
        return false;
    }

    if(!furi_hal_crypto_storage_check_key_slot_is_free(key->slot)) {
        FURI_LOG_E(TAG, "Key slot %d is not free\r\n", key->slot);
        return false;
    }

    //wrap key if needed
    if(key->flags & FuriHalCryptoKeyFlagWrap) {
        uint16_t key_size = (key->size % 16) != 0 ? ((key->size / 16) * 16) + 16 :
                                                    key->size; // Align to 16 bytes
        furi_check(key_size <= sizeof(key->data));
        uint8_t* key_data_wrap = malloc(key_size);

        if(key->type != FuriHalCryptoKeyTypeHmacSha1 &&
           key->type != FuriHalCryptoKeyTypeHmacSha256 &&
           key->type != FuriHalCryptoKeyTypeHmacSha384 &&
           key->type != FuriHalCryptoKeyTypeHmacSha512) {
            furi_hal_crypto_wrap_key(key->size, key->data, key_data_wrap);
            key->size = key_size;
            memcpy(key->data, key_data_wrap, key->size);
            memset(key_data_wrap, 0, key->size);

        } else {
            FuriHalCryptoHmacShaMode hmac_sha_mode = 0xFF;
            if(key->type == FuriHalCryptoKeyTypeHmacSha1) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha1;
            } else if(key->type == FuriHalCryptoKeyTypeHmacSha256) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha256;
            } else if(key->type == FuriHalCryptoKeyTypeHmacSha384) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha384;
            } else if(key->type == FuriHalCryptoKeyTypeHmacSha512) {
                hmac_sha_mode = FuriHalCryptoHmacShaModeSha512;
            }

            size_t key_hmac_data_wrap_size = 0;
            furi_hal_crypto_hmac_wrap_key(
                key->size, key->data, hmac_sha_mode, key_data_wrap, &key_hmac_data_wrap_size);
            furi_check(key_hmac_data_wrap_size <= sizeof(key->data));
            memcpy(key->data, key_data_wrap, key_hmac_data_wrap_size);
            memset(key_data_wrap, 0, key_hmac_data_wrap_size);
            key->size = key_hmac_data_wrap_size;
        }
        free(key_data_wrap);
    }

    //get crc32 of key
    key->crc32 = 0;
    key->crc32 = crc32_calc_buffer(
        key->crc32, (uint8_t*)key, sizeof(FuriHalCryptoKey) - sizeof(key->crc32));

    //write key to NWP flash
    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address =
        FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + key->slot * sizeof(FuriHalCryptoKey);

    status = sl_si91x_command_to_write_common_flash(
        address, (uint8_t*)key, sizeof(FuriHalCryptoKey), 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%lx\r\n", status);
        return false;
    }

    //check if key is written correctly
    FuriHalCryptoKey* key_check = malloc(sizeof(FuriHalCryptoKey));

    key_check->slot = key->slot;
    furi_hal_crypto_storage_read_key(key_check);
    if(memcmp(key, key_check, sizeof(FuriHalCryptoKey)) != 0) {
        FURI_LOG_E(TAG, "Failed to write key\r\n");
        memset(key_check, 0, sizeof(FuriHalCryptoKey));
        free(key_check);
        return false;
    }
    memset(key_check, 0, sizeof(FuriHalCryptoKey));
    free(key_check);

    return true;
}

bool furi_hal_crypto_storage_read_key(FuriHalCryptoKey* key) {
    furi_check(key);

    if(key->slot >= FURI_HAL_CRYPTO_STORAGE_MAX_KEY_SLOT) {
        FURI_LOG_E(TAG, "Key slot %d is out of range\r\n", key->slot);
        return false;
    }
    uint16_t key_slot = key->slot;

    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address =
        FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + key->slot * sizeof(FuriHalCryptoKey);
    status =
        sl_si91x_command_to_read_common_flash(address, sizeof(FuriHalCryptoKey), (uint8_t*)key);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%lx\r\n", status);
        return false;
    }
    if(key->magic_number != FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
        FURI_LOG_E(TAG, "Key slot %d is not valid\r\n", key_slot);
        return false;
    }

    uint32_t crc32 = 0;
    crc32 = crc32_calc_buffer(crc32, (uint8_t*)key, sizeof(FuriHalCryptoKey) - sizeof(key->crc32));
    if(crc32 != key->crc32) {
        FURI_LOG_E(TAG, "Key slot %d is not valid crc\r\n", key_slot);
        return false;
    }

    return true;
}
