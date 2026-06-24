#include "rectangle.h"

#include <gui/widget_i.h>

#define MY_CLASS (&rectangle_lvgl_class)

struct RectangleWidget {
    Widget base;
};

const lv_obj_class_t rectangle_lvgl_class;

static void rectangle_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    UNUSED(obj);
}

RectangleWidget* rectangle_widget_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    RectangleWidget* instance = (RectangleWidget*)obj;
    return instance;
}

void rectangle_widget_free(RectangleWidget* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* rectangle_widget_get_base(RectangleWidget* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void rectangle_widget_set_size(
    RectangleWidget* instance,
    size_t width,
    size_t height,
    int8_t radius) {
    furi_check(instance);
    lv_obj_t* obj = TO_LV_OBJ(instance);

    lv_obj_set_size(obj, width, height);
    lv_obj_set_style_radius(obj, radius, LV_PART_MAIN);
}

void rectangle_widget_set_background(
    RectangleWidget* instance,
    RectangleWidgetBackgroundType type,
    Color color_start,
    Color color_end) {
    furi_check(instance);

    lv_obj_t* obj = TO_LV_OBJ(instance);

    lv_color_t lv_color_start = TO_LV_COLOR(color_start);
    lv_opa_t lv_opa_start = (lv_opa_t)(color_start.a);
    lv_color_t lv_color_end = TO_LV_COLOR(color_end);
    lv_opa_t lv_opa_end = (lv_opa_t)(color_end.a);

    switch(type) {
    case RectangleWidgetBackgroundNone:
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
        break;
    case RectangleWidgetBackgroundSolid:
        lv_obj_set_style_bg_color(obj, lv_color_start, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(obj, lv_opa_start, LV_PART_MAIN);
        break;
    case RectangleWidgetBackgroundGradientH:
        lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(obj, lv_color_start, LV_PART_MAIN);
        lv_obj_set_style_bg_main_opa(obj, lv_opa_start, LV_PART_MAIN);
        lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_HOR, LV_PART_MAIN);
        lv_obj_set_style_bg_grad_color(obj, lv_color_end, LV_PART_MAIN);
        lv_obj_set_style_bg_grad_opa(obj, lv_opa_end, LV_PART_MAIN);
        break;
    case RectangleWidgetBackgroundGradientV:
        lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(obj, lv_color_start, LV_PART_MAIN);
        lv_obj_set_style_bg_main_opa(obj, lv_opa_start, LV_PART_MAIN);
        lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
        lv_obj_set_style_bg_grad_color(obj, lv_color_end, LV_PART_MAIN);
        lv_obj_set_style_bg_grad_opa(obj, lv_opa_end, LV_PART_MAIN);
        break;
    default:
        furi_crash();
    }
}

void rectangle_widget_set_border(RectangleWidget* instance, int8_t width, Color color) {
    furi_check(instance);
    lv_obj_t* obj = TO_LV_OBJ(instance);
    lv_color_t lv_color = TO_LV_COLOR(color);
    lv_opa_t lv_opa = (lv_opa_t)(color.a);

    lv_obj_set_style_border_width(obj, width, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color, LV_PART_MAIN);
    lv_obj_set_style_border_opa(obj, lv_opa, LV_PART_MAIN);
}

const lv_obj_class_t rectangle_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = rectangle_lvgl_constructor,
    .name = "widget-rectangle",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(RectangleWidget),
};
