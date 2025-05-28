/**
 * @file snap_image.h
 * @brief A widget that shows a snapshot of a display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SnapImage opaque structure. */
typedef struct SnapImage SnapImage;

typedef enum {
    SnapImageEffectBlur,
    SnapImageEffectDim,
    SnapImageEffectMax,
} SnapImageEffect;

/**
 * @brief Create a new SnapImage instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created SnapImage instance
 */
SnapImage* snap_image_alloc(Widget* parent);

/**
 * @brief Delete an SnapImage instance.
 *
 * @param[in,out] instance pointer to the SnapImage instance to be deleted
 */
void snap_image_free(SnapImage* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the SnapImage instance to be queried
 * @returns pointer to the base class instance
 */
Widget* snap_image_get_base(SnapImage* instance);

/**
 * @brief Set effect strength for a SnapImage instance.
 *
 * @note effects must be set before calling snap_image_capture_display()
 *
 * @param[in,out] instance pointer to the SnapImage instance to be operated on
 * @param[in] effect value from SnapImageEffect enumeration
 * @param[in] strength 0 - disable effect, 255 - enable fully
 */
void snap_image_set_effect(SnapImage* instance, SnapImageEffect effect, uint8_t strength);

/**
 * @brief Capture the display contents to a SnapImage instance.
 *
 * @param[in,out] instance pointer to the SnapImage instance to be operated on
 */
void snap_image_capture_display(SnapImage* instance);

#ifdef __cplusplus
}
#endif
