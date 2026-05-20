#include <furi_hal_crypto_storage.h>
#include <furi_hal_crypto.h>
#include <sl_si91x_driver.h>
#include <toolbox/crc32_calc.h>
#include <nvm/nvm.h>

#include <sl_si91x_trng.h>
#include <psa/crypto.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>

#define TAG "FuriHalCryptoStorage"

#define FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY (0x464c4950UL) // "FLIP"
#define FURI_HAL_CRYPTO_KEY_ADDRESS_INIT         (0xFFFFFFFFUL)

static uint32_t get_partition_start(FuriHalCryptoPartition partition) {
    switch(partition) {
    case FuriHalCryptoPartitionMain:
        return FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS;
    case FuriHalCryptoPartitionUser:
        return FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS;
    default:
        // unreachable
        furi_assert(false);
        return 0;
    }
}

static uint32_t get_partition_end(FuriHalCryptoPartition partition) {
    switch(partition) {
    case FuriHalCryptoPartitionMain:
        return FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_END_ADDRESS;
    case FuriHalCryptoPartitionUser:
        return FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS;
    default:
        // unreachable
        furi_assert(false);
        return 0;
    }
}

static uint32_t get_abs_address(const FuriHalCryptoKeyAddress* address) {
    return get_partition_start(address->partition) + address->offset;
}

static FuriHalCryptoStatus furi_hal_crypto_storage_check_key_slot_is_free(
    const FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t offset) {
    furi_assert(key);

    sl_status_t status = SL_STATUS_FAIL;
    FuriHalCryptoStatus ret = FuriHalCryptoStatusFail;
    const uint32_t address_start = get_partition_start(partition) + offset;
    const uint32_t address_max = get_partition_end(partition);
    // Calculate the size for writing the key
    const uint32_t key_slot_size = key->length + sizeof(FuriHalCryptoKeySlotHeader);
    if((address_start + key_slot_size) > address_max) {
        FURI_LOG_E(TAG, "Key exceeds storage limits");
        return FuriHalCryptoStatusStorageFull;
    }

    uint8_t* buf = malloc(key_slot_size);

    do {
        status = sl_si91x_command_to_read_common_flash(address_start, key_slot_size, buf);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
            break;
        }

        // Check if the slot is free (all bytes are 0xFF)
        ret = FuriHalCryptoStatusOk;
        for(uint32_t i = 0; i != key_slot_size; ++i) {
            if(buf[i] != 0xff) {
                ret = FuriHalCryptoStatusFail;
                break;
            }
        }
    } while(false);
    memset(buf, 0, key_slot_size);
    free(buf);
    return ret;
}

static FuriHalCryptoStatus furi_hal_crypto_storage_find_empty_slot(
    const FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t id,
    FuriHalCryptoKeyAddress* address) {
    FuriHalCryptoStatus ret = FuriHalCryptoStatusFail;
    const uint32_t address_start = get_partition_start(partition);
    const uint32_t address_max = get_partition_end(partition);
    uint32_t address_offset = 0;

    sl_status_t status = SL_STATUS_FAIL;

    while((address_start + address_offset + sizeof(FuriHalCryptoKeySlotHeader)) <= address_max) {
        FuriHalCryptoKeySlotHeader header;
        // Read the header of the key at the current address
        status = sl_si91x_command_to_read_common_flash(
            address_start + address_offset, sizeof(FuriHalCryptoKeySlotHeader), (uint8_t*)&header);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
            ret = FuriHalCryptoStatusFail;
            break;
        }
        if(header.magic_number == FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
            if(header.type == key->type && header.id == id) {
                // Duplicate key type+id found
                ret = FuriHalCryptoStatusDuplicate;
                break;
            }
            // Found a key, continue to the next slot
            address_offset += sizeof(FuriHalCryptoKeySlotHeader) + header.size;
        } else if(header.magic_number == 0xFFFFFFFF) {
            // Found an empty slot, return the address
            ret = FuriHalCryptoStatusOk;
            break;
        } else {
            furi_crash();
        }
    }

    if((address_start + address_offset + sizeof(FuriHalCryptoKeySlotHeader)) >= address_max) {
        ret = FuriHalCryptoStatusStorageFull;
    }

    if(ret == FuriHalCryptoStatusOk) {
        ret = furi_hal_crypto_storage_check_key_slot_is_free(key, partition, address_offset);
        if(ret == FuriHalCryptoStatusOk) {
            address->partition = partition;
            address->offset = address_offset;
        }
    }

    return ret;
}

static FuriHalCryptoStatus furi_hal_crypto_storage_read_address(
    FuriHalCryptoKey* key,
    FuriHalCryptoKeySlotHeader* header,
    uint32_t address_start) {
    furi_assert(key);
    furi_assert(header);

    sl_status_t status = sl_si91x_command_to_read_common_flash(
        address_start, sizeof(FuriHalCryptoKeySlotHeader), (uint8_t*)header);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    // Read the key data
    uint32_t read_size = MIN(header->size, sizeof(key->data));
    status = sl_si91x_command_to_read_common_flash(
        address_start + sizeof(FuriHalCryptoKeySlotHeader), read_size, key->data);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    return FuriHalCryptoStatusOk;
}

static FuriHalCryptoStatus
    open_slot(const FuriHalCryptoKeyAddress* address, FuriHalCryptoKeySlot* slot) {
    uint32_t address_start = get_abs_address(address);

    if(address_start >= get_partition_end(address->partition)) {
        return FuriHalCryptoStatusStorageFull;
    }

    sl_status_t status = sl_si91x_command_to_read_common_flash(
        address_start, sizeof(FuriHalCryptoKeySlotHeader), (uint8_t*)&slot->header);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    if(slot->header.magic_number != FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY) {
        return FuriHalCryptoStatusNotFound;
    }

    if(slot->header.size > FURI_HAL_CRYPTO_DATA_SIZE_MAX) {
        return FuriHalCryptoStatusNotFound;
    }

    slot->address = *address;

    return FuriHalCryptoStatusOk;
}

static FuriHalCryptoStatus load_key(const FuriHalCryptoKeySlot* slot, FuriHalCryptoKey* key) {
    furi_assert(key);

    furi_check(slot->header.size <= FURI_HAL_CRYPTO_DATA_SIZE_MAX);

    uint32_t address_start = get_abs_address(&slot->address) + sizeof(FuriHalCryptoKeySlotHeader);

    // Read the key data
    sl_status_t status =
        sl_si91x_command_to_read_common_flash(address_start, slot->header.size, key->data);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    key->length = slot->header.size;
    key->type = slot->header.type;
    key->flags = slot->header.flags;

    uint32_t crc32 = 0;
    crc32 = crc32_calc_buffer(
        crc32,
        (uint8_t*)&slot->header,
        sizeof(FuriHalCryptoKeySlotHeader) - sizeof(slot->header.crc32));

    static_assert(FURI_HAL_CRYPTO_DATA_SIZE_MAX == sizeof(key->data));
    bzero(key->data + key->length, FURI_HAL_CRYPTO_DATA_SIZE_MAX - key->length);
    crc32 = crc32_calc_buffer(crc32, (uint8_t*)key->data, FURI_HAL_CRYPTO_DATA_SIZE_MAX);

    if(crc32 != slot->header.crc32) {
        FURI_LOG_E(TAG, "Error: Key CRC32 mismatch for read key\r\n");
        return FuriHalCryptoStatusErrorCrc;
    }

    return FuriHalCryptoStatusOk;
}

FuriHalCryptoStatus furi_hal_crypto_storage_set_access_mode(FuriHalCryptoStorageAccessMode mode) {
    Nvm* nvm = furi_record_open(RECORD_NVM);
    bool success = nvm_write_counter(nvm, NvmKeyCryptoAccessMode, mode);
    furi_record_close(RECORD_NVM);
    if(!success) {
        return FuriHalCryptoStatusFail;
    } else {
        return FuriHalCryptoStatusOk;
    }
}

FuriHalCryptoStorageAccessMode furi_hal_crypto_storage_get_access_mode(void) {
    Nvm* nvm = furi_record_open(RECORD_NVM);
    uint32_t access_mode = 0;
#if defined(FURI_DEBUG)
    FuriHalCryptoStorageAccessMode result = FuriHalCryptoStorageAccessModeReadWrite;
#else
    FuriHalCryptoStorageAccessMode result = FuriHalCryptoStorageAccessModeReadOnly;
#endif
    if(nvm_read_counter(nvm, NvmKeyCryptoAccessMode, &access_mode)) {
        switch(access_mode) {
        case FuriHalCryptoStorageAccessModeReadOnly:
        case FuriHalCryptoStorageAccessModeReadWrite:
            result = access_mode;
            break;
        default:
            break;
        }
    }
    furi_record_close(RECORD_NVM);
    return result;
}

FuriHalCryptoStatus furi_hal_crypto_storage_wipe(FuriHalCryptoPartition partition) {
    if(furi_hal_crypto_storage_get_access_mode() != FuriHalCryptoStorageAccessModeReadWrite) {
        return FuriHalCryptoStatusErrorAccess;
    }

    sl_status_t status = sl_si91x_command_to_write_common_flash(
        get_partition_start(partition),
        NULL,
        get_partition_end(partition) - get_partition_start(partition) + 1,
        1);
    if(status != SL_STATUS_OK) {
        return FuriHalCryptoStatusFail;
    }
    return FuriHalCryptoStatusOk;
}

FuriHalCryptoStatus furi_hal_crypto_storage_write(
    const FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t id) {
    return furi_hal_crypto_storage_write_ex(key, partition, id, NULL);
}

FuriHalCryptoStatus furi_hal_crypto_storage_write_ex(
    const FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t id,
    FuriHalCryptoKeySlot* slot_out) {
    furi_check(key);

    if(furi_hal_crypto_storage_get_access_mode() != FuriHalCryptoStorageAccessModeReadWrite) {
        return FuriHalCryptoStatusErrorAccess;
    }

    FuriHalCryptoKeySlot slot;
    FuriHalCryptoStatus ret =
        furi_hal_crypto_storage_find_empty_slot(key, partition, id, &slot.address);
    if(ret == FuriHalCryptoStatusDuplicate) {
        FURI_LOG_E(TAG, "Key with type %d and id 0x%08lx already exists", key->type, id);
        return ret;
    } else if(ret == FuriHalCryptoStatusStorageFull) {
        FURI_LOG_E(TAG, "No free space for key storage");
        return ret;
    } else if(ret != FuriHalCryptoStatusOk) {
        FURI_LOG_E(TAG, "Failed to find a clean place for key storage: %d", ret);
        return ret;
    }

    // fill out the header
    slot.header.magic_number = FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY;
    slot.header.type = key->type;
    slot.header.flags = key->flags;
    slot.header.id = id;
    slot.header.reserved = UINT16_MAX;
    slot.header.reserved1 = UINT32_MAX;
    slot.header.size = key->length;

    // calculate crc32: header + key + padding zeroes
    slot.header.crc32 = 0;
    slot.header.crc32 = crc32_calc_buffer(
        slot.header.crc32,
        (uint8_t*)&slot.header,
        sizeof(FuriHalCryptoKeySlotHeader) - sizeof(slot.header.crc32));

    uint8_t* crc_buf = malloc(FURI_HAL_CRYPTO_DATA_SIZE_MAX);
    memcpy(crc_buf, key->data, key->length);
    bzero(crc_buf + key->length, FURI_HAL_CRYPTO_DATA_SIZE_MAX - key->length);
    slot.header.crc32 =
        crc32_calc_buffer(slot.header.crc32, crc_buf, FURI_HAL_CRYPTO_DATA_SIZE_MAX);
    bzero(crc_buf, FURI_HAL_CRYPTO_DATA_SIZE_MAX);
    free(crc_buf);

    if(slot_out) {
        memcpy(slot_out, &slot, sizeof(slot));
    }

    // write key to NWP flash
    sl_status_t status = SL_STATUS_FAIL;

    uint32_t abs_address = get_abs_address(&slot.address);

    status = sl_si91x_command_to_write_common_flash(
        abs_address, (uint8_t*)&slot.header, sizeof(FuriHalCryptoKeySlotHeader), 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    status = sl_si91x_command_to_write_common_flash(
        abs_address + sizeof(FuriHalCryptoKeySlotHeader), (uint8_t*)key->data, key->length, 0);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }

    // check if key is written correctly
    FuriHalCryptoKey* key_check = malloc(sizeof(FuriHalCryptoKey));
    FuriHalCryptoKeySlotHeader header_check;

    ret = furi_hal_crypto_storage_read_address(key_check, &header_check, abs_address);

    if(ret == FuriHalCryptoStatusOk) {
        if((memcmp(&slot.header, &header_check, sizeof(FuriHalCryptoKeySlotHeader)) ||
            (memcmp(key->data, key_check->data, key->length)))) {
            FURI_LOG_E(TAG, "Failed to write key\r\n");
            ret = FuriHalCryptoStatusFailWrite;
        }
    }
    free(key_check);

    return ret;
}

FuriHalCryptoStatus furi_hal_crypto_storage_read(
    FuriHalCryptoKey** key_out,
    FuriHalCryptoPartition partition,
    FuriHalCryptoKeyType type,
    uint32_t id) {
    furi_check(key_out);

    return furi_hal_crypto_storage_read_ex(key_out, NULL, partition, type, id);
}

FuriHalCryptoStatus furi_hal_crypto_storage_read_ex(
    FuriHalCryptoKey** key_out,
    FuriHalCryptoKeySlot* slot,
    FuriHalCryptoPartition partition,
    FuriHalCryptoKeyType type,
    uint32_t id) {
    furi_check(key_out);

    FuriHalCryptoStatus ret = FuriHalCryptoStatusFail;

    FuriHalCryptoKeyIter iter = furi_hal_crypto_key_iter_init(partition);
    FuriHalCryptoKey* current_key = NULL;
    FuriHalCryptoKeySlot current_slot;

    while((ret = furi_hal_crypto_key_iter_get_and_advance(&iter, &current_key, &current_slot)) ==
          FuriHalCryptoStatusOk) {
        if(current_slot.header.id == id && current_slot.header.type == type) {
            *key_out = current_key;
            if(slot) {
                *slot = current_slot;
            }
            return FuriHalCryptoStatusOk;
        }
        furi_hal_crypto_key_free(current_key);
    }

    if(ret == FuriHalCryptoStatusStorageFull) {
        ret = FuriHalCryptoStatusNotFound;
    }
    return ret;
}

FuriHalCryptoKeyIter furi_hal_crypto_key_iter_init(FuriHalCryptoPartition partition) {
    return (FuriHalCryptoKeyIter){
        .address = {
            .offset = 0,
            .partition = partition,
        }};
}

FuriHalCryptoStatus furi_hal_crypto_key_iter_get_and_advance(
    FuriHalCryptoKeyIter* iter,
    FuriHalCryptoKey** key_out,
    FuriHalCryptoKeySlot* slot_out) {
    furi_check(key_out);
    furi_check(slot_out);
    furi_check(iter);

    if(get_abs_address(&iter->address) >= get_partition_end(iter->address.partition)) {
        return FuriHalCryptoStatusStorageFull;
    }

    FuriHalCryptoStatus ret = open_slot(&iter->address, slot_out);
    if(ret != FuriHalCryptoStatusOk) {
        return ret;
    }
    FuriHalCryptoKey* key = malloc(sizeof(FuriHalCryptoKey));
    ret = load_key(slot_out, key);
    if(ret != FuriHalCryptoStatusOk) {
        furi_hal_crypto_key_free(key);
        return ret;
    } else {
        *key_out = key;
        iter->address.offset += slot_out->header.size + sizeof(FuriHalCryptoKeySlotHeader);
        return FuriHalCryptoStatusOk;
    }
}
