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
 * @param[in,out] instance pointer to the instance to contain the parsed data
 * @param[in] source pointer to a zero-terminated URL string
 * @returns @c true if the source string could be parsed, @c false otherwise
 */
bool semver_parse(SemVer* instance, const char* source);
