#include "var_item_list.h"

#include <gui/widget_i.h>

#include <lvgl/src/core/lv_obj_class_private.h>
#include <lvgl/src/widgets/label/lv_label_private.h>

#define MY_CLASS        (&var_item_list_lvgl_class)
#define MY_ITEM_CLASS   (&var_item_lvgl_class)
#define MY_EDITOR_CLASS (&var_item_editor_lvgl_class)

#define SYM_INFINITY    "∞"
#define SYM_ARROW_LEFT  "◃"
#define SYM_ARROW_RIGHT "▹"

#define SCROLL_ANIM_DURATION_MS (64)

#define CHECK_RANGE_AND_STEP(min, max, step)                                               \
    do {                                                                                   \
        furi_check(max >= min, "Range error: min > max");                                  \
        furi_check((max - min) % step == 0, "Step error: range must be evenly divisible"); \
    } while(0)

#define SET_EDITOR_LABEL(label, fmt, ...) \
    (lv_label_set_text_fmt(label, "%s " fmt " %s", SYM_ARROW_LEFT, ##__VA_ARGS__, SYM_ARROW_RIGHT))

typedef enum {
    VarItemTypeSpinbox,
    VarItemTypeTimebox,
    VarItemTypeSelector,
    VarItemTypeSwitch,
    VarItemTypeMax,
} VarItemType;

typedef struct {
    char** text;
    uint32_t count;
} VarItemSelectorChoices;

typedef struct {
    lv_label_t base;
    int32_t min;
    int32_t max;
    int32_t step;
    int32_t value;
    uint32_t flags;
    char* suffix;
    VarItemSelectorChoices* choices;
    VarItemChangeCallback callback;
    void* context;
    VarItemType type;
} VarItemEditor;

struct VarItem {
    lv_obj_t base;
    lv_obj_t* cursor;
    lv_obj_t* label;
    VarItemEditor* editor;
};

struct VarItemList {
    Widget base;
    lv_group_t* group;
    VarItemEditor* edited;
};

// Class forward declarations

const lv_obj_class_t var_item_list_lvgl_class;
const lv_obj_class_t var_item_lvgl_class;
const lv_obj_class_t var_item_editor_lvgl_class;

// Function prototypes

static void var_item_editor_clear_choices(VarItemEditor* instance);

// LVGL-specific code

// TODO: Make it a universal fix
static void var_item_list_scroll_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = SCROLL_ANIM_DURATION_MS;
    }
}

// VarItemList

static void var_item_list_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    VarItemList* instance = (VarItemList*)obj;
    instance->group = lv_group_create();
}

static void var_item_list_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    VarItemList* instance = (VarItemList*)obj;
    lv_group_delete(instance->group);
}

// VarItem

static void var_item_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    VarItem* instance = (VarItem*)obj;
    instance->cursor = lv_label_create(obj);
    lv_label_set_text(instance->cursor, SYM_ARROW_RIGHT);
    // TODO: A better way to show and hide the cursor
    lv_obj_set_style_opa(instance->cursor, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_left(instance->cursor, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_right(instance->cursor, 1, LV_PART_MAIN);

    instance->label = lv_label_create(obj);
    lv_obj_set_flex_grow(instance->label, 1);
    lv_label_set_long_mode(instance->label, LV_LABEL_LONG_MODE_WRAP);

    lv_obj_t* editor = lv_obj_class_create_obj(MY_EDITOR_CLASS, obj);
    lv_obj_class_init_obj(editor);
    lv_obj_set_flex_grow(editor, 1);
    lv_label_set_long_mode(editor, LV_LABEL_LONG_MODE_CLIP);

    instance->editor = (VarItemEditor*)editor;
}

static void var_item_lvgl_event(const lv_obj_class_t* class_p, lv_event_t* event) {
    LV_UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_ITEM_CLASS, event);
    if(res != LV_RESULT_OK) return;

    const lv_event_code_t code = lv_event_get_code(event);
    VarItem* instance = lv_event_get_target(event);

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_set_style_opa(instance->cursor, LV_OPA_COVER, LV_PART_MAIN);
    } else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_set_style_opa(instance->cursor, LV_OPA_TRANSP, LV_PART_MAIN);
    }
}

// VarItemSpinbox

static void var_item_editor_lvlgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    VarItemEditor* instance = (VarItemEditor*)obj;
    UNUSED(instance);
}

static void var_item_editor_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    VarItemEditor* instance = (VarItemEditor*)obj;

    if(instance->suffix) {
        free(instance->suffix);
    }
    if(instance->choices) {
        var_item_editor_clear_choices(instance);
    }
}

// Spinbox private functions

static VarItem* var_item_editor_get_item(const VarItemEditor* instance) {
    return (VarItem*)lv_obj_get_parent((const lv_obj_t*)instance);
}

static void var_item_editor_set_range_and_step(
    VarItemEditor* instance,
    int32_t min,
    int32_t max,
    int32_t step) {
    instance->min = min;
    instance->max = max;
    instance->step = step;
    instance->value = min;
}

static void var_item_editor_set_type(VarItemEditor* instance, VarItemType type) {
    instance->type = type;
}

static void var_item_editor_set_choices(
    VarItemEditor* instance,
    const char* choice_text[],
    uint32_t choice_count) {
    furi_assert(instance->type == VarItemTypeSelector);
    furi_assert(instance->choices == NULL);

    instance->choices = malloc(sizeof(VarItemSelectorChoices));
    instance->choices->text = malloc(sizeof(char*) * choice_count);
    instance->choices->count = choice_count;

    for(uint32_t i = 0; i < choice_count; ++i) {
        instance->choices->text[i] = strdup(choice_text[i]);
    }
}

static void var_item_editor_set_choices_key_value(
    VarItemEditor* instance,
    const VarItemKeyValue* key_value,
    uint32_t choice_count) {
    furi_assert(instance->type == VarItemTypeSelector);
    furi_assert(instance->choices == NULL);

    instance->choices = malloc(sizeof(VarItemSelectorChoices));
    instance->choices->text = malloc(sizeof(char*) * choice_count);
    instance->choices->count = choice_count;

    for(uint32_t i = 0; i < choice_count; ++i) {
        instance->choices->text[i] = strdup(key_value[i].key);
    }
}

static void var_item_editor_clear_choices(VarItemEditor* instance) {
    furi_assert(instance->choices);

    VarItemSelectorChoices* choices = instance->choices;
    furi_assert(choices->text);
    furi_assert(choices->count);

    for(uint32_t i = 0; i < choices->count; ++i) {
        free(choices->text[i]);
    }

    free(choices);
    instance->choices = NULL;
}

static void var_item_editor_set_suffix(VarItemEditor* instance, const char* suffix) {
    furi_assert(instance->type != VarItemTypeTimebox && instance->type != VarItemTypeSwitch);
    furi_assert(instance->suffix == NULL);

    if(suffix) {
        instance->suffix = strdup(suffix);
    }
}

static void var_item_editor_update(VarItemEditor* instance) {
    lv_obj_t* label = (lv_obj_t*)instance;

    const bool is_neg_infinity = (instance->value == instance->min) &&
                                 (instance->flags & VarItemFlagMinIsInf);
    const bool is_pos_infinity = (instance->value == instance->max) &&
                                 (instance->flags & VarItemFlagMaxIsInf);

    if(is_neg_infinity || is_pos_infinity) {
        SET_EDITOR_LABEL(label, "%s", SYM_INFINITY);

    } else if(instance->type == VarItemTypeSpinbox) {
        if(instance->suffix) {
            SET_EDITOR_LABEL(label, "%ld %s", instance->value, instance->suffix);
        } else {
            SET_EDITOR_LABEL(label, "%ld", instance->value);
        }

    } else if(instance->type == VarItemTypeTimebox) {
        const int32_t hh = instance->value / 60;
        const int32_t mm = instance->value % 60;

        if(hh == 0) {
            SET_EDITOR_LABEL(label, "%ld", mm);
        } else if(mm == 0) {
            SET_EDITOR_LABEL(label, "%ld h", hh);
        } else {
            SET_EDITOR_LABEL(label, "%ld:%02ld", hh, mm);
        }

    } else if(instance->type == VarItemTypeSelector) {
        const VarItemSelectorChoices* choices = instance->choices;
        const uint32_t index = instance->value;

        furi_check(index < choices->count);

        if(instance->suffix) {
            SET_EDITOR_LABEL(label, "%s %s", choices->text[index], instance->suffix);
        } else {
            SET_EDITOR_LABEL(label, "%s", choices->text[index]);
        }

    } else if(instance->type == VarItemTypeSwitch) {
        SET_EDITOR_LABEL(label, "%s", instance->value ? "ON" : "OFF");

    } else {
        furi_crash();
    }
}

static void var_item_editor_increment(VarItemEditor* instance) {
    if(instance->value < instance->max) {
        instance->value += instance->step;
        var_item_editor_update(instance);
    }
}

static void var_item_editor_decrement(VarItemEditor* instance) {
    if(instance->value > instance->min) {
        instance->value -= instance->step;
        var_item_editor_update(instance);
    }
}

static bool var_item_list_input_callback(Widget* widget, const InputEvent* event) {
    VarItemList* instance = (VarItemList*)widget;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            if(instance->edited) {
                var_item_editor_increment(instance->edited);
            } else {
                lv_group_focus_next(instance->group);
            }

            consumed = true;

        } else if(event->key == InputKeyDown) {
            if(instance->edited) {
                var_item_editor_decrement(instance->edited);
            } else {
                lv_group_focus_prev(instance->group);
            }

            consumed = true;

        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            VarItemEditor* editor = instance->edited;

            if(editor) {
                lv_obj_remove_state((lv_obj_t*)editor, LV_STATE_FOCUSED);
                instance->edited = NULL;

                if(editor->callback) {
                    editor->callback(var_item_editor_get_item(editor), editor->context);
                }

            } else {
                VarItem* item = (VarItem*)lv_group_get_focused(instance->group);

                editor = item->editor;
                lv_obj_add_state((lv_obj_t*)editor, LV_STATE_FOCUSED);
                instance->edited = editor;
            }

            consumed = true;

        } else if(event->key == InputKeyBack) {
            VarItemEditor* editor = instance->edited;

            if(editor) {
                lv_obj_remove_state((lv_obj_t*)editor, LV_STATE_FOCUSED);
                instance->edited = NULL;

                if(editor->callback) {
                    editor->callback(var_item_editor_get_item(editor), editor->context);
                }

                consumed = true;
            }
        }
    }

    return consumed;
}

static VarItem* var_item_alloc(
    VarItemList* parent,
    const char* label,
    VarItemChangeCallback callback,
    void* context) {
    lv_obj_t* obj = lv_obj_class_create_obj(MY_ITEM_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    lv_group_add_obj(parent->group, obj);

    VarItem* instance = (VarItem*)obj;
    lv_label_set_text(instance->label, label);

    VarItemEditor* editor = instance->editor;
    editor->callback = callback;
    editor->context = context;

    return instance;
}

// Public API

VarItemList* var_item_list_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(obj, var_item_list_scroll_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    VarItemList* instance = (VarItemList*)obj;
    widget_set_input_feed_callback((Widget*)instance, var_item_list_input_callback);

    return instance;
}

void var_item_list_free(VarItemList* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* var_item_list_get_base(VarItemList* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

VarItem* var_item_list_add_timebox(
    VarItemList* instance,
    const char* label,
    int32_t min_mn,
    int32_t max_mn,
    int32_t step_mn,
    VarItemChangeCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(label);
    CHECK_RANGE_AND_STEP(min_mn, max_mn, step_mn);

    VarItem* item = var_item_alloc(instance, label, callback, context);

    var_item_editor_set_type(item->editor, VarItemTypeTimebox);
    var_item_editor_set_range_and_step(item->editor, min_mn, max_mn, step_mn);
    var_item_editor_update(item->editor);

    return item;
}

VarItem* var_item_list_add_spinbox(
    VarItemList* instance,
    const char* label,
    const char* suffix,
    int32_t min,
    int32_t max,
    int32_t step,
    VarItemChangeCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(label);
    CHECK_RANGE_AND_STEP(min, max, step);

    VarItem* item = var_item_alloc(instance, label, callback, context);

    var_item_editor_set_type(item->editor, VarItemTypeSpinbox);
    var_item_editor_set_range_and_step(item->editor, min, max, step);
    var_item_editor_set_suffix(item->editor, suffix);
    var_item_editor_update(item->editor);

    return item;
}

VarItem* var_item_list_add_selector(
    VarItemList* instance,
    const char* label,
    const char* suffix,
    const char* choice_text[],
    uint32_t choice_count,
    VarItemChangeCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(label);
    furi_check(choice_text);
    furi_check(choice_count);

    VarItem* item = var_item_alloc(instance, label, callback, context);

    var_item_editor_set_type(item->editor, VarItemTypeSelector);
    var_item_editor_set_range_and_step(item->editor, 0, choice_count - 1, 1);
    var_item_editor_set_choices(item->editor, choice_text, choice_count);
    var_item_editor_set_suffix(item->editor, suffix);
    var_item_editor_update(item->editor);

    return item;
}

VarItem* var_item_list_add_selector_key_value(
    VarItemList* instance,
    const char* label,
    const char* suffix,
    const VarItemKeyValue* choice_key_val,
    uint32_t choice_count,
    VarItemChangeCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(label);
    furi_check(choice_key_val);
    furi_check(choice_count);

    VarItem* item = var_item_alloc(instance, label, callback, context);

    var_item_editor_set_type(item->editor, VarItemTypeSelector);
    var_item_editor_set_range_and_step(item->editor, 0, choice_count - 1, 1);
    var_item_editor_set_choices_key_value(item->editor, choice_key_val, choice_count);
    var_item_editor_set_suffix(item->editor, suffix);
    var_item_editor_update(item->editor);

    return item;
}

VarItem* var_item_list_add_switch(
    VarItemList* instance,
    const char* label,
    VarItemChangeCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(label);

    VarItem* item = var_item_alloc(instance, label, callback, context);

    var_item_editor_set_type(item->editor, VarItemTypeSwitch);
    var_item_editor_set_range_and_step(item->editor, 0, 1, 1);
    var_item_editor_update(item->editor);

    return item;
}

void var_item_set_value(VarItem* item, int32_t value) {
    furi_check(item);

    VarItemEditor* editor = item->editor;
    furi_check(value >= editor->min);
    furi_check(value <= editor->max);
    furi_check(value % editor->step == 0);

    if(editor->value != value) {
        editor->value = value;
        var_item_editor_update(editor);
    }
}

int32_t var_item_get_value(const VarItem* item) {
    furi_check(item);

    const VarItemEditor* editor = item->editor;
    return editor->value;
}

void var_item_set_flags(VarItem* item, uint32_t flags) {
    furi_check(item);

    VarItemEditor* editor = item->editor;
    editor->flags = flags;
    var_item_editor_update(editor);
}

// LVGL classes

const lv_obj_class_t var_item_list_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = var_item_list_lvgl_constructor,
    .destructor_cb = var_item_list_lvgl_destructor,
    .name = "widget-var-item-list",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(VarItemList),
};

const lv_obj_class_t var_item_lvgl_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = var_item_lvgl_constructor,
    .event_cb = var_item_lvgl_event,
    .name = "var-item",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(VarItem),
};

const lv_obj_class_t var_item_editor_lvgl_class = {
    .base_class = &lv_label_class,
    .constructor_cb = var_item_editor_lvlgl_constructor,
    .destructor_cb = var_item_editor_lvgl_destructor,
    .name = "var-item-editor",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(VarItemEditor),
};
