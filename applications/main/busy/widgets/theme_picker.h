/**
 * @file theme_picker.h
 * @brief A widget that provides a graphical theme picker.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ThemePickerCallback)(uint32_t index, void* context);

/** ThemePicker opaque structure. */
typedef struct ThemePicker ThemePicker;

/**
 * @brief Create a new ThemePicker instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created ThemePicker instance
 */
ThemePicker* theme_picker_alloc(Widget* parent);

/**
 * @brief Delete a ThemePicker instance.
 *
 * @param[in,out] instance pointer to the ThemePicker instance to be deleted
 */
void theme_picker_free(ThemePicker* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the ThemePicker instance to be queried
 * @returns pointer to the base class instance
 */
Widget* theme_picker_get_base(ThemePicker* instance);

void theme_picker_add_item(ThemePicker* instance, const char* image_path);

void theme_picker_set_callback(ThemePicker* instance, ThemePickerCallback callback, void* context);

#ifdef __cplusplus
}
#endif
