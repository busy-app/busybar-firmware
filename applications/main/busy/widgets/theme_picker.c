#include "theme_picker.h"

#include <gui/widget_i.h>

#include <gui/modules/image.h>
#include <gui/modules/anim_player.h>

#define MY_CLASS (&theme_picker_lvgl_class)

#define SYM_ARROW_LEFT      "◃"
#define SYM_ARROW_RIGHT     "▹"
#define SYM_ARROW_LEFT_BIG  "<"
#define SYM_ARROW_RIGHT_BIG ">"

struct ThemePicker {
    Widget base;
    Image* image;
    AnimPlayer* anim_player;

    const ThemePickerModel* model;
    uint32_t current_idx;

    ThemePickerCallback callback;
    void* callback_context;
};

const lv_obj_class_t theme_picker_lvgl_class;

// Function prototypes

static bool theme_picker_input_callback(Widget* widget, const InputEvent* event);

// LVGL-specific code

static lv_obj_t* theme_picker_create_decoration(lv_obj_t* parent, bool reverse) {
    lv_obj_t* deco = lv_obj_create(parent);
    lv_obj_set_size(deco, 10, 16);

    lv_obj_t* shadow = lv_obj_create(deco);
    lv_obj_set_size(shadow, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(shadow, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(shadow, reverse ? lv_color_white() : lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(shadow, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_opa(shadow, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(
        shadow, reverse ? lv_color_black() : lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_blend_mode(shadow, LV_BLEND_MODE_MULTIPLY, LV_PART_MAIN);

    lv_obj_t* arrow = lv_label_create(deco);

    if(lv_theme_get_font_normal(parent) == &lv_font_tiny5_8) {
        lv_label_set_text(arrow, reverse ? SYM_ARROW_RIGHT : SYM_ARROW_LEFT);
    } else {
        lv_label_set_text(arrow, reverse ? SYM_ARROW_RIGHT_BIG : SYM_ARROW_LEFT_BIG);
    }

    lv_obj_set_style_text_color(arrow, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_align(arrow, reverse ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(arrow, 1, LV_PART_MAIN);

    return deco;
}

static void theme_picker_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ThemePicker* instance = (ThemePicker*)obj;
    widget_set_input_feed_callback((Widget*)instance, theme_picker_input_callback);

    instance->image = image_alloc((Widget*)obj);
    instance->anim_player = anim_player_alloc((Widget*)obj);

    lv_obj_t* deco_left = theme_picker_create_decoration(obj, false);
    lv_obj_set_style_align(deco_left, LV_ALIGN_TOP_LEFT, LV_PART_MAIN);

    lv_obj_t* deco_right = theme_picker_create_decoration(obj, true);
    lv_obj_set_style_align(deco_right, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN);
}

// Implementation

static void theme_picker_update_image(ThemePicker* instance) {
    const BusyTheme* theme = theme_picker_model_get_item(instance->model, instance->current_idx);

    BusyThemeInfo info;
    busy_theme_get_info(theme, &info);

    const BusyThemeFileType bg_type = info.bg_type;
    const char* bg_path = info.bg_path;

    if(bg_type == BusyThemeFileTypeImage) {
        widget_set_visible((Widget*)instance->image, true);
        widget_set_visible((Widget*)instance->anim_player, false);

        anim_player_pause(instance->anim_player);
        image_set_source(instance->image, bg_path);

    } else if(bg_type == BusyThemeFileTypeAnim) {
        widget_set_visible((Widget*)instance->image, false);
        widget_set_visible((Widget*)instance->anim_player, true);

        if(anim_player_set_source(instance->anim_player, bg_path)) {
            anim_player_loop_whole(instance->anim_player);
        }
    }
}

static bool theme_picker_input_callback(Widget* widget, const InputEvent* event) {
    ThemePicker* instance = (ThemePicker*)widget;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            if(instance->current_idx == theme_picker_model_get_item_count(instance->model) - 1) {
                instance->current_idx = 0;
            } else {
                ++instance->current_idx;
            }

            consumed = true;

        } else if(event->key == InputKeyDown) {
            if(instance->current_idx == 0) {
                instance->current_idx = theme_picker_model_get_item_count(instance->model) - 1;
            } else {
                --instance->current_idx;
            }

            consumed = true;
        }
    }

    if(consumed) {
        theme_picker_update_image(instance);

        if(instance->callback) {
            instance->callback(instance->current_idx, instance->callback_context);
        }
    }

    return consumed;
}

// Public API

ThemePicker* theme_picker_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    ThemePicker* instance = (ThemePicker*)obj;
    return instance;
}

void theme_picker_free(ThemePicker* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* theme_picker_get_base(ThemePicker* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void theme_picker_set_model(ThemePicker* instance, const ThemePickerModel* model) {
    furi_check(instance);
    furi_check(model);

    instance->model = model;
    theme_picker_update_image(instance);
}

void theme_picker_set_callback(ThemePicker* instance, ThemePickerCallback callback, void* context) {
    furi_check(instance);

    instance->callback = callback;
    instance->callback_context = context;
}

void theme_picker_set_current_item(ThemePicker* instance, uint32_t index) {
    furi_check(instance);

    instance->current_idx = index;
    theme_picker_update_image(instance);
}

// LVGL class descriptor

const lv_obj_class_t theme_picker_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = theme_picker_lvgl_constructor,
    .name = "widget-theme-picker",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(ThemePicker),
};
