/**
 * @file semver.h
 * @brief Semantic version (SemVer) utility library.
 *
 * This implementation follows the x.y.z format.
 * No additional fields are allowed.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Structure representing the semantic version values.
 */
typedef struct {
    uint32_t major; /**< Major version value */
    uint32_t minor; /**< Minor version value */
    uint32_t patch; /**< Patch version value */
} SemVer;

/**
 * @brief Parse semantic version from a string.
 *
 * If the @p instance pointer is @c NULL, the @p source string will still be parsed normally.
 * This can be useful if the calling code only needs to validate the source string.
 *
 * @param[in,out] instance pointer to the instance to contain the parsed data (can be @c NULL)
 * @param[in] source pointer to a zero-terminated URL string
 * @returns @c true if the source string could be parsed, @c false otherwise
 */
bool semver_parse(SemVer* instance, const char* source);
