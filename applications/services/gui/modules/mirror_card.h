/**
 * @file mirror_card.h
 * @brief A widget that shows the front display mirrored content.
 *
 * Can be used only on the back display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** MirrorCard opaque structure. */
typedef struct MirrorCard MirrorCard;

/**
 * @brief Create a new MirrorCard instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created MirrorCard instance
 */
MirrorCard* mirror_card_alloc(Widget* parent);

/**
 * @brief Delete a MirrorCard instance.
 *
 * @param[in,out] instance pointer to the MirrorCard instance to be deleted
 */
void mirror_card_free(MirrorCard* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the MirrorCard instance to be queried
 * @returns pointer to the base class instance
 */
Widget* mirror_card_get_base(MirrorCard* instance);

/**
 * @brief Show or hide the top text
 *
 * @param[in,out] instance pointer to the MirrorCard instance to be modified
 * @param[in] show show the top text if @c true, hide if @c false
 */
void mirror_card_set_show_header(MirrorCard* instance, bool show);

/**
 * @brief Set the header text.
 *
 * @param[in,out] instance pointer to the MirrorCard instance to be modified
 * @param[in] header_text text to display in the header
 */
void mirror_card_set_header_text(MirrorCard* instance, const char* header_text);

/**
 * @brief Show or hide the footer
 *
 * @param[in,out] instance pointer to the MirrorCard instance to be modified
 * @param[in] show show the footer if @c true, hide if @c false
 */
void mirror_card_set_show_footer(MirrorCard* instance, bool show);

/**
 * @brief Set the footer primary text.
 *
 * @param[in,out] instance pointer to the MirrorCard instance to be modified
 * @param[in] primary_text text to display in the footer primary position
 */
void mirror_card_set_footer_primary_text(MirrorCard* instance, const char* primary_text);

/**
 * @brief Set the footer secondary text.
 *
 * @param[in,out] instance pointer to the MirrorCard instance to be modified
 * @param[in] secondary_text text to display in the footer secondary position
 */
void mirror_card_set_footer_secondary_text(MirrorCard* instance, const char* secondary_text);

#ifdef __cplusplus
}
#endif
