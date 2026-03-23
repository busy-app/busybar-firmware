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
 * @brief String-to-string mapping
 * 
 * Finds out the index of `input` in `src_array`, returns the element at the
 * same index in `dst_array`.
 * 
 * @param   src_array Array of source strings (to map from)
 * @param   dst_array Array of destination strings (to map to)
 * @param   length Length of either array (the must have equal lengths)
 * @param   input Input string to map
 * 
 */
const char* value_index_map_string(
    const char* const src_array[],
    const char* const dst_array[],
    size_t count,
    const char* input);

#ifdef __cplusplus
}
#endif
