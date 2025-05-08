
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

#ifndef LV_ATTRIBUTE_I_LOAD_ERROR_9X9
#define LV_ATTRIBUTE_I_LOAD_ERROR_9X9
#endif

static const
LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_I_LOAD_ERROR_9X9
uint8_t I_load_error_9x9_map[] = {

    0x4c,0x70,0x47,0x00,0xff,0x00,0xff,0xff,

    0xff,0x80,
    0x80,0x80,
    0xa2,0x80,
    0x94,0x80,
    0x88,0x80,
    0x94,0x80,
    0xa2,0x80,
    0x80,0x80,
    0xff,0x80,

};

const lv_image_dsc_t I_load_error_9x9 = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_I1,
  .header.flags = 0,
  .header.w = 9,
  .header.h = 9,
  .header.stride = 2,
  .header.reserved_2 = 0,
  .data_size = sizeof(I_load_error_9x9_map),
  .data = I_load_error_9x9_map,
  .reserved = NULL,
};

