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
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be queried
 * @returns pointer to the base class instance
 */
Widget* anim_image_get_base(AnimImage* instance);

/**
 * @brief Load and show an animation from a file.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to animation file
 * @returns true if the source was successfully set, false otherwise
 */
bool anim_image_set_source(AnimImage* instance, const char* file_path);

/**
 * @brief Set the range of frames to play.
 *
 * By default, the range to play is (0, number of frames - 1) and the looping is on.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be modified
 * @param[in] begin the number (index) of the first frame, starting from 0
 * @param[in] end the number (index) of the last frame, inclusive
 * @param[in] loop loop the range if true, play once otherwise
 * @param[in] wait_end wait for the end of the previous range if true, start immediately otherwise
 */
void anim_image_set_range(
    AnimImage* instance,
    uint32_t begin,
    uint32_t end,
    bool loop,
    bool wait_end);

/**
 * @brief Set the looping of the current frame range.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be modified
 * @param[in] set loop the current range if @c true, do not loop if @c false
 */
void anim_image_set_loop(AnimImage* instance, bool set);

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

/**
 * @brief Set the animation to the beginning of the current range.
 *
 * @note Rewinding will cancel a waiting range if one is present.
 *
 * @param[in,out] instance pointer to the AnimImage instance to be rewound
 */
void anim_image_rewind(AnimImage* instance);

/**
 * @brief Get the frame rate of the animation.
 *
 * @param[in] instance pointer to the AnimImage instance to be queried
 * @returns frame rate in frames per second
 */
uint32_t anim_image_get_frame_rate(const AnimImage* instance);

/**
 * @brief Get the total number of frames in the animation.
 *
 * @param[in] instance pointer to the AnimImage instance to be queried
 * @returns total number of frames in the animation
 */
uint32_t anim_image_get_frame_count(const AnimImage* instance);

#ifdef __cplusplus
}
#endif
