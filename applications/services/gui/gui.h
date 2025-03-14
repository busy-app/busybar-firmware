#pragma once

#include "widget.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_GUI "gui"

typedef enum {
    GuiDisplayIdFront,
    GuiDisplayIdBack,
    GuiDisplayIdMax,
} GuiDisplayId;

typedef enum {
    GuiLayerIdBottom,
    GuiLayerIdActive,
    GuiLayerIdTop,
    GuiLayerIdSystem,
    GuiLayerIdMax,
} GuiLayerId;

typedef struct Gui Gui;

void gui_lock(Gui* instance);

void gui_unlock(Gui* instance);

Widget* gui_get_root_widget(Gui* instance, GuiDisplayId display_id, GuiLayerId layer_id);

void gui_add_active_widget(Gui* instance, Widget* widget);

void gui_remove_active_widget(Gui* instance, Widget* widget);

#define with_gui(gui, code) \
    {                       \
        gui_lock(gui);      \
        {code};             \
        gui_unlock(gui);    \
    }

#ifdef __cplusplus
}
#endif
