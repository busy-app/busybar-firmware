#include "image_animation.h"

#include <furi/furi.h>

#include <storage/storage.h>
#include <led_display/led_display.h>

#define TAG "ImageAnimation"

#define IMAGE_ANIMATION_FILE_MAGIC     (0x69)
#define IMAGE_ANIMATION_FORMAT_VERSION (0x00)

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
    sizeof(ImageAnimationFileHeader) == 7 * 4,
    "Incorrect size of ImageAnimationFileHeader");

struct ImageAnimation {
    Storage* storage;
    File* file;
    ImageAnimationFileHeader header;

    lv_obj_t* canvas;
    uint32_t frame_size;
    uint8_t canvas_buffer[DOT_MATRIX_BPP * DOT_MATRIX_H * DOT_MATRIX_W];

    uint32_t frame_idx;
    uint32_t frame_total;

    lv_timer_t* timer;
};

static bool image_update_canvas_buffer(ImageAnimation* instance) {
    bool res = false;

    do {
        if(instance->frame_idx == 0) {
            if(!storage_file_seek(instance->file, sizeof(ImageAnimationFileHeader), true)) {
                FURI_LOG_E(TAG, "Failed to seek animation file");
                break;
            }
        }
        size_t bytes_read =
            storage_file_read(instance->file, instance->canvas_buffer, instance->frame_size);
        instance->frame_idx = (instance->frame_idx + 1) % instance->frame_total;

        res = (bytes_read == instance->frame_size);
    } while(false);

    return res;
}

static void image_animation_timer_callback(lv_timer_t* timer) {
    ImageAnimation* instance = lv_timer_get_user_data(timer);
    if(!image_update_canvas_buffer(instance)) {
        FURI_LOG_E(TAG, "Failed to read frame from file");
        return;
    }

    lv_obj_invalidate(instance->canvas);
}

ImageAnimation* image_animation_alloc(lv_obj_t* parent) {
    furi_check(parent);

    ImageAnimation* instance = malloc(sizeof(ImageAnimation));
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->file = storage_file_alloc(instance->storage);

    instance->canvas = lv_canvas_create(parent);

    return instance;
}

void image_animation_free(ImageAnimation* instance) {
    furi_check(instance);

    if(instance->timer) {
        lv_timer_delete(instance->timer);
    }
    lv_obj_delete(instance->canvas);

    storage_file_free(instance->file);
    furi_record_close(RECORD_STORAGE);
    free(instance);
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
            FURI_LOG_D(TAG, "Failed to open file %s", file_path);
            break;
        }

        size_t bytes_read =
            storage_file_read(instance->file, &instance->header, sizeof(ImageAnimationFileHeader));
        if(bytes_read != sizeof(ImageAnimationFileHeader)) {
            FURI_LOG_D(TAG, "Failed to read file header");
            break;
        }

        ImageAnimationFileHeader* header = &instance->header;

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

        instance->timer =
            lv_timer_create(image_animation_timer_callback, 1000 / header->fps, instance);
        lv_timer_set_repeat_count(instance->timer, -1);

        lv_canvas_set_buffer(
            instance->canvas,
            instance->canvas_buffer,
            header->width,
            header->height,
            LV_COLOR_FORMAT_RGB888);
        lv_group_add_obj(lv_group_get_default(), instance->canvas);
        image_update_canvas_buffer(instance);
    }

    return parsed;
}

void image_animation_start(ImageAnimation* instance) {
    furi_check(instance);
    furi_check(instance->file);
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
