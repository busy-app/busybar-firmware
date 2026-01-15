#include "anim_play.h"
#include "../widget_i.h"

struct AnimPlay {
    Widget base;
    lv_obj_t* canvas;
    uint8_t* canvas_buf;

    lv_timer_t* timer;

    char* file_path;
    Storage* storage;
    AnimFile* file;

    AnimPlayFrameCallback frame_cb;
    void* frame_cb_context;
};

extern const lv_obj_class_t anim_play_lvgl_class;
