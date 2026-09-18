#include "anim_player_i.h"

#include <furi/furi.h>
#include <string.h>
#include <storage/storage.h>
#include <assets_images.h>

#define TAG      "AnimPlayer"
#define MY_CLASS (&anim_player_lvgl_class)

static_assert(
    (size_t)AnimPlayerOptionIntermediateInternalBuffer ==
    (size_t)AnimFileOptionIntermediateInternalBuffer);
static_assert((size_t)AnimPlayerOptionMAX == (size_t)AnimFileOptionMAX);

// ==================
// Internal functions
// ==================

static void anim_player_timer_cb(lv_timer_t* timer) {
    AnimPlayer* instance = lv_timer_get_user_data(timer);
    furi_check(instance);
    furi_check(instance->file);

    AnimFileFrameInfo info = anim_file_frame(instance->file);
    lv_obj_invalidate(instance->canvas);

    if(info.flags & AnimFileFrameFlagFinished) lv_timer_pause(instance->timer);

    if(instance->frame_cb) instance->frame_cb(instance, &info, instance->frame_cb_context);
}

static void anim_player_resize_canvas(AnimPlayer* player, size_t width, size_t height) {
    furi_assert(player);

    if(!player->file) return;

    AnimFileInfo file_info = anim_file_info(player->file);
    width = CLAMP(width, file_info.width, 1ULL);
    height = CLAMP(height, file_info.height, 1ULL);

    size_t buffer_size = width * height * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    player->canvas_buf = realloc(player->canvas_buf, buffer_size);
    player->canvas_w = width;
    player->canvas_h = height;
    anim_file_set_out_buf(player->file, width, height, player->canvas_buf);

    lv_canvas_set_buffer(
        player->canvas, player->canvas_buf, width, height, LV_COLOR_FORMAT_ARGB8888);
}

// ==========
// LVGL class
// ==========

static void anim_player_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimPlayer* instance = (AnimPlayer*)obj;
    instance->canvas = lv_canvas_create(obj);
    instance->storage = furi_record_open(RECORD_STORAGE);

    lv_obj_set_size(instance->canvas, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->timer = lv_timer_create(anim_player_timer_cb, UINT32_MAX, obj);
    lv_timer_set_user_data(instance->timer, instance);
    lv_timer_set_repeat_count(instance->timer, -1);
    lv_timer_pause(instance->timer);
}

static void anim_player_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimPlayer* instance = (AnimPlayer*)obj;
    lv_timer_delete(instance->timer);

    if(instance->file) anim_file_free(instance->file);
    if(instance->file_path) free(instance->file_path);
    if(instance->canvas_buf) free(instance->canvas_buf);

    instance->file = NULL;
    instance->file_path = NULL;
    instance->canvas_buf = NULL;

    furi_record_close(RECORD_STORAGE);
}

static void anim_player_lvgl_event(lv_event_t* event) {
    furi_assert(event);

    lv_event_code_t code = lv_event_get_code(event);
    AnimPlayer* player = (AnimPlayer*)lv_event_get_target_obj(event);
    lv_obj_t* lv_base = TO_LV_OBJ(&player->base);

    if(code == LV_EVENT_SIZE_CHANGED) {
        int32_t width = lv_obj_get_width(lv_base);
        int32_t height = lv_obj_get_height(lv_base);

        if((width > 0) && (height > 0)) anim_player_resize_canvas(player, width, height);
    }
}

const lv_obj_class_t anim_player_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = anim_player_lvgl_constructor,
    .destructor_cb = anim_player_lvgl_destructor,
    .name = "widget-anim-play",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(AnimPlayer),
};

// ==========
// Public API
// ==========

AnimPlayer* anim_player_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    lv_obj_add_event_cb(obj, anim_player_lvgl_event, LV_EVENT_SIZE_CHANGED, NULL);

    AnimPlayer* instance = (AnimPlayer*)obj;
    return instance;
}

void anim_player_free(AnimPlayer* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* anim_player_get_base(AnimPlayer* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

bool anim_player_set_source_ex(
    AnimPlayer* instance,
    const char* file_path,
    AnimPlayerOption options) {
    furi_check(instance);
    furi_check(options < AnimPlayerOptionMAX);

    bool path_given = !!file_path;
    bool has_previous_path = !!instance->file_path;

    do {
        if(has_previous_path) {
            if(path_given && strcmp(instance->file_path, file_path) == 0) break;
            free(instance->file_path);
            instance->file_path = NULL;
        }

        anim_player_pause(instance);

        if(instance->file) anim_file_free(instance->file);
        instance->file = NULL;

        if(!path_given) break;
        instance->file_path = strdup(file_path);
        // direct reinterpretation of enum types is safe thanks to `static_assert`s in the beginning of the file
        instance->file = anim_file_alloc(instance->storage, file_path, (AnimFileOption)options);
        if(!instance->file) break;

        AnimFileInfo info = anim_file_info(instance->file);
        anim_player_resize_canvas(instance, info.width, info.height);

        anim_player_set_section(instance, AnimFilePlayFlagLoop, ANIM_FILE_DEFAULT_SECTION);

        size_t period = 1000 / info.fps;
        lv_timer_set_period(instance->timer, period);
        anim_player_start(instance);
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

bool anim_player_set_source(AnimPlayer* instance, const char* file_path) {
    return anim_player_set_source_ex(instance, file_path, AnimPlayerOptionNone);
}

bool anim_player_set_offset(AnimPlayer* instance, float x, float y) {
    furi_check(instance);
    if(!instance->file) return false;
    anim_file_set_offset(instance->file, x, y);
    return true;
}

AnimFile* anim_player_get_file(AnimPlayer* instance) {
    furi_check(instance);
    return instance->file;
}

bool anim_player_set_section(AnimPlayer* instance, AnimFilePlayFlag flags, const char* name) {
    furi_check(instance);
    if(!instance->file) return false;
    return anim_file_set_section(instance->file, flags, name);
}

bool anim_player_start(AnimPlayer* instance) {
    furi_check(instance);
    if(!instance->file) return false;
    lv_timer_resume(instance->timer);
    lv_timer_ready(instance->timer);
    return true;
}

bool anim_player_pause(AnimPlayer* instance) {
    furi_check(instance);
    if(!instance->file) return false;
    lv_timer_pause(instance->timer);
    return true;
}

void anim_player_set_frame_callback(
    AnimPlayer* instance,
    AnimPlayerFrameCallback callback,
    void* context) {
    furi_check(instance);
    if(!callback) furi_check(!context);
    instance->frame_cb = callback;
    instance->frame_cb_context = context;
}
