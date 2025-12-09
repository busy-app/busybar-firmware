#include "progress_view.h"

#include <gui/widget_i.h>

#include "../storage_macros.h"

#define MY_CLASS (&progress_view_lvgl_class)

#define COLOR_RED_1 0xFF0002
#define COLOR_RED_2 0x9B0000
#define COLOR_RED_3 0xFE567A
#define COLOR_RED_4 0xE80919

#define COLOR_GREEN_1 0x00E96B
#define COLOR_GREEN_2 0x00833D
#define COLOR_GREEN_3 0x00FB91
#define COLOR_GREEN_4 0x00FB91

#define COLOR_GREY 0x656C6F

#define CYCLES_SHOWN_MIN (2UL)
#define CYCLES_SHOWN_MAX (10UL)

#define NUM_ELEMENT_SIZES  (CYCLES_SHOWN_MAX - CYCLES_SHOWN_MIN + 1UL)
#define NUM_ELEMENT_STATES (2UL)

#define NUM_GRADIENT_STOPS (2UL)

struct ProgressView {
    Widget base;
};

const lv_obj_class_t progress_view_lvgl_class;

static const uint8_t element_width[NUM_ELEMENT_SIZES][NUM_ELEMENT_STATES];
static const uint32_t element_color[NUM_ELEMENT_STATES][NUM_GRADIENT_STOPS];
static const uint32_t element_color_hl[NUM_ELEMENT_STATES][NUM_GRADIENT_STOPS];

// LVGL-specific code

// void progress_view_lvgl_grow_anim_exec_callback(lv_anim_t* anim, int32_t value) {
//     furi_assert(anim);
//     ProgressView* instance = anim->var;
//
//     const int32_t done_bar_width = MAX(value - 1, 0);
//     lv_obj_set_width(instance->done_bar, done_bar_width);
//
//     const int32_t end_value = anim->end_value;
//
//     if(end_value < lv_obj_get_width(lv_obj_get_parent(instance->done_bar))) {
//         const int32_t next_bar_width = 2 * end_value - (done_bar_width + anim->start_value);
//         lv_obj_set_width(instance->next_bar, next_bar_width);
//     }
// }

void progress_view_lvgl_blink_anim_exec_callback(void* context, int32_t value) {
    furi_assert(context);

    lv_obj_t* block = context;

    const uint32_t* const colors = lv_obj_get_user_data(block);

    const lv_color_t c1 = lv_color_mix(lv_color_hex(colors[0]), lv_color_hex(COLOR_GREY), value);
    const lv_color_t c2 = lv_color_mix(lv_color_hex(colors[1]), lv_color_hex(COLOR_GREY), value);

    lv_obj_set_style_bg_color(block, c1, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(block, c2, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(block, LV_GRAD_DIR_VER, LV_PART_MAIN);
}

static void progress_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(obj, 1, LV_PART_MAIN);

    ProgressView* instance = (ProgressView*)obj;
    UNUSED(instance);
}

// Implementation

// static void
//     progress_view_start_grow_animation(ProgressView* instance, int32_t done, int32_t total) {
//     const int32_t total_width = lv_obj_get_width(lv_obj_get_parent(instance->done_bar));
//     const int32_t sector_width = total_width / total;
//     const int32_t done_width = done * total_width / total;
//     const int32_t prev_done_width = done_width - sector_width;
//
//     lv_anim_t anim;
//     lv_anim_init(&anim);
//
//     lv_anim_set_values(&anim, prev_done_width, done_width);
//     lv_anim_set_duration(&anim, 500);
//     lv_anim_set_delay(&anim, 500);
//     if(done < total) {
//         lv_anim_set_bezier3_param(
//             &anim,
//             LV_BEZIER_VAL_FLOAT(0.37F),
//             LV_BEZIER_VAL_FLOAT(0.0F),
//             LV_BEZIER_VAL_FLOAT(0.3F),
//             LV_BEZIER_VAL_FLOAT(1.78F));
//     } else {
//         lv_anim_set_bezier3_param(
//             &anim,
//             LV_BEZIER_VAL_FLOAT(0.37F),
//             LV_BEZIER_VAL_FLOAT(0.0F),
//             LV_BEZIER_VAL_FLOAT(0.3F),
//             LV_BEZIER_VAL_FLOAT(1.4F));
//     }
//     lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
//     lv_anim_set_custom_exec_cb(&anim, progress_view_lvgl_grow_anim_exec_callback);
//     lv_anim_set_var(&anim, instance);
//
//     lv_anim_start(&anim);
// }

static void progress_view_start_blink_animation(lv_obj_t* block, const uint32_t* const colors) {
    lv_obj_set_user_data(block, (void*)colors);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim, 500);
    lv_anim_set_reverse_duration(&anim, 1500);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_exec_cb(&anim, progress_view_lvgl_blink_anim_exec_callback);
    lv_anim_set_var(&anim, block);
    lv_anim_start(&anim);
}

// Public API

ProgressView* progress_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    ProgressView* instance = (ProgressView*)obj;
    return instance;
}

void progress_view_free(ProgressView* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* progress_view_get_base(ProgressView* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void progress_view_set_progress(
    ProgressView* instance,
    uint32_t interval_idx,
    uint32_t cycles_count,
    bool wait_next) {
    furi_check(instance);
    furi_check(cycles_count >= CYCLES_SHOWN_MIN);

    const uint32_t interval_count = cycles_count * 2 - 1;
    const uint32_t width_idx = MIN(cycles_count - CYCLES_SHOWN_MIN, NUM_ELEMENT_SIZES - 1);

    for(uint32_t i = 0; i < interval_count; ++i) {
        const uint32_t state_idx = i % NUM_ELEMENT_STATES;
        const uint32_t w = element_width[width_idx][state_idx];

        lv_obj_t* block = lv_obj_create(TO_LV_OBJ(instance));
        lv_obj_set_size(block, w, 3);
        lv_obj_set_style_radius(block, 1, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(block, LV_OPA_COVER, LV_PART_MAIN);

        if(i <= interval_idx) {
            const uint32_t* const colors = element_color[state_idx];

            lv_color_t c1 = lv_color_hex(colors[0]);
            lv_color_t c2 = lv_color_hex(colors[1]);

            if(wait_next) {
                c1 = lv_color_darken(c1, LV_OPA_50);
                c2 = lv_color_darken(c1, LV_OPA_50);
            }

            lv_obj_set_style_bg_color(block, c1, LV_PART_MAIN);
            lv_obj_set_style_bg_grad_color(block, c2, LV_PART_MAIN);
            lv_obj_set_style_bg_grad_dir(block, LV_GRAD_DIR_VER, LV_PART_MAIN);

        } else if(wait_next && i == interval_idx + 1) {
            const uint32_t* const colors = element_color_hl[state_idx];
            progress_view_start_blink_animation(block, colors);

        } else {
            lv_obj_set_style_bg_color(block, lv_color_hex(COLOR_GREY), LV_PART_MAIN);
        }
    }
}

// LVGL class descriptor

const lv_obj_class_t progress_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = progress_view_lvgl_constructor,
    .name = "widget-progress-view",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(ProgressView),
};

// Blah

static const uint8_t element_width[NUM_ELEMENT_SIZES][NUM_ELEMENT_STATES] = {
    {31, 8},
    {18, 7},
    {12, 6},
    {8, 6},
    {7, 4},
    {6, 3},
    {5, 2},
    {4, 2},
    {3, 2},
};

static const uint32_t element_color[NUM_ELEMENT_STATES][NUM_GRADIENT_STOPS] = {
    {COLOR_RED_1, COLOR_RED_2},
    {COLOR_GREEN_1, COLOR_GREEN_2},
};

static const uint32_t element_color_hl[NUM_ELEMENT_STATES][NUM_GRADIENT_STOPS] = {
    {COLOR_RED_3, COLOR_RED_4},
    {COLOR_GREEN_3, COLOR_GREEN_4},
};
