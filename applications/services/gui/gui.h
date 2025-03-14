/**
 * @file gui.h
 * @brief GUI system APIs.
 */
#pragma once

#include "widget.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_GUI "gui"

/** Enumeration of available display identifiers. */
typedef enum {
    GuiDisplayIdFront, /**< Front (main display) */
    GuiDisplayIdBack, /**< Back (greyscale) display */
    GuiDisplayIdMax, /**< Special value, equal to display number */
} GuiDisplayId;

/** Enumeration of available layer indentifiers. */
typedef enum {
    GuiLayerIdBottom, /**< Bottom layer - visible only if there is nothing on layers above it */
    GuiLayerIdMain, /**< Main layer - for displaying regular applications */
    GuiLayerIdTop, /**< Top layer - for displaying dialog windows and overlays */
    GuiLayerIdSystem, /**< System layer - for displaying statuses and other persistent info */
    GuiLayerIdMax,
} GuiLayerId;

/** Gui opaque type declaration. */
typedef struct Gui Gui;

/**
 * @brief Lock the GUI system.
 *
 * @warning This function MUST be called BEFORE ANY Gui-related function calls.
 *          Examples include, but not limited to: creating, deleting and modifying of Widgets,
 *          adding or removing of active widgets, etc.
 *          Failure to do so will result in possible instabilities and crashes.
 *
 * @param[in,out] instance pointer to the Gui instance to lock
 */
void gui_lock(Gui* instance);

/**
 * @brief Unlock the GUI system.
 *
 * @warning This function MUST be called AFTER calling gui_lock() and performing the
 *          required Gui-related calls. Failure to do so will result in Gui lockup.
 *
 * @param[in,out] instance pointer to the Gui instance to unlock
 */
void gui_unlock(Gui* instance);

/**
 * @brief Get the root widget of a certain Display and Layer.
 *
 * A root widget cannot be moved, resized or used as the input event source.
 *
 * @param[in,out] instance pointer to the Gui instance
 * @param[in] display_id identifier of the display required
 * @param[in] layer_id identifier of the layer required
 * @return pointer to the root widget
 */
Widget* gui_get_root_widget(Gui* instance, GuiDisplayId display_id, GuiLayerId layer_id);

/**
 * @brief Add a widget to the top of the active stack.
 *
 * The widget on top of the active stack will receive input events until:
 * - It is deleted,
 * - It is removed using gui_remove_active_widget(),
 * - Another Widget has been put to the top position using this function.
 *
 * If the widget has lost its place on the top, calling this function again
 * will make it receive the input events once more.
 *
 * @note There can be only ONE Widget that receives input per display
 *       at each point in time.
 *
 * @param[in,out] instance pointer to the Gui instance
 * @param[in,out] widget pointer to the Widget instance to add to active stack
 */
void gui_add_active_widget(Gui* instance, Widget* widget);

/**
 * @brief Remove a widget from the active stack.
 *
 * Removing a Widget from the active stack will prevent it from
 * receiving the input events (assuming it was on the top).
 *
 * If a Widget on the top position has been removed or deleted,
 * the next Widget will take its place, provided that the stack
 * was not empty.
 *
 * @note It is possible to remove a Widget even when it is not on
 *       the top position.
 *
 * @param[in,out] instance pointer to the Gui instance
 * @param[in,out] widget pointer to the Widget instance to remove from active stack
 */
void gui_remove_active_widget(Gui* instance, Widget* widget);

/**
 * @brief Shorthand for automatically locking and unlocking the GUI.
 *
 * @param[in,out] gui pointer to the Gui instance
 * @param[in] code the code to execute when the Gui will be in locked state
 */
#define with_gui(gui, code) \
    {                       \
        gui_lock(gui);      \
        {code};             \
        gui_unlock(gui);    \
    }

#ifdef __cplusplus
}
#endif
