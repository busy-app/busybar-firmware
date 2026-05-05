#pragma once
#include <furi.h>
#include "furi_hal_crypto.h"

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

typedef struct FuriHalCryptoKeyAddress {
    FuriHalCryptoPartition partition;
    uint32_t offset; // Address in NWP flash where the key is stored (offset from partition start)
} FuriHalCryptoKeyAddress;

typedef struct FuriHalCryptoKeySlot {
    FuriHalCryptoKeySlotHeader header;
    FuriHalCryptoKeyAddress address;
} FuriHalCryptoKeySlot;

typedef struct {
    FuriHalCryptoKeyAddress address;
} FuriHalCryptoKeyIter;

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize a key iterator.
 * @param[in] partition Partition to iterate in.
 */
FuriHalCryptoKeyIter furi_hal_crypto_key_iter_init(FuriHalCryptoPartition partition);

/** Get current key pointed by the key iterator and advance the iterator to the next key.
 * @return FuriHalCryptoStatus status of the operation.
 *    - FuriHalCryptoStatusOk on success
 *    - FuriHalCryptoStatusStorageFull on reaching the end of the storage
 *    - FuriHalCryptoStatusNotFound if there are no more keys
 *    - other errors corresponding to various storage failures
 */
FURI_CHECK_RETURN
FuriHalCryptoStatus furi_hal_crypto_key_iter_get_and_advance(
    FuriHalCryptoKeyIter* iter,
    FuriHalCryptoKey** key_out,
    FuriHalCryptoKeySlot* slot_out);

/** Write a key to the NWP flash.
* @param[in] key Pointer to the key
* @param[in] partition Partition where to store the key.
* @param[in] id ID of the key to write.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FURI_CHECK_RETURN
FuriHalCryptoStatus furi_hal_crypto_storage_write(
    const FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t id);

/** Write a key to the NWP flash, providing its slot metadata.
* @param[in] key Pointer to the key
* @param[in] partition Partition where to store the key.
* @param[in] id ID of the key to write.
* @param[out] slot Key storage slot metadata. Can be NULL.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FURI_CHECK_RETURN
FuriHalCryptoStatus furi_hal_crypto_storage_write_ex(
    const FuriHalCryptoKey* key,
    FuriHalCryptoPartition partition,
    uint32_t id,
    FuriHalCryptoKeySlot* slot_out);

/** Read a key from the NWP flash. Keys inside a partition are uniquely identified by (id, type).
* @param[in] key Pointer to the key
* @param[in] partition Partition where to look for the key.
* @param[in] type Type of the key to read.
* @param[in] id ID of the key to read.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FURI_CHECK_RETURN
FuriHalCryptoStatus furi_hal_crypto_storage_read(
    FuriHalCryptoKey** key,
    FuriHalCryptoPartition partition,
    FuriHalCryptoKeyType type,
    uint32_t id);

/** Read a key and its slot (metadata) from the NWP flash.
* Keys inside a partition are uniquely identified by (id, type).
* @param[in] key Pointer to the key
* @param[out] slot Key storage slot metadata. Can be NULL.
* @param[in] partition Partition where to look for the key.
* @param[in] type Type of the key to read.
* @param[in] id ID of the key to read.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FURI_CHECK_RETURN
FuriHalCryptoStatus furi_hal_crypto_storage_read_ex(
    FuriHalCryptoKey** key,
    FuriHalCryptoKeySlot* slot,
    FuriHalCryptoPartition partition,
    FuriHalCryptoKeyType type,
    uint32_t id);
#ifdef __cplusplus
}
#endif
