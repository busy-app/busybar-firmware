#include "submenu.h"

#include <gui/widget_i.h>

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS      (&lv_submenu_class)
#define MY_ITEM_CLASS (&lv_submenu_item_class)

#define SCROLL_ANIM_DURATION_MS (64)
#define ITEM_INDICATOR_WIDTH_PX (6)

struct Submenu {
    Widget widget;
    lv_group_t* group;
};

typedef struct {
    Widget widget;
    lv_obj_t* label;
    uint32_t index;
    SubmenuItemCallback callback;
    void* context;
} SubmenuItem;

const lv_obj_class_t lv_submenu_class;
const lv_obj_class_t lv_submenu_item_class;

// TODO: Make it a universal fix
static void submenu_scroll_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = SCROLL_ANIM_DURATION_MS;
    }
}

static void submenu_item_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SINGLE_CLICKED) {
        const SubmenuItem* item = lv_event_get_target(event);

        if(item->callback) {
            item->callback(item->index, item->context);
        }
    }
}

static lv_obj_t* submenu_item_alloc(
    Submenu* instance,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* context) {
    lv_obj_t* obj = lv_obj_class_create_obj(MY_ITEM_CLASS, &instance->widget.obj);
    lv_obj_class_init_obj(obj);

    lv_group_add_obj(instance->group, obj);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    SubmenuItem* item = (SubmenuItem*)obj;
    item->label = lv_label_create(obj);
    lv_label_set_text(item->label, label);
    lv_label_set_long_mode(item->label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
    // TODO: Implement current item indicator
    lv_obj_set_style_pad_left(item->label, ITEM_INDICATOR_WIDTH_PX, LV_PART_MAIN);

    item->index = index;
    item->callback = callback;
    item->context = context;

    return obj;
}

static void submenu_obj_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(obj, submenu_scroll_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    Submenu* instance = (Submenu*)obj;
    instance->group = lv_group_create();

    widget_set_current_group(&instance->widget, instance->group);
}

static void submenu_obj_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Submenu* instance = (Submenu*)obj;
    lv_group_delete(instance->group);
}

// Public API

Submenu* submenu_alloc(Widget* widget) {
    furi_check(widget);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, &widget->obj);
    lv_obj_class_init_obj(obj);

    Submenu* instance = (Submenu*)obj;
    return instance;
}

void submenu_free(Submenu* instance) {
    furi_check(instance);
    lv_obj_delete(&instance->widget.obj);
}

void submenu_add_item(
    Submenu* instance,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(label);

    lv_obj_t* item = submenu_item_alloc(instance, label, index, callback, context);
    lv_obj_add_event_cb(item, submenu_item_event_callback, LV_EVENT_SINGLE_CLICKED, NULL);
}

void submenu_reset(Submenu* instance) {
    furi_check(instance);
    lv_obj_clean(&instance->widget.obj);
}

uint32_t submenu_get_selected_item_index(const Submenu* instance) {
    furi_check(instance);
    // For later
    furi_crash("Not implemented");
}

void submenu_set_selected_item_index(Submenu* instance, uint32_t index) {
    furi_check(instance);
    UNUSED(index);
    // For later
    furi_crash("Not implemented");
}

// LVGL class descriptors

const lv_obj_class_t lv_submenu_class = {
    .base_class = &lv_widget_class,
    .constructor_cb = submenu_obj_constructor,
    .destructor_cb = submenu_obj_destructor,
    .name = "submenu",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Submenu),
};

const lv_obj_class_t lv_submenu_item_class = {
    .base_class = &lv_widget_class,
    .name = "submenu-item",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SubmenuItem),
};
