#include "prompt_overlay.h"

#include <gui/modules/anim_play_i.h>
#include <gui/storage_macros.h>

#define MY_CLASS (&prompt_overlay_lvgl_class)

#define ANIM_DELAY_MS    (1000)
#define ANIM_INTERVAL_MS (3000)

#define TARGET_Y_OFFSET (2)

typedef enum {
    PromptOverlayStateBegin,
    PromptOverlayStateAnimBegin,
    PromptOverlayStateAnimEnd,
    PromptOverlayStateEnd,
} PromptOverlayState;

struct PromptOverlay {
    AnimPlay base;
    lv_obj_t* target;
    uint32_t frame_idx;
    PromptOverlayCallback callback;
    void* callback_context;
    bool is_pressed;
    uint32_t time;
};

static const int8_t overlay_offset_animation[] = {
    [0 ... 1] = 0,
    [2 ... 5] = TARGET_Y_OFFSET / 2,
    [6 ... 7] = TARGET_Y_OFFSET,
    [8 ... 11] = TARGET_Y_OFFSET / 2,
    [12 ... 23] = 0,
    [24 ... 29] = TARGET_Y_OFFSET / 2,
};

const lv_obj_class_t prompt_overlay_lvgl_class;

// Function prototypes

static bool prompt_overlay_input_callback(Widget* widget, const InputEvent* event);
static void prompt_overlay_start_animation(PromptOverlay* instance);
static void prompt_overlay_set_target_y_offset(PromptOverlay* instance, int32_t offset);
static void prompt_overlay_frame_callback(
    AnimPlay* anim_play,
    const AnimFileFrameInfo* frame,
    void* context);

// LVGL-specific code

static void prompt_overlay_lvgl_anim_callback(void* context, int32_t value) {
    furi_assert(context);

    PromptOverlay* instance = context;

    if(value == PromptOverlayStateAnimBegin) {
        instance->frame_idx = 0;
        anim_play_start(&instance->base);

    } else if(value == PromptOverlayStateAnimEnd) {
        if(instance->callback) {
            instance->callback(instance->callback_context);
        }
    }
}

static void prompt_overlay_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_style_blend_mode(obj, LV_BLEND_MODE_ADDITIVE, LV_PART_MAIN);

    widget_set_input_feed_callback((Widget*)obj, prompt_overlay_input_callback);

    AnimPlay* anim_play = (AnimPlay*)obj;
    anim_play_set_source(anim_play, GUI_ANIM_PATH("wave_invitation_72x16.anim"));

    prompt_overlay_start_animation((PromptOverlay*)obj);
}

// Implementation

static bool prompt_overlay_input_callback(Widget* widget, const InputEvent* event) {
    PromptOverlay* instance = (PromptOverlay*)widget;

    bool consumed = false;

    if(event->key == InputKeyStart) {
        if(event->type == InputTypePress) {
            instance->is_pressed = true;

            AnimPlay* anim_play = &instance->base;
            AnimFile* file = anim_play_get_file(anim_play);
            if(file) {
                bool success = anim_file_set_section(file, AnimFilePlayFlagNone, "idle");
                UNUSED(success);
            }

            lv_obj_t* obj = TO_LV_OBJ(instance);
            lv_anim_delete(obj, prompt_overlay_lvgl_anim_callback);

            prompt_overlay_set_target_y_offset(instance, TARGET_Y_OFFSET);

            consumed = true;

        } else if(event->type == InputTypeRelease) {
            instance->is_pressed = false;

            prompt_overlay_set_target_y_offset(instance, 0);
            prompt_overlay_start_animation(instance);

            consumed = true;
        }
    }

    return consumed;
}

static void prompt_overlay_start_animation(PromptOverlay* instance) {
    AnimPlay* anim_play = &instance->base;

    AnimFile* file = anim_play_get_file(anim_play);
    if(file) {
        bool success =
            anim_file_set_section(file, AnimFilePlayFlagNone, ANIM_FILE_DEFAULT_SECTION);
        UNUSED(success);
        anim_play_pause(anim_play);
    }

    anim_play_set_frame_callback(anim_play, prompt_overlay_frame_callback, NULL);

    instance->frame_idx = 0;

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_delay(&anim, ANIM_DELAY_MS);
    lv_anim_set_duration(&anim, ANIM_INTERVAL_MS);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_values(&anim, PromptOverlayStateBegin, PromptOverlayStateEnd);
    lv_anim_set_exec_cb(&anim, prompt_overlay_lvgl_anim_callback);
    lv_anim_set_var(&anim, instance);
    lv_anim_start(&anim);
}

static void prompt_overlay_set_target_y_offset(PromptOverlay* instance, int32_t offset) {
    if(instance->target) {
        lv_obj_set_style_translate_y(TO_LV_OBJ(instance->target), offset, LV_PART_MAIN);
    }
}

static void prompt_overlay_frame_callback(
    AnimPlay* anim_play,
    const AnimFileFrameInfo* frame,
    void* context) {
    furi_assert(anim_play);
    UNUSED(context);

    if(frame->flags & FuriFlagError) return;

    int8_t offset = 0;
    if(frame->index < COUNT_OF(overlay_offset_animation)) {
        offset = overlay_offset_animation[frame->index];
    }

    PromptOverlay* instance = (PromptOverlay*)anim_play;

    if(!instance->is_pressed) {
        prompt_overlay_set_target_y_offset(instance, offset);
    }
}

// Public API

PromptOverlay* prompt_overlay_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    PromptOverlay* instance = (PromptOverlay*)obj;
    return instance;
}

void prompt_overlay_free(PromptOverlay* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* prompt_overlay_get_base(PromptOverlay* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void prompt_overlay_set_animation_target(PromptOverlay* instance, Widget* widget) {
    furi_check(instance);
    furi_check(widget);

    instance->target = TO_LV_OBJ(widget);
}

void prompt_overlay_set_callback(
    PromptOverlay* instance,
    PromptOverlayCallback callback,
    void* context) {
    furi_check(instance);

    instance->callback = callback;
    instance->callback_context = context;
}

// LVGL class descriptor

const lv_obj_class_t prompt_overlay_lvgl_class = {
    .base_class = &anim_play_lvgl_class,
    .constructor_cb = prompt_overlay_lvgl_constructor,
    .name = "widget-prompt-overlay",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(PromptOverlay),
};
