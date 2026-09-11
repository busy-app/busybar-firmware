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
 * @brief Reset the slice so that it represents a valid empty string.
 *
 * @param[in,out] instance pointer to the slice to be reset
 */
void string_slice_reset(StringSlice* instance);
