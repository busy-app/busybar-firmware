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

#ifdef __cplusplus
}
#endif
