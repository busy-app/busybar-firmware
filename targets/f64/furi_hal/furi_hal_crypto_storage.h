#pragma once
#include <furi.h>

#define FURI_HAL_CRYPTO_STORAGE_START_ADDRESS   0
#define FURI_HAL_CRYPTO_STORAGE_END_ADDRESS     (1024 * 20)
#define FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY          0x464c4950 // "FLIP"
#define FURI_HAL_CRYPTO_STORAGE_MAX_KEY_SLOT              32

typedef struct {
    uint32_t magic_number;
    uint16_t key_slot;
    uint16_t key_size;
    uint32_t key_type;
    uint32_t key_flags;
    uint8_t key_data[112];
} FURI_PACKED FuriHalCryptoKey;
_Static_assert(sizeof(FuriHalCryptoKey) == 128, "Size check for 'FuriHalCryptoKey' failed.");


#ifdef __cplusplus
extern "C" {
#endif
/*
* Check if the key slot is free.
* @param[in] key_slot Key slot to check.
* @return true if the key slot is free, false otherwise.
*/
bool furi_hal_crypto_storage_check_key_slot_is_free(uint32_t key_slot);

/*
* Write the key to the key slot.
* @param[in] key Pointer to the key to write.
* @return true if the key was written successfully, false otherwise.
*/
bool furi_hal_crypto_storage_write_key(FuriHalCryptoKey* key);

/*
* Read the key from the key slot.
* @param[in] key Pointer to the key to read.
* @return true if the key was read successfully, false otherwise.
*/
bool furi_hal_crypto_storage_read_key(FuriHalCryptoKey* key);

#ifdef __cplusplus
}
#endif
