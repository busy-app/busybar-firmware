/**
 * @brief Interface between `settings_menu` application and all applications
 * categorized as `SETTINGS`.
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SETTINGS_DESCRIPTOR_TITLE_SZ (64)
#define SETTINGS_DESCRIPTOR_PATH_SZ  (128)

/**
 * @brief Dynamic information about a settings submenu.
 * 
 * If you're a `SETTINGS` app and you're given a non-NULL argument, it's a
 * pointer to this structure. You must fill it with data as quickly as possible
 * and exit immediately. This data will be shown in the main settings menu.
 */
typedef struct {
    char front_title[SETTINGS_DESCRIPTOR_TITLE_SZ]; //<! Title on front display
    char back_title[SETTINGS_DESCRIPTOR_TITLE_SZ]; //<! Title on back display
    char menu_extra[SETTINGS_DESCRIPTOR_TITLE_SZ]; //<! Sub-label for menus on both screens

    char front_icon[SETTINGS_DESCRIPTOR_PATH_SZ]; //<! Path to icon on front display
    char back_icon[SETTINGS_DESCRIPTOR_PATH_SZ]; //<! Path to icon on back display
} SettingsAppDescriptor;

#ifdef __cplusplus
}
#endif
