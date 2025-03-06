/**
 * @file submenu.h
 * GUI: SubMenu view module API
 */
#pragma once

#include <gui/view_port.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Submenu opaque structure */
typedef struct Submenu Submenu;

/** Submenu item callback */
typedef void (*SubmenuItemCallback)(uint32_t index, void* context);

/** Allocate and initialize submenu 
 * 
 * This submenu is used to select one option
 *
 * @return     Submenu instance
 */
Submenu* submenu_alloc(ViewPort* view_port);

/** Deinitialize and free submenu
 *
 * @param      submenu  Submenu instance
 */
void submenu_free(Submenu* submenu);

/** Add item to submenu
 *
 * @param      submenu           Submenu instance
 * @param      label             menu item label
 * @param      index             menu item index used for callback, does not have to be unique
 * @param      callback          menu item callback
 * @param      context           menu item callback context
 */
void submenu_add_item(
    Submenu* submenu,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* context);

/** Remove all items from submenu
 *
 * @param      submenu  Submenu instance
 */
void submenu_reset(Submenu* submenu);

/** Get submenu selected item index
 *
 * @param      submenu  Submenu instance
 *
 * @return     Index of the selected item
 */
uint32_t submenu_get_selected_item(Submenu* submenu);

/** Set submenu selected item by index
 *
 * @param      submenu  Submenu instance
 * @param      index    The index of the selected item
 */
void submenu_set_selected_item(Submenu* submenu, uint32_t index);

/** Set optional header for submenu
 *
 * @param      submenu  Submenu instance
 * @param      header   header to set
 */
void submenu_set_header(Submenu* submenu, const char* header);

#ifdef __cplusplus
}
#endif
