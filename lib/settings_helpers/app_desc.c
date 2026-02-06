#include "app_desc.h"

SettingsAppDescriptor* settings_app_descriptor_alloc(void) {
    SettingsAppDescriptor* desc = malloc(sizeof(SettingsAppDescriptor));
    desc->front_title = furi_string_alloc();
    desc->back_title = furi_string_alloc();
    desc->menu_extra = furi_string_alloc();
    desc->front_icon = furi_string_alloc();
    desc->back_icon = furi_string_alloc();
    desc->display_in_menu = true;

    return desc;
}

void settings_app_descriptor_free(SettingsAppDescriptor* desc) {
    furi_string_free(desc->front_title);
    furi_string_free(desc->back_title);
    furi_string_free(desc->menu_extra);
    furi_string_free(desc->front_icon);
    furi_string_free(desc->back_icon);
    free(desc);
}

void settings_app_descriptor_reset(SettingsAppDescriptor* desc) {
    furi_string_reset(desc->front_title);
    furi_string_reset(desc->back_title);
    furi_string_reset(desc->menu_extra);
    furi_string_reset(desc->front_icon);
    furi_string_reset(desc->back_icon);
    desc->display_in_menu = true;
}
