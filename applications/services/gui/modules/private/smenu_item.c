#include "smenu_item_i.h"

#define SMENU_ITEM_FADER_WIDTH         (10)
#define SMENU_ITEM_RIGHT_MAX_WIDTH_PCT (45)

static const lv_obj_class_t smenu_item_right_container_lvgl_class;

const lv_anim_t smenu_item_long_text_anim_template = {
    .act_time = -1000,
    .repeat_cnt = LV_ANIM_REPEAT_INFINITE,
    .repeat_delay = 2600,
};

static const lv_draw_rect_dsc_t smenu_item_fader_descriptor = {
    .bg_opa = LV_OPA_COVER,
    .bg_grad.dir = LV_GRAD_DIR_HOR,
    .bg_grad.stops_count = 2,
    .bg_grad.stops =
        {
            {
                .color = LV_COLOR_MAKE(0x00, 0x00, 0x00),
                .opa = LV_OPA_TRANSP,
                .frac = 0,
            },
            {
                .color = LV_COLOR_MAKE(0x00, 0x00, 0x00),
                .opa = LV_OPA_COVER,
                .frac = 0xFF,
            },
        },
};

/* Implementation */

static void smenu_item_draw_fader(lv_obj_t* object, lv_layer_t* layer) {
    SMenuItem* instance = (SMenuItem*)object;

    lv_area_t object_coords;
    lv_obj_get_coords(object, &object_coords);

    lv_area_t right_container_coords;
    lv_obj_get_coords(instance->right_container, &right_container_coords);

    lv_area_t fader_area = {
        .x1 = right_container_coords.x1 - SMENU_ITEM_FADER_WIDTH,
        .x2 = right_container_coords.x1 - 1,
        .y1 = object_coords.y1,
        .y2 = object_coords.y2,
    };

    lv_draw_rect(layer, &smenu_item_fader_descriptor, &fader_area);
}

static void smenu_item_on_lvgl_event(const lv_obj_class_t* class, lv_event_t* event) {
    if(lv_obj_event_base(class, event) == LV_RESULT_OK) {
        lv_event_code_t event_code = lv_event_get_code(event);
        lv_obj_t* object = lv_event_get_target_obj(event);
        SMenuItem* instance = (SMenuItem*)object;

        if(event_code == LV_EVENT_FOCUSED) {
            lv_obj_scroll_to_view_recursive(object, LV_ANIM_OFF);
            instance->is_active = true;
        } else if(event_code == LV_EVENT_DEFOCUSED) {
            instance->is_active = false;
        } else if(event_code == LV_EVENT_DRAW_POST) {
            if(instance->do_draw_fader) {
                smenu_item_draw_fader(object, lv_event_get_layer(event));
            }
        }
    }
}

static void
    smenu_item_right_container_on_lvgl_event(const lv_obj_class_t* class, lv_event_t* event) {
    if(lv_obj_event_base(class, event) == LV_RESULT_OK) {
        lv_event_code_t event_code = lv_event_get_code(event);
        lv_obj_t* container = lv_event_get_target_obj(event);

        if(event_code == LV_EVENT_LAYOUT_CHANGED) {
            lv_obj_t* target = NULL;
            int32_t fixed_width = 0;

            for(uint32_t i = 0; i < lv_obj_get_child_count(container); i++) {
                lv_obj_t* child = lv_obj_get_child(container, i);

                if(lv_obj_has_flag(child, SMENU_ITEM_RIGHT_FLEX_GROW_FLAG)) {
                    target = child;
                } else if(lv_obj_is_layout_positioned(child)) {
                    fixed_width += lv_obj_get_width(child);
                }
            }

            if(target) {
                int32_t row_width = lv_obj_get_content_width(lv_obj_get_parent(container));
                int32_t active_width_cap = lv_obj_get_style_max_width(target, LV_PART_MAIN);
                int32_t next_width_cap = row_width * SMENU_ITEM_RIGHT_MAX_WIDTH_PCT / 100 -
                                         fixed_width -
                                         lv_obj_get_style_space_left(container, LV_PART_MAIN) -
                                         lv_obj_get_style_space_right(container, LV_PART_MAIN);

                if(next_width_cap > 0 && active_width_cap != next_width_cap) {
                    lv_obj_set_style_max_width(target, next_width_cap, LV_PART_MAIN);
                }
            }
        }
    }
}

static void smenu_item_style_front(const lv_obj_class_t* class, Widget* widget) {
    UNUSED(class);

    lv_obj_t* object = TO_LV_OBJ(widget);
    SMenuItem* instance = (SMenuItem*)widget;

    lv_obj_set_style_text_color(object, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_text_color(object, lv_color_white(), LV_PART_MAIN | LV_STATE_FOCUSED);

    lv_obj_set_style_pad_hor(instance->cursor_label, 1, LV_PART_MAIN);

    instance->do_draw_fader = true;
}

static void smenu_item_style_back(const lv_obj_class_t* class, Widget* widget) {
    UNUSED(class);

    lv_obj_t* object = TO_LV_OBJ(widget);
    SMenuItem* instance = (SMenuItem*)widget;

    lv_obj_set_style_pad_left(object, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(object, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(object, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_color(object, lv_color_black(), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_FOCUSED);

    lv_obj_set_style_pad_left(instance->cursor_label, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_right(instance->cursor_label, 1, LV_PART_MAIN);

    lv_obj_set_style_pad_left(instance->left_container, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(instance->left_container, 1, LV_PART_MAIN);

    lv_obj_set_style_pad_hor(instance->right_container, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(instance->right_container, 1, LV_PART_MAIN);

    instance->do_draw_fader = false;
}

static void smenu_item_lvgl_constructor(const lv_obj_class_t* class, lv_obj_t* object) {
    UNUSED(class);

    SMenuItem* instance = (SMenuItem*)object;

    lv_obj_set_flex_flow(object, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(object, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_remove_flag(object, LV_OBJ_FLAG_SCROLLABLE);

    instance->cursor_label = lv_label_create(object);
    lv_label_set_text_static(instance->cursor_label, "▶");
    lv_obj_add_flag(instance->cursor_label, LV_OBJ_FLAG_HIDDEN);

    instance->left_container = lv_obj_create(object);
    lv_obj_remove_style(instance->left_container, NULL, LV_PART_MAIN);
    lv_obj_set_flex_flow(instance->left_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(
        instance->left_container, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_flex_grow(instance->left_container, 1);
    lv_obj_set_size(instance->left_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_remove_flag(instance->left_container, LV_OBJ_FLAG_SCROLLABLE);

    instance->right_container =
        lv_obj_class_create_obj(&smenu_item_right_container_lvgl_class, object);
    lv_obj_class_init_obj(instance->right_container);

    lv_obj_remove_style(instance->right_container, NULL, LV_PART_MAIN);
    lv_obj_set_flex_flow(instance->right_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(
        instance->right_container, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_remove_flag(instance->right_container, LV_OBJ_FLAG_SCROLLABLE);
}

/* Public API */

Widget* smenu_item_get_base(SMenuItem* instance) {
    furi_check(instance);

    return &instance->base;
}

void smenu_item_set_cursor_state(SMenuItem* instance, SMenuItemCursorState state) {
    furi_check(instance);

    if(state == SMenuItemCursorStateHidden) {
        lv_obj_add_flag(instance->cursor_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_opa_t opacity;
        if(state == SMenuItemCursorStateTransparent) {
            opacity = LV_OPA_TRANSP;
        } else if(state == SMenuItemCursorStateNormal) {
            opacity = LV_OPA_COVER;
        } else {
            furi_crash();
        }

        lv_obj_remove_flag(instance->cursor_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(instance->cursor_label, opacity, LV_PART_MAIN);
    }
}

void smenu_item_set_callback(SMenuItem* instance, SMenuItemCallback callback) {
    furi_check(instance);

    instance->callback = callback;
}

/* LVGL Class Descriptor */

const lv_obj_class_t smenu_item_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = smenu_item_lvgl_constructor,
    .event_cb = smenu_item_on_lvgl_event,
    .name = "widget-smenu-item",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SMenuItem),
    .user_data =
        (void*)&(const WidgetClassData){
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = smenu_item_style_front,
                    [GuiDisplayIdBack] = smenu_item_style_back,
                },
        },
};

/* 
 * Flex never resizes content-sized children, so the container's max_width clamp
 * never reaches them; without a definite width a label can't ellipsize or scroll.
 * flex_grow on the child is no alternative: in a hugging container the grow
 * budget is fed by the child itself and never recovers from the zero first pass. 
 */
static const lv_obj_class_t smenu_item_right_container_lvgl_class = {
    .base_class = &lv_obj_class,
    .event_cb = smenu_item_right_container_on_lvgl_event,
    .name = "smenu-item-right-container",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(lv_obj_t),
};
