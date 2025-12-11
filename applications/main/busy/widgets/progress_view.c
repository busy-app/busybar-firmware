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

#define COLOR_GREY_1 0x656C6F
#define COLOR_GREY_2 0x9FAAAF

#define CYCLES_SHOWN_COUNT_MAX (7UL)

#define ELEMENT_SIZES_COUNT  (CYCLES_SHOWN_COUNT_MAX)
#define ELEMENT_STATES_COUNT (2UL)

#define GRADIENT_STOPS_COUNT (2UL)

#define GROW_ANIM_DURATION_MS    (333)
#define GROW_ANIM_DELAY_MS       (1000)

#define BLINK_ANIM_DELAY_MS      (1000)
#define BLINK_ANIM_DURATION_1_MS (500)
#define BLINK_ANIM_DURATION_2_MS (1500)

#define SCROLL_ANIM_DELAY_MS (600)

#define ELEMENT_GAP_PX (1)

struct ProgressView {
    Widget base;
};

const lv_obj_class_t progress_view_lvgl_class;

static const uint8_t element_width[ELEMENT_SIZES_COUNT][ELEMENT_STATES_COUNT];
static const uint32_t element_color[ELEMENT_STATES_COUNT][GRADIENT_STOPS_COUNT];
static const uint32_t element_color_hl[ELEMENT_STATES_COUNT][GRADIENT_STOPS_COUNT];

// LVGL-specific code
static void progress_view_lvgl_scroll_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) {
            lv_anim_set_delay(anim, SCROLL_ANIM_DELAY_MS);
        }
    }
}

void progress_view_lvgl_grow_anim_exec_callback(void* context, int32_t value) {
    furi_assert(context);
    lv_obj_t* shutter = context;

    lv_obj_set_width(shutter, lv_pct(value));
}

static void progress_view_lvgl_update_blink_anim_params(lv_anim_t* anim) {
    if(anim->reverse_play_in_progress) {
        lv_anim_set_bezier3_param(
            anim,
            LV_BEZIER_VAL_FLOAT(0.7F),
            LV_BEZIER_VAL_FLOAT(0.0F),
            LV_BEZIER_VAL_FLOAT(0.2F),
            LV_BEZIER_VAL_FLOAT(1.0F));
    } else {
        lv_anim_set_bezier3_param(
            anim,
            LV_BEZIER_VAL_FLOAT(0.2F),
            LV_BEZIER_VAL_FLOAT(0.0F),
            LV_BEZIER_VAL_FLOAT(0.7F),
            LV_BEZIER_VAL_FLOAT(1.0F));
    }
}

void progress_view_lvgl_blink_anim_exec_callback(lv_anim_t* anim, int32_t value) {
    furi_assert(anim);
    lv_obj_t* element = anim->var;

    progress_view_lvgl_update_blink_anim_params(anim);

    const uint32_t* const colors = lv_obj_get_user_data(element);

    const lv_color_t c1 = lv_color_mix(lv_color_hex(colors[0]), lv_color_hex(COLOR_GREY_1), value);
    const lv_color_t c2 = lv_color_mix(lv_color_hex(colors[1]), lv_color_hex(COLOR_GREY_1), value);

    lv_obj_set_style_bg_color(element, c1, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(element, c2, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(element, LV_GRAD_DIR_VER, LV_PART_MAIN);
}

static void progress_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(obj, ELEMENT_GAP_PX, LV_PART_MAIN);
    lv_obj_add_event_cb(
        obj, progress_view_lvgl_scroll_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    ProgressView* instance = (ProgressView*)obj;
    UNUSED(instance);
}

// Implementation

static void progress_view_start_grow_animation(lv_obj_t* element) {
    lv_obj_t* shutter = lv_obj_create(element);

    lv_obj_set_height(shutter, LV_PCT(100));
    lv_obj_set_style_bg_opa(shutter, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(shutter, lv_color_hex(COLOR_GREY_2), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(shutter, lv_color_hex(COLOR_GREY_1), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(shutter, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_align(shutter, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN);

    lv_anim_t anim;
    lv_anim_init(&anim);
    // Animating shutter width from 100 to 0 percent
    lv_anim_set_values(&anim, 100, 0);
    lv_anim_set_delay(&anim, GROW_ANIM_DELAY_MS);
    lv_anim_set_duration(&anim, GROW_ANIM_DURATION_MS);
    lv_anim_set_exec_cb(&anim, progress_view_lvgl_grow_anim_exec_callback);
    lv_anim_set_var(&anim, shutter);
    lv_anim_start(&anim);
}
static void progress_view_start_blink_animation(lv_obj_t* element, const uint32_t* const colors) {
    lv_obj_set_user_data(element, (void*)colors);

    lv_anim_t anim;
    lv_anim_init(&anim);

    progress_view_lvgl_update_blink_anim_params(&anim);

    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_delay(&anim, BLINK_ANIM_DELAY_MS);
    lv_anim_set_duration(&anim, BLINK_ANIM_DURATION_1_MS);
    lv_anim_set_reverse_duration(&anim, BLINK_ANIM_DURATION_2_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
    lv_anim_set_custom_exec_cb(&anim, progress_view_lvgl_blink_anim_exec_callback);
    lv_anim_set_var(&anim, element);
    lv_anim_start(&anim);
}

static void progress_view_scroll_to_idx(
    ProgressView* instance,
    uint32_t interval_idx,
    bool enable_animation) {
    if(interval_idx >= CYCLES_SHOWN_COUNT_MAX - 1) {
        const lv_obj_t* obj = TO_LV_OBJ(instance);

        const uint32_t max_idx = lv_obj_get_child_count(obj) - 1;
        const uint32_t target_idx = MIN(interval_idx + CYCLES_SHOWN_COUNT_MAX, max_idx);

        lv_obj_t* target = lv_obj_get_child(obj, target_idx);
        lv_obj_scroll_to_view(target, enable_animation ? LV_ANIM_ON : LV_ANIM_OFF);
    }
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
    furi_check(cycles_count > 0);

    const uint32_t interval_count = cycles_count * 2 - 1;
    const uint32_t width_idx = MIN(cycles_count, ELEMENT_SIZES_COUNT) - 1;

    for(uint32_t i = 0; i < interval_count; ++i) {
        const uint32_t state_idx = i % ELEMENT_STATES_COUNT;
        const uint32_t w = element_width[width_idx][state_idx];

        lv_obj_t* element = lv_obj_create(TO_LV_OBJ(instance));
        lv_obj_set_size(element, w, 3);
        lv_obj_set_style_radius(element, 1, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(element, LV_OPA_COVER, LV_PART_MAIN);

        if(i <= interval_idx) {
            const uint32_t* const colors = element_color[state_idx];

            lv_color_t c1 = lv_color_hex(colors[0]);
            lv_color_t c2 = lv_color_hex(colors[1]);

            if(wait_next) {
                c1 = lv_color_darken(c1, LV_OPA_50);
                c2 = lv_color_darken(c2, LV_OPA_50);
            }

            lv_obj_set_style_bg_color(element, c1, LV_PART_MAIN);
            lv_obj_set_style_bg_grad_color(element, c2, LV_PART_MAIN);
            lv_obj_set_style_bg_grad_dir(element, LV_GRAD_DIR_VER, LV_PART_MAIN);

            if(!wait_next && i == interval_idx) {
                progress_view_start_grow_animation(element);
            }

        } else if(wait_next && i == interval_idx + 1) {
            const uint32_t* const colors = element_color_hl[state_idx];
            progress_view_start_blink_animation(element, colors);

        } else {
            lv_obj_set_style_bg_color(element, lv_color_hex(COLOR_GREY_1), LV_PART_MAIN);
        }
    }

    progress_view_scroll_to_idx(instance, interval_idx, !wait_next);
}

// LVGL class descriptor

const lv_obj_class_t progress_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = progress_view_lvgl_constructor,
    .name = "widget-progress-view",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(ProgressView),
};

// Element parameters

static const uint8_t element_width[ELEMENT_SIZES_COUNT][ELEMENT_STATES_COUNT] = {
    {72, 0},
    {31, 8},
    {18, 7},
    {12, 6},
    {8, 6},
    {7, 4},
    {6, 3},
};

static const uint32_t element_color[ELEMENT_STATES_COUNT][GRADIENT_STOPS_COUNT] = {
    {COLOR_RED_1, COLOR_RED_2},
    {COLOR_GREEN_1, COLOR_GREEN_2},
};

static const uint32_t element_color_hl[ELEMENT_STATES_COUNT][GRADIENT_STOPS_COUNT] = {
    {COLOR_RED_3, COLOR_RED_4},
    {COLOR_GREEN_3, COLOR_GREEN_4},
};
