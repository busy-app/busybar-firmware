/**
 * @file nvm.h
 * @brief Non-volatile flash memory API.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for Nvm instance access.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_NVM)`
 */
#define RECORD_NVM "nvm"

/**
 * @brief Enumeration of available Nvm key ranges.
 *
 * @warning Keys 0x0-0xFFFF are reserved, do not use
 * @warning Keys 0x87000-0x87FFF are reserved, do not use
 */
typedef enum {
    NvmKeyRangeMin = 0x0UL, /**< Minimum key value (inclusive) */
    NvmKeyRangeUser1Min = 0x10000UL, /**< Free to use key range 1 start (inclusive) */
    NvmKeyBlePairingData = 0x10001UL,
    /* Add more key ranges here */
    NvmKeyRangeUser1Max = 0x87000UL, /**< Free to use key range 1 end (non-inclusive) */
    NvmKeyRangeUser2Min = 0x88000UL, /**< Free to use key range 2 start (inclusive) */
    /* Add more key ranges here */
    NvmKeyRangeMax = 0x1000000UL, /**< Maximum key value (non-inclusive) */
} NvmKeyRange;

typedef struct Nvm Nvm;

/**
 * @brief Check if a record exists under a given key and optionally get its length.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the record to be checked
 * @param[out] len optional pointer to the variable to contain the record length (may be NULL)
 *
 * @returns true if the record exists, false otherwise
 */
bool nvm_exists(Nvm* instance, uint32_t key, size_t* len);

/**
 * @brief Read the record data under a given key.
 *
 * @note Value of @p len MUST match the data length given during writing,
 *       otherwise an error will occur.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the record to be read
 * @param[out] data pointer to the buffer to hold the data (must be large enough)
 * @param[in] len length of the data to be read (see note), in bytes
 *
 * @returns true if the record could be read, false otherwise
 */
bool nvm_read(Nvm* instance, uint32_t key, void* data, size_t len);

/**
 * @brief Read the record data under a given key partially.
 *
 * @note Value of @p len - @p offset MUST match the data length given during writing,
 *       otherwise an error will occur.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the record to be read
 * @param[out] data pointer to the buffer to hold the data (must be large enough)
 * @param[in] offset offset from the beginning of the data, in bytes
 * @param[in] len length of the data to be read (see note), in bytes
 *
 * @returns true if the record could be read, false otherwise
 */
bool nvm_read_partial(Nvm* instance, uint32_t key, void* data, size_t offset, size_t len);

/**
 * @brief Write data to a record under a given key.
 *
 * @note If a record under the given key already exists,
 *       it will be overwritten with the new data.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the record to be written
 * @param[in] data pointer to the data to be written
 * @param[in] len length of the data to be written
 *
 * @returns true if the record could be written, false otherwise
 */
bool nvm_write(Nvm* instance, uint32_t key, const void* data, size_t len);

/**
 * @brief Read the counter value under a given key.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the counter to be read
 * @param[out] value pointer to the variable to hold the counter value
 *
 * @returns true if the counter could be read, false otherwise
 */
bool nvm_read_counter(Nvm* instance, uint32_t key, uint32_t* value);

/**
 * @brief Write the counter value under a given key.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the counter to be written
 * @param[in] value counter value to be written
 *
 * @returns true if the counter could be written, false otherwise
 */
bool nvm_write_counter(Nvm* instance, uint32_t key, uint32_t value);

/**
 * @brief Increment the counter under a given key by 1 and optionally get its value.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the counter to be incremented
 * @param[out] value optional pointer to the variable to hold the new counter value (may be NULL)
 *
 * @returns true if the counter could be incremented, false otherwise
 */
bool nvm_increment_counter(Nvm* instance, uint32_t key, uint32_t* value);

/**
 * @brief Delete the record under a given key.
 *
 * @param[in,out] instance pointer to the Nvm instance
 * @param[in] key key associated with the record to be deleted
 *
 * @returns true if the record could be deleted, false otherwise
 */
bool nvm_delete(Nvm* instance, uint32_t key);

/**
 * @brief Repack the storage to optimise space.
 *
 * @note The operation will only be performed if the driver
 *       logic decides that it is needed.
 *
 * @param[in,out] instance pointer to the Nvm instance
 *
 * @returns true if the storage could be repacked, false otherwise
 */
bool nvm_repack(Nvm* instance);

/**
 * @brief Erase all data from the storage.
 *
 * @warning This will ERASE ALL THE DATA on the storage, use with caution
 *
 * @param[in,out] instance pointer to the Nvm instance
 *
 * @returns true if the storage could be erased, false otherwise.
 */
bool nvm_erase_all(Nvm* instance);

#ifdef __cplusplus
}
#endif
