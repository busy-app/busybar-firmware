#include "anim_play_i.h"

#include <furi/furi.h>
#include <string.h>
#include <storage/storage.h>
#include <assets_images.h>

#define TAG             "AnimPlay"
#define MY_CLASS        (&anim_play_lvgl_class)
#define BYTES_PER_PIXEL 3

// ==================
// Internal functions
// ==================

static void anim_play_timer_cb(lv_timer_t* timer) {
    AnimPlay* instance = lv_timer_get_user_data(timer);
    furi_check(instance);
    furi_check(instance->file);

    AnimFileFrameInfo info = anim_file_frame(instance->file);
    lv_obj_invalidate(instance->canvas);

    if(instance->frame_cb) instance->frame_cb(instance, &info, instance->frame_cb_context);
}

// ==========
// LVGL class
// ==========

static void anim_play_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimPlay* instance = (AnimPlay*)obj;
    instance->canvas = lv_canvas_create(obj);
    instance->storage = furi_record_open(RECORD_STORAGE);

    instance->timer = lv_timer_create(anim_play_timer_cb, UINT32_MAX, obj);
    lv_timer_set_user_data(instance->timer, instance);
    lv_timer_set_repeat_count(instance->timer, -1);
    lv_timer_pause(instance->timer);
}

static void anim_play_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimPlay* instance = (AnimPlay*)obj;
    lv_timer_delete(instance->timer);

    if(instance->canvas_buf) free(instance->canvas_buf);
    if(instance->file_path) free(instance->file_path);
    if(instance->file) anim_file_free(instance->file);

    furi_record_close(RECORD_STORAGE);
}

const lv_obj_class_t anim_play_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = anim_play_lvgl_constructor,
    .destructor_cb = anim_play_lvgl_destructor,
    .name = "widget-anim-play",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(AnimPlay),
};

// ==========
// Public API
// ==========

AnimPlay* anim_play_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    AnimPlay* instance = (AnimPlay*)obj;
    return instance;
}

void anim_play_free(AnimPlay* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* anim_play_get_base(AnimPlay* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

bool anim_play_set_source(AnimPlay* instance, const char* file_path) {
    furi_check(instance);

    bool path_given = !!file_path;
    bool has_previous_path = !!instance->file_path;

    do {
        if(has_previous_path) {
            if(path_given && strcmp(instance->file_path, file_path) == 0) break;
            free(instance->file_path);
            instance->file_path = NULL;
        }

        if(instance->file) anim_file_free(instance->file);

        if(!path_given) break;
        instance->file_path = strdup(file_path);
        instance->file = anim_file_alloc(instance->storage, file_path);
        if(!instance->file) break;

        AnimFileInfo info = anim_file_info(instance->file);
        instance->canvas_buf = realloc(instance->canvas_buf, info.out_buffer_size);
        anim_file_set_out_buf(instance->file, instance->canvas_buf);

        lv_canvas_set_buffer(
            instance->canvas,
            instance->canvas_buf,
            info.width,
            info.height,
            LV_COLOR_FORMAT_RGB888);

        size_t period = 1000 / info.fps;
        lv_timer_set_period(instance->timer, period);
        anim_play_start(instance);
    } while(0);

    bool loaded_successfully = !!instance->file;

    if(path_given && !loaded_successfully) {
        const lv_image_header_t* header = &I_load_error_9x9.header;
        void* data = (void*)I_load_error_9x9.data;
        lv_canvas_set_buffer(instance->canvas, data, header->w, header->h, header->cf);
        lv_obj_invalidate(instance->canvas);
    }

    return loaded_successfully;
}

AnimFile* anim_play_get_file(AnimPlay* instance) {
    furi_check(instance);
    return instance->file;
}

void anim_play_loop_whole(AnimPlay* instance) {
    furi_check(instance);
    furi_check(instance->file);
    bool success = anim_file_set_section_indexed(
        instance->file, AnimFilePlayFlagLoop, ANIM_FILE_WHOLE_SECTION_INDEX);
    furi_assert(success);
}

void anim_play_start(AnimPlay* instance) {
    furi_check(instance);
    furi_check(instance->file);
    lv_timer_resume(instance->timer);
}

void anim_play_pause(AnimPlay* instance) {
    furi_check(instance);
    lv_timer_pause(instance->timer);
}

void anim_play_set_frame_callback(
    AnimPlay* instance,
    AnimPlayFrameCallback callback,
    void* context) {
    furi_check(instance);
    if(!callback) furi_check(!context);
    instance->frame_cb = callback;
    instance->frame_cb_context = context;
}
