/**
 * @file theme_picker.h
 * @brief A widget that provides a graphical theme picker.
 */
#pragma once

#include <gui/widget.h>

#include "../helpers/theme_picker_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ThemePicker opaque structure. */
typedef struct ThemePicker ThemePicker;

/**
 * @brief ThemePickerCallback function type.
 *
 * @param[in] index numeric index of the currently selected theme
 * @param[in,out] context pointer to the user-specific object
 */
typedef void (*ThemePickerCallback)(uint32_t index, void* context);

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

/**
 * @brief Set the function to be called upon theme selection.
 *
 * @param[in,out] instance pointer to the ThemePicker instance to be modified
 * @param[in] callback pointer to the function to be called upon theme selection
 * @param[in,out] context pointer to a user-specified object (will be passed to the callback)
 */
void theme_picker_set_callback(ThemePicker* instance, ThemePickerCallback callback, void* context);

/**
 * @brief Set the model to show in this ThemePicker instance.
 *
 * @param[in,out] instance pointer to the ThemePicker instance to be modified
 * @param[in] model pointer to a ThemePickerModel instance to be shown
 */
void theme_picker_set_model(ThemePicker* instance, const ThemePickerModel* model);

/**
 * @brief Set the currently selected item index.
 *
 * @pre theme_picker_set_model() MUST be called before calling this function.
 *
 * @param[in,out] instance pointer to the ThemePicker instance to be modified
 * @param[in] index numeric index of the desired item
 */
void theme_picker_set_current_item(ThemePicker* instance, uint32_t index);

#ifdef __cplusplus
}
#endif
