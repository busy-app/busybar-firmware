#include "slider_view.h"

#include <gui/widget_i.h>

#include <m-array.h>

#define BACK_BAR_BG_COLOR_HEX        (0x444444)
#define BACK_DISABLED_TEXT_COLOR_HEX (0x444444)

typedef struct {
    int32_t level;
    char* path;
} SliderViewLevelImage;

ARRAY_DEF(SliderViewLevelImageList, SliderViewLevelImage, M_POD_OPLIST);

struct SliderView {
    Widget base;
    lv_obj_t* bar;
    lv_obj_t* image;
    lv_obj_t* text_container;
    lv_obj_t* left_arrow_label;
    lv_obj_t* value_label;
    lv_obj_t* right_arrow_label;

    FontRegistry* font_registry;
    const lv_font_t* font_text;

    SliderViewLevelImageList_t level_images;
    const char* active_image_path;

    char* suffix;
    int32_t min_value;
    int32_t max_value;
    int32_t step;
    int32_t value;

    SliderViewCallback callback;
    void* callback_context;
};

static const lv_obj_class_t slider_view_lvgl_class;

static void slider_view_refresh(SliderView* instance);

static bool slider_view_input_callback(Widget* widget, const InputEvent* event) {
    SliderView* instance = (SliderView*)widget;

    if(event->type != InputTypeShort) return false;

    int32_t delta;
    switch(event->key) {
    case InputKeyUp:
        delta = instance->step;
        break;

    case InputKeyDown:
        delta = -instance->step;
        break;

    default:
        return false;
    }

    instance->value = CLAMP(instance->value + delta, instance->max_value, instance->min_value);
    slider_view_refresh(instance);

    if(instance->callback) instance->callback(instance->value, instance->callback_context);

    return true;
}

static void slider_view_style_front(Widget* widget) {
    SliderView* instance = (SliderView*)widget;

    instance->font_text = font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_7);

    lv_obj_set_height(instance->bar, LV_PCT(100));

    lv_image_set_align(instance->image, LV_IMAGE_ALIGN_RIGHT_MID);
    lv_obj_set_align(instance->image, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_pad_left(instance->image, 1, LV_PART_MAIN);

    lv_obj_set_align(instance->text_container, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_font(instance->text_container, instance->font_text, LV_PART_MAIN);
    lv_obj_set_style_pad_column(instance->text_container, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_right(instance->text_container, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(instance->text_container, 1, LV_PART_MAIN);

    lv_obj_set_style_text_opa(
        instance->left_arrow_label, LV_OPA_50, LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_text_opa(
        instance->right_arrow_label, LV_OPA_50, LV_PART_MAIN | LV_STATE_DISABLED);
}

static void slider_view_style_back(Widget* widget) {
    SliderView* instance = (SliderView*)widget;

    instance->font_text = font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_9);

    lv_obj_set_flex_flow(TO_LV_OBJ(instance), LV_FLEX_FLOW_COLUMN_REVERSE);
    lv_obj_set_flex_align(
        TO_LV_OBJ(instance), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(TO_LV_OBJ(instance), 6, LV_PART_MAIN);

    lv_obj_set_style_margin_left(instance->bar, 2, LV_PART_MAIN);
    lv_obj_set_style_margin_right(instance->bar, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(instance->bar, 4, LV_PART_MAIN);
    lv_obj_set_style_height(instance->bar, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(instance->bar, lv_color_hex(BACK_BAR_BG_COLOR_HEX), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(instance->bar, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_recolor(instance->image, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_recolor_opa(instance->image, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_pad_column(instance->text_container, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->text_container, instance->font_text, LV_PART_MAIN);

    lv_obj_set_style_text_color(
        instance->left_arrow_label,
        lv_color_hex(BACK_DISABLED_TEXT_COLOR_HEX),
        LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_text_color(
        instance->right_arrow_label,
        lv_color_hex(BACK_DISABLED_TEXT_COLOR_HEX),
        LV_PART_MAIN | LV_STATE_DISABLED);
}

static void slider_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    SliderView* instance = (SliderView*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);

    instance->bar = lv_bar_create(TO_LV_OBJ(instance));
    lv_obj_set_width(instance->bar, LV_PCT(100));
    lv_obj_set_style_bg_opa(instance->bar, LV_OPA_COVER, LV_PART_INDICATOR);

    instance->text_container = lv_obj_create(TO_LV_OBJ(instance));
    lv_obj_set_size(instance->text_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(instance->text_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_text_color(instance->text_container, lv_color_white(), LV_PART_MAIN);

    instance->left_arrow_label = lv_label_create(instance->text_container);
    lv_label_set_text_static(instance->left_arrow_label, "<");

    instance->value_label = lv_label_create(instance->text_container);

    instance->right_arrow_label = lv_label_create(instance->text_container);
    lv_label_set_text_static(instance->right_arrow_label, ">");

    instance->image = lv_image_create(TO_LV_OBJ(instance));
    lv_obj_add_flag(instance->image, LV_OBJ_FLAG_HIDDEN);

    instance->suffix = NULL;
    instance->min_value = lv_bar_get_min_value(instance->bar);
    instance->max_value = lv_bar_get_max_value(instance->bar);
    instance->step = 1;
    instance->value = 0;

    SliderViewLevelImageList_init(instance->level_images);
    instance->active_image_path = NULL;

    instance->callback = NULL;

    slider_view_refresh(instance);
}

static void slider_view_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    SliderView* instance = (SliderView*)obj;

    if(instance->font_text) {
        font_registry_unload_font(instance->font_registry, instance->font_text);
    }

    furi_record_close(RECORD_FONT_REGISTRY);

    free(instance->suffix);

    SliderViewLevelImageList_it_t it;
    SliderViewLevelImageList_it(it, instance->level_images);
    for(; !SliderViewLevelImageList_end_p(it); SliderViewLevelImageList_next(it)) {
        free(SliderViewLevelImageList_ref(it)->path);
    }

    SliderViewLevelImageList_clear(instance->level_images);
}

static void slider_view_refresh(SliderView* instance) {
    furi_check(instance);

    lv_bar_set_value(instance->bar, instance->value, LV_ANIM_OFF);

    const char* path = NULL;
    SliderViewLevelImageList_it_t it;
    SliderViewLevelImageList_it(it, instance->level_images);
    for(; !SliderViewLevelImageList_end_p(it); SliderViewLevelImageList_next(it)) {
        const SliderViewLevelImage* level_image = SliderViewLevelImageList_cref(it);

        if(instance->value >= level_image->level) {
            path = level_image->path;
            break;
        }
    }

    if(path != instance->active_image_path) {
        if(path) {
            lv_image_set_src(instance->image, path);
            lv_obj_remove_flag(instance->image, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(instance->image, LV_OBJ_FLAG_HIDDEN);
        }

        instance->active_image_path = path;
    }

    if(instance->suffix) {
        lv_label_set_text_fmt(
            instance->value_label, "%" PRIi32 "%s", instance->value, instance->suffix);
    } else {
        lv_label_set_text_fmt(instance->value_label, "%" PRIi32, instance->value);
    }

    lv_obj_set_state(
        instance->left_arrow_label, LV_STATE_DISABLED, instance->value <= instance->min_value);

    lv_obj_set_state(
        instance->right_arrow_label, LV_STATE_DISABLED, instance->value >= instance->max_value);
}

/* Public API */

SliderView* slider_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* lv_object = lv_obj_class_create_obj(&slider_view_lvgl_class, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(lv_object);

    SliderView* instance = (SliderView*)lv_object;

    return instance;
}

void slider_view_free(SliderView* instance) {
    furi_check(instance);

    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* slider_view_get_base(SliderView* instance) {
    furi_check(instance);

    return &instance->base;
}

void slider_view_add_level_image(SliderView* instance, int32_t level, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    SliderViewLevelImageList_push_back(
        instance->level_images,
        (SliderViewLevelImage){
            .level = level,
            .path = strdup(file_path),
        });

    slider_view_refresh(instance);
}

void slider_view_set_bar_gradient(SliderView* instance, Color start, Color end) {
    furi_check(instance);

    lv_obj_set_style_bg_color(instance->bar, TO_LV_COLOR(start), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(instance->bar, TO_LV_COLOR(end), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_dir(instance->bar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
}

void slider_view_set_suffix(SliderView* instance, const char* suffix) {
    furi_check(instance);

    free(instance->suffix);
    instance->suffix = suffix ? strdup(suffix) : NULL;

    slider_view_refresh(instance);
}

void slider_view_set_range(SliderView* instance, int32_t min, int32_t max) {
    furi_check(instance);
    furi_check(min <= max);

    lv_bar_set_range(instance->bar, min, max);

    instance->min_value = min;
    instance->max_value = max;

    instance->value = CLAMP(instance->value, max, min);

    slider_view_refresh(instance);
}

void slider_view_set_step(SliderView* instance, int32_t step) {
    furi_check(instance);
    furi_check(step > 0);

    instance->step = step;
}

void slider_view_set_value(SliderView* instance, int32_t value) {
    furi_check(instance);

    instance->value = CLAMP(value, instance->max_value, instance->min_value);

    slider_view_refresh(instance);
}

void slider_view_set_callback(SliderView* instance, SliderViewCallback callback, void* context) {
    furi_check(instance);

    instance->callback = callback;
    instance->callback_context = context;
}

/* LVGL class descriptor */

static const lv_obj_class_t slider_view_lvgl_class = {
    .base_class = &widget_lvgl_class,

    .constructor_cb = slider_view_lvgl_constructor,
    .destructor_cb = slider_view_lvgl_destructor,

    .name = "widget-slider-view",

    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),

    .instance_size = sizeof(SliderView),

    .user_data =
        (void*)&(const WidgetClassData){
            .input_callback = slider_view_input_callback,
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = slider_view_style_front,
                    [GuiDisplayIdBack] = slider_view_style_back,
                },
        },
};
