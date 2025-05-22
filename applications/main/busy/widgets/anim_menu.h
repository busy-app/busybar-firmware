/**
 * @file anim_menu.h
 * @brief A widget that shows an advanced animated menu.
 *
 * Currently, only two items are supported.
 */
#pragma once

#include <gui/modules/anim_image.h>

#ifdef __cplusplus
extern "C" {
#endif

/** AnimMenu opaque structure. */
typedef struct AnimMenu AnimMenu;

/**
 * @brief AnimMenu item callback function type.
 *
 * @param[in] index index of the clicked item
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*AnimMenuCallback)(uint32_t index, void* context);

/**
 * @brief Create a new AnimMenu instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created AnimMenu instance
 */
AnimMenu* anim_menu_alloc(Widget* parent);

/**
 * @brief Delete a AnimMenu instance.
 *
 * @param[in,out] instance pointer to the AnimMenu instance to be deleted
 */
void anim_menu_free(AnimMenu* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the AnimMenu instance to be queried
 * @returns pointer to the base class instance
 */
Widget* anim_menu_get_base(AnimMenu* instance);

/**
 * @brief Get a pointer to the underlying AnimImage instance.
 *
 * The return value can be used in all AnimImage methods.
 *
 * @param[in,out] instance pointer to the AnimMenu instance to be queried
 * @returns pointer to the AnimImage instance
 */
AnimImage* anim_menu_get_anim_image(AnimMenu* instance);

/**
 * @brief Set the animation file for the AnimMenu instance.
 *
 * @param[in,out] instance pointer to the AnimMenu instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to animation file
 * @param[in] idle_frames number of frames in the idle sequence
 * @param[in] transition_frames number of frames in the transition sequence
 * @returns true if the source was successfully set, false otherwise
 */
bool anim_menu_set_source(
    AnimMenu* instance,
    const char* file_path,
    uint32_t idle_frames,
    uint32_t transition_frames);

/**
 * @brief Set a function to be called when an item has been clicked.
 *
 * @param[in,out] instance pointer to the AnimMenu instance to be modified
 * @param[in] callback pointer to the function to be called when the item is clicked
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 */
void anim_menu_set_callback(AnimMenu* instance, AnimMenuCallback callback, void* context);

#ifdef __cplusplus
}
#endif
