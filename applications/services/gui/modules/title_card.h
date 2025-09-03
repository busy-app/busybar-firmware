/**
 * @file title_card.h
 * @brief A widget that displays a title card with an icon and text title.
 */
#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/** TitleCard opaque structure. */
typedef struct TitleCard TitleCard;

/**
 * @brief Create a new TitleCard instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @returns pointer to the newly created TitleCard instance
 */
TitleCard* title_card_alloc(Widget* parent);

/**
 * @brief Delete a TitleCard instance.
 *
 * @param[in,out] instance pointer to the TitleCard instance to be deleted
 */
void title_card_free(TitleCard* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the TitleCard instance to be queried
 * @returns pointer to the base class instance
 */
Widget* title_card_get_base(TitleCard* instance);

/**
 * @brief Set the title card icon from a file.
 *
 * @param[in,out] instance pointer to the TitleCard instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to icon file
 */
void title_card_set_icon(TitleCard* instance, const char* file_path);

/**
 * @brief Set the title card text title.
 *
 * @param[in,out] instance pointer to the TitleCard instance to be modified
 * @param[in] title zero-terminated string containing the title text to be shown
 */
void title_card_set_title(TitleCard* instance, const char* title);

#ifdef __cplusplus
}
#endif
