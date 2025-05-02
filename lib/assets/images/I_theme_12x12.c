
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

#ifndef LV_ATTRIBUTE_I_THEME_12X12
#define LV_ATTRIBUTE_I_THEME_12X12
#endif

static const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_I_THEME_12X12 uint8_t
    I_theme_12x12_map[] = {

        0x4c, 0x70, 0x47, 0x00, 0xff, 0xff, 0xff, 0xff,

        0x07, 0xc0, 0x18, 0x20, 0x21, 0x90, 0x4d, 0x90, 0x8c, 0x10, 0x80, 0x10,
        0xb2, 0x20, 0xb7, 0x40, 0x82, 0x80, 0x80, 0x80, 0x61, 0x80, 0x1e, 0x00,

};

const lv_image_dsc_t I_theme_12x12 = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_I1,
    .header.flags = 0,
    .header.w = 12,
    .header.h = 12,
    .header.stride = 2,
    .header.reserved_2 = 0,
    .data_size = sizeof(I_theme_12x12_map),
    .data = I_theme_12x12_map,
    .reserved = NULL,
};
