/**
 * @file l10n.h
 * @brief Localization service
 * 
 * Some of the APIs provided by this service use an internal string buffer to
 * decrease verbosity of the calling code. As such, `const char*`s returned by
 * these functions are only valid until the next time that this internal buffer
 * is overwritten. If you prefer not to think about lifetimes, you may
 * immediately `strdup`, `strcpy`, or `furi_string_set_str` the returned
 * strings. If you prefer less verbosity, you have to be a little careful about
 * calling these functions.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "l10n_common.h"

#include <furi.h>

#define RECORD_L10N "l10n"

typedef struct L10nSrv L10nSrv;
typedef struct L10nContext L10nContext;

/**
 * @brief Gets information about a locale
 * 
 * @param[in] id Locale ID
 * 
 * @returns Locale info
 */
const L10nLocaleInfo* l10n_locale_info(L10nLocale id);

/**
 * @brief Sets the global locale
 * 
 * @warning The locale will be applied after a reboot, not immediately.
 * 
 * @param[in] service Service handle
 * @param[in] id Locale ID
 */
void l10n_set_global_locale(L10nSrv* service, L10nLocale id);

/**
 * @brief Gets the global locale
 * 
 * @param[in] service Service handle
 * 
 * @returns Locale ID
 */
L10nLocale l10n_get_global_locale(L10nSrv* service);

/**
 * @brief Where to fetch the templates from
 */
typedef enum {
    L10nSourceStorage, // <! Fetch templates from Storage
    L10nSourceFlash, // <! Fetch templates from the flash
} L10nSource;

/**
 * @brief Creates a translation context
 * 
 * @param[in] service Localization handle
 * @param[in] app_id_or_path Application id (when `L10nSourceFlash`) or path to
 *                           directory containing the locale files (when
 *                           `L10nSourceStorage`)
 * @param[in] source Where to get the templates from
 * 
 * @returns Translation context
 */
L10nContext* l10n_context_open(L10nSrv* service, const char* app_id_or_path, L10nSource source);

/**
 * @brief Closes a translation context
 * 
 * @param[in] context Context handle
 */
void l10n_context_close(L10nContext* context);

/**
 * @brief Fetches and fills in a translation template
 * 
 * @param[in] context Context handle
 * @param[in] key Key to fetch the template
 * @param[in] ... Args to paste into the template
 * 
 * @warning The returned string is only valid until the next call to any of the
 * functions marked with this warning: `l10n_get`, `l10n_get_resource`.
 * 
 * @returns C-string. Read function warning.
 */
const char* l10n_get(L10nContext* context, L10nKey key, ...);

/**
 * @brief Fetches and fills in a translation template
 * 
 * @param[in] context Context handle
 * @param[out] buf Buffer to fill with the translation
 * @param[in] buf_size Size of the buffer
 * @param[in] key Key to fetch the template
 * @param[in] ... Args to paste into the template
 */
void l10n_get_into(L10nContext* context, char* buf, size_t buf_size, L10nKey key, ...);

/**
 * @brief Fetches and fills in a translation template
 * 
 * @param[in] context Context handle
 * @param[out] string String to fill with the translation
 * @param[in] key Key to fetch the template
 * @param[in] ... Args to paste into the template
 */
void l10n_get_furi_str(L10nContext* context, FuriString* string, L10nKey key, ...);

/**
 * @brief Gets the path to a localized resource
 * 
 * Fills in a path template with the most appropriate localized variant of a
 * resource. If the locale is set to `RuRu` and this function is given a
 * template of `"/ext/blah/blah/my_anim%s.anim"`, it will first search for
 * `my_anim_ru-RU.anim`, or fall back to `my_anim_en-US.anim`, or finally
 * `my_anim.anim`. If none were found, returns `my_anim.anim`.
 * 
 * @param[in] context Context handle
 * @param[in] template Template of a path to the resource. Must have exactly one
 *                     formatting placeholder, which should be `%s`.
 * 
 * @warning The returned string is only valid until the next call to any of the
 * functions marked with this warning: `l10n_get`, `l10n_get_resource`.
 * 
 * @returns C-string with a path to the localized variant of the requested
 *          resource. Read function warning.
 */
const char* l10n_get_resource(L10nContext* context, const char* template);

#ifdef __cplusplus
}
#endif
