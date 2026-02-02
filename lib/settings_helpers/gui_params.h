/**
 * @brief
 */

#pragma once

#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAIN_SETTINGS_APP "settings_menu"

#define SETTINGS_NAV_BAR_HEIGHT 16

#define SETTINGS_ASSETS_PATH(path) EXT_PATH("apps_assets/settings") "/" path
#define SETTINGS_IMG_PATH(path)    SETTINGS_ASSETS_PATH("images") "/" path
#define SETTINGS_ANIM_PATH(path)   SETTINGS_ASSETS_PATH("animations") "/" path
#define SETTINGS_ICON_BACK         SETTINGS_IMG_PATH("settings_back_7x7.bin")

#ifdef __cplusplus
}
#endif
