/**
 * @file l10n_table.h
 * @brief Implements lookup of translation templates
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "./l10n_common.h"
#include <furi.h>
#include <storage/storage.h>

/**
 * @brief A list of translation templates for a specific app and locale
 */
typedef struct L10nTable L10nTable;

/**
 * @brief Finds a translation table right in the firmware image
 * 
 * @param[in] app_id Application ID
 * @param[in] locale Locale
 * 
 * @returns Translation table for the `(app_id, locale)` tuple.
 *          NULL if not found.
 */
const L10nTable* l10n_table_alloc_builtin(const char* app_id, L10nLocale locale);

/**
 * @brief Creates a translation table from a file in storage
 * 
 * @param[in] file File to get the data from
 * 
 * @returns Allocated translation table. NULL if not found.
 */
const L10nTable* l10n_table_alloc_from_storage(File* file);

/**
 * @brief Frees a localization table
 * 
 * @param[in] table Localization table
 */
void l10n_table_free(const L10nTable* table);

/**
 * @brief Returns the template associated with a key in the table
 * 
 * @param[in] table Localization table
 * @param[in] key Key to use
 * 
 * @returns String template associated with the key. NULL if not found
 */
const char* l10n_table_get(const L10nTable* table, L10nKey key);

#ifdef __cplusplus
}
#endif
