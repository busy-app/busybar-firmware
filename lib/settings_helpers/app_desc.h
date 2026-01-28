/**
 * @brief Interface between `settings_menu` application and all applications
 * categorized as `SETTINGS`.
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

/**
 * @brief Dynamic information about a settings submenu.
 * 
 * If you're a `SETTINGS` app and you're given a non-NULL argument, it's a
 * pointer to this structure. You must fill it with data as quickly as possible
 * and exit immediately. This data will be shown in the main settings menu.
 */
typedef struct {
    FuriString* front_title; //<! Title on front display
    FuriString* back_title; //<! Title on back display
    FuriString* menu_extra; //<! Sub-label for menus on both displays

    FuriString* front_icon; //<! Path to icon on front display
    FuriString* back_icon; //<! Path to icon on back display

    bool display_in_menu; //<! Show application in settings menu
} SettingsAppDescriptor;

SettingsAppDescriptor* settings_app_descriptor_alloc(void);
void settings_app_descriptor_free(SettingsAppDescriptor* desc);

void settings_app_descriptor_reset(SettingsAppDescriptor* desc);

#ifdef __cplusplus
}
#endif
