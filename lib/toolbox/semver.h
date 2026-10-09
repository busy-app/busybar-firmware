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
 * @brief Enumeration of semantic version orderings.
 */
typedef enum {
    SemVerOrderingEqual, /**< Versions are equal */
    SemVerOrderingNewer, /**< The version in question is newer than the other one */
    SemVerOrderingOlder, /**< The version in question is older than the other one */
} SemVerOrdering;

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

/**
 * @brief Compare two semantic versions.
 *
 * @param[in] instance pointer to the instance to be compared
 * @param[in] other pointer to the other instance to compare with
 * @returns @ref SemVerOrderingEqual both versions are equal
 * @returns @ref SemVerOrderingNewer @p instance is newer than @p other
 * @returns @ref SemVerOrderingOlder @p instance is older than @p other
 */
SemVerOrdering semver_compare(const SemVer* instance, const SemVer* other);
