
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

#ifndef LV_ATTRIBUTE_I_TICK_RED_6X5
#define LV_ATTRIBUTE_I_TICK_RED_6X5
#endif

static const
LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_I_TICK_RED_6X5
uint8_t I_tick_red_6x5_map[] = {

    0x4c,0x70,0x47,0x00,0x00,0x00,0xff,0x80,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0x00,

    0x00,0x20,
    0x00,0xa0,
    0x82,0x80,
    0xa6,0x00,
    0x28,0x00,

};

const lv_image_dsc_t I_tick_red_6x5 = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_I2,
  .header.flags = 0,
  .header.w = 6,
  .header.h = 5,
  .header.stride = 2,
  .header.reserved_2 = 0,
  .data_size = sizeof(I_tick_red_6x5_map),
  .data = I_tick_red_6x5_map,
  .reserved = NULL,
};

