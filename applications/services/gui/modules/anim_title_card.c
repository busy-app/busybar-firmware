#include "anim_title_card.h"
#include "../storage_macros.h"
#include "../widget_i.h"
#include "anim_image.h"

#define MY_CLASS       (&anim_title_card_lvgl_class)
#define MY_LABEL_CLASS (&anim_title_card_label_lvgl_class)

typedef struct {
    uint32_t next_frame_idx;
    int32_t offset;
} AnimTitleCardBackgroundAnimFrame;

struct AnimTitleCard {
    Widget base;

    AnimImage* background_anim_image;
    AnimImage* icon_anim_image;
    lv_obj_t* title_label;

    uint32_t background_anim_frame_idx;
};

const lv_obj_class_t anim_title_card_lvgl_class;
const lv_obj_class_t anim_title_card_label_lvgl_class;

static uint32_t anim_title_card_background_frame_callback(AnimImage* anim_image, void* context);
static void anim_title_card_background_completed_callback(AnimImage* anim_image, void* context);

static const AnimTitleCardBackgroundAnimFrame background_anim_frames[] = {
    {.next_frame_idx = 6, .offset = 1}, /* 2 frame */
    {.next_frame_idx = 8, .offset = 2}, /* 6 frame */
    {.next_frame_idx = 12, .offset = 1}, /* 8 frame */
    {.next_frame_idx = 24, .offset = 0}, /* 12 frame */
    {.next_frame_idx = 30, .offset = 1}, /* 24 frame */
    {.next_frame_idx = 2, .offset = 0}, /* 30 frame */
};

/* LVGL-specific code */

static void anim_title_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimTitleCard* instance = (AnimTitleCard*)obj;

    instance->background_anim_frame_idx = 0;

    lv_obj_set_flex_flow(TO_LV_OBJ(obj), LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        TO_LV_OBJ(obj), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    instance->icon_anim_image = anim_image_alloc(&instance->base);

    instance->title_label = lv_obj_class_create_obj(MY_LABEL_CLASS, obj);
    lv_obj_class_init_obj(instance->title_label);
    lv_obj_set_style_text_color(instance->title_label, lv_color_white(), LV_PART_MAIN);

    instance->background_anim_image = anim_image_alloc(&instance->base);
    lv_obj_add_flag(TO_LV_OBJ(instance->background_anim_image), LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(TO_LV_OBJ(instance->background_anim_image), LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_blend_mode(
        TO_LV_OBJ(instance->background_anim_image), LV_BLEND_MODE_ADDITIVE, LV_PART_MAIN);
    anim_image_set_source(
        instance->background_anim_image, GUI_ANIM_PATH("wave_invitation_72x16.anim"));
    anim_image_set_frame_callback(
        instance->background_anim_image,
        background_anim_frames[COUNT_OF(background_anim_frames) - 1].next_frame_idx,
        anim_title_card_background_frame_callback,
        instance);
    anim_image_set_completed_callback(
        instance->background_anim_image, anim_title_card_background_completed_callback, instance);
    anim_image_set_loop(instance->background_anim_image, false);
    anim_image_stop(instance->background_anim_image);
}

/* Implementation */

static void anim_title_card_title_anim_exec_callback(void* var, int32_t value) {
    lv_obj_set_style_translate_x(var, value, LV_PART_MAIN);
}

static void anim_title_card_background_completed_callback(AnimImage* anim_image, void* context) {
    furi_assert(anim_image);
    furi_assert(context);

    AnimTitleCard* instance = context;

    lv_obj_add_flag(TO_LV_OBJ(instance->background_anim_image), LV_OBJ_FLAG_HIDDEN);
}

uint32_t anim_title_card_background_frame_callback(AnimImage* anim_image, void* context) {
    furi_assert(anim_image);
    furi_assert(context);

    AnimTitleCard* instance = context;
    const AnimTitleCardBackgroundAnimFrame* frame =
        &background_anim_frames[instance->background_anim_frame_idx++];

    lv_obj_set_style_translate_y(TO_LV_OBJ(instance->title_label), frame->offset, LV_PART_MAIN);
    lv_obj_set_style_translate_y(
        TO_LV_OBJ(instance->icon_anim_image), frame->offset, LV_PART_MAIN);

    return frame->next_frame_idx;
}

/* Public API */

AnimTitleCard* anim_title_card_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    AnimTitleCard* instance = (AnimTitleCard*)obj;

    return instance;
}

void anim_title_card_free(AnimTitleCard* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* anim_title_card_get_base(AnimTitleCard* instance) {
    furi_check(instance);
    return &instance->base;
}

void anim_title_card_set_icon(AnimTitleCard* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    anim_image_set_source(instance->icon_anim_image, file_path);
    anim_image_stop(instance->icon_anim_image);
}

void anim_title_card_set_title(AnimTitleCard* instance, const char* title) {
    furi_check(instance);
    furi_check(title);

    lv_label_set_text(instance->title_label, title);
}

void anim_title_card_run_background_anim(AnimTitleCard* instance) {
    furi_check(instance);

    instance->background_anim_frame_idx = 0;

    lv_obj_remove_flag(TO_LV_OBJ(instance->background_anim_image), LV_OBJ_FLAG_HIDDEN);
    anim_image_rewind(instance->background_anim_image);
    anim_image_start(instance->background_anim_image);
}

void anim_title_card_run_icon_anim(AnimTitleCard* instance, uint32_t start, uint32_t stop) {
    furi_check(instance);

    anim_image_set_range(instance->icon_anim_image, start, stop, false, false);
}

void anim_title_card_run_title_anim(
    AnimTitleCard* instance,
    int32_t start,
    int32_t stop,
    uint32_t duration) {
    furi_check(instance);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, instance->title_label);
    lv_anim_set_values(&anim, start, stop);
    lv_anim_set_duration(&anim, duration);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_exec_cb(&anim, anim_title_card_title_anim_exec_callback);
    lv_anim_start(&anim);
}

/* LVGL class descriptor */

const lv_obj_class_t anim_title_card_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = anim_title_card_lvgl_constructor,
    .name = "widget-anim-title-card",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(AnimTitleCard),
};

const lv_obj_class_t anim_title_card_label_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "anim-title-card-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
