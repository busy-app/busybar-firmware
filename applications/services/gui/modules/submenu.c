#include "submenu.h"
#include "private/menu_base_i.h"

#include <gui/modules/label.h>

#include <lvgl_addons/extensions/lv_label_ext.h>
#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS        (&submenu_lvgl_class)
#define MY_ITEM_CLASS   (&submenu_item_lvgl_class)
#define MY_CURSOR_CLASS (&submenu_cursor_lvgl_class)

#define SYM_ARROW_RIGHT "▶" // U+25B6

#define SCROLL_ANIM_DURATION_MS        0
#define LONG_TEXT_ANIM_SPEED_PX_PER_M  1000
#define LONG_TEXT_ANIM_START_DELAY_MS  1000
#define LONG_TEXT_ANIM_REPEAT_DELAY_MS 2500

struct Submenu {
    MenuBase base;

    lv_anim_t item_anim_template;
};

typedef struct {
    lv_obj_t base;
    lv_obj_t* cursor;
    lv_obj_t* primary_label;
    lv_obj_t* auxiliary_label;
    uint32_t index;
    SubmenuItemCallback callback;
    void* context;

    bool awaits_lazy_setup;
} SubmenuItem;

const lv_obj_class_t submenu_lvgl_class;
const lv_obj_class_t submenu_item_lvgl_class;
const lv_obj_class_t submenu_cursor_lvgl_class;

static bool submenu_input_callback(Widget* widget, const InputEvent* event) {
    Submenu* instance = (Submenu*)widget;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            lv_group_focus_next(instance->base.group);
            consumed = true;

        } else if(event->key == InputKeyDown) {
            lv_group_focus_prev(instance->base.group);
            consumed = true;

        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            const SubmenuItem* item = (SubmenuItem*)lv_group_get_focused(instance->base.group);

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
    const char* primary_text,
    const char* auxiliary_text,
    uint32_t index,
    SubmenuItemCallback callback,
    void* context) {
    lv_obj_t* obj = lv_obj_class_create_obj(MY_ITEM_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    SubmenuItem* instance = (SubmenuItem*)obj;
    instance->index = index;
    instance->callback = callback;
    instance->context = context;
    instance->awaits_lazy_setup = true;

    lv_label_set_text(instance->primary_label, primary_text);

    if(auxiliary_text) {
        lv_label_set_text(instance->auxiliary_label, auxiliary_text);
        lv_obj_clear_flag(instance->auxiliary_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_static(instance->auxiliary_label, "");
        lv_obj_add_flag(instance->auxiliary_label, LV_OBJ_FLAG_HIDDEN);
    }

    lv_group_add_obj(parent->base.group, obj);

    return obj;
}

static void submenu_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Submenu* instance = (Submenu*)obj;

    lv_anim_init(&instance->item_anim_template);
    lv_anim_set_delay(&instance->item_anim_template, LONG_TEXT_ANIM_START_DELAY_MS);
    lv_anim_set_repeat_delay(&instance->item_anim_template, LONG_TEXT_ANIM_REPEAT_DELAY_MS);
    lv_anim_set_repeat_count(&instance->item_anim_template, LV_ANIM_REPEAT_INFINITE);
}

static void submenu_item_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    SubmenuItem* instance = (SubmenuItem*)obj;
    Submenu* parent = (Submenu*)lv_obj_get_parent(obj);

    instance->cursor = lv_obj_class_create_obj(MY_CURSOR_CLASS, obj);
    lv_obj_class_init_obj(instance->cursor);
    lv_label_set_text(instance->cursor, SYM_ARROW_RIGHT);

    instance->primary_label = lv_label_create(obj);
    lv_label_set_long_mode(instance->primary_label, LV_LABEL_LONG_MODE_CLIP);
    lv_obj_set_flex_grow(instance->primary_label, 1);
    lv_obj_set_style_anim(instance->primary_label, &parent->item_anim_template, LV_PART_MAIN);

    instance->auxiliary_label = lv_label_create(obj);
    lv_obj_add_flag(instance->auxiliary_label, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_align(instance->auxiliary_label, LV_ALIGN_RIGHT_MID);
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

        if(instance->awaits_lazy_setup) {
            lv_label_ext_set_anim_speed(instance->primary_label, LONG_TEXT_ANIM_SPEED_PX_PER_M);
            instance->awaits_lazy_setup = false;
        }

        lv_label_set_long_mode(instance->primary_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
    } else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_remove_state(instance->cursor, LV_STATE_FOCUSED);
        lv_label_set_long_mode(instance->primary_label, LV_LABEL_LONG_MODE_CLIP);
    }
}

// Public API

Submenu* submenu_alloc(Widget* widget) {
    furi_check(widget);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)widget);
    lv_obj_class_init_obj(obj);

    Submenu* instance = (Submenu*)obj;

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
    const char* primary_text,
    const char* auxiliary_text,
    uint32_t index,
    SubmenuItemCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(primary_text);

    submenu_item_alloc(instance, primary_text, auxiliary_text, index, callback, context);
}

void submenu_reset(Submenu* instance) {
    furi_check(instance);
    lv_obj_clean((lv_obj_t*)instance);
}

uint32_t submenu_get_selected_item_index(const Submenu* instance) {
    furi_check(instance);

    const SubmenuItem* item = (SubmenuItem*)lv_group_get_focused(instance->base.group);
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
    .base_class = &menu_base_lvgl_class,
    .constructor_cb = submenu_lvgl_constructor,
    .name = "widget-submenu",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Submenu),
    .user_data =
        (void*)&(const WidgetClassData){
            .input_callback = submenu_input_callback,
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = NULL,
                    [GuiDisplayIdBack] = NULL,
                },
        },
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
