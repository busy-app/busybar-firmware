
#include <furi_hal_crypto_storage.h>
#include <furi_hal_crypto.h>
#include <sl_si91x_driver.h>
#include <toolbox_f64/crc32_calc.h>

#define TAG "FuriHalCryptoStorage"

#define FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY (0x464c4950UL) // "FLIP"
#define FURI_HAL_CRYPTO_STORAGE_DATA_SIZE        (100UL)
#define FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MATTER (996UL)

_Static_assert(
    sizeof(FuriHalCryptoKeyHeader) + FURI_HAL_CRYPTO_STORAGE_DATA_SIZE == 128,
    "Size check failed.");
_Static_assert(
    sizeof(FuriHalCryptoKeyHeader) + FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MATTER == 1024,
    "Size check failed.");
#define FURI_HAL_CRYPTO_STORAGE_SLOT_SIZE \
    (sizeof(FuriHalCryptoKeyHeader) + FURI_HAL_CRYPTO_STORAGE_DATA_SIZE)
#define FURI_HAL_CRYPTO_STORAGE_SLOT_SIZE_MATTER \
    (sizeof(FuriHalCryptoKeyHeader) + FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MATTER)

#define FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MIN        (1UL)
#define FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MAX        (31UL)
#define FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MIN_MATTER (0UL)
#define FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MAX_MATTER (7UL)

static uint32_t furi_hal_crypto_storage_get_start_address(FuriHalCryptoPartition partition) {
    uint32_t address = 0;
    switch(partition) {
    case FuriHalCryptoPartition1:
        address = FURI_HAL_CRYPTO_STORAGE_PARTITION_1_START_ADDRESS;
        break;
    case FuriHalCryptoPartition2:
        address = FURI_HAL_CRYPTO_STORAGE_PARTITION_2_START_ADDRESS;
        break;
    case FuriHalCryptoPartitionMatter:
        address = FURI_HAL_CRYPTO_STORAGE_PARTITION_MATTER_START_ADDRESS;
        break;
    case FuriHalCryptoPartitionUser:
        address = FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS;
        break;
    default:
        furi_crash();
        break;
    }
    return address;
}

static bool
    furi_hal_crypto_storage_check_slot(FuriHalCryptoPartition partition, uint16_t key_slot) {
    bool ret = false;
    switch(partition) {
    case FuriHalCryptoPartition1:
    case FuriHalCryptoPartition2:
    case FuriHalCryptoPartitionUser:
#if FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MIN != 0
        ret =
            (FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MIN <= key_slot &&
             key_slot <= FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MAX);
#else
        ret = (key_slot <= FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MAX);
#endif
        break;
    case FuriHalCryptoPartitionMatter:
#if FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MIN_MATTER != 0
        ret =
            (FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MIN_MATTER <= key_slot &&
             key_slot <= FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MAX_MATTER);
#else
        ret = (key_slot <= FURI_HAL_CRYPTO_STORAGE_KEY_SLOT_MAX_MATTER);
#endif
        break;
    default:
        furi_crash();
        break;
    }
    return ret;
}

static uint32_t furi_hal_crypto_storage_get_slot_size(FuriHalCryptoPartition partition) {
    uint32_t slot_size = 0;
    switch(partition) {
    case FuriHalCryptoPartition1:
    case FuriHalCryptoPartition2:
    case FuriHalCryptoPartitionUser:
        slot_size = FURI_HAL_CRYPTO_STORAGE_SLOT_SIZE;
        break;
    case FuriHalCryptoPartitionMatter:
        slot_size = FURI_HAL_CRYPTO_STORAGE_SLOT_SIZE_MATTER;
        break;
    default:
        furi_crash();
        break;
    }
    return slot_size;
}

static uint32_t furi_hal_crypto_storage_get_address_start_slot(
    FuriHalCryptoPartition partition,
    uint16_t key_slot) {
    return furi_hal_crypto_storage_get_start_address(partition) +
           key_slot * furi_hal_crypto_storage_get_slot_size(partition);
}

bool furi_hal_crypto_storage_check_key_slot_is_free(
    FuriHalCryptoPartition partition,
    uint16_t key_slot) {
    sl_status_t status = SL_STATUS_FAIL;

    uint16_t key_slot_size = furi_hal_crypto_storage_get_slot_size(partition);

    uint32_t address = furi_hal_crypto_storage_get_address_start_slot(partition, key_slot);
    uint8_t* buf = malloc(key_slot_size);
    bool ret = false;
    do {
        status = sl_si91x_command_to_read_common_flash(address, key_slot_size, buf);

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

bool furi_hal_crypto_storage_write_key(FuriHalCryptoPartition partition, FuriHalCryptoKey* key) {
    furi_check(key);
    furi_check(key->header.size <= key->length);
    furi_check(key->header.magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY);

    if(!furi_hal_crypto_storage_check_slot(partition, key->header.slot)) {
        FURI_LOG_E(TAG, "Key slot %d is out of range\r\n", key->header.slot);
        return false;
    }

    if(!furi_hal_crypto_storage_check_key_slot_is_free(partition, key->header.slot)) {
        FURI_LOG_E(TAG, "Key slot %d is not free\r\n", key->header.slot);
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
    uint32_t address = furi_hal_crypto_storage_get_address_start_slot(partition, key->header.slot);

    status = sl_si91x_command_to_write_common_flash(
        address, (uint8_t*)&key->header, sizeof(FuriHalCryptoKeyHeader), 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx\r\n", status);
        return false;
    }

    status = sl_si91x_command_to_write_common_flash(
        address + sizeof(FuriHalCryptoKeyHeader), (uint8_t*)key->data, key->length, 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx\r\n", status);
        return false;
    }

    //check if key is written correctly
    FuriHalCryptoKey* key_check = furi_hal_crypto_storage_alloc_key(partition);

    key_check->header.slot = key->header.slot;
    furi_hal_crypto_storage_read_key(partition, key_check);
    bool ret = false;
    if(!(memcmp(&key->header, &key_check->header, sizeof(FuriHalCryptoKeyHeader)) &&
         !(memcmp(key->data, key_check->data, key->length)))) {
        ret = true;
    } else {
        FURI_LOG_E(TAG, "Failed to write key\r\n");
    }
    memset(key_check->data, 0, key_check->length);
    memset(&key_check->header, 0, sizeof(FuriHalCryptoKeyHeader));

    furi_hal_crypto_storage_free_key(key_check);
    key_check = NULL;

    return ret;
}

FuriHalCryptoKey* furi_hal_crypto_storage_alloc_key(FuriHalCryptoPartition partition) {
    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    key->header.magic_number = FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY;
    if(partition == FuriHalCryptoPartitionMatter) {
        key->length = FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MATTER;
        key->header.flags = FuriHalCryptoKeyFlagBigData;
    } else {
        key->length = FURI_HAL_CRYPTO_STORAGE_DATA_SIZE;
    }
    key->data = malloc(key->length);
    return key;
}

void furi_hal_crypto_storage_free_key(FuriHalCryptoKey* key) {
    furi_check(key);
    if(key->data) {
        free(key->data);
    }
    free(key);
}

bool furi_hal_crypto_storage_read_key(FuriHalCryptoPartition partition, FuriHalCryptoKey* key) {
    furi_check(key);

    if(!furi_hal_crypto_storage_check_slot(partition, key->header.slot)) {
        FURI_LOG_E(TAG, "Key slot %d is out of range\r\n", key->header.slot);
        return false;
    }
    uint16_t key_slot = key->header.slot;

    sl_status_t status = SL_STATUS_FAIL;
    uint32_t address = furi_hal_crypto_storage_get_address_start_slot(partition, key_slot);

    status = sl_si91x_command_to_read_common_flash(
        address, sizeof(FuriHalCryptoKeyHeader), (uint8_t*)&key->header);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return false;
    }

    if(key->header.magic_number != FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
        FURI_LOG_E(TAG, "Key slot %d is not valid\r\n", key_slot);
        return false;
    }

    status = sl_si91x_command_to_read_common_flash(
        address + sizeof(FuriHalCryptoKeyHeader), key->length, (uint8_t*)key->data);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return false;
    }

    uint32_t crc32 = 0;
    crc32 = crc32_calc_buffer(
        crc32, (uint8_t*)&key->header, sizeof(FuriHalCryptoKeyHeader) - sizeof(key->header.crc32));
    crc32 = crc32_calc_buffer(crc32, (uint8_t*)key->data, key->length);

    if(crc32 != key->header.crc32) {
        FURI_LOG_E(TAG, "Key slot %d is not valid crc\r\n", key_slot);
        return false;
    }

    return true;
}
