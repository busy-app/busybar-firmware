
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#elif defined(LV_BUILD_TEST)
#include "../lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_I_PAUSE_5X5
#define LV_ATTRIBUTE_I_PAUSE_5X5
#endif

static const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_I_PAUSE_5X5 uint8_t
    I_pause_5x5_map[] = {

        0x00,
        0x00,
        0x00,
        0x00,
        0xff,
        0xff,
        0xff,
        0xff,

        0xd8,
        0xd8,
        0xd8,
        0xd8,
        0xd8,

};

const lv_image_dsc_t I_pause_5x5 = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_I1,
    .header.flags = 0,
    .header.w = 5,
    .header.h = 5,
    .header.stride = 1,
    .header.reserved_2 = 0,
    .data_size = sizeof(I_pause_5x5_map),
    .data = I_pause_5x5_map,
    .reserved = NULL,
};
