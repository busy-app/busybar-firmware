#pragma once

#include "smenu_item.h"
#include "../../widget_i.h"

#define SMENU_ITEM_RIGHT_FLEX_GROW_FLAG          LV_OBJ_FLAG_WIDGET_1
#define SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M (1000)

#ifdef __cplusplus
extern "C" {
#endif

struct SMenuItem {
    Widget base;

    lv_obj_t* cursor_label;
    lv_obj_t* left_container;
    lv_obj_t* right_container;

    SMenuItemCallback callback;

    bool do_draw_fader;
    bool is_active;
};

extern const lv_obj_class_t smenu_item_lvgl_class;
extern const lv_anim_t smenu_item_long_text_anim_template;

#ifdef __cplusplus
}
#endif
