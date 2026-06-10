#include "menu_base_i.h"

#define MY_CLASS (&menu_base_lvgl_class)

#define ROUND_INT_DIV(n, d)     \
    ({                          \
        __typeof__(n) _n = (n); \
        __typeof__(d) _d = (d); \
        (_n + _d / 2) / _d;     \
    })

#define SCROLL_ANIM_DURATION_MS 0

const lv_obj_class_t menu_base_lvgl_class;

static void menu_base_scroll_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = SCROLL_ANIM_DURATION_MS;
    }
}

static bool
    menu_base_draw_scrollbar_thumb(lv_obj_t* obj, lv_layer_t* layer, const lv_area_t* track_area) {
    MenuBase* instance = (MenuBase*)obj;

    lv_scroll_snap_t snap_y = lv_obj_get_scroll_snap_y(obj);
    if(snap_y == LV_SCROLL_SNAP_NONE) return false;

    int32_t child_count = lv_obj_get_child_count(obj);
    int32_t track_height = lv_area_get_height(track_area);
    int32_t focused_item_idx = lv_obj_get_index(lv_group_get_focused(instance->group));
    int32_t thumb_height =
        LV_MAX(track_height / child_count, lv_obj_get_style_length(obj, LV_PART_SCROLLBAR));
    int32_t thumb_y =
        (child_count > 1) ?
            ROUND_INT_DIV((track_height - thumb_height) * focused_item_idx, child_count - 1) :
            0;

    lv_area_t thumb_area = {
        .x1 = track_area->x1,
        .y1 = track_area->y1 + thumb_y,
        .x2 = track_area->x2,
        .y2 = track_area->y1 + thumb_y + thumb_height - 1,
    };

    lv_draw_rect_dsc_t thumb_dsc;
    lv_draw_rect_dsc_init(&thumb_dsc);
    thumb_dsc.bg_color = lv_obj_get_style_bg_color(obj, LV_PART_SCROLLBAR);
    thumb_dsc.bg_opa = lv_obj_get_style_bg_opa(obj, LV_PART_SCROLLBAR);
    thumb_dsc.radius = lv_obj_get_style_radius(obj, LV_PART_SCROLLBAR);

    lv_draw_rect(layer, &thumb_dsc, &thumb_area);

    return true;
}

static void menu_base_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(obj, menu_base_scroll_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    MenuBase* instance = (MenuBase*)obj;

    instance->group = lv_group_create();
    lv_group_set_wrap(instance->group, false);
}

static void menu_base_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    MenuBase* instance = (MenuBase*)obj;
    lv_group_delete(instance->group);
}

const lv_obj_class_t menu_base_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = menu_base_lvgl_constructor,
    .destructor_cb = menu_base_lvgl_destructor,
    .name = "widget-menu-base",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(MenuBase),
    .user_data =
        (void*)&(const WidgetClassData){
            .draw_scrollbar_thumb = menu_base_draw_scrollbar_thumb,
        },
};
