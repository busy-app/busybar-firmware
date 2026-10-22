/**
 * @file slice.h
 * @brief Slice types for various containers.
 */
#pragma once

#include <stddef.h>

/**
 * @brief Slice of a character string.
 */
typedef struct {
    const char* first_char; /**< The beginning of the slice */
    size_t length; /**< Slice length, in bytes */
} StringSlice;

/**
 * @brief Offsets in a string slice.
 *
 * Offsets are counted relative to the beginning of the source string.
 */
typedef struct {
    size_t start; /**< Offset of the slice's beginning  */
    size_t end; /**< Offset of the slice's end */
} StringSliceOffset;

/**
 * @brief Reset the slice so that it represents a valid empty string.
 *
 * @param[in,out] instance pointer to the slice to be reset
 */
void string_slice_reset(StringSlice* instance);

/**
 * @brief Get the offsets from a string slice.
 *
 * @param[in] instance pointer to the slice to be converted
 * @param[out] offset pointer to the offset structure to hold the results
 * @param[in] source zero-terminated string that the slice looks into
 */
void string_slice_to_offset(
    const StringSlice* instance,
    StringSliceOffset* offset,
    const char* source);

/**
 * @brief Build a string slice from the source string and offsets.
 *
 * @param[out] instance pointer to the slice to be populated
 * @param[in] offset pointer to the offset structure containing the offsets
 * @param[in] source zero-terminated string that the slice will look into
 */
void string_slice_from_offset(
    StringSlice* instance,
    const StringSliceOffset* offset,
    const char* source);
