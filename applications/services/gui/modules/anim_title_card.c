#include "anim_title_card.h"
#include "../storage_macros.h"
#include "../widget_i.h"
#include "anim_play.h"

#define TAG            "AnimTitleCard"
#define MY_CLASS       (&anim_title_card_lvgl_class)
#define MY_LABEL_CLASS (&anim_title_card_label_lvgl_class)

#define TARGET_Y_OFFSET (2)

struct AnimTitleCard {
    Widget base;

    AnimPlay* background_anim;
    AnimPlay* icon_anim;
    lv_obj_t* title_label;
};

const lv_obj_class_t anim_title_card_lvgl_class;
const lv_obj_class_t anim_title_card_label_lvgl_class;

static const int8_t card_offset_animation[] = {
    [0 ... 1] = 0,
    [2 ... 5] = TARGET_Y_OFFSET / 2,
    [6 ... 7] = TARGET_Y_OFFSET,
    [8 ... 11] = TARGET_Y_OFFSET / 2,
    [12 ... 23] = 0,
    [24 ... 29] = TARGET_Y_OFFSET / 2,
};

static void
    anim_title_card_frame(AnimPlay* instance, const AnimFileFrameInfo* info, void* context) {
    furi_assert(instance);
    furi_assert(context);
    AnimTitleCard* card = context;

    if(info->flags & AnimFileFrameFlagError) return;

    int32_t offset = 0;
    if(info->index >= COUNT_OF(card_offset_animation)) {
        offset = 0;
    } else {
        offset = card_offset_animation[info->index];
    }

    lv_obj_set_style_translate_y(TO_LV_OBJ(card->title_label), offset, LV_PART_MAIN);
    lv_obj_set_style_translate_y(TO_LV_OBJ(card->icon_anim), offset, LV_PART_MAIN);

    if(info->flags & AnimFileFrameFlagFinished) {
        lv_obj_add_flag(TO_LV_OBJ(card->background_anim), LV_OBJ_FLAG_HIDDEN);
        anim_play_pause(card->background_anim);
    }
}

/* LVGL-specific code */

static void anim_title_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimTitleCard* instance = (AnimTitleCard*)obj;

    lv_obj_set_flex_flow(TO_LV_OBJ(obj), LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        TO_LV_OBJ(obj), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    instance->icon_anim = anim_play_alloc(&instance->base);

    instance->title_label = lv_obj_class_create_obj(MY_LABEL_CLASS, obj);
    lv_obj_class_init_obj(instance->title_label);
    lv_obj_set_style_text_color(instance->title_label, lv_color_white(), LV_PART_MAIN);

    instance->background_anim = anim_play_alloc(&instance->base);
    lv_obj_add_flag(TO_LV_OBJ(instance->background_anim), LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(TO_LV_OBJ(instance->background_anim), LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_blend_mode(
        TO_LV_OBJ(instance->background_anim), LV_BLEND_MODE_ADDITIVE, LV_PART_MAIN);

    anim_play_set_frame_callback(instance->background_anim, anim_title_card_frame, instance);
    if(anim_play_set_source(
           instance->background_anim, GUI_ANIM_PATH("wave_invitation_72x16.anim"))) {
        anim_play_pause(instance->background_anim);
    }
}

/* Implementation */

static void anim_title_card_title_anim_exec_callback(void* var, int32_t value) {
    lv_obj_set_style_translate_x(var, value, LV_PART_MAIN);
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

    anim_play_set_source(instance->icon_anim, file_path);
    anim_play_pause(instance->icon_anim);
}

void anim_title_card_set_title(AnimTitleCard* instance, const char* title) {
    furi_check(instance);
    furi_check(title);

    lv_label_set_text(instance->title_label, title);
}

void anim_title_card_run_background_anim(AnimTitleCard* instance) {
    furi_check(instance);

    lv_obj_remove_flag(TO_LV_OBJ(instance->background_anim), LV_OBJ_FLAG_HIDDEN);

    AnimFile* file = anim_play_get_file(instance->background_anim);
    if(file) {
        if(!anim_file_set_section(file, AnimFilePlayFlagNone, ANIM_FILE_WHOLE_SECTION)) {
            FURI_LOG_E(TAG, "failed to reset icon animation");
        }
        anim_play_start(instance->background_anim);
    }
}

void anim_title_card_run_icon_anim(AnimTitleCard* instance, const char* section) {
    furi_check(instance);

    AnimFile* file = anim_play_get_file(instance->icon_anim);
    if(file) {
        if(!anim_file_set_section(file, AnimFilePlayFlagNone, section)) {
            FURI_LOG_E(TAG, "icon doesn't have \"%s\" section", section);
        }
        anim_play_start(instance->icon_anim);
    }
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
