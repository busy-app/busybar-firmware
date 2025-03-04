#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_GUI_LVGL "gui"

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

void gui_acquire(Gui* instance);

void gui_release(Gui* instance);

lv_display_t* gui_get_display(Gui* instance, GuiDisplayId display_id);

lv_obj_t* gui_get_layer(Gui* instance, GuiDisplayId display_id, GuiLayerId layer_id);

#define with_gui(gui, code) \
    {                       \
        gui_acquire(gui);   \
        {code};             \
        gui_release(gui);   \
    }

#define with_gui_layer(gui, display_id, layer_id, code)             \
    {                                                               \
        gui_acquire(gui);                                           \
        lv_obj_t* layer = gui_get_layer(gui, display_id, layer_id); \
        {code};                                                     \
        gui_release(gui);                                           \
    }

#ifdef __cplusplus
}
#endif
