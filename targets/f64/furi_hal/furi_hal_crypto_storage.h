#pragma once
#include <furi.h>

#define FURI_HAL_CRYPTO_STORAGE_START_ADDRESS (0UL)
#define FURI_HAL_CRYPTO_STORAGE_END_ADDRESS   (0x00005000UL) // 20KB partition

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS (0x00000000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_END_ADDRESS   (0x00003FFFUL) // 16KB partition

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS (0x00004000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS   (0x00004FFFUL) // 4KB partition

typedef enum {
    FuriHalCryptoPartitionMain,
    FuriHalCryptoPartitionUser,
    FuriHalCryptoPartitionMax,
} FuriHalCryptoPartition;

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
    FuriHalCryptoKeyTypeMatterDAC,
    FuriHalCryptoKeyTypeMatterPAI,
    FuriHalCryptoKeyTypeMatterCD,
    FuriHalCryptoKeyTypeMatterVID_PID,
    FuriHalCryptoKeyTypeMatterSPAKE2,
    FuriHalCryptoKeyTypeNone = 0xFFFFFFFF,
} FuriHalCryptoKeyType;
_Static_assert(sizeof(FuriHalCryptoKeyType) == 4, "Size check for 'FuriHalCryptoKeyType' failed.");

typedef enum {
    FuriHalCryptoKeyFlagWrap = (1 << 0UL),
    FuriHalCryptoKeyFlagNone = 0xFFFFFFFF,
} FuriHalCryptoKeyFlag;
_Static_assert(sizeof(FuriHalCryptoKeyFlag) == 4, "Size check for 'FuriHalCryptoKeyFlag' failed.");

typedef struct {
    uint32_t magic_number;
    uint16_t reserved;
    uint16_t size;
    FuriHalCryptoKeyType type;
    FuriHalCryptoKeyFlag flags;
    uint32_t id;
    uint32_t reserved1;
    uint32_t crc32;
} FURI_PACKED FuriHalCryptoKeyHeader;
_Static_assert(
    sizeof(FuriHalCryptoKeyHeader) == 28,
    "Size check for 'FuriHalCryptoKeyHeader' failed.");

typedef struct {
    FuriHalCryptoKeyHeader header;
    uint16_t length;
    uint8_t* data;
    FuriHalCryptoPartition partition;
} FURI_PACKED FuriHalCryptoKey;

typedef enum {
    FuriHalCryptoStatusOk,
    FuriHalCryptoStatusFail,
    FuriHalCryptoStatusFailWrite,
    FuriHalCryptoStatusStorageFull,
    FuriHalCryptoStatusDuplicate,
    FuriHalCryptoStatusNotFound,
    FuriHalCryptoStatusErrorCrc,
} FuriHalCryptoStatus;

#ifdef __cplusplus
extern "C" {
#endif

/*
* Allocate a key structure.
* @param[in] partition Partition to get the start address of.
* @return Pointer to the allocated key structure.
*/
FuriHalCryptoKey* furi_hal_crypto_storage_alloc(FuriHalCryptoPartition partition);

/*
* Free the key structure.
* @param[in] key Pointer to the key structure to free.
*/
void furi_hal_crypto_storage_free(FuriHalCryptoKey* key);

/** Write a key to the NWP flash.
* @param[in] key Pointer to the key
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_storage_write(FuriHalCryptoKey* key);

/** Read a key from the NWP flash.
* @param[in] key Pointer to the key
* @param[in] type Type of the key to read.
* @param[in] id ID of the key to read.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus
    furi_hal_crypto_storage_read(FuriHalCryptoKey* key, FuriHalCryptoKeyType type, uint32_t id);

/** Generate a random buffer of the specified size.
* @param[out] buf Pointer to the buffer to fill with random data.
* @param[in] size Size of the buffer to fill.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_storage_gen_random_buf(uint8_t* buf, size_t size);
#ifdef __cplusplus
}
#endif
