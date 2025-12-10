#include "prompt_overlay.h"

#include <gui/modules/anim_image_i.h>
#include <gui/storage_macros.h>

#define MY_CLASS (&prompt_overlay_lvgl_class)

#define ANIM_DELAY_MS    (1000)
#define ANIM_INTERVAL_MS (3000)

#define ANIM_START_VALUE (0)
#define ANIM_END_VALUE   (1)

struct PromptOverlay {
    AnimImage base;
    lv_obj_t* target;
    uint32_t frame_idx;
};

typedef struct {
    uint32_t next_frame_idx;
    int32_t target_offset_px;
} PromptOverlayFrame;

static const PromptOverlayFrame anim_frames[] = {
    {.next_frame_idx = 6, .target_offset_px = 1}, /* frame 2 */
    {.next_frame_idx = 8, .target_offset_px = 2}, /* frame 6 */
    {.next_frame_idx = 12, .target_offset_px = 1}, /* frame 8 */
    {.next_frame_idx = 24, .target_offset_px = 0}, /* frame 12 */
    {.next_frame_idx = 30, .target_offset_px = 1}, /* frame 24 */
    {.next_frame_idx = 2, .target_offset_px = 0}, /* frame 30 */
};

const lv_obj_class_t prompt_overlay_lvgl_class;

static uint32_t prompt_overlay_animation_frame_callback(AnimImage* anim, void* context);
static void prompt_overlay_animation_completed_callback(AnimImage* anim, void* context);

// LVGL-specific code

static void prompt_overlay_lvgl_anim_callback(void* context, int32_t value) {
    furi_assert(context);

    PromptOverlay* instance = context;

    if(value == ANIM_START_VALUE) {
        instance->frame_idx = 0;
        anim_image_start(&instance->base);
        lv_obj_remove_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_HIDDEN);
    }
}

static void prompt_overlay_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_style_blend_mode(obj, LV_BLEND_MODE_ADDITIVE, LV_PART_MAIN);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

    AnimImage* anim_image = (AnimImage*)obj;
    anim_image_set_source(anim_image, GUI_ANIM_PATH("wave_invitation_72x16.anim"));
    anim_image_set_loop(anim_image, false);
    anim_image_stop(anim_image);

    const uint32_t init_frame_idx = anim_frames[COUNT_OF(anim_frames) - 1].next_frame_idx;
    anim_image_set_frame_callback(
        anim_image, init_frame_idx, prompt_overlay_animation_frame_callback, NULL);
    anim_image_set_completed_callback(
        anim_image, prompt_overlay_animation_completed_callback, NULL);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_delay(&anim, ANIM_DELAY_MS);
    lv_anim_set_duration(&anim, ANIM_INTERVAL_MS);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_values(&anim, ANIM_START_VALUE, ANIM_END_VALUE);
    lv_anim_set_exec_cb(&anim, prompt_overlay_lvgl_anim_callback);
    lv_anim_set_var(&anim, obj);
    lv_anim_start(&anim);
}

// Implementation

static uint32_t prompt_overlay_animation_frame_callback(AnimImage* anim, void* context) {
    furi_assert(anim);
    UNUSED(context);

    PromptOverlay* instance = (PromptOverlay*)anim;
    const PromptOverlayFrame* frame = &anim_frames[instance->frame_idx++];

    if(instance->target) {
        lv_obj_set_style_translate_y(
            TO_LV_OBJ(instance->target), frame->target_offset_px, LV_PART_MAIN);
    }

    return frame->next_frame_idx;
}

static void prompt_overlay_animation_completed_callback(AnimImage* anim, void* context) {
    furi_assert(anim);
    UNUSED(context);

    lv_obj_add_flag(TO_LV_OBJ(anim), LV_OBJ_FLAG_HIDDEN);
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

// LVGL class descriptor

const lv_obj_class_t prompt_overlay_lvgl_class = {
    .base_class = &anim_image_lvgl_class,
    .constructor_cb = prompt_overlay_lvgl_constructor,
    .name = "widget-prompt-overlay",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(PromptOverlay),
};
