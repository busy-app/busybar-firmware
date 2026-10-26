#include "smenu_item_editable.h"
#include "private/smenu_item_i.h"

#include <lvgl_addons/extensions/lv_label_ext.h>

typedef struct {
    char** choices;
    int32_t choice_count;
} SMenuItemEditableSelectorData;

typedef struct {
    char* suffix;
} SMenuItemEditableSpinboxData;

typedef struct {
    void (*update_ui)(SMenuItemEditable* instance);
    void (*free_data)(void* type_data);
} SMenuItemEditableTypeApi;

struct SMenuItemEditable {
    SMenuItem base;

    lv_obj_t* primary_label;
    lv_obj_t* left_arrow_label;
    lv_obj_t* value_label;
    lv_obj_t* right_arrow_label;

    lv_label_long_mode_t standby_long_mode;

    int32_t min;
    int32_t max;
    int32_t value;
    int32_t step;

    void* type_data;
    const SMenuItemEditableTypeApi* type_api;
};

const lv_obj_class_t smenu_item_editable_lvgl_class;

/* Item Type API */

static void* smenu_item_selector_data_alloc(const char* choices_text[], int32_t choices_count) {
    SMenuItemEditableSelectorData* data = malloc(sizeof(*data));
    data->choices = calloc(choices_count, sizeof(*data->choices));
    data->choice_count = choices_count;

    for(int32_t i = 0; i < choices_count; i++) {
        data->choices[i] = strdup(choices_text[i]);
    }

    return data;
}

static void smenu_item_selector_ui_update(SMenuItemEditable* instance) {
    SMenuItemEditableSelectorData* data = instance->type_data;

    lv_label_set_text_fmt(instance->value_label, "%s", data->choices[instance->value]);
}

static void smenu_item_selector_data_free(void* type_data) {
    SMenuItemEditableSelectorData* data = type_data;

    for(int32_t i = 0; i < data->choice_count; i++) {
        free(data->choices[i]);
    }

    free(data->choices);
    free(data);
}

static void* smenu_item_spinbox_data_alloc(const char* suffix) {
    SMenuItemEditableSpinboxData* data = malloc(sizeof(*data));
    data->suffix = suffix ? strdup(suffix) : NULL;

    return data;
}

static void smenu_item_spinbox_ui_update(SMenuItemEditable* instance) {
    SMenuItemEditableSpinboxData* data = instance->type_data;

    if(data->suffix) {
        lv_label_set_text_fmt(
            instance->value_label, "%" PRIi32 " %s", instance->value, data->suffix);
    } else {
        lv_label_set_text_fmt(instance->value_label, "%" PRIi32, instance->value);
    }
}

static void smenu_item_spinbox_data_free(void* type_data) {
    SMenuItemEditableSpinboxData* data = type_data;

    free(data->suffix);
    free(data);
}

static void smenu_item_timebox_ui_update(SMenuItemEditable* instance) {
    int32_t hours = instance->value / 60;
    int32_t minutes = instance->value % 60;

    if(hours == 0) {
        lv_label_set_text_fmt(instance->value_label, "%" PRIi32 " m", minutes);
    } else if(minutes == 0) {
        lv_label_set_text_fmt(instance->value_label, "%" PRIi32 " h", hours);
    } else {
        lv_label_set_text_fmt(instance->value_label, "%" PRIi32 ":%02" PRIi32, hours, minutes);
    }
}

static void smenu_item_switch_ui_update(SMenuItemEditable* instance) {
    lv_label_set_text_static(instance->value_label, instance->value ? "On" : "Off");
}

/* Implementation */

static void smenu_item_editable_set_edited(SMenuItemEditable* instance, bool is_edited) {
    lv_obj_set_state(instance->base.right_container, LV_STATE_EDITED, is_edited);
    lv_obj_set_state(TO_LV_OBJ(instance), LV_STATE_FOCUSED, !is_edited);

    if(is_edited) {
        lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);
        lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_label_ext_set_anim_speed(
            instance->value_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
    } else {
        lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_DOTS);
        lv_label_set_long_mode(instance->primary_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_label_ext_set_anim_speed(
            instance->primary_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
    }
}

static void smenu_item_editable_update(SMenuItemEditable* instance) {
    lv_obj_set_state(
        instance->left_arrow_label, LV_STATE_DISABLED, instance->value == instance->min);
    lv_obj_set_state(
        instance->right_arrow_label, LV_STATE_DISABLED, instance->value == instance->max);

    instance->type_api->update_ui(instance);

    if(lv_obj_has_state(instance->base.right_container, LV_STATE_EDITED)) {
        lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_label_ext_set_anim_speed(
            instance->value_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
    }
}

static void smenu_item_editable_increment(SMenuItemEditable* instance) {
    if(instance->value < instance->max) {
        int32_t remainder = instance->value % instance->step;
        remainder = (remainder < 0) ? remainder + instance->step : remainder;

        uint32_t distance = instance->step - remainder;
        instance->value = (distance <= (uint32_t)instance->max - (uint32_t)instance->value) ?
                              instance->value + (int32_t)distance :
                              instance->max;

        if(instance->base.callback) {
            instance->base.callback(&instance->base);
        }

        smenu_item_editable_update(instance);
    }
}

static void smenu_item_editable_decrement(SMenuItemEditable* instance) {
    if(instance->value > instance->min) {
        int32_t remainder = instance->value % instance->step;
        remainder = (remainder < 0) ? remainder + instance->step : remainder;

        uint32_t distance = (remainder > 0) ? remainder : instance->step;
        instance->value = (distance <= (uint32_t)instance->value - (uint32_t)instance->min) ?
                              instance->value - (int32_t)distance :
                              instance->min;

        if(instance->base.callback) {
            instance->base.callback(&instance->base);
        }

        smenu_item_editable_update(instance);
    }
}

static void smenu_item_editable_value_init(
    SMenuItemEditable* instance,
    int32_t min,
    int32_t max,
    int32_t step) {
    instance->min = min;
    instance->max = max;
    instance->step = step;
    instance->value = min;

    smenu_item_editable_update(instance);
}

static void smenu_item_editable_on_lvgl_event(const lv_obj_class_t* class, lv_event_t* event) {
    if(lv_obj_event_base(class, event) == LV_RESULT_OK) {
        lv_event_code_t event_code = lv_event_get_code(event);
        SMenuItemEditable* instance = (SMenuItemEditable*)lv_event_get_target_obj(event);

        if(event_code == LV_EVENT_FOCUSED) {
            lv_label_set_long_mode(instance->primary_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
            lv_label_ext_set_anim_speed(
                instance->primary_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
        } else if(event_code == LV_EVENT_DEFOCUSED) {
            lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);
            lv_obj_remove_state(instance->base.right_container, LV_STATE_EDITED);
            lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_DOTS);
        }
    }
}

static bool smenu_item_editable_on_input(Widget* widget, const InputEvent* event) {
    SMenuItemEditable* instance = (SMenuItemEditable*)widget;

    bool is_consumed = false;
    if(instance->base.is_active && event->type == InputTypeShort) {
        InputKey key = event->key;

        if(lv_obj_has_state(instance->base.right_container, LV_STATE_EDITED)) {
            if(key == InputKeyUp) {
                smenu_item_editable_increment(instance);
                is_consumed = true;
            } else if(key == InputKeyDown) {
                smenu_item_editable_decrement(instance);
                is_consumed = true;
            } else if(key == InputKeyOk || key == InputKeyStart || key == InputKeyBack) {
                smenu_item_editable_set_edited(instance, false);
                is_consumed = true;
            }
        } else if(key == InputKeyOk || key == InputKeyStart) {
            smenu_item_editable_set_edited(instance, true);
            is_consumed = true;
        }
    }

    return is_consumed;
}

static void smenu_item_editable_style_front(const lv_obj_class_t* class, Widget* widget) {
    widget_style(class, widget);

    SMenuItemEditable* instance = (SMenuItemEditable*)widget;

    lv_obj_set_style_text_color(
        instance->base.right_container, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->base.right_container, lv_color_white(), LV_PART_MAIN | LV_STATE_EDITED);

    instance->standby_long_mode = LV_LABEL_LONG_MODE_CLIP;
    lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);

    lv_obj_set_style_text_color(
        instance->left_arrow_label, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_pad_hor(instance->value_label, 1, LV_PART_MAIN);

    lv_obj_set_style_text_color(
        instance->right_arrow_label, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DISABLED);
}

static void smenu_item_editable_style_back(const lv_obj_class_t* class, Widget* widget) {
    widget_style(class, widget);

    SMenuItemEditable* instance = (SMenuItemEditable*)widget;

    lv_obj_set_style_radius(instance->base.right_container, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->base.right_container, lv_color_black(), LV_PART_MAIN | LV_STATE_EDITED);
    lv_obj_set_style_bg_opa(
        instance->base.right_container, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_EDITED);

    instance->standby_long_mode = LV_LABEL_LONG_MODE_DOTS;
    lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);

    lv_obj_set_style_text_color(
        instance->left_arrow_label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_pad_hor(instance->value_label, 2, LV_PART_MAIN);

    lv_obj_set_style_text_color(
        instance->right_arrow_label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);
}

static void smenu_item_editable_lvgl_constructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenuItemEditable* instance = (SMenuItemEditable*)object;

    instance->primary_label = lv_label_create(instance->base.left_container);
    lv_obj_set_flex_grow(instance->primary_label, 1);
    lv_label_ext_set_max_lines(instance->primary_label, 1);
    lv_obj_set_style_anim(
        instance->primary_label, &smenu_item_long_text_anim_template, LV_PART_MAIN);

    instance->left_arrow_label = lv_label_create(instance->base.right_container);
    lv_label_set_text_static(instance->left_arrow_label, "‹");

    instance->value_label = lv_label_create(instance->base.right_container);
    lv_label_ext_set_max_lines(instance->value_label, 1);
    lv_obj_set_style_text_align(instance->value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_add_flag(instance->value_label, SMENU_ITEM_RIGHT_FLEX_GROW_FLAG);
    lv_obj_set_style_anim(
        instance->value_label, &smenu_item_long_text_anim_template, LV_PART_MAIN);

    instance->right_arrow_label = lv_label_create(instance->base.right_container);
    lv_label_set_text_static(instance->right_arrow_label, "›");
}

static void smenu_item_editable_lvgl_destructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenuItemEditable* instance = (SMenuItemEditable*)object;

    if(instance->type_api->free_data) {
        instance->type_api->free_data(instance->type_data);
    }
}

/* Public API */

static SMenuItemEditable* smenu_item_editable_alloc(Widget* parent) {
    furi_check(parent);

    SMenuItemEditable* instance = (SMenuItemEditable*)lv_obj_class_create_obj(
        &smenu_item_editable_lvgl_class, TO_LV_OBJ(parent));

    lv_obj_class_init_obj(TO_LV_OBJ(instance));

    return instance;
}

SMenuItemEditable* smenu_item_editable_switch_alloc(Widget* parent) {
    static const SMenuItemEditableTypeApi type_api = {
        .update_ui = smenu_item_switch_ui_update,
        .free_data = NULL,
    };

    SMenuItemEditable* instance = smenu_item_editable_alloc(parent);
    instance->type_data = NULL;
    instance->type_api = &type_api;

    smenu_item_editable_value_init(instance, 0, 1, 1);

    return instance;
}

SMenuItemEditable* smenu_item_editable_timebox_alloc(
    Widget* parent,
    int32_t min_minutes,
    int32_t max_minutes,
    int32_t step_minutes) {
    furi_check(min_minutes <= max_minutes);
    furi_check(min_minutes >= 0);
    furi_check(step_minutes > 0);

    static const SMenuItemEditableTypeApi type_api = {
        .update_ui = smenu_item_timebox_ui_update,
        .free_data = NULL,
    };

    SMenuItemEditable* instance = smenu_item_editable_alloc(parent);
    instance->type_data = NULL;
    instance->type_api = &type_api;

    smenu_item_editable_value_init(instance, min_minutes, max_minutes, step_minutes);

    return instance;
}

SMenuItemEditable* smenu_item_editable_spinbox_alloc(
    Widget* parent,
    const char* suffix,
    int32_t min,
    int32_t max,
    int32_t step) {
    furi_check(min <= max);
    furi_check(step > 0);

    static const SMenuItemEditableTypeApi type_api = {
        .update_ui = smenu_item_spinbox_ui_update,
        .free_data = smenu_item_spinbox_data_free,
    };

    SMenuItemEditable* instance = smenu_item_editable_alloc(parent);
    instance->type_data = smenu_item_spinbox_data_alloc(suffix);
    instance->type_api = &type_api;

    smenu_item_editable_value_init(instance, min, max, step);

    return instance;
}

SMenuItemEditable* smenu_item_editable_selector_alloc(
    Widget* parent,
    const char* choices_text[],
    int32_t choices_count) {
    furi_check(choices_text);
    furi_check(choices_count > 0);

    static const SMenuItemEditableTypeApi type_api = {
        .update_ui = smenu_item_selector_ui_update,
        .free_data = smenu_item_selector_data_free,
    };

    SMenuItemEditable* instance = smenu_item_editable_alloc(parent);
    instance->type_data = smenu_item_selector_data_alloc(choices_text, choices_count);
    instance->type_api = &type_api;

    smenu_item_editable_value_init(instance, 0, choices_count - 1, 1);

    return instance;
}

void smenu_item_editable_set_primary_text(SMenuItemEditable* instance, const char* text) {
    furi_check(instance);
    furi_check(text);

    lv_label_set_text(instance->primary_label, text);

    if(lv_obj_has_state(TO_LV_OBJ(instance), LV_STATE_FOCUSED)) {
        lv_label_ext_set_anim_speed(
            instance->primary_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
    }
}

void smenu_item_editable_set_value(SMenuItemEditable* instance, int32_t value) {
    furi_check(instance);

    value = CLAMP(value, instance->max, instance->min);

    if(instance->value != value) {
        instance->value = value;
        smenu_item_editable_update(instance);
    }
}

int32_t smenu_item_editable_get_value(SMenuItemEditable* instance) {
    furi_check(instance);

    return instance->value;
}

SMenuItem* smenu_item_editable_get_base(SMenuItemEditable* instance) {
    furi_check(instance);

    return &instance->base;
}

/* LVGL Class Descriptor */

const lv_obj_class_t smenu_item_editable_lvgl_class = {
    .base_class = &smenu_item_lvgl_class,
    .constructor_cb = smenu_item_editable_lvgl_constructor,
    .destructor_cb = smenu_item_editable_lvgl_destructor,
    .event_cb = smenu_item_editable_on_lvgl_event,
    .name = "widget-smenu-item-editable",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SMenuItemEditable),
    .user_data =
        (void*)&(const WidgetClassData){
            .input_callback = smenu_item_editable_on_input,
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = smenu_item_editable_style_front,
                    [GuiDisplayIdBack] = smenu_item_editable_style_back,
                },
        },
};
