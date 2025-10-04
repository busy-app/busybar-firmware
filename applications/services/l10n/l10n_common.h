/**
 * @file l10n_common.h
 * @brief Abstract localization definitions
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @brief Key used to fetch a template from the translation table. Only makes
 * sense in the context of a specific application.
 */
typedef size_t L10nKey;

/**
 * @brief Information about a locale
 */
typedef struct {
    const char* self_name; // <! example: "Русский"
    const char* iso_name; // <! example: "en-US"
} L10nLocaleInfo;

/**
 * @brief Supported locales
 * 
 * @warning When adding a new member this enum, do so just above
 * `L10nLocaleCOUNT`. Do not add a new locale in between older ones. Otherwise
 * the user's locale setting might change after a firmware update.
 */
typedef enum {
    L10nLocaleEnUs,
    L10nLocaleRuRu,
    L10nLocaleCOUNT, // <! Do not use
} L10nLocale;

#ifdef __cplusplus
}
#endif
