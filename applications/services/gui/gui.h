#pragma once

#include <lvgl.h>

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
    GuiInputIdEncoder,
    GuiInputIdButtons,
    GuiInputIdMax,
} GuiInputId;

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

void gui_set_active_widget(Gui* instance, Widget* widget);

lv_obj_t* gui_get_layer(Gui* instance, GuiDisplayId display_id, GuiLayerId layer_id);

#define with_gui(gui, code) \
    {                       \
        gui_lock(gui);      \
        {code};             \
        gui_unlock(gui);    \
    }

#define with_gui_layer(gui, display_id, layer_id, code)             \
    {                                                               \
        gui_lock(gui);                                              \
        lv_obj_t* layer = gui_get_layer(gui, display_id, layer_id); \
        {code};                                                     \
        gui_unlock(gui);                                            \
    }

#ifdef __cplusplus
}
#endif
