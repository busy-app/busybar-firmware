#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_GUI_LVGL "gui_lvgl"

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

typedef struct GuiLvgl GuiLvgl;

void gui_lvgl_acquire(GuiLvgl* instance);

void gui_lvgl_release(GuiLvgl* instance);

lv_display_t* gui_lvgl_get_display(GuiLvgl* instance, GuiDisplayId display_id);

lv_obj_t* gui_lvgl_get_layer(GuiLvgl* instance, GuiDisplayId display_id, GuiLayerId layer_id);

#define with_gui(gui, code)    \
    {                          \
        gui_lvgl_acquire(gui); \
        {code};                \
        gui_lvgl_release(gui); \
    }

#define with_gui_layer(gui, display_id, layer_id, code)                  \
    {                                                                    \
        gui_lvgl_acquire(gui);                                           \
        lv_obj_t* layer = gui_lvgl_get_layer(gui, display_id, layer_id); \
        {code};                                                          \
        gui_lvgl_release(gui);                                           \
    }

#ifdef __cplusplus
}
#endif
