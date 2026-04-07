#pragma once
#include <furi.h>

#define FURI_HAL_CRYPTO_STORAGE_START_ADDRESS (0UL)
#define FURI_HAL_CRYPTO_STORAGE_END_ADDRESS   (0x00005000UL) // 20KB partition

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_START_ADDRESS (0x00000000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_MAIN_END_ADDRESS   (0x00003FFFUL) // 16KB partition

#define FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_START_ADDRESS (0x00004000UL)
#define FURI_HAL_CRYPTO_STORAGE_PARTITION_USER_END_ADDRESS   (0x00004FFFUL) // 4KB partition

#define FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MAX (996UL) // Maximum data size for keys

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

    FuriHalCryptoKeyTypeCsrDerEcdsa256,
    FuriHalCryptoKeyTypeCrtDerEcdsa256,

    FuriHalCryptoKeyTypeMatterAttestation,
    FuriHalCryptoKeyTypeMatterSetup,
    FuriHalCryptoKeyTypeMatterDeviceInfo,

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
} FURI_PACKED FuriHalCryptoKeySlotHeader;
_Static_assert(
    sizeof(FuriHalCryptoKeySlotHeader) == 28,
    "Size check for 'FuriHalCryptoKeySlotHeader' failed.");

typedef struct {
    FuriHalCryptoKeySlotHeader header;
    FuriHalCryptoPartition partition;
    uint32_t address; // Address in NWP flash where the key is stored
    uint16_t length;
    uint8_t data[FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MAX];
} FuriHalCryptoKeyDeprecated;

typedef struct {
    FuriHalCryptoPartition partition;
    uint32_t offset; // Address in NWP flash where the key is stored (offset from partition start)
} FuriHalCryptoKeyAddress;

typedef struct {
    FuriHalCryptoKeySlotHeader header;
    FuriHalCryptoKeyAddress address;
} FuriHalCryptoKeySlot;

typedef struct {
    FuriHalCryptoKeyType type;
    FuriHalCryptoKeyFlag flags;
    uint16_t length;
    uint8_t data[FURI_HAL_CRYPTO_STORAGE_DATA_SIZE_MAX];
} FuriHalCryptoKey;

typedef struct {
    FuriHalCryptoKeyAddress address;
} FuriHalCryptoKeyIter;

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

FuriHalCryptoKeySlot* furi_hal_crypto_storage_save(
    const FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t id);

FuriHalCryptoStatus furi_hal_crypto_storage_load(
    FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t id);

FuriHalCryptoKeyIter furi_hal_crypto_key_iter_init(FuriHalCryptoPartition partition);

FuriHalCryptoStatus furi_hal_crypto_key_iter_get(
    const FuriHalCryptoKeyIter* iter,
    FuriHalCryptoKey* key_out,
    FuriHalCryptoKeySlot* slot_out);

FuriHalCryptoStatus furi_hal_crypto_key_iter_advance(FuriHalCryptoKeyIter* iter);

/*
* Allocate a key structure.
* @param[in] partition Partition to get the start address of.
* @return Pointer to the allocated key structure.
*/
FuriHalCryptoKeyDeprecated* furi_hal_crypto_storage_alloc(FuriHalCryptoPartition partition);

/*
* Free the key structure.
* @param[in] key Pointer to the key structure to free.
*/
void furi_hal_crypto_storage_free(FuriHalCryptoKeyDeprecated* key);

/** Write a key to the NWP flash.
* @param[in] key Pointer to the key
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_storage_write(FuriHalCryptoKeyDeprecated* key);

/** Read a key from the NWP flash.
* @param[in] key Pointer to the key
* @param[in] type Type of the key to read.
* @param[in] id ID of the key to read.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_storage_read(
    FuriHalCryptoKeyDeprecated* key,
    FuriHalCryptoKeyType type,
    uint32_t id);

/** Get the next key in the storage.
* @param[in] key Pointer to the key structure to fill with the next key.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_storage_get_next_key(FuriHalCryptoKeyDeprecated* key);

/** Generate an asymmetric public key from a private key.
* @param[in] key Pointer to the private key.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus
    furi_hal_crypto_storage_gen_asymmetric_pub_key(FuriHalCryptoKeyDeprecated* key);

/** Generate a CSR in DER format for ECDSA 256.
* @param[in] key Pointer to the private key.
* @param[in] subject_name Subject name for the CSR.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_storage_gen_csr_der_ecdsa256(
    FuriHalCryptoKeyDeprecated* key,
    const char* subject_name);

/** Generate a random buffer of the specified size.
* @param[out] buf Pointer to the buffer to fill with random data.
* @param[in] size Size of the buffer to fill.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_storage_gen_random_buf(uint8_t* buf, size_t size);
#ifdef __cplusplus
}
#endif
