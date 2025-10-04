/**
 * @file l10n_table.h
 * @brief Localization table definitions ofr internal use
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "l10n_common.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief A list of translation templates for a specific app and locale
 */
typedef struct L10nTable {
    const char* const* entries;
    size_t entry_cnt;
    bool is_owned; // <! true = dynamically allocated in RAM and should be freed, false = in flash
} L10nTable;

#ifdef __cplusplus
}
#endif
