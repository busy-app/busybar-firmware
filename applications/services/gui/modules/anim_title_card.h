/**
 * @file anim_title_card.h
 * @brief An animated title card widget with icon and text animations.
 */
#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/** AnimTitleCard opaque structure. */
typedef struct AnimTitleCard AnimTitleCard;

/**
 * @brief Create a new AnimTitleCard instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @returns pointer to the newly created AnimTitleCard instance
 */
AnimTitleCard* anim_title_card_alloc(Widget* parent);

/**
 * @brief Delete an AnimTitleCard instance.
 *
 * @param[in,out] instance pointer to the AnimTitleCard instance to be deleted
 */
void anim_title_card_free(AnimTitleCard* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the AnimTitleCard instance to be queried
 * @returns pointer to the base class instance
 */
Widget* anim_title_card_get_base(AnimTitleCard* instance);

/**
 * @brief Set the title card icon from a file.
 *
 * @param[in,out] instance pointer to the AnimTitleCard instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to icon file
 */
void anim_title_card_set_icon(AnimTitleCard* instance, const char* file_path);

/**
 * @brief Set the title card text title.
 *
 * @param[in,out] instance pointer to the AnimTitleCard instance to be modified
 * @param[in] title zero-terminated string containing the title text to be shown
 */
void anim_title_card_set_title(AnimTitleCard* instance, const char* title);

/**
 * @brief Run the background animation.
 *
 * @param[in,out] instance pointer to the AnimTitleCard instance to be animated
 */
void anim_title_card_run_background_anim(AnimTitleCard* instance);

/**
 * @brief Run the icon animation between specified frame ranges.
 *
 * @param[in,out] instance pointer to the AnimTitleCard instance to be animated
 * @param[in] section Named section to play back
 */
void anim_title_card_run_icon_anim(AnimTitleCard* instance, const char* section);

/**
 * @brief Run the title text animation with positioning and duration.
 *
 * @param[in,out] instance pointer to the AnimTitleCard instance to be animated
 * @param[in] start starting position for the title animation
 * @param[in] stop ending position for the title animation
 * @param[in] duration animation duration in milliseconds
 */
void anim_title_card_run_title_anim(
    AnimTitleCard* instance,
    int32_t start,
    int32_t stop,
    uint32_t duration);

#ifdef __cplusplus
}
#endif
