#include "smenu_item_simple.h"
#include "private/smenu_item_i.h"

#include <lvgl_addons/extensions/lv_label_ext.h>

struct SMenuItemSimple {
    SMenuItem base;

    lv_obj_t* primary_label;
    lv_obj_t* auxiliary_label;
    lv_obj_t* arrow_label;

    lv_label_long_mode_t standby_long_mode;
};

const lv_obj_class_t smenu_item_simple_lvgl_class;

/* Implementation */

static bool smenu_item_simple_on_input(Widget* widget, const InputEvent* event) {
    SMenuItemSimple* instance = (SMenuItemSimple*)widget;

    bool is_consumed = false;
    if(instance->base.is_active && event->type == InputTypeShort) {
        if(event->key == InputKeyOk || event->key == InputKeyStart) {
            if(instance->base.callback) {
                instance->base.callback(&instance->base);
            }

            is_consumed = true;
        }
    }

    return is_consumed;
}

static void smenu_item_simple_on_lvgl_event(const lv_obj_class_t* class, lv_event_t* event) {
    if(lv_obj_event_base(class, event) == LV_RESULT_OK) {
        lv_event_code_t event_code = lv_event_get_code(event);
        SMenuItemSimple* instance = (SMenuItemSimple*)lv_event_get_target_obj(event);

        if(event_code == LV_EVENT_FOCUSED) {
            lv_label_set_long_mode(instance->primary_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
            lv_label_ext_set_anim_speed(
                instance->primary_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
        } else if(event_code == LV_EVENT_DEFOCUSED) {
            lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);
        }
    }
}

static void smenu_item_simple_style_front(const lv_obj_class_t* class, Widget* widget) {
    widget_style(class, widget);

    SMenuItemSimple* instance = (SMenuItemSimple*)widget;

    instance->standby_long_mode = LV_LABEL_LONG_MODE_CLIP;
    lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);

    lv_obj_set_style_pad_right(instance->auxiliary_label, 1, LV_PART_MAIN);
}

static void smenu_item_simple_style_back(const lv_obj_class_t* class, Widget* widget) {
    widget_style(class, widget);

    SMenuItemSimple* instance = (SMenuItemSimple*)widget;

    instance->standby_long_mode = LV_LABEL_LONG_MODE_DOTS;
    lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);

    lv_obj_set_style_pad_right(instance->auxiliary_label, 2, LV_PART_MAIN);
}

static void smenu_item_simple_lvgl_constructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenuItemSimple* instance = (SMenuItemSimple*)object;

    instance->primary_label = lv_label_create(instance->base.left_container);
    lv_obj_set_flex_grow(instance->primary_label, 1);
    lv_label_ext_set_max_lines(instance->primary_label, 1);
    lv_obj_set_style_anim(
        instance->primary_label, &smenu_item_long_text_anim_template, LV_PART_MAIN);

    instance->auxiliary_label = lv_label_create(instance->base.right_container);
    lv_label_ext_set_max_lines(instance->auxiliary_label, 1);
    lv_obj_set_style_text_align(instance->auxiliary_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_label_set_long_mode(instance->auxiliary_label, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_add_flag(instance->auxiliary_label, SMENU_ITEM_RIGHT_FLEX_GROW_FLAG);

    instance->arrow_label = lv_label_create(instance->base.right_container);
    lv_label_set_text_static(instance->arrow_label, ">");
}

/* Public API */

SMenuItemSimple* smenu_item_simple_alloc(Widget* parent) {
    furi_check(parent);

    SMenuItemSimple* instance = (SMenuItemSimple*)lv_obj_class_create_obj(
        &smenu_item_simple_lvgl_class, TO_LV_OBJ(parent));

    lv_obj_class_init_obj(TO_LV_OBJ(instance));

    return instance;
}

void smenu_item_simple_set_primary_text(SMenuItemSimple* instance, const char* text) {
    furi_check(instance);
    furi_check(text);

    lv_label_set_text(instance->primary_label, text);
}

void smenu_item_simple_set_auxiliary_text(SMenuItemSimple* instance, const char* auxiliary_text) {
    furi_check(instance);

    if(auxiliary_text) {
        lv_label_set_text(instance->auxiliary_label, auxiliary_text);
        lv_obj_remove_flag(instance->auxiliary_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_static(instance->auxiliary_label, "");
        lv_obj_add_flag(instance->auxiliary_label, LV_OBJ_FLAG_HIDDEN);
    }
}

void smenu_item_simple_set_arrow_enabled(SMenuItemSimple* instance, bool is_arrow_enabled) {
    furi_check(instance);

    lv_obj_update_flag(instance->arrow_label, LV_OBJ_FLAG_HIDDEN, !is_arrow_enabled);
}

SMenuItem* smenu_item_simple_get_base(SMenuItemSimple* instance) {
    furi_check(instance);

    return &instance->base;
}

/* LVGL Class Descriptor */

const lv_obj_class_t smenu_item_simple_lvgl_class = {
    .base_class = &smenu_item_lvgl_class,
    .constructor_cb = smenu_item_simple_lvgl_constructor,
    .event_cb = smenu_item_simple_on_lvgl_event,
    .name = "widget-smenu-item-simple",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SMenuItemSimple),
    .user_data =
        (void*)&(const WidgetClassData){
            .input_callback = smenu_item_simple_on_input,
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = smenu_item_simple_style_front,
                    [GuiDisplayIdBack] = smenu_item_simple_style_back,
                },
        },
};
