
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

#ifndef LV_ATTRIBUTE_I_ACTIVE_INDICATOR_RIGHT_28X7
#define LV_ATTRIBUTE_I_ACTIVE_INDICATOR_RIGHT_28X7
#endif

static const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST
    LV_ATTRIBUTE_I_ACTIVE_INDICATOR_RIGHT_28X7 uint8_t I_active_indicator_right_28x7_map[] = {

        0x4c, 0x70, 0x47, 0x00, 0x22, 0x22, 0x22, 0xff, 0x66, 0x66, 0x66, 0xff, 0x11,
        0x11, 0x11, 0xff,

        0x00, 0x0f, 0x00, 0x00, 0x50, 0x00, 0x0a, 0x00, 0xff, 0x00, 0x05, 0x50, 0x00,
        0xaa, 0x0f, 0xff, 0x00, 0x55, 0x50, 0x0a, 0xaa, 0xff, 0xff, 0x05, 0x55, 0x50,
        0xaa, 0xaa, 0x0f, 0xff, 0x00, 0x55, 0x50, 0x0a, 0xaa, 0x00, 0xff, 0x00, 0x05,
        0x50, 0x00, 0xaa, 0x00, 0x0f, 0x00, 0x00, 0x50, 0x00, 0x0a,

};

const lv_image_dsc_t I_active_indicator_right_28x7 = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_I2,
    .header.flags = 0,
    .header.w = 28,
    .header.h = 7,
    .header.stride = 7,
    .header.reserved_2 = 0,
    .data_size = sizeof(I_active_indicator_right_28x7_map),
    .data = I_active_indicator_right_28x7_map,
    .reserved = NULL,
};
