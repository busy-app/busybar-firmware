#pragma once

#include "anim_image.h"

#include <storage/storage.h>

#include "../widget_i.h"

extern const lv_obj_class_t anim_image_lvgl_class;

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
    uint32_t frame_rate;
    uint32_t frame_count;
    size_t frame_size;
    uint32_t current_idx;
    AnimImageRange current_range;
    AnimImageRange waiting_range;
    bool has_waiting_range;
    bool is_loaded;
};
