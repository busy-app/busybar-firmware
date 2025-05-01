#include "anim_image.h"

#include <furi/furi.h>

#include <gui/widget_i.h>
#include <storage/storage.h>

#include <assets/assets_images.h>

#define TAG "AnimImage"

#define ANIM_IMAGE_FILE_MAGIC     (0x69)
#define ANIM_IMAGE_FORMAT_VERSION (0x00)
#define ANIM_IMAGE_MAX_FPS        (100)

#define MY_CLASS (&anim_image_lvgl_class)

typedef struct {
    uint32_t magic;
    uint32_t format_version;
    uint32_t fps;
    uint32_t bytes_per_pixel;
    uint32_t width;
    uint32_t height;
    uint32_t frame_count;
} FURI_PACKED AnimImageFileHeader;

static_assert(
    sizeof(AnimImageFileHeader) == 7 * sizeof(uint32_t),
    "Incorrect size of AnimImageFileHeader");

typedef struct {
    uint32_t begin_idx;
    uint32_t end_idx;
    bool loop;
} AnimImageRange;

struct AnimImage {
    Widget base;
    lv_obj_t* canvas;
    lv_timer_t* timer;
    uint8_t* canvas_buf;
    File* file;
    uint32_t frame_count;
    size_t frame_size;
    uint32_t current_idx;
    AnimImageRange current_range;
    AnimImageRange waiting_range;
    bool has_waiting_range;
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

static void anim_image_update(AnimImage* instance) {
    do {
        if(instance->current_idx == instance->current_range.begin_idx) {
            const size_t offset = sizeof(AnimImageFileHeader) +
                                  instance->current_range.begin_idx * instance->frame_size;
            if(!storage_file_seek(instance->file, offset, true)) {
                FURI_LOG_E(TAG, "Failed to seek animation file");
                break;
            }
        }

        size_t bytes_read =
            storage_file_read(instance->file, instance->canvas_buf, instance->frame_size);

        if(bytes_read != instance->frame_size) {
            FURI_LOG_E(TAG, "Failed to read a frame from file");
            break;
        }

        lv_obj_invalidate(instance->canvas);

        if(++instance->current_idx > instance->current_range.end_idx) {
            if(instance->has_waiting_range) {
                instance->has_waiting_range = false;
                instance->current_range = instance->waiting_range;
                instance->current_idx = instance->current_range.begin_idx;

            } else if(instance->current_range.loop) {
                instance->current_idx = instance->current_range.begin_idx;

            } else {
                lv_timer_pause(instance->timer);
            }
        }
    } while(false);
}

static void anim_image_timer_callback(lv_timer_t* timer) {
    AnimImage* instance = lv_timer_get_user_data(timer);
    anim_image_update(instance);
}

static bool anim_image_validate_header(const AnimImageFileHeader* header, size_t file_size) {
    bool validated = false;

    do {
        if(header->magic != ANIM_IMAGE_FILE_MAGIC) {
            FURI_LOG_E(TAG, "Invalid magic num: %ld", header->magic);
            break;
        }

        if(header->format_version != ANIM_IMAGE_FORMAT_VERSION) {
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

        const size_t data_size =
            header->frame_count * header->bytes_per_pixel * header->width * header->height;

        if((data_size + sizeof(AnimImageFileHeader)) != file_size) {
            FURI_LOG_E(
                TAG,
                "Frames: %ld, BPP: %ld, Width: %ld, Height: %ld",
                header->frame_count,
                header->bytes_per_pixel,
                header->width,
                header->height);

            FURI_LOG_E(
                TAG,
                "Incorrect file size: %zu. Expected size: %zu",
                file_size,
                data_size + sizeof(AnimImageFileHeader));

            break;
        }

        validated = true;
    } while(false);

    return validated;
}

static void anim_image_set_range_internal(
    AnimImage* instance,
    uint32_t begin,
    uint32_t end,
    bool loop,
    bool wait_end) {
    const uint32_t begin_idx_new = MIN(begin, instance->frame_count - 1);
    const uint32_t end_idx_new = MIN(end, instance->frame_count - 1);

    if(wait_end) {
        instance->waiting_range.begin_idx = begin_idx_new;
        instance->waiting_range.end_idx = end_idx_new;
        instance->waiting_range.loop = loop;

        instance->has_waiting_range = true;

    } else {
        instance->current_range.begin_idx = begin_idx_new;
        instance->current_range.end_idx = end_idx_new;
        instance->current_range.loop = loop;

        instance->current_idx = instance->current_range.begin_idx;
        instance->has_waiting_range = false;

        anim_image_update(instance);
    }

    if(lv_timer_get_paused(instance->timer)) {
        lv_timer_resume(instance->timer);
    }
}

static void anim_image_set_placeholder(AnimImage* instance) {
    const lv_image_header_t* header = &I_load_error_9x9.header;
    void* data = (void*)I_load_error_9x9.data;

    lv_canvas_set_buffer(instance->canvas, data, header->w, header->h, header->cf);
    lv_obj_invalidate(instance->canvas);
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
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* anim_image_get_base(AnimImage* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

bool anim_image_set_source(AnimImage* instance, const char* file_path) {
    furi_check(instance);

    bool success = false;

    AnimImageFileHeader header;

    do {
        if(storage_file_is_open(instance->file)) {
            storage_file_close(instance->file);
        }

        if(!storage_file_open(instance->file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file %s", file_path);
            break;
        }

        size_t bytes_read =
            storage_file_read(instance->file, &header, sizeof(AnimImageFileHeader));

        if(bytes_read != sizeof(AnimImageFileHeader)) {
            FURI_LOG_E(TAG, "Failed to read file header");
            break;
        }

        if(!anim_image_validate_header(&header, storage_file_size(instance->file))) {
            FURI_LOG_E(TAG, "Corrupt or invalid file");
            break;
        }

        success = true;

    } while(false);

    if(success) {
        instance->frame_size = header.height * header.width * header.bytes_per_pixel;
        instance->frame_count = header.frame_count;
        instance->canvas_buf = realloc(instance->canvas_buf, instance->frame_size);

        lv_canvas_set_buffer(
            instance->canvas,
            instance->canvas_buf,
            header.width,
            header.height,
            header.bytes_per_pixel == 3 ? LV_COLOR_FORMAT_RGB888 : LV_COLOR_FORMAT_L8);

        lv_timer_set_period(instance->timer, 1000 / header.fps);

        anim_image_set_range_internal(instance, 0, header.frame_count - 1, true, false);

    } else {
        anim_image_set_placeholder(instance);
    }

    return success;
}

void anim_image_set_range(
    AnimImage* instance,
    uint32_t begin,
    uint32_t end,
    bool loop,
    bool wait_end) {
    furi_check(instance);
    furi_check(begin <= end);

    if(instance->timer) {
        anim_image_set_range_internal(instance, begin, end, loop, wait_end);
    }
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
