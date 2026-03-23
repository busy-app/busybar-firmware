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

#define SETTINGS_APP_DESCRIPTOR_MAGIC 0x00AA5500

/**
 * @brief Dynamic information about a settings submenu.
 *
 * If you're a `SETTINGS` app, you're given a non-NULL argument and first 4 bytes equal to
 * SETTINGS_APP_DESCRIPTOR_MAGIC, it's a pointer to this structure.
 * You must fill it with data as quickly as possible and exit immediately.
 * This data will be shown in the main settings menu.
 */
typedef struct {
    uint32_t
        magic; //<! Magic value to identify descriptor is not a string, set to SETTINGS_APP_DESCRIPTOR_MAGIC

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

bool settings_app_descriptor_is_valid(void* arg);

#ifdef __cplusplus
}
#endif
