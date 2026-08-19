#include <furi.h>
#include "canvas_i.h"
#include <lvgl.h>

static Widget*
    canvas_image_update(CanvasWidget* widget, Widget* root, const CanvasElement* element) {
    if(!element) return image_get_base(widget->image);

    if(!widget->image) {
        widget->image = image_alloc(root);
    }
    image_set_source_no_cache(widget->image, furi_string_get_cstr(element->image.file_path));
    image_set_opacity(widget->image, element->image.opacity);

    return image_get_base(widget->image);
}

static void canvas_image_delete(CanvasWidget* widget) {
    furi_assert(widget->image);
    image_free(widget->image);
}

static Widget*
    canvas_raw_update(CanvasWidget* widget, Widget* root, const CanvasElement* element) {
    if(!element) return image_get_base(widget->image);

    if(!widget->image) {
        widget->image = image_alloc(root);
    }

    image_set_source_raw(
        widget->image,
        element->raw_image.format,
        element->raw_image.width,
        element->raw_image.height,
        element->raw_image.data,
        element->raw_image.data_size);
    image_set_opacity(widget->image, element->raw_image.opacity);

    return image_get_base(widget->image);
}

static void canvas_raw_delete(CanvasWidget* widget) {
    furi_assert(widget->image);
    image_free(widget->image);
}

static Widget*
    canvas_anim_player_update(CanvasWidget* widget, Widget* root, const CanvasElement* element) {
    if(!element) return anim_player_get_base(widget->anim_player);

    if(!widget->anim_player) {
        widget->anim_player = anim_player_alloc(root);
    }
    if(anim_player_set_source(
           widget->anim_player, furi_string_get_cstr(element->anim_player.file_path))) {
        anim_player_set_section(
            widget->anim_player,
            element->anim_player.flags,
            furi_string_get_cstr(element->anim_player.section));
    }

    Widget* base = anim_player_get_base(widget->anim_player);
    widget_set_opacity(base, element->anim_player.opacity);
    return base;
}

static void canvas_anim_player_delete(CanvasWidget* widget) {
    furi_assert(widget->anim_player);
    anim_player_free(widget->anim_player);
}

static Widget*
    canvas_text_update(CanvasWidget* widget, Widget* root, const CanvasElement* element) {
    if(!element) return label_get_base(widget->text);

    if(!widget->text) {
        widget->text = label_alloc(root);
    }
    label_set_text(widget->text, element->text.text_str);
    label_set_font(widget->text, element->text.font_path);
    label_set_text_color(widget->text, element->text.color);

    Widget* base = label_get_base(widget->text);
    if(element->text.width) {
        widget_set_width(base, element->text.width);
    } else {
        widget_set_width_content(base);
    }
    if(element->text.scroll_rate_cpm) {
        label_set_long_content_anim_speed(widget->text, element->text.scroll_rate_cpm);
        label_set_long_content_anim_start_delay(widget->text, element->text.scroll_start_delay);
        label_set_long_content_anim_repeat_delay(widget->text, element->text.scroll_repeat_delay);
        label_set_long_content_mode(widget->text, LabelLongContentModeScrollCircular);
    } else {
        label_set_long_content_mode(widget->text, LabelLongContentModeClip);
    }
    return base;
}

static void canvas_text_delete(CanvasWidget* widget) {
    furi_assert(widget->text);
    label_free(widget->text);
}

static const RectangleWidgetBackgroundType rectangle_fill_types[] = {
    [RectangleFillNone] = RectangleWidgetBackgroundNone,
    [RectangleFillSolid] = RectangleWidgetBackgroundSolid,
    [RectangleFillGradientH] = RectangleWidgetBackgroundGradientH,
    [RectangleFillGradientV] = RectangleWidgetBackgroundGradientV,
};
static_assert(COUNT_OF(rectangle_fill_types) == RectangleFillMax);
static_assert((size_t)RectangleWidgetBackgroundMax == (size_t)RectangleFillMax);

static Widget*
    canvas_countdown_update(CanvasWidget* widget, Widget* root, const CanvasElement* element) {
    if(!element) return label_get_base(widget->text);

    if(!widget->countdown) {
        widget->countdown = countdown_alloc(root);
    }
    countdown_set_text_color(widget->countdown, element->countdown.color);
    countdown_set_text_font(widget->countdown, FONT_BUSY_SUPERSCRIPT_7);
    countdown_begin(
        widget->countdown,
        element->countdown.timestamp,
        element->countdown.direction,
        element->countdown.hours);
    return countdown_get_base(widget->countdown);
}

static void canvas_countdown_delete(CanvasWidget* widget) {
    furi_assert(widget->countdown);
    countdown_free(widget->countdown);
}

static Widget*
    canvas_rectangle_update(CanvasWidget* widget, Widget* root, const CanvasElement* element) {
    if(!element) return rectangle_widget_get_base(widget->rectangle);

    if(!widget->rectangle) {
        widget->rectangle = rectangle_widget_alloc(root);
    }
    rectangle_widget_set_size(
        widget->rectangle,
        element->rectangle.width,
        element->rectangle.height,
        element->rectangle.radius);
    furi_assert(element->rectangle.fill < COUNT_OF(rectangle_fill_types));
    rectangle_widget_set_background(
        widget->rectangle,
        rectangle_fill_types[element->rectangle.fill],
        element->rectangle.fill_color[0],
        element->rectangle.fill_color[1]);
    rectangle_widget_set_border(
        widget->rectangle, element->rectangle.border_width, element->rectangle.border_color);
    return rectangle_widget_get_base(widget->rectangle);
}

static void canvas_rectangle_delete(CanvasWidget* widget) {
    furi_assert(widget->rectangle);
    rectangle_widget_free(widget->rectangle);
}

static const struct {
    Widget* (*update)(CanvasWidget* widget, Widget* root, const CanvasElement* element);
    void (*delete)(CanvasWidget* widget);
} canvas_widgets[] = {
    [CanvasElementTypeImage] = {canvas_image_update, canvas_image_delete},
    [CanvasElementTypeAnimPlayer] = {canvas_anim_player_update, canvas_anim_player_delete},
    [CanvasElementTypeText] = {canvas_text_update, canvas_text_delete},
    [CanvasElementTypeCountdown] = {canvas_countdown_update, canvas_countdown_delete},
    [CanvasElementTypeRectangle] = {canvas_rectangle_update, canvas_rectangle_delete},
    [CanvasElementTypeRawImage] = {canvas_raw_update, canvas_raw_delete},
};

static_assert(COUNT_OF(canvas_widgets) == CanvasElementTypeMax);

/**
 * LVGL applies `pos_x` and `pos_y` relative to the anchor point selected by
 * `Align`. We want alignment to behave like in Flipper Zero: the anchor point
 * is always relative to the top left of the screen, and the object is then
 * aligned relative to this anchor point.
 */
static void canvas_widget_reanchor(Widget* root, Align align, int32_t* x, int32_t* y) {
    furi_assert(root);
    furi_assert(x);
    furi_assert(y);

    int32_t disp_width = widget_get_max_width(root);
    int32_t disp_height = widget_get_max_height(root);
    AlignBitmask align_bm = widget_align_to_bitmask(align);

    furi_assert(disp_width > 0);
    furi_assert(disp_height > 0);

    int32_t lvgl_anchor_x;
    if(align_bm & AlignBitmaskLeft) lvgl_anchor_x = 0;
    if(align_bm & AlignBitmaskHorCenter) lvgl_anchor_x = disp_width / 2;
    if(align_bm & AlignBitmaskRight) lvgl_anchor_x = disp_width;

    int32_t lvgl_anchor_y;
    if(align_bm & AlignBitmaskTop) lvgl_anchor_y = 0;
    if(align_bm & AlignBitmaskVerCenter) lvgl_anchor_y = disp_height / 2;
    if(align_bm & AlignBitmaskBottom) lvgl_anchor_y = disp_height;

    *x -= lvgl_anchor_x;
    *y -= lvgl_anchor_y;
}

void canvas_widget_update(CanvasWidget* widget, Widget* root, const CanvasElement* element) {
    furi_assert(widget);
    furi_assert(root);
    furi_assert(element);

    furi_assert(widget->type < CanvasElementTypeMax);
    furi_assert(canvas_widgets[widget->type].update);
    Widget* base = canvas_widgets[widget->type].update(widget, root, element);

    int32_t x = element->x;
    int32_t y = element->y;
    Align align = element->align;

    canvas_widget_reanchor(root, align, &x, &y);
    widget_set_align(base, align);
    widget_set_pos(base, x, y);
}

void canvas_widget_delete(CanvasWidget* widget) {
    furi_assert(widget->type < CanvasElementTypeMax);
    furi_assert(canvas_widgets[widget->type].delete);
    canvas_widgets[widget->type].delete(widget);
}

void canvas_widget_to_front(CanvasWidget* widget) {
    furi_assert(widget);
    Widget* base = canvas_widgets[widget->type].update(widget, NULL, NULL);
    widget_move_to_foreground(base);
}
