#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "l10n_table_i.h"
#include "l10n_common.h"
#include <stddef.h>

/**
 * @brief Associates an (app, locale) tuple with a translation table
 */
typedef struct {
    const char* app_id;
    L10nLocale locale;
    const L10nTable* table;
} L10nAppListEntry;

extern const size_t L10N_APP_LIST_COUNT;
extern const L10nAppListEntry L10N_APP_LIST[];

#ifdef __cplusplus
}
#endif
