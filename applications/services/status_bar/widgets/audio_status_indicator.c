#include "audio_status_indicator.h"
#include "../storage_macros.h"

#include <gui/widget_i.h>

#define MY_CLASS (&audio_status_indicator_lvgl_class)

struct AudioStatusIndicator {
    Widget base;
    lv_obj_t* sound_on_image;
    lv_obj_t* sound_off_image;
};

const lv_obj_class_t audio_status_indicator_lvgl_class;

/* LVGL-specific code */

static void audio_status_indicator_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AudioStatusIndicator* instance = (AudioStatusIndicator*)obj;

    instance->sound_on_image = lv_image_create(obj);
    lv_obj_center(instance->sound_on_image);
    lv_image_set_src(instance->sound_on_image, STATUS_BAR_IMG_PATH("sound_on_8x8.bin"));

    instance->sound_off_image = lv_image_create(obj);
    lv_obj_center(instance->sound_off_image);
    lv_image_set_src(instance->sound_off_image, STATUS_BAR_IMG_PATH("sound_off_8x8.bin"));
}

/* Public API */

AudioStatusIndicator* audio_status_indicator_alloc(Widget* parent) {
    furi_assert(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    AudioStatusIndicator* instance = (AudioStatusIndicator*)obj;
    return instance;
}

void audio_status_indicator_free(AudioStatusIndicator* instance) {
    furi_assert(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* audio_status_indicator_get_base(AudioStatusIndicator* instance) {
    furi_assert(instance);
    return (Widget*)instance;
}

void audio_status_indicator_set_volume(AudioStatusIndicator* instance, float volume) {
    furi_assert(instance);

    bool is_sound_on = volume > 0.0f;

    lv_obj_update_flag(instance->sound_on_image, LV_OBJ_FLAG_HIDDEN, !is_sound_on);
    lv_obj_update_flag(instance->sound_off_image, LV_OBJ_FLAG_HIDDEN, is_sound_on);
}

const lv_obj_class_t audio_status_indicator_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = audio_status_indicator_lvgl_constructor,
    .name = "widget-audio-state-indicator",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(AudioStatusIndicator),
};
