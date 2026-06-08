#include "widget_i.h"

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS WIDGET_CLASS

#define WIDGET_SCROLLBAR_FADE_DELAY_MS    2000
#define WIDGET_SCROLLBAR_FADE_DURATION_MS 350

typedef struct {
    lv_scrollbar_mode_t mode;
    lv_dir_t scroll_direction;
    lv_opa_t scroll_bar_opacity;
} WidgetScrollBarSetup;

static const WidgetScrollBarSetup widget_scrollbar_modes_map[] = {
    [WidgetScrollBarModeOff] =
        {
            .mode = LV_SCROLLBAR_MODE_OFF,
            .scroll_direction = LV_DIR_NONE,
            .scroll_bar_opacity = LV_OPA_TRANSP,
        },
    [WidgetScrollBarModeOn] =
        {
            .mode = LV_SCROLLBAR_MODE_ON,
            .scroll_direction = LV_DIR_VER,
            .scroll_bar_opacity = LV_OPA_COVER,
        },
    [WidgetScrollBarModeAuto] =
        {
            .mode = LV_SCROLLBAR_MODE_AUTO,
            .scroll_direction = LV_DIR_VER,
            .scroll_bar_opacity = LV_OPA_TRANSP,
        },
};

static_assert(COUNT_OF(widget_scrollbar_modes_map) == WidgetScrollBarModesCount);

static bool widget_input_feed_default(Widget* instance, const InputEvent* event) {
    bool consumed = false;

    lv_obj_t* obj = TO_LV_OBJ(instance);

    if(lv_obj_get_scrollbar_mode(obj) != LV_SCROLLBAR_MODE_OFF) {
        const int32_t delta = lv_display_get_vertical_resolution(lv_obj_get_display(obj)) / 8;

        if(event->type == InputTypeShort) {
            if(event->key == InputKeyUp) {
                lv_obj_scroll_by_bounded(obj, 0, -delta, false);
                consumed = true;

            } else if(event->key == InputKeyDown) {
                lv_obj_scroll_by_bounded(obj, 0, delta, false);
                consumed = true;
            }
        }
    }

    return consumed;
}

/* LVGL-specific code */

static void widget_scrollbar_fade_anim_callback(void* variable, int32_t value) {
    lv_obj_set_style_bg_opa(variable, value, LV_PART_SCROLLBAR);
}

static void widget_scrollbar_show(lv_obj_t* obj) {
    lv_anim_delete(obj, widget_scrollbar_fade_anim_callback);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_SCROLLBAR);
}

static void widget_scrollbar_run_fade_out_anim(lv_obj_t* obj) {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_delay(&anim, WIDGET_SCROLLBAR_FADE_DELAY_MS);
    lv_anim_set_duration(&anim, WIDGET_SCROLLBAR_FADE_DURATION_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&anim, widget_scrollbar_fade_anim_callback);
    lv_anim_start(&anim);
}

static bool widget_scrollbar_is_visible(lv_obj_t* obj) {
    lv_scrollbar_mode_t mode = lv_obj_get_scrollbar_mode(obj);

    if(mode == LV_SCROLLBAR_MODE_OFF) return false;
    if(lv_obj_get_style_bg_opa(obj, LV_PART_SCROLLBAR) < LV_OPA_MIN) return false;
    if(mode == LV_SCROLLBAR_MODE_ON) return true;
    if(lv_obj_get_scroll_top(obj) <= 0 && lv_obj_get_scroll_bottom(obj) <= 0) return false;

    return true;
}

static void widget_draw_scrollbar_track(lv_obj_t* obj, lv_layer_t* layer) {
    if(!widget_scrollbar_is_visible(obj)) return;

    int32_t width = lv_obj_get_style_width(obj, LV_PART_SCROLLBAR);
    int32_t pad_right = lv_obj_get_style_pad_right(obj, LV_PART_SCROLLBAR);
    int32_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_SCROLLBAR);
    int32_t pad_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_SCROLLBAR);

    lv_area_t track_area = {
        .x1 = obj->coords.x2 - pad_right - width + 1,
        .y1 = obj->coords.y1 + pad_top,
        .x2 = obj->coords.x2 - pad_right,
        .y2 = obj->coords.y2 - pad_bottom,
    };

    lv_draw_rect_dsc_t track_dsc;
    lv_draw_rect_dsc_init(&track_dsc);
    track_dsc.bg_color = lv_obj_get_style_outline_color(obj, LV_PART_SCROLLBAR);
    track_dsc.bg_opa = LV_OPA_MIX2(
        lv_obj_get_style_bg_opa(obj, LV_PART_SCROLLBAR),
        lv_obj_get_style_outline_opa(obj, LV_PART_SCROLLBAR));

    lv_draw_rect(layer, &track_dsc, &track_area);
}

static void widget_lvgl_event_callback(const lv_obj_class_t* class_p, lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_current_target(e);

    switch(lv_event_get_code(e)) {
    case LV_EVENT_DRAW_POST:
        widget_draw_scrollbar_track(obj, lv_event_get_layer(e));
        break;

    case LV_EVENT_SCROLL_BEGIN:
    /* fall-through */
    case LV_EVENT_SCROLL:
        if(lv_obj_get_scrollbar_mode(obj) == LV_SCROLLBAR_MODE_AUTO) {
            widget_scrollbar_show(obj);
            widget_scrollbar_run_fade_out_anim(obj);
        }
        break;

    default:
        break;
    }

    lv_obj_event_base(class_p, e);
}

static void widget_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

/* Public API */

static const AlignBitmask align_to_bitmask_lut[AlignMax] = {
    [AlignDefault] = AlignBitmaskTop | AlignBitmaskLeft,
    [AlignTopLeft] = AlignBitmaskTop | AlignBitmaskLeft,
    [AlignTopMid] = AlignBitmaskTop | AlignBitmaskHorCenter,
    [AlignTopRight] = AlignBitmaskTop | AlignBitmaskRight,
    [AlignLeftMid] = AlignBitmaskVerCenter | AlignBitmaskLeft,
    [AlignCenter] = AlignBitmaskVerCenter | AlignBitmaskHorCenter,
    [AlignRightMid] = AlignBitmaskVerCenter | AlignBitmaskRight,
    [AlignBottomLeft] = AlignBitmaskBottom | AlignBitmaskLeft,
    [AlignBottomMid] = AlignBitmaskBottom | AlignBitmaskHorCenter,
    [AlignBottomRight] = AlignBitmaskBottom | AlignBitmaskRight,
};

AlignBitmask widget_align_to_bitmask(Align align) {
    furi_check(align < AlignMax);
    return align_to_bitmask_lut[align];
}

Widget* widget_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    return (Widget*)obj;
}

void widget_free(Widget* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

void widget_set_visible(Widget* instance, bool visible) {
    furi_check(instance);
    if(visible) {
        lv_obj_remove_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    }
}

bool widget_is_visible(const Widget* instance) {
    furi_check(instance);
    return !lv_obj_has_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_HIDDEN);
}

void widget_set_ignore_layout(Widget* instance, bool ignore_layout) {
    furi_check(instance);
    lv_obj_update_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_IGNORE_LAYOUT, ignore_layout);
}

bool widget_does_ignore_layout(const Widget* instance) {
    furi_check(instance);
    return lv_obj_has_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_IGNORE_LAYOUT);
}

void widget_set_width(Widget* instance, int32_t width) {
    furi_check(instance);
    lv_obj_set_width((lv_obj_t*)instance, width);
}

int32_t widget_get_width(const Widget* instance) {
    furi_check(instance);
    return lv_obj_get_width((const lv_obj_t*)instance);
}

void widget_set_height(Widget* instance, int32_t height) {
    furi_check(instance);
    lv_obj_set_height((lv_obj_t*)instance, height);
}

int32_t widget_get_height(const Widget* instance) {
    furi_check(instance);
    return lv_obj_get_height((const lv_obj_t*)instance);
}

void widget_set_size(Widget* instance, int32_t width, int32_t height) {
    furi_check(instance);
    lv_obj_set_size((lv_obj_t*)instance, width, height);
}

void widget_set_max_width(Widget* instance, int32_t max_width) {
    furi_check(instance);
    lv_obj_set_style_max_width(TO_LV_OBJ(instance), max_width, LV_PART_MAIN);
}

void widget_set_max_height(Widget* instance, int32_t max_height) {
    furi_check(instance);
    lv_obj_set_style_max_height(TO_LV_OBJ(instance), max_height, LV_PART_MAIN);
}

void widget_set_max_size(Widget* instance, int32_t max_width, int32_t max_height) {
    furi_check(instance);
    lv_obj_set_style_max_width(TO_LV_OBJ(instance), max_width, LV_PART_MAIN);
    lv_obj_set_style_max_height(TO_LV_OBJ(instance), max_height, LV_PART_MAIN);
}

void widget_set_size_content(Widget* instance) {
    furi_check(instance);
    lv_obj_set_size((lv_obj_t*)instance, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}

void widget_set_width_content(Widget* instance) {
    furi_check(instance);
    lv_obj_set_width((lv_obj_t*)instance, LV_SIZE_CONTENT);
}

void widget_set_height_content(Widget* instance) {
    furi_check(instance);
    lv_obj_set_height((lv_obj_t*)instance, LV_SIZE_CONTENT);
}

int32_t widget_get_max_width(const Widget* instance) {
    furi_check(instance);
    return lv_obj_get_style_max_width(TO_LV_OBJ(instance), LV_PART_MAIN);
}

int32_t widget_get_max_height(const Widget* instance) {
    furi_check(instance);
    return lv_obj_get_style_max_height(TO_LV_OBJ(instance), LV_PART_MAIN);
}

void widget_set_pos_x(Widget* instance, int32_t x) {
    furi_check(instance);
    lv_obj_set_x((lv_obj_t*)instance, x);
}

void widget_set_pos_y(Widget* instance, int32_t y) {
    furi_check(instance);
    lv_obj_set_y((lv_obj_t*)instance, y);
}

void widget_set_pos(Widget* instance, int32_t x, int32_t y) {
    furi_check(instance);
    lv_obj_set_pos((lv_obj_t*)instance, x, y);
}

void widget_set_align(Widget* instance, Align align) {
    furi_check(instance);
    furi_check(align < AlignMax);
    lv_obj_set_align((lv_obj_t*)instance, (lv_align_t)align);
}

void widget_move_to_foreground(Widget* instance) {
    furi_check(instance);
    lv_obj_move_foreground((lv_obj_t*)instance);
}

void widget_move_to_background(Widget* instance) {
    furi_check(instance);
    lv_obj_move_background((lv_obj_t*)instance);
}

void widget_set_scrollbar_mode(Widget* instance, WidgetScrollBarMode mode) {
    furi_check(instance);
    furi_check(mode < WidgetScrollBarModesCount);

    lv_obj_t* lv_object = TO_LV_OBJ(instance);

    lv_anim_delete(lv_object, widget_scrollbar_fade_anim_callback);

    const WidgetScrollBarSetup* setup = &widget_scrollbar_modes_map[mode];

    lv_obj_set_scrollbar_mode(lv_object, setup->mode);
    lv_obj_set_scroll_dir(lv_object, setup->scroll_direction);
    lv_obj_set_style_bg_opa(lv_object, setup->scroll_bar_opacity, LV_PART_SCROLLBAR);
}

void widget_set_background_color(Widget* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_bg_color((lv_obj_t*)instance, TO_LV_COLOR(color), LV_PART_MAIN);
    lv_obj_set_style_bg_opa((lv_obj_t*)instance, (lv_opa_t)(color.a), LV_PART_MAIN);
}

void widget_set_padding(Widget* instance, int32_t left, int32_t right, int32_t top, int32_t bottom) {
    furi_check(instance);
    lv_obj_set_style_pad_left((lv_obj_t*)instance, left, LV_PART_MAIN);
    lv_obj_set_style_pad_right((lv_obj_t*)instance, right, LV_PART_MAIN);
    lv_obj_set_style_pad_top((lv_obj_t*)instance, top, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom((lv_obj_t*)instance, bottom, LV_PART_MAIN);
}

void widget_set_margin(Widget* instance, int32_t left, int32_t right, int32_t top, int32_t bottom) {
    furi_check(instance);
    lv_obj_set_style_margin_left((lv_obj_t*)instance, left, LV_PART_MAIN);
    lv_obj_set_style_margin_right((lv_obj_t*)instance, right, LV_PART_MAIN);
    lv_obj_set_style_margin_top((lv_obj_t*)instance, top, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom((lv_obj_t*)instance, bottom, LV_PART_MAIN);
}

void widget_set_blend_mode(Widget* instance, WidgetBlendMode blend_mode) {
    furi_check(instance);
    furi_check(blend_mode < WidgetBlendModesCount);
    lv_obj_set_style_blend_mode(TO_LV_OBJ(instance), (lv_blend_mode_t)blend_mode, LV_PART_MAIN);
}

WidgetBlendMode widget_get_blend_mode(const Widget* instance) {
    furi_check(instance);
    return (WidgetBlendMode)lv_obj_get_style_blend_mode(TO_LV_OBJ(instance), LV_PART_MAIN);
}

/* Private API */

bool widget_input(Widget* instance, const InputEvent* event) {
    bool consumed = false;

    do {
        lv_obj_t* lv_object = TO_LV_OBJ(instance);

        if(lv_obj_has_flag(lv_object, LV_OBJ_FLAG_HIDDEN)) {
            break;
        }

        const lv_obj_class_t* lv_class = lv_obj_get_class(lv_object);
        const WidgetClassData* class_data = lv_class->user_data;
        if(class_data && class_data->input_callback) {
            consumed = class_data->input_callback(instance, event);

            if(consumed) break;
        }

        const uint32_t child_count = lv_obj_get_child_count(lv_object);

        for(uint32_t i = 0; i < child_count; ++i) {
            lv_obj_t* child = lv_obj_get_child(lv_object, i);

            if(IS_WIDGET_CLASS(child)) {
                // Recursion should not be a problem
                // when the widget tree is not too deep
                if(widget_input((Widget*)child, event)) {
                    consumed = true;
                }
            }
        }

        if(!consumed) {
            consumed = widget_input_feed_default(instance, event);
        }

    } while(false);

    return consumed;
}

void widget_style(Widget* instance, GuiDisplayId display_id) {
    lv_obj_t* lv_object = TO_LV_OBJ(instance);
    const lv_obj_class_t* lv_class = lv_obj_get_class(lv_object);

    if(lv_class->user_data) {
        const WidgetClassData* class_data = lv_class->user_data;
        WidgetStyleCallback style_callback = class_data->style_callbacks[display_id];

        if(style_callback) style_callback(instance);
    }
}

/* LVGL class descriptor */

const lv_obj_class_t widget_lvgl_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = widget_lvgl_constructor,
    .event_cb = widget_lvgl_event_callback,
    .name = "widget",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Widget),
};
