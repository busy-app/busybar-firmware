#include "transition_overlay.h"

#include <gui/widget_i.h>

#include <gui/modules/anim_player.h>
#include <gui/modules/snap_image.h>

#define MY_CLASS (&transition_overlay_lvgl_class)

#define PRESS_OFFSET_Y_START (0)
#define PRESS_OFFSET_Y_END   (3)

#define MASK_ANIM_START (0)
#define MASK_ANIM_END   (1)

struct TransitionOverlay {
    Widget base;
    SnapImage* snap;
    AnimPlayer* mask;
    lv_obj_t* color;
    Widget* press_widget;
    TransitionOverlayPreset preset;
};

const lv_obj_class_t transition_overlay_lvgl_class;

// LVGL-specific code

static void transition_overlay_lvgl_color_anim_exec_callback(lv_anim_t* anim, int32_t value) {
    furi_assert(anim);

    TransitionOverlay* instance = anim->var;

    lv_obj_set_style_bg_opa(instance->color, value, LV_PART_MAIN);

    if(anim->end_value == value) {
        widget_set_visible((Widget*)instance->snap, false);
    }
}

static void transition_overlay_lvgl_mask_anim_exec_callback(lv_anim_t* anim, int32_t value) {
    furi_assert(anim);

    TransitionOverlay* instance = anim->var;

    if(anim->end_value == value) {
        widget_set_visible((Widget*)instance->snap, false);
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
    instance->mask = anim_player_alloc((Widget*)obj);

    lv_obj_set_size(instance->color, LV_PCT(100), LV_PCT(100));

    widget_set_visible((Widget*)instance, false);
}

static void transition_overlay_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TransitionOverlay* instance = (TransitionOverlay*)obj;
    snap_image_free(instance->snap);
    lv_obj_del(obj);
    anim_player_free(instance->mask);
}

// Implementation

static void transition_overlay_animate_color(TransitionOverlay* instance) {
    const TransitionOverlayPreset* preset = &instance->preset;
    const TransitionOverlayBlendMode blend_mode = preset->blend_mode;

    lv_blend_mode_t lv_blend_mode;

    if(blend_mode == TransitionOverlayBlendModeNormal) {
        lv_blend_mode = LV_BLEND_MODE_NORMAL;
    } else if(blend_mode == TransitionOverlayBlendModeMultiply) {
        lv_blend_mode = LV_BLEND_MODE_MULTIPLY;
    } else if(blend_mode == TransitionOverlayBlendModeAdd) {
        lv_blend_mode = LV_BLEND_MODE_ADDITIVE;
    } else {
        furi_crash();
    }

    lv_obj_set_style_blend_mode(instance->color, lv_blend_mode, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(instance->color, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim, preset->timings.in_ms);
    lv_anim_set_reverse_duration(&anim, preset->timings.out_ms);

    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_custom_exec_cb(&anim, transition_overlay_lvgl_color_anim_exec_callback);
    lv_anim_set_completed_cb(&anim, transition_overlay_lvgl_anim_completed_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);

    widget_set_visible((Widget*)instance->color, true);
}

static void transition_overlay_animate_mask(TransitionOverlay* instance) {
    const TransitionOverlayPreset* preset = &instance->preset;
    const TransitionOverlayBlendMode blend_mode = preset->blend_mode;

    lv_blend_mode_t lv_blend_mode;

    if(blend_mode == TransitionOverlayBlendModeMultiply) {
        lv_blend_mode = LV_BLEND_MODE_MULTIPLY;
    } else if(blend_mode == TransitionOverlayBlendModeAdd) {
        lv_blend_mode = LV_BLEND_MODE_ADDITIVE;
    } else {
        furi_crash();
    }

    lv_obj_set_style_blend_mode(TO_LV_OBJ(instance->mask), lv_blend_mode, LV_PART_MAIN);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, MASK_ANIM_START, MASK_ANIM_END);
    lv_anim_set_duration(&anim, preset->timings.in_ms);
    lv_anim_set_reverse_duration(&anim, preset->timings.out_ms);

    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_custom_exec_cb(&anim, transition_overlay_lvgl_mask_anim_exec_callback);
    lv_anim_set_completed_cb(&anim, transition_overlay_lvgl_anim_completed_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);

    anim_player_start(instance->mask);

    widget_set_visible((Widget*)instance->mask, true);
}

static void transition_overlay_animate_press(TransitionOverlay* instance) {
    const TransitionOverlayPreset* preset = &instance->preset;

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, PRESS_OFFSET_Y_START, PRESS_OFFSET_Y_END);
    lv_anim_set_duration(&anim, preset->timings.in_ms);
    lv_anim_set_reverse_duration(&anim, preset->timings.in_ms);

    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_set_custom_exec_cb(&anim, transition_overlay_lvgl_press_anim_exec_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

static void transition_overlay_reset(TransitionOverlay* instance) {
    lv_anim_delete(instance, NULL);

    anim_player_pause(instance->mask);
    anim_player_set_section(instance->mask, AnimFilePlayFlagNone, ANIM_FILE_DEFAULT_SECTION);

    if(instance->press_widget) {
        widget_set_pos(instance->press_widget, 0, 0);
    }

    widget_set_pos((Widget*)instance->snap, 0, 0);
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

void transition_overlay_set_preset(
    TransitionOverlay* instance,
    const TransitionOverlayPreset* preset) {
    furi_check(instance);
    furi_check(preset);

    instance->preset = *preset;

    if(preset->type == TransitionOverlayTypeColor) {
        lv_obj_set_style_bg_color(instance->color, TO_LV_COLOR(preset->mask.color), LV_PART_MAIN);

    } else if(preset->type == TransitionOverlayTypeMask) {
        anim_player_set_source(instance->mask, preset->mask.file_path);
        anim_player_pause(instance->mask);
    }
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
        const TransitionOverlayPreset* preset = &instance->preset;

        if(preset->type == TransitionOverlayTypeColor) {
            transition_overlay_animate_color(instance);
        } else if(preset->type == TransitionOverlayTypeMask) {
            transition_overlay_animate_mask(instance);
        }

        if(preset->effect == TransitionOverlayEffectPress) {
            transition_overlay_animate_press(instance);
        }
    }
}

// LVGL class descriptor

const lv_obj_class_t transition_overlay_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = transition_overlay_lvgl_constructor,
    .destructor_cb = transition_overlay_lvgl_destructor,
    .name = "widget-transition-overlay",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(TransitionOverlay),
};
