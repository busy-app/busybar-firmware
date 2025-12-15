/**
 * @file prompt_overlay.h
 * @brief A widget that prompts the user to press the Start button.
 *
 * If set, can drive the position of the target widget in accordance to
 * the internally set animation.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** PromptOverlay opaque structure. */
typedef struct PromptOverlay PromptOverlay;

typedef void (*PromptOverlayCallback)(void* context);

/**
 * @brief Create a new PromptOverlay instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created PromptOverlay instance
 */
PromptOverlay* prompt_overlay_alloc(Widget* parent);

/**
 * @brief Delete a PromptOverlay instance.
 *
 * @param[in,out] instance pointer to the PromptOverlay instance to be deleted
 */
void prompt_overlay_free(PromptOverlay* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the PromptOverlay instance to be queried
 * @returns pointer to the base class instance
 */
Widget* prompt_overlay_get_base(PromptOverlay* instance);

/**
 * @brief Set a widget that will be moved according to the overlay animation.
 *
 * @param[in,out] instance pointer to the PromptOverlay instance to be modified
 * @param[in,out] widget pointer to the Widget instance to be animated
 */
void prompt_overlay_set_animation_target(PromptOverlay* instance, Widget* widget);

void prompt_overlay_set_callback(
    PromptOverlay* instance,
    PromptOverlayCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
