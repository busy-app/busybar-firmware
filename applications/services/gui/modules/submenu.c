#include "submenu.h"

#include <gui/widget_i.h>

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS        (&submenu_lvgl_class)
#define MY_ITEM_CLASS   (&submenu_item_lvgl_class)
#define MY_CURSOR_CLASS (&submenu_cursor_lvgl_class)

#define SYM_ARROW_RIGHT "▹"

#define SCROLL_ANIM_DURATION_MS (0)

struct Submenu {
    Widget base;
    lv_group_t* group;
};

typedef struct {
    lv_obj_t base;
    lv_obj_t* cursor;
    lv_obj_t* label;
    uint32_t index;
    SubmenuItemCallback callback;
    void* context;
} SubmenuItem;

const lv_obj_class_t submenu_lvgl_class;
const lv_obj_class_t submenu_item_lvgl_class;
const lv_obj_class_t submenu_cursor_lvgl_class;

// TODO: Make it a universal fix
static void submenu_scroll_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = SCROLL_ANIM_DURATION_MS;
    }
}

static bool submenu_input_callback(Widget* widget, const InputEvent* event) {
    Submenu* instance = (Submenu*)widget;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            lv_group_focus_next(instance->group);
            consumed = true;

        } else if(event->key == InputKeyDown) {
            lv_group_focus_prev(instance->group);
            consumed = true;

        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            const SubmenuItem* item = (SubmenuItem*)lv_group_get_focused(instance->group);

            if(item->callback) {
                item->callback(item->index, item->context);
            }

            consumed = true;
        }
    }

    return consumed;
}

static lv_obj_t* submenu_item_alloc(
    Submenu* parent,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* context) {
    lv_obj_t* obj = lv_obj_class_create_obj(MY_ITEM_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    SubmenuItem* instance = (SubmenuItem*)obj;
    instance->index = index;
    instance->callback = callback;
    instance->context = context;

    lv_label_set_text(instance->label, label);
    lv_group_add_obj(parent->group, obj);

    return obj;
}

static void submenu_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(obj, submenu_scroll_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    Submenu* instance = (Submenu*)obj;
    instance->group = lv_group_create();
    lv_group_set_wrap(instance->group, false);
}

static void submenu_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Submenu* instance = (Submenu*)obj;
    lv_group_delete(instance->group);
}

static void submenu_item_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    SubmenuItem* instance = (SubmenuItem*)obj;

    instance->cursor = lv_obj_class_create_obj(MY_CURSOR_CLASS, obj);
    lv_obj_class_init_obj(instance->cursor);
    lv_label_set_text(instance->cursor, SYM_ARROW_RIGHT);

    instance->label = lv_label_create(obj);
    lv_label_set_long_mode(instance->label, LV_LABEL_LONG_MODE_WRAP);
}

static void submenu_item_lvgl_event(const lv_obj_class_t* class_p, lv_event_t* event) {
    LV_UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_ITEM_CLASS, event);
    if(res != LV_RESULT_OK) return;

    const lv_event_code_t code = lv_event_get_code(event);
    SubmenuItem* instance = lv_event_get_target(event);

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_add_state(instance->cursor, LV_STATE_FOCUSED);
    } else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_remove_state(instance->cursor, LV_STATE_FOCUSED);
    }
}

// Public API

Submenu* submenu_alloc(Widget* widget) {
    furi_check(widget);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)widget);
    lv_obj_class_init_obj(obj);

    Submenu* instance = (Submenu*)obj;
    widget_set_input_feed_callback((Widget*)instance, submenu_input_callback);

    return instance;
}

void submenu_free(Submenu* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* submenu_get_base(Submenu* instance) {
    furi_check(instance);
    return (Widget*)instance;
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
    UNUSED(item);
}

void submenu_reset(Submenu* instance) {
    furi_check(instance);
    lv_obj_clean((lv_obj_t*)instance);
}

uint32_t submenu_get_selected_item_index(const Submenu* instance) {
    furi_check(instance);

    // TODO DOESN'T WORK!!!
    const SubmenuItem* item = (SubmenuItem*)lv_group_get_focused(instance->group);
    if(item) {
        return item->index;
    }

    return 0;
}

void submenu_set_selected_item_index(Submenu* instance, uint32_t index) {
    furi_check(instance);

    uint32_t child_count = lv_obj_get_child_count((lv_obj_t*)instance);
    for(uint32_t i = 0; i < child_count; i++) {
        SubmenuItem* item = (SubmenuItem*)lv_obj_get_child((lv_obj_t*)instance, i);
        if(item->index == index) {
            lv_group_focus_obj((lv_obj_t*)item);
            lv_obj_scroll_to_view((lv_obj_t*)item, LV_ANIM_OFF);
            break;
        }
    }
}

// LVGL class descriptors

const lv_obj_class_t submenu_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = submenu_lvgl_constructor,
    .destructor_cb = submenu_lvgl_destructor,
    .name = "widget-submenu",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Submenu),
};

const lv_obj_class_t submenu_item_lvgl_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = submenu_item_lvgl_constructor,
    .event_cb = submenu_item_lvgl_event,
    .name = "submenu-item",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SubmenuItem),
};

const lv_obj_class_t submenu_cursor_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "submenu-cursor",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
