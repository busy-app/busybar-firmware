#include "transition_overlay.h"

#include <gui/widget_i.h>
#include <gui/modules/snap_image.h>

#define MY_CLASS (&transition_overlay_lvgl_class)

#define ANIM_DURATION_MS (300)

struct TransitionOverlay {
    Widget base;
    SnapImage* snap;
    lv_obj_t* dimmer;
};

const lv_obj_class_t transition_overlay_lvgl_class;

// LVGL-specific code

static void transition_overlay_lvgl_anim_exec_callback(lv_anim_t* anim, int32_t value) {
    furi_assert(anim);

    TransitionOverlay* instance = anim->var;

    if(value == anim->end_value) {
        lv_obj_add_flag((lv_obj_t*)instance->snap, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_set_style_bg_opa(instance->dimmer, value, LV_PART_MAIN);
}

static void transition_overlay_lvgl_anim_completed_callback(lv_anim_t* anim) {
    furi_assert(anim);

    TransitionOverlay* instance = anim->var;
    furi_assert(instance);

    lv_obj_add_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag((lv_obj_t*)instance->snap, LV_OBJ_FLAG_HIDDEN);
}

static void transition_overlay_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

    TransitionOverlay* instance = (TransitionOverlay*)obj;
    instance->snap = snap_image_alloc((Widget*)obj);
    instance->dimmer = lv_obj_create(obj);

    lv_obj_set_size(instance->dimmer, LV_PCT(100), LV_PCT(100));
}

// Public API

TransitionOverlay* transition_overlay_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    TransitionOverlay* instance = (TransitionOverlay*)obj;
    return instance;
}

void transition_overlay_free(TransitionOverlay* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* transition_overlay_get_base(TransitionOverlay* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void transition_overlay_set_color(TransitionOverlay* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_bg_color(instance->dimmer, TO_LV_COLOR(color), LV_PART_MAIN);
}

void transition_overlay_show(TransitionOverlay* instance) {
    furi_check(instance);

    snap_image_capture_display(instance->snap);

    lv_obj_set_style_bg_opa(instance->dimmer, LV_OPA_TRANSP, LV_PART_MAIN);

    widget_set_visible((Widget*)instance, true);
}

void transition_overlay_start(TransitionOverlay* instance) {
    furi_check(instance);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim, ANIM_DURATION_MS / 2);
    lv_anim_set_reverse_duration(&anim, ANIM_DURATION_MS / 2);

    lv_anim_set_custom_exec_cb(&anim, transition_overlay_lvgl_anim_exec_callback);
    lv_anim_set_completed_cb(&anim, transition_overlay_lvgl_anim_completed_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

// LVGL class descriptor

const lv_obj_class_t transition_overlay_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = transition_overlay_lvgl_constructor,
    .name = "widget-transition-overlay",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(TransitionOverlay),
};
