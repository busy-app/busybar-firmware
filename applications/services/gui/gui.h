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

/** GuiLayer opaque type declaration. */
typedef struct GuiLayer GuiLayer;

typedef struct GuiInputSubscription GuiInputSubscription;

typedef bool (*GuiInputCallback)(const InputEvent* event, void* context);

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
 * @brief
 */
GuiLayer* gui_get_layer(Gui* instance, GuiLayerId layer_id);

/**
 * @brief Get the root widget of a Layer on a certain display.
 *
 * A root widget cannot be moved, resized or used as the input event source.
 *
 * @param[in,out] layer pointer to the GuiLayer instance
 * @param[in] display_id identifier of the display required
 * @return pointer to the root widget
 */
Widget* gui_layer_get_root_widget(GuiLayer* layer, GuiDisplayId display_id);

/**
 * @brief
 */
GuiInputSubscription*
    gui_layer_subscribe_to_input_events(GuiLayer* layer, GuiInputCallback callback, void* context);

/**
 * @brief
 */
void gui_layer_unsubscribe_from_input_events(GuiLayer* layer, GuiInputSubscription* subscription);

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
