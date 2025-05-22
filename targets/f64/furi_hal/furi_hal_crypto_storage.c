#include <furi_hal_crypto_storage.h>
#include "sl_si91x_driver.h"

#define TAG "FuriHalCryptoStorage"

bool furi_hal_crypto_storage_check_key_slot_is_free(uint32_t key_slot) {
    sl_status_t status = 0;
    uint32_t address = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + key_slot * sizeof(FuriHalCryptoKey);
    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    bool ret = false;
    uint32_t i = 0;
    do {
        if(key == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate memory\r\n");
            break;
        }
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

    if(key->key_slot >= FURI_HAL_CRYPTO_STORAGE_MAX_KEY_SLOT) {
        FURI_LOG_E(TAG, "Key slot %d is out of range\r\n", key->key_slot);
        return false;
    }

    if(!furi_hal_crypto_storage_check_key_slot_is_free(key->key_slot)) {
        FURI_LOG_E(TAG, "Key slot %d is not free\r\n", key->key_slot);
        return false;
    }

    sl_status_t status = 0;
    uint32_t address =
        FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + key->key_slot * sizeof(FuriHalCryptoKey);
    status = sl_si91x_command_to_write_common_flash(
        address, (uint8_t*)key, sizeof(FuriHalCryptoKey), 0);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%lx\r\n", status);
        return false;
    }
    return true;
}

bool furi_hal_crypto_storage_read_key(FuriHalCryptoKey* key) {
    furi_check(key);

    if(key->key_slot >= FURI_HAL_CRYPTO_STORAGE_MAX_KEY_SLOT) {
        FURI_LOG_E(TAG, "Key slot %d is out of range\r\n", key->key_slot);
        return false;
    }
    uint16_t key_slot = key->key_slot;

    sl_status_t status = 0;
    uint32_t address =
        FURI_HAL_CRYPTO_STORAGE_START_ADDRESS + key->key_slot * sizeof(FuriHalCryptoKey);
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
    return true;
}
