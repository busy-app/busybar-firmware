/**
 * @file anim_image.h
 * @brief A widget that displays an animated image.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** AnimImage opaque structure. */
typedef struct AnimImage AnimImage;

/**
 * @brief Create a new AnimImage instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created AnimImage instance
 */
AnimImage* anim_image_alloc(Widget* parent);

/**
 * @brief Delete an AnimImage instance.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be deleted
 */
void anim_image_free(AnimImage* instance);

/**
 * @brief Load and show an animation from a file.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to animation file
 * @returns true if the source was successfully set, false otherwise
 */
bool anim_image_set_source(AnimImage* instance, const char* file_path);

/**
 * @brief Start playing the animation.
 *
 * The animation file MUST be loaded using anim_image_set_source()
 * before calling this function.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be started
 */
void anim_image_start(AnimImage* instance);

/**
 * @brief Stop a playing animation.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be stopped
 */
void anim_image_stop(AnimImage* instance);

#ifdef __cplusplus
}
#endif
