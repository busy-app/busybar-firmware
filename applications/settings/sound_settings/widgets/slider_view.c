#include "slider_view.h"

#include <m-array.h>

#include <gui/widget_i.h>

#define MY_CLASS                (&slider_view_lvgl_class)
#define MY_BAR_CLASS            (&slider_view_bar_lvgl_class)
#define MY_IMAGE_CLASS          (&slider_view_image_lvgl_class)
#define MY_TEXT_CONTAINER_CLASS (&slider_view_text_container_lvgl_class)
#define MY_ARROW_LABEL_CLASS    (&slider_view_arrow_label_lvgl_class)

typedef struct {
    int32_t level;
    lv_obj_t* image;
} LevelImage;

ARRAY_DEF(LevelImageList, LevelImage, M_POD_OPLIST);

struct SliderView {
    Widget base;
    lv_obj_t* bar;
    lv_obj_t* left_arrow_label;
    lv_obj_t* value_label;
    lv_obj_t* right_arrow_label;
    lv_obj_t* active_image;

    LevelImageList_t level_images;

    char* suffix;
    int32_t min_value;
    int32_t max_value;
    int32_t step;
    int32_t value;

    SliderViewCallback callback;
    void* context;
};

const lv_obj_class_t slider_view_lvgl_class;
const lv_obj_class_t slider_view_bar_lvgl_class;
const lv_obj_class_t slider_view_text_container_lvgl_class;
const lv_obj_class_t slider_view_arrow_label_lvgl_class;
const lv_obj_class_t slider_view_image_lvgl_class;

static void slider_view_update(SliderView* instance);

/* LVGL-specific code */

static bool slider_view_input_callback(Widget* widget, const InputEvent* event) {
    SliderView* instance = (SliderView*)widget;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyUp:
            if(instance->value < instance->max_value) {
                instance->value += instance->step;
                if(instance->value > instance->max_value) {
                    instance->value = instance->max_value;
                }

                slider_view_update(instance);

                if(instance->callback) {
                    instance->callback(instance->value, instance->context);
                }

                consumed = true;
            }
            break;

        case InputKeyDown:
            if(instance->value > instance->min_value) {
                instance->value -= instance->step;
                if(instance->value < instance->min_value) {
                    instance->value = instance->min_value;
                }

                slider_view_update(instance);

                if(instance->callback) {
                    instance->callback(instance->value, instance->context);
                }

                consumed = true;
            }
            break;

        default:
            break;
        }
    }

    return consumed;
}

static void slider_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    SliderView* instance = (SliderView*)obj;

    instance->bar = lv_obj_class_create_obj(MY_BAR_CLASS, obj);
    lv_obj_class_init_obj(instance->bar);
    lv_obj_set_style_bg_opa(instance->bar, LV_OPA_COVER, LV_PART_INDICATOR);

    lv_obj_t* text_container = lv_obj_class_create_obj(MY_TEXT_CONTAINER_CLASS, obj);
    lv_obj_class_init_obj(text_container);
    lv_obj_set_flex_flow(text_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_text_color(text_container, lv_color_white(), LV_PART_MAIN);

    instance->left_arrow_label = lv_obj_class_create_obj(MY_ARROW_LABEL_CLASS, text_container);
    lv_obj_class_init_obj(instance->left_arrow_label);
    lv_label_set_text(instance->left_arrow_label, "<");

    instance->value_label = lv_label_create(text_container);
    lv_obj_class_init_obj(instance->value_label);

    instance->right_arrow_label = lv_obj_class_create_obj(MY_ARROW_LABEL_CLASS, text_container);
    lv_obj_class_init_obj(instance->right_arrow_label);
    lv_label_set_text(instance->right_arrow_label, ">");

    instance->min_value = lv_bar_get_min_value(instance->bar);
    instance->max_value = lv_bar_get_max_value(instance->bar);
    instance->step = 1;
    instance->value = 0;
    instance->callback = NULL;
    instance->suffix = NULL;
    instance->active_image = NULL;

    LevelImageList_init(instance->level_images);

    slider_view_update(instance);
}

static void slider_view_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    SliderView* instance = (SliderView*)obj;
    free(instance->suffix);
    LevelImageList_clear(instance->level_images);
}

/* Implementation */

static void slider_view_update(SliderView* instance) {
    furi_check(instance);

    lv_bar_set_value(instance->bar, instance->value, LV_ANIM_OFF);

    if(instance->suffix) {
        lv_label_set_text_fmt(instance->value_label, "%ld%s", instance->value, instance->suffix);
    } else {
        lv_label_set_text_fmt(instance->value_label, "%ld", instance->value);
    }

    lv_obj_set_state(
        instance->left_arrow_label, LV_STATE_DISABLED, instance->value <= instance->min_value);
    lv_obj_set_state(
        instance->right_arrow_label, LV_STATE_DISABLED, instance->value >= instance->max_value);

    lv_obj_t* image = NULL;
    LevelImageList_it_t it;
    LevelImageList_it(it, instance->level_images);
    for(; !LevelImageList_end_p(it); LevelImageList_next(it)) {
        const LevelImage* level_image = LevelImageList_cref(it);
        if(instance->value >= level_image->level) {
            image = level_image->image;
            break;
        }
    }

    if(instance->active_image != image) {
        if(instance->active_image) {
            lv_obj_add_flag(instance->active_image, LV_OBJ_FLAG_HIDDEN);
        }

        if(image) {
            lv_obj_remove_flag(image, LV_OBJ_FLAG_HIDDEN);
        }

        instance->active_image = image;
    }
}

/* Public API */

SliderView* slider_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    SliderView* instance = (SliderView*)obj;
    widget_set_input_feed_callback(&instance->base, slider_view_input_callback);

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

    lv_obj_t* image = lv_obj_class_create_obj(MY_IMAGE_CLASS, TO_LV_OBJ(instance));
    lv_obj_class_init_obj(image);
    lv_image_set_src(image, file_path);
    lv_obj_add_flag(image, LV_OBJ_FLAG_HIDDEN);

    LevelImageList_push_back(
        instance->level_images,
        (LevelImage){
            .level = level,
            .image = image,
        });

    slider_view_update(instance);
}

void slider_view_set_suffix(SliderView* instance, const char* suffix) {
    furi_check(instance);

    free(instance->suffix);

    instance->suffix = (suffix) ? strdup(suffix) : NULL;
    slider_view_update(instance);
}

void slider_view_set_bar_gradient(SliderView* instance, Color start, Color end) {
    lv_obj_set_style_bg_color(instance->bar, TO_LV_COLOR(start), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(instance->bar, TO_LV_COLOR(end), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_dir(instance->bar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
}

void slider_view_set_range(SliderView* instance, int32_t min, int32_t max) {
    furi_check(instance);
    furi_check(min <= max);

    lv_bar_set_range(instance->bar, min, max);

    instance->min_value = min;
    instance->max_value = max;

    if(instance->value < min) {
        instance->value = min;
    } else if(instance->value > max) {
        instance->value = max;
    }

    slider_view_update(instance);
}

void slider_view_set_step(SliderView* instance, int32_t step) {
    furi_check(instance);
    furi_check(step > 0);

    instance->step = step;
}

void slider_view_set_value(SliderView* instance, int32_t value) {
    furi_check(instance);

    instance->value = (value < instance->min_value) ? instance->min_value :
                      (value > instance->max_value) ? instance->max_value :
                                                      value;

    slider_view_update(instance);
}

void slider_view_set_callback(SliderView* instance, SliderViewCallback callback, void* context) {
    furi_check(instance);

    instance->callback = callback;
    instance->context = context;
}

/* LVGL class descriptors */

const lv_obj_class_t slider_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = slider_view_lvgl_constructor,
    .destructor_cb = slider_view_lvgl_destructor,
    .name = "widget-slider-view",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(SliderView),
};

const lv_obj_class_t slider_view_bar_lvgl_class = {
    .base_class = &lv_bar_class,
    .name = "slider-view-bar",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
};

const lv_obj_class_t slider_view_text_container_lvgl_class = {
    .base_class = &lv_obj_class,
    .name = "slider-view-text-container",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t slider_view_arrow_label_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "slider-view-arrow-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t slider_view_image_lvgl_class = {
    .base_class = &lv_image_class,
    .name = "slider-view-image",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
