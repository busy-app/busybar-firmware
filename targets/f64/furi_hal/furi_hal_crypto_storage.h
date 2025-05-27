#pragma once
#include <furi.h>

#define FURI_HAL_CRYPTO_STORAGE_START_ADDRESS (0UL)
#define FURI_HAL_CRYPTO_STORAGE_END_ADDRESS   (0x00005000UL) // 20KB partition

#define FURI_HAL_CRYPTO_STORAGE_MAGIC_NUMBER_KEY (0x464c4950UL) // "FLIP"
#define FURI_HAL_CRYPTO_STORAGE_MAX_KEY_SLOT     (32UL)

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_1_START_ADDRESS (0x00000000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_1_END_ADDRESS   (0x00000FFFUL) // 4KB partition

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_2_START_ADDRESS (0x00001000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_2_END_ADDRESS   (0x00001FFFUL) // 4KB partition

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_MATTER_START_ADDRESS (0x00002000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_MATTER_END_ADDRESS   (0x00003FFFUL) // 8KB partition

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS (0x00004000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS   (0x00004FFFUL) // 4KB partition

typedef enum {
    FuriHalCryptoKeyTypeAes128,
    FuriHalCryptoKeyTypeAes192,
    FuriHalCryptoKeyTypeAes256,
    FuriHalCryptoKeyTypeHmacSha1,
    FuriHalCryptoKeyTypeHmacSha256,
    FuriHalCryptoKeyTypeHmacSha384,
    FuriHalCryptoKeyTypeHmacSha512,
    FuriHalCryptoKeyTypeEcdsaPriv224,
    FuriHalCryptoKeyTypeEcdsaPriv256,
    FuriHalCryptoKeyTypeEcdsaPub224,
    FuriHalCryptoKeyTypeEcdsaPub256,
    FuriHalCryptoKeyTypeNone = 0xFFFFFFFF,
} FuriHalCryptoKeyType;
_Static_assert(sizeof(FuriHalCryptoKeyType) == 4, "Size check for 'FuriHalCryptoKeyType' failed.");

typedef enum {
    FuriHalCryptoKeyFlagWrap = (1 << 0),
    FuriHalCryptoKeyFlagUnwrap = (1 << 1),
    FuriHalCryptoKeyFlagPublicKey = (1 << 2),
    FuriHalCryptoKeyFlagPrivateKey = (1 << 3),
    FuriHalCryptoKeyFlagSignature = (1 << 4),
    FuriHalCryptoKeyFlagNone = 0xFFFFFFFF,
} FuriHalCryptoKeyFlag;
_Static_assert(sizeof(FuriHalCryptoKeyFlag) == 4, "Size check for 'FuriHalCryptoKeyFlag' failed.");

typedef struct {
    uint32_t magic_number;
    uint16_t slot;
    uint16_t size;
    FuriHalCryptoKeyType type;
    FuriHalCryptoKeyFlag flags;
    uint32_t id;
    uint32_t reserved;
    uint8_t data[100];
    uint32_t crc32;
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
bool furi_hal_crypto_storage_check_key_slot_is_free(uint16_t key_slot);

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
