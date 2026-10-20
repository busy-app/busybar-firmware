#include "smenu_item_editable.h"
#include "private/smenu_item_i.h"

#include <lvgl_addons/extensions/lv_label_ext.h>

typedef struct {
    void (*format)(SMenuItemEditable* instance);
    void (*free_state)(SMenuItemEditable* instance);
} SMenuItemEditableKind;

struct SMenuItemEditable {
    SMenuItem base;

    lv_obj_t* primary_label;
    lv_obj_t* arrow_left;
    lv_obj_t* value_label;
    lv_obj_t* arrow_right;

    const SMenuItemEditableKind* kind;

    int32_t min;
    int32_t max;
    int32_t value;
    uint32_t step;

    char** choices;
    uint32_t choice_count;
    char* suffix;

    lv_label_long_mode_t standby_long_mode;
};

const lv_obj_class_t smenu_item_editable_lvgl_class;

/* Item Kinds */

static void smenu_item_selector_format(SMenuItemEditable* instance) {
    lv_label_set_text_fmt(instance->value_label, "%s", instance->choices[instance->value]);
}

static void smenu_item_selector_free_state(SMenuItemEditable* instance) {
    for(uint32_t i = 0; i < instance->choice_count; ++i) {
        free(instance->choices[i]);
    }

    free(instance->choices);
}

static void smenu_item_spinbox_format(SMenuItemEditable* instance) {
    if(instance->suffix) {
        lv_label_set_text_fmt(instance->value_label, "%ld %s", instance->value, instance->suffix);
    } else {
        lv_label_set_text_fmt(instance->value_label, "%ld", instance->value);
    }
}

static void smenu_item_spinbox_free_state(SMenuItemEditable* instance) {
    if(instance->suffix) {
        free(instance->suffix);
    }
}

static void smenu_item_timebox_format(SMenuItemEditable* instance) {
    int32_t hours = instance->value / 60;
    int32_t minutes = instance->value % 60;

    if(hours == 0) {
        lv_label_set_text_fmt(instance->value_label, "%ld m", minutes);
    } else if(minutes == 0) {
        lv_label_set_text_fmt(instance->value_label, "%ld h", hours);
    } else {
        lv_label_set_text_fmt(instance->value_label, "%ld:%02ld", hours, minutes);
    }
}

static void smenu_item_switch_format(SMenuItemEditable* instance) {
    lv_label_set_text_fmt(instance->value_label, "%s", instance->value ? "On" : "Off");
}

static const SMenuItemEditableKind smenu_item_selector_kind = {
    .format = smenu_item_selector_format,
    .free_state = smenu_item_selector_free_state,
};

static const SMenuItemEditableKind smenu_item_spinbox_kind = {
    .format = smenu_item_spinbox_format,
    .free_state = smenu_item_spinbox_free_state,
};

static const SMenuItemEditableKind smenu_item_timebox_kind = {
    .format = smenu_item_timebox_format,
    .free_state = NULL,
};

static const SMenuItemEditableKind smenu_item_switch_kind = {
    .format = smenu_item_switch_format,
    .free_state = NULL,
};

/* Implementation */

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
        }
    }
}

static void smenu_item_editable_set_edited(SMenuItemEditable* instance, bool is_edited) {
    lv_obj_set_state(instance->base.right_container, LV_STATE_EDITED, is_edited);
    lv_obj_set_state(TO_LV_OBJ(instance), LV_STATE_FOCUSED, !is_edited);

    if(is_edited) {
        lv_label_set_long_mode(instance->primary_label, instance->standby_long_mode);
        lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_label_ext_set_anim_speed(
            instance->value_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
    } else {
        lv_label_set_long_mode(instance->primary_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_label_ext_set_anim_speed(
            instance->primary_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
        lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_DOTS);
    }
}

static void smenu_item_editable_update(SMenuItemEditable* instance) {
    lv_obj_set_state(instance->arrow_left, LV_STATE_DISABLED, instance->value == instance->min);
    lv_obj_set_state(instance->arrow_right, LV_STATE_DISABLED, instance->value == instance->max);

    instance->kind->format(instance);

    if(lv_obj_has_state(instance->base.right_container, LV_STATE_EDITED)) {
        lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_label_ext_set_anim_speed(
            instance->value_label, SMENU_ITEM_LONG_TEXT_ANIM_SPEED_PX_PER_M);
    }
}

static void smenu_item_editable_increment(SMenuItemEditable* instance) {
    if(instance->value < instance->max) {
        int64_t step_index = ((int64_t)instance->value - instance->min) / instance->step + 1;
        instance->value = instance->min + step_index * instance->step;

        if(instance->base.callback) {
            instance->base.callback(&instance->base);
        }

        smenu_item_editable_update(instance);
    }
}

static void smenu_item_editable_decrement(SMenuItemEditable* instance) {
    if(instance->value > instance->min) {
        int64_t step_index = ((int64_t)instance->value - instance->min - 1) / instance->step;
        instance->value = instance->min + step_index * instance->step;

        if(instance->base.callback) {
            instance->base.callback(&instance->base);
        }

        smenu_item_editable_update(instance);
    }
}

static bool smenu_item_editable_on_input(Widget* widget, const InputEvent* event) {
    SMenuItemEditable* instance = (SMenuItemEditable*)widget;

    bool is_consumed = false;

    if(instance->base.is_active && event->type == InputTypeShort) {
        if(lv_obj_has_state(instance->base.right_container, LV_STATE_EDITED)) {
            if(event->key == InputKeyUp) {
                smenu_item_editable_increment(instance);
                is_consumed = true;
            } else if(event->key == InputKeyDown) {
                smenu_item_editable_decrement(instance);
                is_consumed = true;
            } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
                smenu_item_editable_set_edited(instance, false);
                is_consumed = true;
            } else if(event->key == InputKeyBack) {
                smenu_item_editable_set_edited(instance, false);
                if(instance->base.callback) {
                    instance->base.callback(&instance->base);
                }

                is_consumed = true;
            }
        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            smenu_item_editable_set_edited(instance, true);
            is_consumed = true;
        }
    }

    return is_consumed;
}

static void smenu_item_editable_setup(
    SMenuItemEditable* instance,
    int32_t min,
    int32_t max,
    uint32_t step) {
    furi_check(max >= min, "Range error: min > max");
    furi_check(step != 0, "Step error: step must be non-zero");
    furi_check(((int64_t)max - min) % step == 0, "Step error: range must be evenly divisible");

    instance->min = min;
    instance->max = max;
    instance->step = step;
    instance->value = min;

    smenu_item_editable_update(instance);
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
        instance->arrow_left, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_pad_hor(instance->value_label, 1, LV_PART_MAIN);

    lv_obj_set_style_text_color(
        instance->arrow_right, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DISABLED);
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
        instance->arrow_left, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_pad_hor(instance->value_label, 2, LV_PART_MAIN);

    lv_obj_set_style_text_color(
        instance->arrow_right, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);
}

static void smenu_item_editable_lvgl_constructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenuItemEditable* instance = (SMenuItemEditable*)object;

    instance->primary_label = lv_label_create(instance->base.left_container);
    lv_obj_set_flex_grow(instance->primary_label, 1);
    lv_label_ext_set_max_lines(instance->primary_label, 1);
    lv_obj_set_style_anim(
        instance->primary_label, &smenu_item_long_text_anim_template, LV_PART_MAIN);

    instance->arrow_left = lv_label_create(instance->base.right_container);
    lv_label_set_text_static(instance->arrow_left, "‹");

    instance->value_label = lv_label_create(instance->base.right_container);
    lv_label_ext_set_max_lines(instance->value_label, 1);
    lv_obj_set_style_text_align(instance->value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(instance->value_label, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_add_flag(instance->value_label, SMENU_ITEM_RIGHT_FLEX_GROW_FLAG);
    lv_obj_set_style_anim(
        instance->value_label, &smenu_item_long_text_anim_template, LV_PART_MAIN);

    instance->arrow_right = lv_label_create(instance->base.right_container);
    lv_label_set_text_static(instance->arrow_right, "›");
}

static void smenu_item_editable_lvgl_destructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenuItemEditable* instance = (SMenuItemEditable*)object;

    if(instance->kind->free_state) {
        instance->kind->free_state(instance);
    }
}

/* Public API */

static SMenuItemEditable*
    smenu_item_editable_alloc(Widget* parent, const SMenuItemEditableKind* kind) {
    furi_check(parent);

    SMenuItemEditable* instance = (SMenuItemEditable*)lv_obj_class_create_obj(
        &smenu_item_editable_lvgl_class, TO_LV_OBJ(parent));

    instance->kind = kind;

    lv_obj_class_init_obj(TO_LV_OBJ(instance));

    return instance;
}

SMenuItemEditable* smenu_item_editable_switch_alloc(Widget* parent) {
    SMenuItemEditable* instance = smenu_item_editable_alloc(parent, &smenu_item_switch_kind);

    smenu_item_editable_setup(instance, 0, 1, 1);

    return instance;
}

SMenuItemEditable* smenu_item_editable_timebox_alloc(
    Widget* parent,
    int32_t min_minutes,
    int32_t max_minutes,
    uint32_t step_minutes) {
    SMenuItemEditable* instance = smenu_item_editable_alloc(parent, &smenu_item_timebox_kind);

    smenu_item_editable_setup(instance, min_minutes, max_minutes, step_minutes);

    return instance;
}

SMenuItemEditable* smenu_item_editable_spinbox_alloc(
    Widget* parent,
    const char* suffix,
    int32_t min,
    int32_t max,
    uint32_t step) {
    SMenuItemEditable* instance = smenu_item_editable_alloc(parent, &smenu_item_spinbox_kind);
    instance->suffix = suffix ? strdup(suffix) : NULL;

    smenu_item_editable_setup(instance, min, max, step);

    return instance;
}

SMenuItemEditable* smenu_item_editable_selector_alloc(
    Widget* parent,
    const char* choices_text[],
    uint32_t choices_count) {
    furi_check(choices_text);
    furi_check(choices_count != 0);

    SMenuItemEditable* instance = smenu_item_editable_alloc(parent, &smenu_item_selector_kind);
    instance->choices = calloc(choices_count, sizeof(*instance->choices));
    instance->choice_count = choices_count;

    for(uint32_t i = 0; i < choices_count; i++) {
        instance->choices[i] = strdup(choices_text[i]);
    }

    smenu_item_editable_setup(instance, 0, choices_count - 1, 1);

    return instance;
}

void smenu_item_editable_set_primary_text(SMenuItemEditable* instance, const char* text) {
    furi_check(instance);
    furi_check(text);

    lv_label_set_text(instance->primary_label, text);
}

void smenu_item_editable_set_value(SMenuItemEditable* instance, int32_t value) {
    furi_check(instance);

    value = CLAMP(value, instance->max, instance->min);

    int64_t offset = (int64_t)value - instance->min;
    value = instance->min + offset / instance->step * instance->step;

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
