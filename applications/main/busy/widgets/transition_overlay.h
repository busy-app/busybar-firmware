/**
 * @file transition_overlay.h
 * @brief
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TransitionOverlayColorModeOff,
    TransitionOverlayColorModeNormal,
    TransitionOverlayColorModeMultiply,
    TransitionOverlayColorModeAdd,
    TransitionOverlayColorModeMax,
} TransitionOverlayColorMode;

typedef enum {
    TransitionOverlayMaskModeOff,
    TransitionOverlayMaskModeMultiply,
    TransitionOverlayMaskModeAdd,
    TransitionOverlayMaskModeMax,
} TransitionOverlayMaskMode;

/** TransitionOverlay opaque structure. */
typedef struct TransitionOverlay TransitionOverlay;

/**
 * @brief Create a new TransitionOverlay instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created TransitionOverlay instance
 */
TransitionOverlay* transition_overlay_alloc(Widget* parent);

/**
 * @brief Delete a TransitionOverlay instance.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be deleted
 */
void transition_overlay_free(TransitionOverlay* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be queried
 * @returns pointer to the base class instance
 */
Widget* transition_overlay_get_base(TransitionOverlay* instance);

/**
 * @brief Set the animation segment times for a TransitionOverlay instance.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be modified
 * @param[in] in_ms time for the fade-in animation segment, in milliseconds
 * @param[in] out_ms time for the fade-out animation segment, in milliseconds
 */
void transition_overlay_set_timings(TransitionOverlay* instance, uint32_t in_ms, uint32_t out_ms);

/**
 * @brief Set the overlay color for a TransitionOverlay instance.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be modified
 * @param[in] color color value for the overlay transition effect
 */
void transition_overlay_set_color(TransitionOverlay* instance, Color color);

/**
 * @brief Set the color blending mode for a TransitionOverlay instance.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be modified
 * @param[in] mode blending mode for the color overlay effect
 */
void transition_overlay_set_color_mode(
    TransitionOverlay* instance,
    TransitionOverlayColorMode mode);

/**
 * @brief Set the animated overlay mask for a TransitionOverlay instance.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be modified
 * @param[in] file_path full path to the mask animation file
 */
void transition_overlay_set_mask(TransitionOverlay* instance, const char* file_path);

/**
 * @brief Set the mask blending mode for a TransitionOverlay instance.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be modified
 * @param[in] mode blending mode for the mask overlay effect
 */
void transition_overlay_set_mask_mode(TransitionOverlay* instance, TransitionOverlayMaskMode mode);

/**
 * @brief Capture the current display contents and show the overlay.
 *
 * A static image will be shown until transition_overlay_start() is called.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be shown
 */
void transition_overlay_show(TransitionOverlay* instance);

/**
 * @brief Start the transition animation.
 *
 * The overlay will be automatically hidden upon completion of the animation.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be started
 */
void transition_overlay_start(TransitionOverlay* instance);

#ifdef __cplusplus
}
#endif
