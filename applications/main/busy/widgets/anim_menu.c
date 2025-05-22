#include "anim_menu.h"

#include <gui/modules/anim_image_i.h>

#include <storage/storage.h>

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS (&anim_menu_lvgl_class)

struct AnimMenu {
    AnimImage base;
    AnimMenuCallback callback;
    void* context;
    uint32_t idle_frames;
    uint32_t transition_frames;
    uint32_t current_idx;
};

typedef struct {
    uint32_t begin;
    uint32_t end;
} AnimMenuFrameRange;

const lv_obj_class_t anim_menu_lvgl_class;

static inline void anim_menu_calc_idle_range(const AnimMenu* instance, AnimMenuFrameRange* range) {
    furi_assert(instance->idle_frames);
    furi_assert(instance->transition_frames);

    range->begin = instance->current_idx * (instance->idle_frames + instance->transition_frames);
    range->end = range->begin + instance->idle_frames - 1;
}

static inline void
    anim_menu_calc_transition_range(const AnimMenu* instance, AnimMenuFrameRange* range) {
    furi_assert(instance->idle_frames);
    furi_assert(instance->transition_frames);

    range->begin = instance->idle_frames +
                   (instance->current_idx * (instance->idle_frames + instance->transition_frames));
    range->end = range->begin + instance->transition_frames - 1;
}

static bool anim_menu_input_callback(Widget* widget, const InputEvent* event) {
    AnimMenu* instance = (AnimMenu*)widget;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            if(!instance->current_idx) {
                AnimMenuFrameRange range;
                // Transition from item 0 to item 1
                anim_menu_calc_transition_range(instance, &range);
                anim_image_set_range((AnimImage*)instance, range.begin, range.end, false, false);
                // Important: read the code before attempting to move the below line
                instance->current_idx = 1;
                // Item 1 idle
                anim_menu_calc_idle_range(instance, &range);
                anim_image_set_range((AnimImage*)instance, range.begin, range.end, true, true);
            }

            consumed = true;

        } else if(event->key == InputKeyDown) {
            if(instance->current_idx) {
                AnimMenuFrameRange range;
                // Transition from item 1 to item 0
                anim_menu_calc_transition_range(instance, &range);
                anim_image_set_range((AnimImage*)instance, range.begin, range.end, false, false);
                // Important: read the code before attempting to move the below line
                instance->current_idx = 0;
                // Item 0 idle
                anim_menu_calc_idle_range(instance, &range);
                anim_image_set_range((AnimImage*)instance, range.begin, range.end, true, true);
            }

            consumed = true;

        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            if(instance->callback) {
                instance->callback(instance->current_idx, instance->context);
            }

            consumed = true;
        }
    }

    return consumed;
}

static void anim_menu_lvlg_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimMenu* instance = (AnimMenu*)obj;
    widget_set_input_feed_callback((Widget*)instance, anim_menu_input_callback);
}

static void anim_menu_lvlg_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
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

AnimImage* anim_menu_get_anim_image(AnimMenu* instance) {
    furi_check(instance);
    return (AnimImage*)instance;
}

bool anim_menu_set_source(
    AnimMenu* instance,
    const char* file_path,
    uint32_t idle_frames,
    uint32_t transition_frames) {
    furi_check(instance);
    furi_check(file_path);
    furi_check(idle_frames > 0);
    furi_check(transition_frames > 0);

    const bool success = anim_image_set_source((AnimImage*)instance, file_path);

    if(success) {
        instance->idle_frames = idle_frames;
        instance->transition_frames = transition_frames;

        anim_image_set_range((AnimImage*)instance, 0, idle_frames - 1, true, false);
        anim_image_start((AnimImage*)instance);
    }

    return success;
}

void anim_menu_set_callback(AnimMenu* instance, AnimMenuCallback callback, void* context) {
    furi_check(instance);

    instance->callback = callback;
    instance->context = context;
}

// LVGL class descriptors

const lv_obj_class_t anim_menu_lvgl_class = {
    .base_class = &anim_image_lvgl_class,
    .constructor_cb = anim_menu_lvlg_constructor,
    .destructor_cb = anim_menu_lvlg_destructor,
    .name = "widget-anim-menu",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(AnimMenu),
};
