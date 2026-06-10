/**
 * @file menu_base.h
 * @brief Base widget class for all menu-type widgets.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** MenuBase opaque structure. */
typedef struct MenuBase MenuBase;

/**
 * @brief Create a new MenuBase instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @returns pointer to the newly created MenuBase instance
 */
MenuBase* menu_base_alloc(Widget* parent);

/**
 * @brief Delete a MenuBase instance.
 *
 * @param[in,out] instance pointer to the MenuBase instance to be deleted
 */
void menu_base_free(MenuBase* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the MenuBase instance to be queried
 * @returns pointer to the base class instance
 */
Widget* menu_base_get_base(MenuBase* instance);

#ifdef __cplusplus
}
#endif
