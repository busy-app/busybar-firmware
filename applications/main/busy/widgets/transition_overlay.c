#include "transition_overlay.h"

#include <gui/widget_i.h>

#include <gui/modules/anim_image.h>
#include <gui/modules/snap_image.h>

#define MY_CLASS (&transition_overlay_lvgl_class)

#define PRESS_OFFSET_Y_START (0)
#define PRESS_OFFSET_Y_END   (3)

#define MASK_ANIM_START (0)
#define MASK_ANIM_END   (1)

struct TransitionOverlay {
    Widget base;
    SnapImage* snap;
    AnimImage* mask;
    lv_obj_t* color;
    Widget* press_widget;
    struct {
        uint32_t in_ms;
        uint32_t out_ms;
    } timings;
    TransitionOverlayColorMode color_mode;
    TransitionOverlayMaskMode mask_mode;
    bool enable_press_effect;
};

const lv_obj_class_t transition_overlay_lvgl_class;

// LVGL-specific code

static void transition_overlay_lvgl_anim_exec_callback(lv_anim_t* anim, int32_t value) {
    furi_assert(anim);

    TransitionOverlay* instance = anim->var;

    if(anim->end_value == value) {
        widget_set_visible((Widget*)instance->snap, false);
    }

    if(anim->user_data == instance->color) {
        lv_obj_set_style_bg_opa(instance->color, value, LV_PART_MAIN);
    }
}

static void transition_overlay_lvgl_anim_completed_callback(lv_anim_t* anim) {
    furi_assert(anim);

    TransitionOverlay* instance = anim->var;
    furi_assert(instance);

    widget_set_visible((Widget*)instance->snap, true);
    widget_set_visible((Widget*)instance, false);
}

static void transition_overlay_lvgl_press_anim_exec_callback(lv_anim_t* anim, int32_t value) {
    furi_assert(anim);

    TransitionOverlay* instance = anim->var;

    widget_set_pos_y((Widget*)instance->snap, value);

    if(instance->press_widget) {
        widget_set_pos_y(instance->press_widget, value);
    }
}

static void transition_overlay_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TransitionOverlay* instance = (TransitionOverlay*)obj;
    instance->snap = snap_image_alloc((Widget*)obj);
    instance->color = lv_obj_create(obj);
    instance->mask = anim_image_alloc((Widget*)obj);

    lv_obj_set_size(instance->color, LV_PCT(100), LV_PCT(100));

    widget_set_visible((Widget*)instance, false);
}

// Implementation

static void transition_overlay_animate_color(TransitionOverlay* instance) {
    const TransitionOverlayColorMode mode = instance->color_mode;

    if(mode != TransitionOverlayColorModeOff) {
        lv_blend_mode_t blend_mode;

        if(mode == TransitionOverlayColorModeNormal) {
            blend_mode = LV_BLEND_MODE_NORMAL;
        } else if(mode == TransitionOverlayColorModeMultiply) {
            blend_mode = LV_BLEND_MODE_MULTIPLY;
        } else if(mode == TransitionOverlayColorModeAdd) {
            blend_mode = LV_BLEND_MODE_ADDITIVE;
        } else {
            furi_crash();
        }

        lv_obj_set_style_blend_mode(instance->color, blend_mode, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(instance->color, LV_OPA_TRANSP, LV_PART_MAIN);

        lv_anim_t anim;
        lv_anim_init(&anim);

        lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_anim_set_duration(&anim, instance->timings.in_ms);
        lv_anim_set_reverse_duration(&anim, instance->timings.out_ms);

        lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
        lv_anim_set_custom_exec_cb(&anim, transition_overlay_lvgl_anim_exec_callback);
        lv_anim_set_completed_cb(&anim, transition_overlay_lvgl_anim_completed_callback);
        lv_anim_set_user_data(&anim, instance->color);
        lv_anim_set_var(&anim, instance);

        lv_anim_start(&anim);

        widget_set_visible((Widget*)instance->color, true);
    }
}

static void transition_overlay_animate_mask(TransitionOverlay* instance) {
    const TransitionOverlayMaskMode mode = instance->mask_mode;

    if(mode != TransitionOverlayMaskModeOff) {
        lv_blend_mode_t blend_mode;

        if(mode == TransitionOverlayMaskModeMultiply) {
            blend_mode = LV_BLEND_MODE_MULTIPLY;
        } else if(mode == TransitionOverlayMaskModeAdd) {
            blend_mode = LV_BLEND_MODE_ADDITIVE;
        } else {
            furi_crash();
        }

        lv_obj_set_style_blend_mode(TO_LV_OBJ(instance->mask), blend_mode, LV_PART_MAIN);

        lv_anim_t anim;
        lv_anim_init(&anim);

        lv_anim_set_values(&anim, MASK_ANIM_START, MASK_ANIM_END);
        lv_anim_set_duration(&anim, instance->timings.in_ms);
        lv_anim_set_reverse_duration(&anim, instance->timings.out_ms);

        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_custom_exec_cb(&anim, transition_overlay_lvgl_anim_exec_callback);
        lv_anim_set_completed_cb(&anim, transition_overlay_lvgl_anim_completed_callback);
        lv_anim_set_var(&anim, instance);

        lv_anim_start(&anim);

        anim_image_start(instance->mask);

        widget_set_visible((Widget*)instance->mask, true);
    }
}

static void transition_overlay_animate_press(TransitionOverlay* instance) {
    if(instance->enable_press_effect) {
        lv_anim_t anim;
        lv_anim_init(&anim);

        lv_anim_set_values(&anim, PRESS_OFFSET_Y_START, PRESS_OFFSET_Y_END);
        lv_anim_set_duration(&anim, instance->timings.in_ms);
        lv_anim_set_reverse_duration(&anim, instance->timings.in_ms);

        lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
        lv_anim_set_custom_exec_cb(&anim, transition_overlay_lvgl_press_anim_exec_callback);
        lv_anim_set_var(&anim, instance);

        lv_anim_start(&anim);
    }
}

static void transition_overlay_reset(TransitionOverlay* instance) {
    lv_anim_delete(instance, NULL);

    if(instance->press_widget) {
        widget_set_pos(instance->press_widget, 0, 0);
    }
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
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* transition_overlay_get_base(TransitionOverlay* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void transition_overlay_set_timings(TransitionOverlay* instance, uint32_t in_ms, uint32_t out_ms) {
    furi_check(instance);

    instance->timings.in_ms = in_ms;
    instance->timings.out_ms = out_ms;
}

void transition_overlay_set_color(TransitionOverlay* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_bg_color(instance->color, TO_LV_COLOR(color), LV_PART_MAIN);
}

void transition_overlay_set_color_mode(
    TransitionOverlay* instance,
    TransitionOverlayColorMode mode) {
    furi_check(instance);
    furi_check(mode < TransitionOverlayColorModeMax);

    instance->color_mode = mode;
}

void transition_overlay_set_mask(TransitionOverlay* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    anim_image_set_source(instance->mask, file_path);
    anim_image_set_loop(instance->mask, false);
    anim_image_stop(instance->mask);
}

void transition_overlay_set_mask_mode(TransitionOverlay* instance, TransitionOverlayMaskMode mode) {
    furi_check(instance);
    furi_check(mode < TransitionOverlayMaskModeMax);

    instance->mask_mode = mode;
}

void transition_overlay_enable_press_effect(TransitionOverlay* instance, bool enable) {
    furi_check(instance);

    instance->enable_press_effect = enable;
}

void transition_overlay_set_pressed_widget(TransitionOverlay* instance, Widget* widget) {
    furi_check(instance);

    instance->press_widget = widget;
}

void transition_overlay_show(TransitionOverlay* instance) {
    furi_check(instance);

    transition_overlay_reset(instance);
    snap_image_capture_display(instance->snap);

    widget_set_visible((Widget*)instance->color, false);
    widget_set_visible((Widget*)instance->mask, false);
    widget_set_visible((Widget*)instance, true);
}

void transition_overlay_start(TransitionOverlay* instance) {
    furi_check(instance);

    if(widget_is_visible((Widget*)instance)) {
        transition_overlay_animate_color(instance);
        transition_overlay_animate_mask(instance);
        transition_overlay_animate_press(instance);
    }
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
