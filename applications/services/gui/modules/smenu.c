#include "smenu.h"
#include "private/smenu_item_i.h"

#include <lvgl_addons/extensions/lv_group_ext.h>

#define SMENU_ROUNDING_INT_DIV(dividend, divisor)    \
    ({                                               \
        __typeof__(dividend) _dividend = (dividend); \
        __typeof__(divisor) _divisor = (divisor);    \
        (_dividend + _divisor / 2) / _divisor;       \
    })

struct SMenu {
    Widget base;

    lv_group_t* items_group;

    SMenuItemCursorState standby_cursor_state;
    bool is_cursor_enabled;
};

const lv_obj_class_t smenu_lvgl_class;

/* Implementation */

static bool smenu_on_input(Widget* widget, const InputEvent* event) {
    SMenu* instance = (SMenu*)widget;
    Widget* item = (Widget*)lv_group_get_focused(instance->items_group);

    bool is_consumed = false;
    if(item && widget_input(item, event)) {
        is_consumed = true;
    } else if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            lv_group_focus_next(instance->items_group);
            is_consumed = true;
        } else if(event->key == InputKeyDown) {
            lv_group_focus_prev(instance->items_group);
            is_consumed = true;
        }
    }

    return is_consumed;
}

static void smenu_on_item_focus_change(lv_event_t* event) {
    lv_event_code_t event_code = lv_event_get_code(event);

    SMenuItemCursorState cursor_state;
    if(event_code == LV_EVENT_FOCUSED) {
        cursor_state = SMenuItemCursorStateNormal;
    } else if(event_code == LV_EVENT_DEFOCUSED) {
        SMenu* instance = lv_event_get_user_data(event);
        cursor_state = instance->standby_cursor_state;
    } else {
        furi_crash();
    }

    SMenuItem* item_instance = (SMenuItem*)lv_event_get_current_target_obj(event);
    smenu_item_set_cursor_state(item_instance, cursor_state);
}

static void smenu_on_child_created(lv_event_t* event) {
    lv_obj_t* object = lv_event_get_current_target_obj(event);

    if(object == lv_event_get_target_obj(event)) {
        lv_obj_t* item_object = lv_event_get_param(event);

        if(item_object && lv_obj_has_class(item_object, &smenu_item_lvgl_class)) {
            SMenu* instance = (SMenu*)object;

            if(instance->is_cursor_enabled) {
                lv_obj_add_event_cb(
                    item_object, smenu_on_item_focus_change, LV_EVENT_FOCUSED, instance);
                lv_obj_add_event_cb(
                    item_object, smenu_on_item_focus_change, LV_EVENT_DEFOCUSED, instance);
            }

            SMenuItem* item_instance = (SMenuItem*)item_object;
            smenu_item_set_cursor_state(item_instance, instance->standby_cursor_state);
            lv_group_add_obj(instance->items_group, item_object);
        }
    }
}

static void smenu_on_lvgl_event(const lv_obj_class_t* class, lv_event_t* event) {
    if(lv_obj_event_base(class, event) == LV_RESULT_OK) {
        lv_event_code_t event_code = lv_event_get_code(event);

        if(event_code == LV_EVENT_CHILD_CREATED) {
            smenu_on_child_created(event);
        }
    }
}

static bool
    smenu_draw_scrollbar_thumb(lv_obj_t* object, lv_layer_t* layer, const lv_area_t* track_area) {
    SMenu* instance = (SMenu*)object;
    lv_obj_t* focused_item = lv_group_get_focused(instance->items_group);

    bool is_drawn = false;
    if(lv_obj_get_scroll_snap_y(object) != LV_SCROLL_SNAP_NONE && focused_item) {
        int32_t items_count = lv_group_get_obj_count(instance->items_group);
        int32_t focused_item_idx =
            lv_group_ext_get_index_by_obj(instance->items_group, focused_item);

        int32_t track_height = lv_area_get_height(track_area);
        int32_t thumb_height =
            MAX(track_height / items_count, lv_obj_get_style_length(object, LV_PART_SCROLLBAR));
        int32_t thumb_y = (items_count > 1) ? SMENU_ROUNDING_INT_DIV(
                                                  (track_height - thumb_height) * focused_item_idx,
                                                  items_count - 1) :
                                              0;

        lv_draw_rect_dsc_t thumb_descriptor;
        lv_draw_rect_dsc_init(&thumb_descriptor);
        thumb_descriptor.bg_color = lv_obj_get_style_bg_color(object, LV_PART_SCROLLBAR);
        thumb_descriptor.bg_opa = lv_obj_get_style_bg_opa(object, LV_PART_SCROLLBAR);
        thumb_descriptor.radius = lv_obj_get_style_radius(object, LV_PART_SCROLLBAR);

        lv_draw_rect(
            layer,
            &thumb_descriptor,
            &(lv_area_t){
                .x1 = track_area->x1,
                .y1 = track_area->y1 + thumb_y,
                .x2 = track_area->x2,
                .y2 = track_area->y1 + thumb_y + thumb_height - 1,
            });

        is_drawn = true;
    }

    return is_drawn;
}

static void smenu_style_front(const lv_obj_class_t* class, Widget* widget) {
    UNUSED(class);

    lv_obj_t* object = TO_LV_OBJ(widget);
    SMenu* instance = (SMenu*)widget;

    lv_obj_set_scroll_snap_y(object, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_style_pad_right(object, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_row(object, -1, LV_PART_MAIN);

    instance->standby_cursor_state =
        instance->is_cursor_enabled ? SMenuItemCursorStateTransparent : SMenuItemCursorStateHidden;
}

static void smenu_style_back(const lv_obj_class_t* class, Widget* widget) {
    UNUSED(class);

    lv_obj_t* object = TO_LV_OBJ(widget);
    SMenu* instance = (SMenu*)widget;

    lv_obj_set_style_pad_left(object, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_right(object, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(object, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_row(object, 2, LV_PART_MAIN);

    instance->standby_cursor_state = SMenuItemCursorStateHidden;
}

static void smenu_lvgl_constructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenu* instance = (SMenu*)object;

    lv_obj_set_flex_flow(object, LV_FLEX_FLOW_COLUMN);
    widget_set_scrollbar_enabled(&instance->base, true);

    instance->items_group = lv_group_create();
    lv_group_set_wrap(instance->items_group, false);
}

static void smenu_lvgl_destructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenu* instance = (SMenu*)object;

    lv_group_delete(instance->items_group);
}

/* Public API */

SMenu* smenu_alloc(Widget* parent, bool is_cursor_enabled) {
    furi_check(parent);

    SMenu* instance = (SMenu*)lv_obj_class_create_obj(&smenu_lvgl_class, TO_LV_OBJ(parent));
    instance->is_cursor_enabled = is_cursor_enabled;

    lv_obj_class_init_obj(TO_LV_OBJ(instance));

    return instance;
}

void smenu_free(SMenu* instance) {
    furi_check(instance);

    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* smenu_get_base(SMenu* instance) {
    furi_check(instance);

    return &instance->base;
}

void smenu_set_selected_item_index(SMenu* instance, size_t index) {
    furi_check(instance);
    furi_check(index < lv_group_get_obj_count(instance->items_group));

    lv_group_focus_obj(lv_group_get_obj_by_index(instance->items_group, index));
}

size_t smenu_get_selected_item_index(SMenu* instance) {
    furi_check(instance);

    return lv_group_ext_get_index_by_obj(
        instance->items_group, lv_group_get_focused(instance->items_group));
}

SMenu* smenu_get_from_item(SMenuItem* item) {
    furi_check(item);

    lv_obj_t* parent = lv_obj_get_parent(TO_LV_OBJ(item));
    return lv_obj_check_type(parent, &smenu_lvgl_class) ? (SMenu*)parent : NULL;
}

void smenu_reset(SMenu* instance) {
    furi_check(instance);

    lv_obj_clean(TO_LV_OBJ(instance));
}

/* LVGL Class Descriptor */

const lv_obj_class_t smenu_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = smenu_lvgl_constructor,
    .destructor_cb = smenu_lvgl_destructor,
    .event_cb = smenu_on_lvgl_event,
    .name = "widget-smenu",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(SMenu),
    .user_data =
        (void*)&(const WidgetClassData){
            .input_callback = smenu_on_input,
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = smenu_style_front,
                    [GuiDisplayIdBack] = smenu_style_back,
                },
            .draw_scrollbar_thumb = smenu_draw_scrollbar_thumb,
        },
};
