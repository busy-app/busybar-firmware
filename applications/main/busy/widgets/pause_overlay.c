#include "pause_overlay.h"

#include <gui/widget_i.h>
#include <gui/modules/snap_image.h>
#include <gui/modules/anim_play.h>

#include "../storage_macros.h"

#define MY_CLASS (&pause_overlay_lvgl_class)

#define FRAMES_TO_MS(x) ((x) * 1000 / 60)

#define BLUR_STRENGTH (255)
#define DIM_STRENGTH  (160)

#define TEXT_OFFSET_START (-4)
#define TEXT_OFFSET_END   (-1)

#define TEXT_ANIM_DELAY_MS    (FRAMES_TO_MS(5))
#define TEXT_ANIM_DURATION_MS (FRAMES_TO_MS(20))
#define HIDE_ANIM_DURATION_MS (200)

#define BLINK_DELAY_MS  (TEXT_ANIM_DELAY_MS + TEXT_ANIM_DURATION_MS + FRAMES_TO_MS(15))
#define BLINK_PERIOD_MS (FRAMES_TO_MS(40))

struct PauseOverlay {
    Widget base;
    SnapImage* snap;
    AnimPlay* mask;
    lv_obj_t* layout;
    lv_obj_t* pause_img;
};

const lv_obj_class_t pause_overlay_lvgl_class;

// LVGL-specific code

static void pause_overlay_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    PauseOverlay* instance = (PauseOverlay*)obj;
    instance->snap = snap_image_alloc((Widget*)instance);
    snap_image_set_effect(instance->snap, SnapImageEffectBlur, BLUR_STRENGTH);
    snap_image_set_effect(instance->snap, SnapImageEffectDim, DIM_STRENGTH);

    instance->layout = lv_obj_create(obj);
    lv_obj_set_size(instance->layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_column(instance->layout, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(instance->layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        instance->layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_align(instance->layout, LV_ALIGN_CENTER, 0, TEXT_OFFSET_END);

    instance->pause_img = lv_image_create(instance->layout);
    lv_image_set_src(instance->pause_img, BUSY_IMG_PATH("pause_5x5.bin"));

    lv_obj_t* label = lv_label_create(instance->layout);
    lv_label_set_text(label, "PAUSED");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);

    instance->mask = anim_play_alloc((Widget*)instance);
    anim_play_set_source(instance->mask, BUSY_ANIM_PATH("transition_pause_72x16.anim"));
    anim_play_pause(instance->mask);

    lv_obj_set_style_blend_mode(TO_LV_OBJ(instance->mask), LV_BLEND_MODE_ADDITIVE, LV_PART_MAIN);

    widget_set_visible((Widget*)instance, false);
}

static void pause_overlay_lvgl_show_anim_callback(void* var, int32_t value) {
    furi_assert(var);

    PauseOverlay* instance = var;
    lv_obj_set_y(instance->layout, value);
}

static void pause_overlay_lvgl_show_anim_start_callback(lv_anim_t* anim) {
    furi_assert(anim);

    PauseOverlay* instance = anim->var;
    furi_assert(instance);

    widget_set_visible((Widget*)instance->layout, true);
}

static void pause_overlay_lvgl_opacity_anim_callback(void* var, int32_t value) {
    furi_assert(var);

    lv_obj_set_style_opa(var, value, LV_PART_MAIN);
}

static void pause_overlay_lvgl_hide_anim_finished_callback(lv_anim_t* anim) {
    furi_assert(anim);

    PauseOverlay* instance = anim->var;
    furi_assert(instance);

    widget_set_visible((Widget*)instance, false);
}

// Implementation

static void pause_overlay_animate_show(PauseOverlay* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, TEXT_OFFSET_START, TEXT_OFFSET_END);
    lv_anim_set_delay(&anim, TEXT_ANIM_DELAY_MS);
    lv_anim_set_duration(&anim, TEXT_ANIM_DURATION_MS);
    // Overshoot effect curve
    lv_anim_set_bezier3_param(
        &anim,
        LV_BEZIER_VAL_FLOAT(0.6F),
        LV_BEZIER_VAL_FLOAT(0.0F),
        LV_BEZIER_VAL_FLOAT(0.3F),
        LV_BEZIER_VAL_FLOAT(3.0F));

    lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
    lv_anim_set_start_cb(&anim, pause_overlay_lvgl_show_anim_start_callback);
    lv_anim_set_exec_cb(&anim, pause_overlay_lvgl_show_anim_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);

    lv_obj_set_style_opa(TO_LV_OBJ(instance), LV_OPA_COVER, LV_PART_MAIN);

    AnimFile* file = anim_play_get_file(instance->mask);
    if(file) {
        bool success =
            anim_file_set_section(file, AnimFilePlayFlagNone, ANIM_FILE_DEFAULT_SECTION);
        furi_assert(success);
        anim_play_start(instance->mask);
    }

    widget_set_visible((Widget*)instance->layout, false);
    widget_set_visible((Widget*)instance, true);
}

static void pause_overlay_animate_hide(PauseOverlay* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&anim, HIDE_ANIM_DURATION_MS);

    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, pause_overlay_lvgl_opacity_anim_callback);
    lv_anim_set_completed_cb(&anim, pause_overlay_lvgl_hide_anim_finished_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

static void pause_overlay_start_blink(PauseOverlay* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_delay(&anim, TEXT_ANIM_DELAY_MS + TEXT_ANIM_DURATION_MS);
    lv_anim_set_duration(&anim, BLINK_PERIOD_MS);
    lv_anim_set_reverse_duration(&anim, BLINK_PERIOD_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim, pause_overlay_lvgl_opacity_anim_callback);
    lv_anim_set_var(&anim, instance->pause_img);

    lv_anim_start(&anim);
}

static void pause_overlay_stop_blink(PauseOverlay* instance) {
    lv_anim_delete(instance->pause_img, pause_overlay_lvgl_opacity_anim_callback);
}

static void pause_overlay_stop_animations(PauseOverlay* instance) {
    lv_anim_delete(instance, NULL);
    lv_anim_delete(instance->pause_img, NULL);
}

// Public API

PauseOverlay* pause_overlay_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    PauseOverlay* instance = (PauseOverlay*)obj;
    return instance;
}

void pause_overlay_free(PauseOverlay* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* pause_overlay_get_base(PauseOverlay* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void pause_overlay_show(PauseOverlay* instance, bool show) {
    furi_check(instance);

    pause_overlay_stop_animations(instance);

    if(show) {
        snap_image_capture_display(instance->snap);
        pause_overlay_start_blink(instance);
        pause_overlay_animate_show(instance);

    } else {
        pause_overlay_stop_blink(instance);
        pause_overlay_animate_hide(instance);
    }
}

// LVGL class descriptor

const lv_obj_class_t pause_overlay_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = pause_overlay_lvgl_constructor,
    .name = "widget-pause-overlay",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(PauseOverlay),
};
