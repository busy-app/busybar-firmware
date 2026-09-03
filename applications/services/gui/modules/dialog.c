#include "dialog.h"

#include <gui/widget_i.h>

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS          (&dialog_lvgl_class)
#define MY_TEXT_CLASS     (&dialog_text_lvgl_class)
#define MY_TEXT_SUB_CLASS (&dialog_text_sub_lvgl_class)
#define MY_OPTION_CLASS   (&dialog_option_lvgl_class)
#define MY_CURSOR_CLASS   (&dialog_cursor_lvgl_class)

#define ARROW_CHAR "▶" // U+25B6

struct Dialog {
    Widget base;
    lv_obj_t* icon;
    lv_obj_t* text_cont;
    lv_obj_t* text_main;
    lv_obj_t* text_sub;
    lv_obj_t* options_cont;
    lv_group_t* options_group;
    DialogCallback callback;
    void* context;
};

typedef struct {
    lv_obj_t base;
    lv_obj_t* label;
    lv_obj_t* arrow;
    uint8_t index;
} DialogOption;

const lv_obj_class_t dialog_lvgl_class;
const lv_obj_class_t dialog_text_lvgl_class;
const lv_obj_class_t dialog_text_sub_lvgl_class;
const lv_obj_class_t dialog_option_lvgl_class;
const lv_obj_class_t dialog_cursor_lvgl_class;

static bool dialog_input_callback(Widget* widget, const InputEvent* event) {
    Dialog* instance = (Dialog*)widget;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            lv_group_focus_next(instance->options_group);
            consumed = true;

        } else if(event->key == InputKeyDown) {
            lv_group_focus_prev(instance->options_group);
            consumed = true;

        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            if(instance->callback) {
                DialogOption* option =
                    (DialogOption*)lv_group_get_focused(instance->options_group);
                furi_check(option);
                instance->callback(option->index, instance->context);
            }

            consumed = true;
        }
    }

    return consumed;
}

static lv_obj_t* dialog_option_alloc(lv_obj_t* parent, uint32_t index) {
    lv_obj_t* obj = lv_obj_class_create_obj(MY_OPTION_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    DialogOption* instance = (DialogOption*)obj;

    instance->index = index;
    lv_label_set_text(instance->label, "");

    return obj;
}

static void dialog_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    Dialog* instance = (Dialog*)obj;
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);

    instance->icon = lv_image_create(obj);
    lv_obj_add_flag(instance->icon, LV_OBJ_FLAG_HIDDEN);

    instance->text_cont = lv_obj_create(obj);
    lv_obj_set_style_pad_all(instance->text_cont, 0, LV_PART_MAIN);

    instance->text_main = lv_obj_class_create_obj(MY_TEXT_CLASS, instance->text_cont);
    lv_obj_class_init_obj(instance->text_main);
    lv_label_set_long_mode(instance->text_main, LV_LABEL_LONG_MODE_DOTS);
    lv_label_set_text(instance->text_main, "");

    instance->text_sub = lv_obj_class_create_obj(MY_TEXT_SUB_CLASS, instance->text_cont);
    lv_obj_class_init_obj(instance->text_sub);
    lv_label_set_long_mode(instance->text_sub, LV_LABEL_LONG_MODE_DOTS);
    lv_label_set_text(instance->text_sub, "");

    // Force single line
    lv_coord_t line_height =
        lv_font_get_line_height(lv_obj_get_style_text_font(instance->text_sub, LV_PART_MAIN));
    lv_obj_set_style_max_height(instance->text_sub, line_height, LV_PART_MAIN);

    lv_obj_add_flag(instance->text_sub, LV_OBJ_FLAG_HIDDEN);

    instance->options_cont = lv_obj_create(obj);
    lv_obj_set_flex_flow(instance->options_cont, LV_FLEX_FLOW_COLUMN);
    lv_flex_flow_t parent_flow = lv_obj_get_style_flex_flow(obj, LV_PART_MAIN);
    if(parent_flow == LV_FLEX_FLOW_COLUMN) {
        lv_obj_set_flex_flow(instance->text_cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(
            instance->text_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_size(instance->text_cont, LV_PCT(100), LV_SIZE_CONTENT);

        lv_obj_set_size(instance->options_cont, LV_PCT(100), LV_SIZE_CONTENT);
    } else {
        lv_obj_set_style_margin_right(instance->icon, 2, LV_PART_MAIN);
        lv_obj_set_flex_flow(instance->text_cont, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(
            instance->text_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_size(instance->text_cont, LV_PCT(100), LV_PCT(100));
        lv_obj_set_flex_grow(instance->text_cont, 1);

        lv_obj_set_size(instance->options_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_row(instance->options_cont, -2, LV_PART_MAIN);
    }

    instance->options_group = lv_group_create();
    lv_group_set_wrap(instance->options_group, false);

    lv_obj_t* option = dialog_option_alloc(instance->options_cont, 0);
    lv_group_add_obj(instance->options_group, option);

    option = dialog_option_alloc(instance->options_cont, 1);
    lv_group_add_obj(instance->options_group, option);
}

static void dialog_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    Dialog* instance = (Dialog*)obj;
    lv_group_delete(instance->options_group);
}

static void dialog_option_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    DialogOption* instance = (DialogOption*)obj;

    instance->arrow = lv_obj_class_create_obj(MY_CURSOR_CLASS, obj);
    lv_obj_class_init_obj(instance->arrow);
    lv_label_set_text(instance->arrow, ARROW_CHAR);

    instance->label = lv_label_create(obj);
    lv_label_set_long_mode(instance->label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(instance->label, LV_SIZE_CONTENT);
}

static void dialog_option_lvgl_event(const lv_obj_class_t* class_p, lv_event_t* event) {
    LV_UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_OPTION_CLASS, event);
    if(res != LV_RESULT_OK) return;

    const lv_event_code_t code = lv_event_get_code(event);
    DialogOption* instance = lv_event_get_target(event);

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_add_state(instance->arrow, LV_STATE_FOCUSED);
        lv_obj_add_state(instance->label, LV_STATE_FOCUSED);
    } else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_remove_state(instance->arrow, LV_STATE_FOCUSED);
        lv_obj_remove_state(instance->label, LV_STATE_FOCUSED);
    }
}

// Public API

Dialog* dialog_alloc(Widget* widget) {
    furi_check(widget);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)widget);
    lv_obj_class_init_obj(obj);

    Dialog* instance = (Dialog*)obj;

    return instance;
}

void dialog_free(Dialog* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* dialog_get_base(Dialog* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void dialog_set_text(Dialog* instance, const char* text) {
    furi_check(instance);
    lv_label_set_text(instance->text_main, text);
}

void dialog_set_text_sub(Dialog* instance, const char* text) {
    furi_check(instance);
    if(text) {
        lv_label_set_text(instance->text_sub, text);
        lv_obj_remove_flag(instance->text_sub, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(instance->text_sub, LV_OBJ_FLAG_HIDDEN);
    }
}

void dialog_set_options(Dialog* instance, const char* text_0, const char* text_1) {
    furi_check(instance);

    lv_obj_t* option = lv_group_get_obj_by_index(instance->options_group, 0);
    lv_label_set_text(((DialogOption*)option)->label, text_0);

    option = lv_group_get_obj_by_index(instance->options_group, 1);
    lv_label_set_text(((DialogOption*)option)->label, text_1);
}

void dialog_set_option_colors(Dialog* instance, Color color_0, Color color_1) {
    furi_check(instance);

    lv_obj_t* option = lv_group_get_obj_by_index(instance->options_group, 0);
    lv_obj_set_style_text_color(
        ((DialogOption*)option)->arrow, TO_LV_COLOR(color_0), LV_STATE_FOCUSED);
    lv_obj_set_style_text_color(
        ((DialogOption*)option)->label, TO_LV_COLOR(color_0), LV_STATE_FOCUSED);

    option = lv_group_get_obj_by_index(instance->options_group, 1);
    lv_obj_set_style_text_color(
        ((DialogOption*)option)->arrow, TO_LV_COLOR(color_1), LV_STATE_FOCUSED);
    lv_obj_set_style_text_color(
        ((DialogOption*)option)->label, TO_LV_COLOR(color_1), LV_STATE_FOCUSED);
}

void dialog_select_option(Dialog* instance, uint8_t index) {
    furi_check(instance);
    furi_check(index <= 1);
    lv_obj_t* target = lv_group_get_obj_by_index(instance->options_group, index);
    lv_group_focus_obj(target);
}

void dialog_set_callback(Dialog* instance, DialogCallback callback, void* context) {
    furi_check(instance);

    instance->callback = callback;
    instance->context = context;
}

void dialog_set_icon(Dialog* instance, const char* icon_source) {
    furi_check(instance);
    if(icon_source) {
        lv_image_set_src(instance->icon, icon_source);
        lv_obj_remove_flag(instance->icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(instance->icon, LV_OBJ_FLAG_HIDDEN);
    }
}

// LVGL class descriptors

const lv_obj_class_t dialog_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = dialog_lvgl_constructor,
    .destructor_cb = dialog_lvgl_destructor,
    .name = "widget-dialog",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Dialog),
    .user_data =
        (void*)&(const WidgetClassData){
            .input_callback = dialog_input_callback,
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = NULL,
                    [GuiDisplayIdBack] = NULL,
                },
        },
};

const lv_obj_class_t dialog_text_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "dialog-text",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t dialog_text_sub_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "dialog-text-sub",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t dialog_option_lvgl_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = dialog_option_lvgl_constructor,
    .event_cb = dialog_option_lvgl_event,
    .name = "dialog-option",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(DialogOption),
};

const lv_obj_class_t dialog_cursor_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "dialog-cursor",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
