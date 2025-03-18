/**
 * @file gui.h
 * @brief GUI system APIs.
 */
#pragma once

#include <input/input.h>

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
    GuiLayerIdSystem, /**< System layer - for displaying statuses and other persistent info */
    GuiLayerIdTop, /**< Top layer - for displaying dialog windows and overlays */
    GuiLayerIdMain, /**< Main layer - for displaying regular applications */
    GuiLayerIdBottom, /**< Bottom layer - visible only if there is nothing on layers above it */
    GuiLayerIdMax, /**< Special value, not to be used in application code */
} GuiLayerId;

/** Gui opaque type declaration. */
typedef struct Gui Gui;

typedef struct GuiInputSubscription GuiInputSubscription;

typedef void (*GuiInputCallback)(const InputEvent* event, void* context);

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

GuiInputSubscription*
    gui_subscribe_to_input_events(Gui* instance, GuiInputCallback callback, void* context);

void gui_unsubscribe_from_input_events(Gui* instance, GuiInputSubscription* subscription);

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
