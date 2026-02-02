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
    TransitionOverlayTypeColor,
    TransitionOverlayTypeMask,
    TransitionOverlayTypeMax,
} TransitionOverlayType;

typedef enum {
    TransitionOverlayBlendModeNormal,
    TransitionOverlayBlendModeMultiply,
    TransitionOverlayBlendModeAdd,
    TransitionOverlayBlendModeMax,
} TransitionOverlayBlendMode;

typedef enum {
    TransitionOverlayEffectNone,
    TransitionOverlayEffectPress,
    TransitionOverlayEffectMax,
} TransitionOverlayEffect;

typedef struct {
    uint32_t in_ms;
    uint32_t out_ms;
} TransitionOverlayTimings;

typedef struct {
    TransitionOverlayType type;
    TransitionOverlayBlendMode blend_mode;
    TransitionOverlayTimings timings;
    TransitionOverlayEffect effect;
    union {
        Color color;
        const char* file_path;
    } mask;
} TransitionOverlayPreset;

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
 * @brief
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be modified
 * @param[in] preset
 */
void transition_overlay_set_preset(
    TransitionOverlay* instance,
    const TransitionOverlayPreset* preset);

/**
 * @brief Set the widget to participate in the press effect for a TransitionOverlay instance.
 *
 * @param[in,out] instance pointer to the TransitionOverlay instance to be modified
 * @param[in,out] widget pointer to the widget to participate in the effect
 */
void transition_overlay_set_pressed_widget(TransitionOverlay* instance, Widget* widget);

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
