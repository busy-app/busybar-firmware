#include "lottie_animation.h"

#include <furi/furi.h>
#include <gui/widget_i.h>

#include <lottie/lottie_service.h>

#define TAG "LottieAnimation"

#define MY_CLASS (&lottie_animation_lvgl_class)

#define COLOR_FORMAT (LV_COLOR_FORMAT_ARGB8888)

#define GUI_MUTEX (LV_GLOBAL_DEFAULT()->lv_general_mutex.furi_mutex)

struct LottieAnimation {
    Widget base;
    lv_obj_t* canvas;
    uint32_t* canvas_buf;
    size_t canvas_buf_size;
    LottieServiceTask* lottie_task;
};

static void lottie_animation_update_callback(const void* canvas_buf, void* context) {
    furi_assert(canvas_buf);
    furi_assert(context);

    LottieAnimation* instance = context;

    // HACK Avoid deadlocks by introducing a finite timeout
    if(furi_mutex_acquire(GUI_MUTEX, LV_DEF_REFR_PERIOD) == FuriStatusOk) {
        memcpy(instance->canvas_buf, canvas_buf, instance->canvas_buf_size);
        lv_obj_invalidate((lv_obj_t*)instance);
        furi_mutex_release(GUI_MUTEX);
    }
}

// LVGL-specific code

const lv_obj_class_t lottie_animation_lvgl_class;

static void lottie_animation_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    LottieAnimation* instance = (LottieAnimation*)obj;
    LottieService* lottie_srv = furi_record_open(RECORD_LOTTIE);

    instance->canvas = lv_canvas_create(obj);
    instance->lottie_task =
        lottie_service_task_alloc(lottie_srv, lottie_animation_update_callback, instance);
}

static void lottie_animation_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    LottieAnimation* instance = (LottieAnimation*)obj;
    lottie_service_task_free(instance->lottie_task);

    if(instance->canvas_buf) {
        free(instance->canvas_buf);
    }
}

// Implementation

static void lottie_animation_init_canvas(
    LottieAnimation* instance,
    const LottieServiceTaskInfo* task_info) {
    instance->canvas_buf = realloc(instance->canvas_buf, task_info->canvas_buf_size);
    instance->canvas_buf_size = task_info->canvas_buf_size;

    lv_canvas_set_buffer(
        instance->canvas,
        instance->canvas_buf,
        task_info->canvas_width,
        task_info->canvas_height,
        COLOR_FORMAT);

    lv_draw_buf_t* draw_buf = lv_canvas_get_draw_buf(instance->canvas);
    lv_draw_buf_set_flag(draw_buf, LV_IMAGE_FLAGS_PREMULTIPLIED);
}

// Public API

LottieAnimation* lottie_animation_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    LottieAnimation* instance = (LottieAnimation*)obj;
    return instance;
}

void lottie_animation_free(LottieAnimation* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* lottie_animation_get_base(LottieAnimation* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

bool lottie_animation_set_source(LottieAnimation* instance, const char* file_path, uint32_t fps) {
    furi_check(instance);
    furi_check(file_path);

    bool success = false;

    do {
        if(!lottie_service_task_set_source(instance->lottie_task, file_path, fps)) break;

        LottieServiceTaskInfo task_info;
        if(!lottie_service_task_get_info(instance->lottie_task, &task_info)) break;

        lottie_animation_init_canvas(instance, &task_info);
        if(!lottie_service_task_start(instance->lottie_task)) break;

        success = true;
    } while(false);

    return success;
}

bool lottie_animation_override_slot(LottieAnimation* instance, const char* slot_str) {
    furi_check(instance);
    furi_check(slot_str);

    return lottie_service_task_override_slot(instance->lottie_task, slot_str);
}

// LVGL class descriptor

const lv_obj_class_t lottie_animation_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = lottie_animation_lvgl_constructor,
    .destructor_cb = lottie_animation_lvgl_destructor,
    .name = "widget-lottie-animation",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(LottieAnimation),
};
