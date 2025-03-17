#include "anim_image.h"

#include <furi/furi.h>

#include <gui/widget_i.h>
#include <storage/storage.h>

#define TAG "AnimImage"

#define IMAGE_ANIMATION_FILE_MAGIC     (0x69)
#define IMAGE_ANIMATION_FORMAT_VERSION (0x00)

#define ANIM_IMAGE_MAX_FPS (100)

#define MY_CLASS (&anim_image_lvgl_class)

typedef struct {
    uint32_t magic;
    uint32_t format_version;
    uint32_t fps;
    uint32_t bytes_per_pixel;
    uint32_t width;
    uint32_t height;
    uint32_t frames;
} FURI_PACKED AnimImageFileHeader;

static_assert(
    sizeof(AnimImageFileHeader) == 7 * sizeof(uint32_t),
    "Incorrect size of AnimImageFileHeader");

struct AnimImage {
    Widget base;
    lv_obj_t* canvas;
    lv_timer_t* timer;
    uint8_t* canvas_buf;
    File* file;

    AnimImageFileHeader header;

    uint32_t frame_size;
    uint32_t frame_idx;
    uint32_t frame_total;
};

const lv_obj_class_t anim_image_lvgl_class;

// Function prototypes

static void anim_image_timer_callback(lv_timer_t* timer);

// LVGL-specific code

void anim_image_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimImage* instance = (AnimImage*)obj;
    instance->canvas = lv_canvas_create(obj);
    instance->timer = lv_timer_create(anim_image_timer_callback, UINT32_MAX, obj);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    instance->file = storage_file_alloc(storage);

    lv_timer_set_repeat_count(instance->timer, -1);
    lv_timer_pause(instance->timer);
}

void anim_image_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AnimImage* instance = (AnimImage*)obj;
    lv_timer_delete(instance->timer);

    if(instance->canvas_buf) {
        free(instance->canvas_buf);
    }

    storage_file_free(instance->file);
    furi_record_close(RECORD_STORAGE);
}

// Implementation

static bool anim_image_update_canvas_buffer(AnimImage* instance) {
    bool success = false;

    do {
        if(instance->frame_idx == 0) {
            if(!storage_file_seek(instance->file, sizeof(AnimImageFileHeader), true)) {
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

static void anim_image_timer_callback(lv_timer_t* timer) {
    AnimImage* instance = lv_timer_get_user_data(timer);

    if(!anim_image_update_canvas_buffer(instance)) {
        FURI_LOG_E(TAG, "Failed to read frame from file");
        return;
    }

    lv_obj_invalidate(instance->canvas);
}

static bool anim_image_validate_header(const AnimImageFileHeader* header, size_t file_size) {
    bool validated = false;

    do {
        if(header->magic != IMAGE_ANIMATION_FILE_MAGIC) {
            FURI_LOG_E(TAG, "Invalid magic num: %ld", header->magic);
            break;
        }

        if(header->format_version != IMAGE_ANIMATION_FORMAT_VERSION) {
            FURI_LOG_E(TAG, "Invalid  format version: %ld", header->format_version);
            break;
        }

        if(header->fps > ANIM_IMAGE_MAX_FPS) {
            FURI_LOG_E(
                TAG, "Unsupported fps: %ld. Must be less than %d", header->fps, ANIM_IMAGE_MAX_FPS);
            break;
        }

        if(header->bytes_per_pixel != 3 && header->bytes_per_pixel != 1) {
            FURI_LOG_E(
                TAG,
                "Unsupported bytes per pixel: %lu. Must be 3 (RGB888) or 1 (L8)",
                header->bytes_per_pixel);
            break;
        }

        const size_t frames_data_size =
            header->frames * header->bytes_per_pixel * header->width * header->height;

        if((frames_data_size + sizeof(AnimImageFileHeader)) != file_size) {
            FURI_LOG_E(
                TAG,
                "Frames: %ld, BPP: %ld, Width: %ld, Height: %ld",
                header->frames,
                header->bytes_per_pixel,
                header->width,
                header->height);

            FURI_LOG_E(
                TAG,
                "Incorrect file size: %zu. Expected size: %zu",
                file_size,
                frames_data_size + sizeof(AnimImageFileHeader));

            break;
        }

        validated = true;
    } while(false);

    return validated;
}

// Public API

AnimImage* anim_image_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    AnimImage* instance = (AnimImage*)obj;
    return instance;
}

void anim_image_free(AnimImage* instance) {
    furi_check(instance);
    lv_obj_delete(instance->canvas);
}

bool anim_image_set_source(AnimImage* instance, const char* file_path) {
    furi_check(instance);

    bool success = false;

    AnimImageFileHeader* header = &instance->header;

    do {
        if(storage_file_is_open(instance->file)) {
            storage_file_close(instance->file);
        }

        if(!storage_file_open(instance->file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file %s", file_path);
            break;
        }

        size_t bytes_read =
            storage_file_read(instance->file, &instance->header, sizeof(AnimImageFileHeader));

        if(bytes_read != sizeof(AnimImageFileHeader)) {
            FURI_LOG_E(TAG, "Failed to read file header");
            break;
        }

        if(!anim_image_validate_header(&instance->header, storage_file_size(instance->file))) {
            FURI_LOG_E(TAG, "Corrupt or invalid file");
            break;
        }

        success = true;

    } while(false);

    if(success) {
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

        anim_image_update_canvas_buffer(instance);
    }

    return success;
}

void anim_image_start(AnimImage* instance) {
    furi_check(instance);

    if(instance->timer) {
        lv_timer_resume(instance->timer);
    }
}

void anim_image_stop(AnimImage* instance) {
    furi_check(instance);

    if(instance->timer) {
        lv_timer_pause(instance->timer);
    }
}

// LVGL class descriptor

const lv_obj_class_t anim_image_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = anim_image_lvgl_constructor,
    .destructor_cb = anim_image_lvgl_destructor,
    .name = "widget-anim-image",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(AnimImage),
};
