#include "menu.h"

#include <gui/widget_i.h>

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS          (&menu_lvgl_class)
#define MY_ITEM_CLASS     (&menu_item_lvgl_class)
#define MY_ICON_CLASS     (&menu_icon_lvgl_class)
#define MY_SUBLABEL_CLASS (&menu_sublabel_lvgl_class)
#define MY_ARROW_CLASS    (&menu_arrow_lvgl_class)

#define SCROLL_ANIM_DURATION_MS (0)

struct Menu {
    Widget base;
    lv_group_t* group;
};

typedef struct {
    lv_obj_t base;
    lv_obj_t* icon;
    lv_obj_t* label;
    lv_obj_t* sub_label;
    lv_obj_t* arrow;
    uint32_t index;
    MenuItemCallback callback;
    void* context;
} MenuItem;

const lv_obj_class_t menu_lvgl_class;
const lv_obj_class_t menu_item_lvgl_class;
const lv_obj_class_t menu_icon_lvgl_class;
const lv_obj_class_t menu_sublabel_lvgl_class;
const lv_obj_class_t menu_arrow_lvgl_class;

// TODO: Make it a universal fix
static void menu_scroll_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = SCROLL_ANIM_DURATION_MS;
    }
}

static bool menu_input_callback(Widget* widget, const InputEvent* event) {
    Menu* instance = (Menu*)widget;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            lv_group_focus_next(instance->group);
            consumed = true;

        } else if(event->key == InputKeyDown) {
            lv_group_focus_prev(instance->group);
            consumed = true;

        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            const MenuItem* item = (MenuItem*)lv_group_get_focused(instance->group);

            if(item->callback) {
                item->callback(item->index, item->context);
            }

            consumed = true;
        }
    }

    return consumed;
}

static lv_obj_t* menu_item_alloc(
    Menu* parent,
    const char* label,
    const char* sub_label,
    const char* icon_source,
    uint32_t index,
    MenuItemCallback callback,
    void* context) {
    lv_obj_t* obj = lv_obj_class_create_obj(MY_ITEM_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    MenuItem* instance = (MenuItem*)obj;
    instance->index = index;
    instance->callback = callback;
    instance->context = context;

    lv_image_set_src(instance->icon, icon_source);

    lv_label_set_text(instance->label, label);
    lv_group_add_obj(parent->group, obj);

    if(sub_label) {
        if(strlen(sub_label)) {
            lv_label_set_text(instance->sub_label, sub_label);
        }
        lv_label_set_text(instance->arrow, ">");
    }

    return obj;
}

static void menu_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(obj, menu_scroll_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    Menu* instance = (Menu*)obj;
    instance->group = lv_group_create();
    lv_group_set_wrap(instance->group, false);
}

static void menu_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Menu* instance = (Menu*)obj;
    lv_group_delete(instance->group);
}

static void menu_item_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    MenuItem* instance = (MenuItem*)obj;

    instance->icon = lv_obj_class_create_obj(MY_ICON_CLASS, obj);
    lv_obj_class_init_obj(instance->icon);

    instance->label = lv_label_create(obj);
    lv_obj_set_flex_grow(instance->label, 1);

    instance->sub_label = lv_obj_class_create_obj(MY_SUBLABEL_CLASS, obj);
    lv_obj_class_init_obj(instance->sub_label);

    lv_label_set_long_mode(instance->sub_label, LV_LABEL_LONG_MODE_CLIP);

    instance->arrow = lv_obj_class_create_obj(MY_ARROW_CLASS, obj);
    lv_obj_class_init_obj(instance->arrow);
}

static void menu_item_lvgl_event(const lv_obj_class_t* class_p, lv_event_t* event) {
    LV_UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_ITEM_CLASS, event);
    if(res != LV_RESULT_OK) return;

    const lv_event_code_t code = lv_event_get_code(event);
    MenuItem* instance = lv_event_get_target(event);

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_add_state(instance->icon, LV_STATE_FOCUSED);
        lv_obj_add_state(instance->sub_label, LV_STATE_FOCUSED);
        lv_obj_add_state(instance->arrow, LV_STATE_FOCUSED);
    } else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_remove_state(instance->icon, LV_STATE_FOCUSED);
        lv_obj_remove_state(instance->sub_label, LV_STATE_FOCUSED);
        lv_obj_remove_state(instance->arrow, LV_STATE_FOCUSED);
    }
}

// Public API

Menu* menu_alloc(Widget* widget) {
    furi_check(widget);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)widget);
    lv_obj_class_init_obj(obj);

    Menu* instance = (Menu*)obj;
    widget_set_input_feed_callback((Widget*)instance, menu_input_callback);

    return instance;
}

void menu_free(Menu* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* menu_get_base(Menu* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void menu_add_item(
    Menu* instance,
    const char* label,
    const char* sub_label,
    const char* icon_source,
    uint32_t index,
    MenuItemCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(label);
    furi_check(icon_source);

    lv_obj_t* item =
        menu_item_alloc(instance, label, sub_label, icon_source, index, callback, context);
    UNUSED(item);
}

void menu_reset(Menu* instance) {
    furi_check(instance);
    lv_obj_clean((lv_obj_t*)instance);
}

uint32_t menu_get_selected_item_index(const Menu* instance) {
    furi_check(instance);

    uint32_t ret;

    const lv_obj_t* focused = lv_group_get_focused(instance->group);
    const uint32_t item_count = lv_group_get_obj_count(instance->group);

    for(ret = 0; ret < item_count; ++ret) {
        if(focused == lv_group_get_obj_by_index(instance->group, ret)) {
            break;
        }
    }

    return ret;
}

void menu_set_selected_item_index(Menu* instance, uint32_t index) {
    furi_check(instance);
    furi_check(index < lv_group_get_obj_count(instance->group));

    lv_group_focus_obj(lv_group_get_obj_by_index(instance->group, index));
}

// LVGL class descriptors

const lv_obj_class_t menu_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = menu_lvgl_constructor,
    .destructor_cb = menu_lvgl_destructor,
    .name = "widget-menu",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Menu),
};

const lv_obj_class_t menu_item_lvgl_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = menu_item_lvgl_constructor,
    .event_cb = menu_item_lvgl_event,
    .name = "menu-item",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(MenuItem),
};

const lv_obj_class_t menu_icon_lvgl_class = {
    .base_class = &lv_image_class,
    .name = "menu-icon",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t menu_sublabel_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "menu-sublabel",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t menu_arrow_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "menu-arrow",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
