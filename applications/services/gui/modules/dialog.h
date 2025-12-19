/**
 * @file dialog.h
 * @brief Simple two options selection widget.
 */
#pragma once

#include <gui/widget.h>
#include <toolbox/color.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Dialog opaque structure. */
typedef struct Dialog Dialog;

/**
 * @brief Dialog callback function type.
 *
 * @param[in] selected option (0/1)
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*DialogCallback)(uint8_t result, void* context);

/**
 * @brief Create a new Dialog instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Dialog instance
 */
Dialog* dialog_alloc(Widget* parent);

/**
 * @brief Delete a Dialog instance.
 *
 * @param[in,out] instance pointer to the Dialog instance to be deleted
 */
void dialog_free(Dialog* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the Dialog instance to be queried
 * @returns pointer to the base class instance
 */
Widget* dialog_get_base(Dialog* instance);

/**
 * @brief Set a dialog selection callback.
 *
 * @param[in,out] instance pointer to the Dialog instance to be modified
 * @param[in] callback callback function pointer
 * @param[in] callback callback context
 */
void dialog_set_calback(Dialog* instance, DialogCallback callback, void* context);

/**
 * @brief Set a text label value.
 *
 * @param[in,out] instance pointer to the Dialog instance to be modified
 * @param[in] text zero-terminated string containing the text
 */
void dialog_set_text(Dialog* instance, const char* text);

/**
 * @brief Set options text.
 *
 * @param[in,out] instance pointer to the Dialog instance to be modified
 * @param[in] text_0 text of the first option
 * @param[in] text_1 text of the second option
 */
void dialog_set_options(Dialog* instance, const char* text_0, const char* text_1);

/**
 * @brief Set options color.
 *
 * @param[in,out] instance pointer to the Dialog instance to be modified
 * @param[in] color_0 color of the first option
 * @param[in] color_1 color of the second option
 */
void dialog_set_option_colors(Dialog* instance, Color color_0, Color color_1);

#ifdef __cplusplus
}
#endif
