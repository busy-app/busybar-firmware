/**
 * @file pause_overlay.h
 * @brief A widget that provides a dimmed pause overlay.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** PauseOverlay opaque structure. */
typedef struct PauseOverlay PauseOverlay;

/**
 * @brief Create a new PauseOverlay instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created PauseOverlay instance
 */
PauseOverlay* pause_overlay_alloc(Widget* parent);

/**
 * @brief Delete a PauseOverlay instance.
 *
 * @param[in,out] instance pointer to the PauseOverlay instance to be deleted
 */
void pause_overlay_free(PauseOverlay* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the PauseOverlay instance to be queried
 * @returns pointer to the base class instance
 */
Widget* pause_overlay_get_base(PauseOverlay* instance);

void pause_overlay_show(PauseOverlay* instance, bool show);

#ifdef __cplusplus
}
#endif
