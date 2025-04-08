/**
 * @file lottie_animation.h
 * @brief A widget that can play Lottie animations.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** LottieAnimation opaque structure. */
typedef struct LottieAnimation LottieAnimation;

/**
 * @brief Create a new LottieAnimation instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created LottieAnimation instance
 */
LottieAnimation* lottie_animation_alloc(Widget* parent);

/**
 * @brief Delete a LottieAnimation instance.
 *
 * @param[in,out] instance pointer to the LottieAnimation instance to be deleted
 */
void lottie_animation_free(LottieAnimation* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the LottieAnimation instance to be queried
 * @returns pointer to the base class instance
 */
Widget* lottie_animation_get_base(LottieAnimation* instance);

/**
 * @brief Load and show a Lottie animation from a file.
 *
 * @param[in,out] instance pointer to the LottieAnimation instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to animation file
 * @returns true if the source was successfully set, false otherwise
 */
bool lottie_animation_set_source(LottieAnimation* instance, const char* file_path);

/**
 * @brief Override a slot in a Lottie animation.
 *
 * Documentation on Lottie slots: https://lottie.github.io/lottie-spec/dev/specs/helpers/#slots
 *
 * @param[in,out] instance pointer to the LottieAnimation instance to be modified
 * @param[in] slot_str zero-terminated string containing the JSON object to override the slot with
 * @returns true if the slot was successfully overridden, false otherwise
 */
bool lottie_animation_override_slot(LottieAnimation* instance, const char* slot_str);

#ifdef __cplusplus
}
#endif
