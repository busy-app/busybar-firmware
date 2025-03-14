#include "image_animation.h"

#include <furi/furi.h>

#include <gui/widget_i.h>
#include <storage/storage.h>

#define TAG "ImageAnimation"

#define IMAGE_ANIMATION_FILE_MAGIC     (0x69)
#define IMAGE_ANIMATION_FORMAT_VERSION (0x00)

#define MY_CLASS (&anim_image_lvgl_class)

typedef struct {
    uint32_t magic;
    uint32_t format_version;
    uint32_t fps;
    uint32_t bytes_per_pixel;
    uint32_t width;
    uint32_t height;
    uint32_t frames;
} FURI_PACKED ImageAnimationFileHeader;

static_assert(
    sizeof(ImageAnimationFileHeader) == 7 * sizeof(uint32_t),
    "Incorrect size of ImageAnimationFileHeader");

struct ImageAnimation {
    Widget base;
    lv_obj_t* canvas;
    lv_timer_t* timer;
    uint8_t* canvas_buf;

    Storage* storage;
    File* file;

    ImageAnimationFileHeader header;

    uint32_t frame_size;
    uint32_t frame_idx;
    uint32_t frame_total;
};

const lv_obj_class_t anim_image_lvgl_class;

// Function prototypes

static void image_animation_timer_callback(lv_timer_t* timer);

// LVGL-specific code

void anim_image_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ImageAnimation* instance = (ImageAnimation*)obj;
    instance->canvas = lv_canvas_create(obj);
    instance->timer = lv_timer_create(image_animation_timer_callback, UINT32_MAX, obj);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->file = storage_file_alloc(instance->storage);

    lv_timer_set_repeat_count(instance->timer, -1);
    lv_timer_pause(instance->timer);
}

void anim_image_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ImageAnimation* instance = (ImageAnimation*)obj;
    lv_timer_delete(instance->timer);

    if(instance->canvas_buf) {
        free(instance->canvas_buf);
    }

    storage_file_free(instance->file);
    furi_record_close(RECORD_STORAGE);
}

// Implementation

static bool image_animation_update_canvas_buffer(ImageAnimation* instance) {
    bool success = false;

    do {
        if(instance->frame_idx == 0) {
            if(!storage_file_seek(instance->file, sizeof(ImageAnimationFileHeader), true)) {
                FURI_LOG_E(TAG, "Failed to seek animation file");
                break;
            }
        }

        size_t bytes_read =
            storage_file_read(instance->file, instance->canvas_buf, instance->frame_size);
        instance->frame_idx = (instance->frame_idx + 1) % instance->frame_total;

        if(bytes_read != instance->frame_size) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static void image_animation_timer_callback(lv_timer_t* timer) {
    ImageAnimation* instance = lv_timer_get_user_data(timer);

    if(!image_animation_update_canvas_buffer(instance)) {
        FURI_LOG_E(TAG, "Failed to read frame from file");
        return;
    }

    lv_obj_invalidate(instance->canvas);
}

// Public API

ImageAnimation* image_animation_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    ImageAnimation* instance = (ImageAnimation*)obj;
    return instance;
}

void image_animation_free(ImageAnimation* instance) {
    furi_check(instance);
    lv_obj_delete(instance->canvas);
}

bool image_animation_set_source(ImageAnimation* instance, const char* file_path) {
    furi_check(instance);

    ImageAnimationFileHeader* header = &instance->header;

    bool parsed = false;

    do {
        FileInfo file_info = {};
        if(storage_common_stat(instance->storage, file_path, &file_info) != FSE_OK) {
            FURI_LOG_D(TAG, "File not exist: %s", file_path);
            break;
        }
        if(file_info.size < sizeof(ImageAnimationFileHeader)) {
            FURI_LOG_D(TAG, "Incorrect file size: %lld", file_info.size);
            break;
        }

        if(!storage_file_open(instance->file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            storage_file_close(instance->file);
            FURI_LOG_D(TAG, "Failed to open file %s", file_path);
            break;
        }

        size_t bytes_read =
            storage_file_read(instance->file, &instance->header, sizeof(ImageAnimationFileHeader));
        if(bytes_read != sizeof(ImageAnimationFileHeader)) {
            storage_file_close(instance->file);
            FURI_LOG_D(TAG, "Failed to read file header");
            break;
        }

        if(header->magic != IMAGE_ANIMATION_FILE_MAGIC) {
            FURI_LOG_D(TAG, "Invalid magic num: %ld", header->magic);
            break;
        }

        if(header->format_version != IMAGE_ANIMATION_FORMAT_VERSION) {
            FURI_LOG_D(TAG, "Invalid  format version: %ld", header->format_version);
            break;
        }

        if(header->fps > 100) {
            FURI_LOG_D(TAG, "Unsupported fps: %ld. Must be less than 100", header->fps);
            break;
        }

        if(header->bytes_per_pixel != 3 && header->bytes_per_pixel != 1) {
            FURI_LOG_E(
                TAG,
                "Unsupported bytes per pixel: %lu. Must be 3 (RGB888) or 1 (L8)",
                header->bytes_per_pixel);
            break;
        }

        uint32_t frames_data_size =
            header->frames * header->bytes_per_pixel * header->width * header->height;
        if((frames_data_size + sizeof(ImageAnimationFileHeader)) != file_info.size) {
            FURI_LOG_D(
                TAG,
                "Frames: %ld, BPP: %ld, Width: %ld, Height: %ld",
                header->frames,
                header->bytes_per_pixel,
                header->width,
                header->height);
            FURI_LOG_D(
                TAG,
                "Incorrect file size: %lld. Expected size: %ld",
                file_info.size,
                frames_data_size + sizeof(ImageAnimationFileHeader));
            break;
        }

        parsed = true;

    } while(false);

    if(parsed) {
        instance->frame_size = header->height * header->width * header->bytes_per_pixel;
        instance->frame_total = header->frames;
        instance->frame_idx = 0;

        instance->canvas_buf = realloc(instance->canvas_buf, instance->frame_size);

        lv_canvas_set_buffer(
            instance->canvas,
            instance->canvas_buf,
            header->width,
            header->height,
            header->bytes_per_pixel == 3 ? LV_COLOR_FORMAT_RGB888 : LV_COLOR_FORMAT_L8);

        lv_timer_set_period(instance->timer, 1000 / header->fps);
        lv_timer_resume(instance->timer);

        image_animation_update_canvas_buffer(instance);
    }

    return parsed;
}

void image_animation_start(ImageAnimation* instance) {
    furi_check(instance);

    if(instance->timer) {
        lv_timer_resume(instance->timer);
    }
}

void image_animation_stop(ImageAnimation* instance) {
    furi_check(instance);

    if(instance->timer) {
        lv_timer_pause(instance->timer);
    }
}

// LVGL class descriptor

const lv_obj_class_t anim_image_lvgl_class = {
    .base_class = &lv_widget_class,
    .constructor_cb = anim_image_lvgl_constructor,
    .destructor_cb = anim_image_lvgl_destructor,
    .name = "widget-anim-image",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(ImageAnimation),
};
