#pragma once

#include <m-core.h>

#include "storage_macros.h"

#define BUSY_THEMES_DIR BUSY_ASSETS_PATH("themes")

#define BUSY_THEME_INIT(a)        ((a) = busy_theme_alloc())
#define BUSY_THEME_INIT_SET(a, b) ((a) = busy_theme_alloc_set(b))
#define BUSY_THEME_OPLIST           \
    (INIT(BUSY_THEME_INIT),         \
     INIT_SET(BUSY_THEME_INIT_SET), \
     SET(busy_theme_set),           \
     CLEAR(busy_theme_free),        \
     EQUAL(busy_theme_is_equal),    \
     TYPE(BusyTheme*))

typedef struct BusyTheme BusyTheme;

typedef enum {
    BusyThemeFileTypeImage,
    BusyThemeFileTypeAnim,
    BusyThemeFileTypeLottieAnim,
    BusyThemeFileTypeMax,
} BusyThemeFileType;

typedef struct {
    const char* name;
    const char* bg_path;
    BusyThemeFileType bg_type;
} BusyThemeInfo;

BusyTheme* busy_theme_alloc(void);

BusyTheme* busy_theme_alloc_default(void);

BusyTheme* busy_theme_alloc_set(const BusyTheme* other);

void busy_theme_free(BusyTheme* instance);

void busy_theme_set(BusyTheme* instance, const BusyTheme* other);

bool busy_theme_read(BusyTheme* instance, const char* name);

const char* busy_theme_get_name(const BusyTheme* instance);

void busy_theme_get_info(const BusyTheme* instance, BusyThemeInfo* info);

void busy_theme_set_default(BusyTheme* instance);

bool busy_theme_is_default(const BusyTheme* instance);

bool busy_theme_is_equal(const BusyTheme* instance, const BusyTheme* other);
