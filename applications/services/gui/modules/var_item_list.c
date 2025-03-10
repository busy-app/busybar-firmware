#include "var_item_list.h"

#include <gui/view_port_i.h>

#include <lvgl/src/core/lv_obj_class_private.h>
#include <lvgl/src/widgets/label/lv_label_private.h>

#define MY_CLASS         (&lv_var_item_list_class)
#define MY_ITEM_CLASS    (&lv_var_item_class)
#define MY_SPINBOX_CLASS (&lv_var_item_spinbox_class)

#define SYM_INFINITY    "∞"
#define SYM_ARROW_LEFT  "◃"
#define SYM_ARROW_RIGHT "▹"

#define SCROLL_ANIM_DURATION_MS (64)
#define ITEM_INDICATOR_WIDTH_PX (6)

#define CHECK_RANGE_AND_STEP(min, max, step)                                               \
    do {                                                                                   \
        furi_check(max >= min, "Range error: min > max");                                  \
        furi_check((max - min) % step == 0, "Step error: range must be evenly divisible"); \
    } while(0)

#define SET_SPINBOX_LABEL(label, fmt, ...) \
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
    lv_label_t label;
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
} VarItemSpinbox;

struct VarItem {
    lv_obj_t obj;
    lv_obj_t* label;
    VarItemSpinbox* spinbox;
};

struct VarItemList {
    ViewPort view_port;
    lv_group_t* group;
};

// Class forward declarations

const lv_obj_class_t lv_var_item_list_class;
const lv_obj_class_t lv_var_item_class;
const lv_obj_class_t lv_var_item_spinbox_class;

// Function prototypes

static void var_item_spinbox_update(VarItemSpinbox* instance);
static void var_item_spinbox_increment(VarItemSpinbox* instance);
static void var_item_spinbox_decrement(VarItemSpinbox* instance);
static void var_item_spinbox_finish_editing(VarItemSpinbox* instance);
static void var_item_spinbox_grab_input(VarItemSpinbox* instance, bool enable);
static void var_item_spinbox_clear_choices(VarItemSpinbox* instance);

// LVGL-specific code

// HACK: This should be done by Gui
static void var_item_list_redirect_input_to_group(lv_obj_t* obj, lv_group_t* group) {
    const lv_display_t* display = lv_obj_get_display(obj);

    for(lv_indev_t* indev = lv_indev_get_next(NULL); indev != NULL;
        indev = lv_indev_get_next(indev)) {
        if(lv_indev_get_display(indev) == display) {
            lv_indev_set_group(indev, group);
        }
    }
}

// TODO: Make it a universal fix
static void var_item_list_scroll_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = SCROLL_ANIM_DURATION_MS;
    }
}

// VarItemList

static void lv_var_item_list_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    VarItemList* instance = (VarItemList*)obj;
    instance->group = lv_group_create();
    lv_group_set_editing(instance->group, true);
}

static void lv_var_item_list_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    VarItemList* instance = (VarItemList*)obj;
    lv_group_delete(instance->group);
}

// VarItem

static void lv_var_item_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    // TODO: Implement current item indicator
    lv_obj_set_style_pad_left(obj, ITEM_INDICATOR_WIDTH_PX, LV_PART_MAIN);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    // lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);

    VarItem* instance = (VarItem*)obj;
    instance->label = lv_label_create(obj);
    lv_label_set_long_mode(instance->label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);

    lv_obj_t* spinbox = lv_obj_class_create_obj(MY_SPINBOX_CLASS, obj);
    lv_obj_class_init_obj(spinbox);
    // TODO: Calculate horizontal pos based on available width
    lv_obj_set_pos(spinbox, 38, 0);

    instance->spinbox = (VarItemSpinbox*)spinbox;
}

static void lv_var_item_event(const lv_obj_class_t* class_p, lv_event_t* event) {
    LV_UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_ITEM_CLASS, event);
    if(res != LV_RESULT_OK) return;

    const lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* target = lv_event_get_target(event);

    if(code == LV_EVENT_SHORT_CLICKED) {
        VarItem* instance = (VarItem*)target;
        var_item_spinbox_grab_input(instance->spinbox, true);
    }
}

// VarItemSpinbox

static void lv_var_item_spinbox_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    VarItemSpinbox* instance = (VarItemSpinbox*)obj;
    UNUSED(instance);
}

static void lv_var_item_spinbox_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    VarItemSpinbox* instance = (VarItemSpinbox*)obj;
    var_item_spinbox_grab_input(instance, false);

    if(instance->suffix) {
        free(instance->suffix);
    }
    if(instance->choices) {
        var_item_spinbox_clear_choices(instance);
    }
}

static void lv_var_item_spinbox_event(const lv_obj_class_t* class_p, lv_event_t* event) {
    LV_UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_SPINBOX_CLASS, event);
    if(res != LV_RESULT_OK) return;

    const lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* target = lv_event_get_target(event);

    VarItemSpinbox* spinbox = (VarItemSpinbox*)target;

    if(code == LV_EVENT_SHORT_CLICKED) {
        var_item_spinbox_finish_editing(spinbox);

    } else if(code == LV_EVENT_KEY) {
        const uint32_t key = *((uint32_t*)lv_event_get_param(event));

        if(key == LV_KEY_RIGHT) {
            var_item_spinbox_increment(spinbox);
        } else if(key == LV_KEY_LEFT) {
            var_item_spinbox_decrement(spinbox);
        } else if(key == LV_KEY_ESC) {
            var_item_spinbox_finish_editing(spinbox);
        }
    }
}

// Spinbox private functions

static VarItem* var_item_spinbox_get_item(const VarItemSpinbox* instance) {
    return (VarItem*)lv_obj_get_parent((const lv_obj_t*)instance);
}

static VarItemList* var_item_spinbox_get_list(const VarItemSpinbox* instance) {
    const lv_obj_t* item = lv_obj_get_parent((const lv_obj_t*)instance);
    return (VarItemList*)lv_obj_get_parent(item);
}

static void var_item_spinbox_grab_input(VarItemSpinbox* instance, bool enable) {
    lv_obj_t* obj = (lv_obj_t*)instance;
    lv_group_t* group;

    if(enable) {
        VarItemList* list = var_item_spinbox_get_list(instance);
        group = list->group;
        lv_group_add_obj(group, obj);
    } else {
        lv_group_remove_obj(obj);
        group = lv_group_get_default();
    }

    var_item_list_redirect_input_to_group(obj, group);
}

static void var_item_spinbox_set_range_and_step(
    VarItemSpinbox* instance,
    int32_t min,
    int32_t max,
    int32_t step) {
    instance->min = min;
    instance->max = max;
    instance->step = step;
    instance->value = min;
}

static void var_item_spinbox_set_type(VarItemSpinbox* instance, VarItemType type) {
    instance->type = type;
}

static void var_item_spinbox_set_choices(
    VarItemSpinbox* instance,
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

static void var_item_spinbox_clear_choices(VarItemSpinbox* instance) {
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

static void var_item_spinbox_set_suffix(VarItemSpinbox* instance, const char* suffix) {
    furi_assert(instance->type != VarItemTypeTimebox && instance->type != VarItemTypeSwitch);
    furi_assert(instance->suffix == NULL);

    if(suffix) {
        instance->suffix = strdup(suffix);
    }
}

static void var_item_spinbox_update(VarItemSpinbox* instance) {
    lv_obj_t* label = (lv_obj_t*)instance;

    const bool is_neg_infinity = (instance->value == instance->min) &&
                                 (instance->flags & VarItemFlagMinIsInf);
    const bool is_pos_infinity = (instance->value == instance->max) &&
                                 (instance->flags & VarItemFlagMaxIsInf);

    if(is_neg_infinity || is_pos_infinity) {
        SET_SPINBOX_LABEL(label, "%s", SYM_INFINITY);

    } else if(instance->type == VarItemTypeSpinbox) {
        if(instance->suffix) {
            SET_SPINBOX_LABEL(label, "%ld %s", instance->value, instance->suffix);
        } else {
            SET_SPINBOX_LABEL(label, "%ld", instance->value);
        }

    } else if(instance->type == VarItemTypeTimebox) {
        const int32_t hh = instance->value / 60;
        const int32_t mm = instance->value % 60;

        if(hh == 0) {
            SET_SPINBOX_LABEL(label, "%ld", mm);
        } else if(mm == 0) {
            SET_SPINBOX_LABEL(label, "%ld h", hh);
        } else {
            SET_SPINBOX_LABEL(label, "%ld:%02ld", hh, mm);
        }

    } else if(instance->type == VarItemTypeSelector) {
        const VarItemSelectorChoices* choices = instance->choices;
        const uint32_t index = instance->value;

        furi_check(index < choices->count);

        if(instance->suffix) {
            SET_SPINBOX_LABEL(label, "%s %s", choices->text[index], instance->suffix);
        } else {
            SET_SPINBOX_LABEL(label, "%s", choices->text[index]);
        }

    } else if(instance->type == VarItemTypeSwitch) {
        SET_SPINBOX_LABEL(label, "%s", instance->value ? "ON" : "OFF");

    } else {
        furi_crash();
    }
}

static void var_item_spinbox_finish_editing(VarItemSpinbox* instance) {
    var_item_spinbox_grab_input(instance, false);

    VarItem* item = var_item_spinbox_get_item(instance);

    if(instance->callback) {
        instance->callback(item, instance->context);
    }
}

static void var_item_spinbox_increment(VarItemSpinbox* instance) {
    if(instance->value < instance->max) {
        instance->value += instance->step;
        var_item_spinbox_update(instance);
    }
}

static void var_item_spinbox_decrement(VarItemSpinbox* instance) {
    if(instance->value > instance->min) {
        instance->value -= instance->step;
        var_item_spinbox_update(instance);
    }
}

static VarItem* var_item_alloc(
    VarItemList* var_item_list,
    const char* label,
    VarItemChangeCallback callback,
    void* context) {
    lv_obj_t* obj = lv_obj_class_create_obj(MY_ITEM_CLASS, (lv_obj_t*)var_item_list);
    lv_obj_class_init_obj(obj);

    VarItem* instance = (VarItem*)obj;
    lv_label_set_text(instance->label, label);

    VarItemSpinbox* spinbox = instance->spinbox;
    spinbox->callback = callback;
    spinbox->context = context;

    return instance;
}

// Public API

VarItemList* var_item_list_alloc(ViewPort* view_port) {
    furi_check(view_port);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)view_port);
    lv_obj_class_init_obj(obj);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(obj, var_item_list_scroll_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    VarItemList* instance = (VarItemList*)obj;
    return instance;
}

void var_item_list_free(VarItemList* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
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

    var_item_spinbox_set_type(item->spinbox, VarItemTypeTimebox);
    var_item_spinbox_set_range_and_step(item->spinbox, min_mn, max_mn, step_mn);
    var_item_spinbox_update(item->spinbox);

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

    var_item_spinbox_set_type(item->spinbox, VarItemTypeSpinbox);
    var_item_spinbox_set_range_and_step(item->spinbox, min, max, step);
    var_item_spinbox_set_suffix(item->spinbox, suffix);
    var_item_spinbox_update(item->spinbox);

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

    var_item_spinbox_set_type(item->spinbox, VarItemTypeSelector);
    var_item_spinbox_set_range_and_step(item->spinbox, 0, choice_count - 1, 1);
    var_item_spinbox_set_choices(item->spinbox, choice_text, choice_count);
    var_item_spinbox_set_suffix(item->spinbox, suffix);
    var_item_spinbox_update(item->spinbox);

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

    var_item_spinbox_set_type(item->spinbox, VarItemTypeSwitch);
    var_item_spinbox_set_range_and_step(item->spinbox, 0, 1, 1);
    var_item_spinbox_update(item->spinbox);

    return item;
}

void var_item_set_value(VarItem* item, int32_t value) {
    furi_check(item);

    VarItemSpinbox* spinbox = item->spinbox;

    furi_check(value <= spinbox->min);
    furi_check(value >= spinbox->max);
    furi_check(value % spinbox->step == 0);

    if(spinbox->value != value) {
        spinbox->value = value;
        var_item_spinbox_update(spinbox);
    }
}

int32_t var_item_get_value(const VarItem* item) {
    furi_check(item);

    const VarItemSpinbox* spinbox = item->spinbox;
    return spinbox->value;
}

void var_item_set_flags(VarItem* item, uint32_t flags) {
    furi_check(item);

    VarItemSpinbox* spinbox = item->spinbox;
    spinbox->flags = flags;
    var_item_spinbox_update(spinbox);
}

// LVGL classes

const lv_obj_class_t lv_var_item_list_class = {
    .base_class = &lv_view_port_class,
    .constructor_cb = lv_var_item_list_constructor,
    .destructor_cb = lv_var_item_list_destructor,
    .name = "var-item-list",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(VarItemList),
};

const lv_obj_class_t lv_var_item_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = lv_var_item_constructor,
    .event_cb = lv_var_item_event,
    .name = "var-item",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(VarItem),
};

const lv_obj_class_t lv_var_item_spinbox_class = {
    .base_class = &lv_label_class,
    .constructor_cb = lv_var_item_spinbox_constructor,
    .destructor_cb = lv_var_item_spinbox_destructor,
    .event_cb = lv_var_item_spinbox_event,
    .name = "var-item-spinbox",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .instance_size = sizeof(VarItemSpinbox),
};
