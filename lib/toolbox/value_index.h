#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Get the index of a int32_t array element which is closest to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
size_t value_index_int32(const int32_t value, const int32_t values[], size_t values_count);

/** Get the index of a uint32_t array element which is closest to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
size_t value_index_uint32(const uint32_t value, const uint32_t values[], size_t values_count);

/** Get the index of a float array element which is closest to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
size_t value_index_float(const float value, const float values[], size_t values_count);

/** Get the index of a bool array element which is equal to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
size_t value_index_bool(const bool value, const bool values[], size_t values_count);

/** Get the index of a string array element which is equal to the given value.
 *
 * Returned index corresponds to the first element found.
 * If no suitable elements were found, the function returns 0.
 *
 * @param   value           value to be searched.
 * @param   values          pointer to the array to perform the search in.
 * @param   values_count    array size.
 *
 * @return value's index.
 */
size_t value_index_string(const char* value, const char* const values[], size_t values_count);

/**
 * @brief Maps a string value using two arrays logically representing an associative array
 */
#define VALUE_INDEX_MAP_STRING(_src_array, _dst_array, _input) ({  \
        const char* const* src_array = _src_array;                 \
        const char* const* dst_array = _dst_array;                 \
        const char* input = _input;                                \
        size_t src_size = COUNT_OF(_src_array);                    \
        size_t dst_size = COUNT_OF(_dst_array);                    \
        furi_assert(src_size == dst_size);                         \
        dst_array[value_index_string(input, src_array, src_size)]; \
    })

#ifdef __cplusplus
}
#endif
