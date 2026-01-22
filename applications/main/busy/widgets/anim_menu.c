#include "anim_menu.h"

#include <gui/modules/anim_play_i.h>

#include <storage/storage.h>

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS (&anim_menu_lvgl_class)

struct AnimMenu {
    AnimPlay base;
    AnimMenuCallback callback;
    void* context;
    size_t option_count;
    size_t current_idx;
};

typedef struct {
    uint32_t begin;
    uint32_t end;
} AnimMenuFrameRange;

const lv_obj_class_t anim_menu_lvgl_class;

static bool anim_menu_input_callback(Widget* widget, const InputEvent* event) {
    AnimMenu* instance = (AnimMenu*)widget;

    bool consumed = false;
    char section_name[36]; // strlen("transition-4294967295-to-4294967295") + 1

    if(event->type == InputTypeShort) {
        size_t previous_idx = instance->current_idx;

        switch(event->key) {
        case InputKeyUp:
            consumed = true;
            if(instance->current_idx == instance->option_count - 1) break;
            instance->current_idx++;
            break;

        case InputKeyDown:
            consumed = true;
            if(instance->current_idx == 0) break;
            instance->current_idx--;
            break;

        case InputKeyOk:
        /* fall-through */
        case InputKeyStart:
            if(instance->callback) {
                instance->callback(instance->current_idx, instance->context);
            }
            consumed = true;
            break;

        default:
            break;
        }

        do {
            size_t current_idx = instance->current_idx;
            if(current_idx == previous_idx) break;

            AnimFile* file = anim_play_get_file(&instance->base);
            if(!file) break;

            snprintf(
                section_name,
                sizeof(section_name),
                "transition-%zu-to-%zu",
                previous_idx,
                current_idx);
            if(!anim_file_set_section(file, AnimFilePlayFlagNone, section_name)) break;

            snprintf(section_name, sizeof(section_name), "item-%zu", current_idx);
            if(!anim_file_set_section(
                   file, AnimFilePlayFlagFinishCurrent | AnimFilePlayFlagLoop, section_name))
                break;
        } while(0);
    }

    return consumed;
}

static void anim_menu_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimMenu* instance = (AnimMenu*)obj;
    widget_set_input_feed_callback((Widget*)instance, anim_menu_input_callback);
}

static void anim_menu_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimMenu* instance = (AnimMenu*)obj;
    UNUSED(instance);
}

// Public API

AnimMenu* anim_menu_alloc(Widget* widget) {
    furi_check(widget);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)widget);
    lv_obj_class_init_obj(obj);

    AnimMenu* instance = (AnimMenu*)obj;
    return instance;
}

void anim_menu_free(AnimMenu* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* anim_menu_get_base(AnimMenu* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

AnimPlay* anim_menu_get_anim_play(AnimMenu* instance) {
    furi_check(instance);
    return (AnimPlay*)instance;
}

bool anim_menu_set_source(AnimMenu* instance, const char* file_path, size_t options) {
    furi_check(instance);
    furi_check(file_path);
    furi_check(options > 0);

    if(!anim_play_set_source(&instance->base, file_path)) return false;

    instance->option_count = options;
    AnimFile* file = anim_play_get_file(&instance->base);
    furi_assert(file);

    if(!anim_file_set_section(file, AnimFilePlayFlagLoop, "item-0")) return false;

    return true;
}

void anim_menu_set_callback(AnimMenu* instance, AnimMenuCallback callback, void* context) {
    furi_check(instance);

    instance->callback = callback;
    instance->context = context;
}

// LVGL class descriptors

const lv_obj_class_t anim_menu_lvgl_class = {
    .base_class = &anim_play_lvgl_class,
    .constructor_cb = anim_menu_lvgl_constructor,
    .destructor_cb = anim_menu_lvgl_destructor,
    .name = "widget-anim-menu",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(AnimMenu),
};
