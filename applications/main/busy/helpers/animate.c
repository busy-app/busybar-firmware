#include "animate.h"

#include <gui/widget_i.h>

#include <furi.h>

static void animate_pos_y_animation_callback(void* context, int32_t value) {
    furi_assert(context);

    Widget* target = context;
    widget_set_pos_y(target, value);
}

void animate_pos_y(Widget* target, int32_t from, int32_t to, uint32_t time_ms) {
    furi_assert(target);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, from, to);
    lv_anim_set_delay(&anim, 10);
    lv_anim_set_duration(&anim, time_ms);
    lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
    lv_anim_set_exec_cb(&anim, animate_pos_y_animation_callback);
    lv_anim_set_bezier3_param(
        &anim,
        LV_BEZIER_VAL_FLOAT(0.5F),
        LV_BEZIER_VAL_FLOAT(0.0F),
        LV_BEZIER_VAL_FLOAT(0.5F),
        LV_BEZIER_VAL_FLOAT(2.5F));

    lv_anim_set_var(&anim, target);
    lv_anim_start(&anim);
}
