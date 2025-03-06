#pragma once

#include <lvgl.h>

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
