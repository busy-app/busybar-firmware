#pragma once

#include "led_display.h"

#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi_hal_bus.h>
#include <furi_hal_gpio.h>
#include <furi_hal_dma.h>

// LED current per color channel (HC | Gain)
// #define CUR_GAIN_R (0x20 | 5)
// #define CUR_GAIN_G (0x20 | 0)
// #define CUR_GAIN_B (0x00 | 18)

#define DISPLAY_GAMMA (0.35f) // Default gamma value

#define BRIGHTNESS_VAL_MAX 100

#define LED_DRIVER_CHAIN (3)
#define PIXEL_BUF_LEN    (3 * 2 * 2) // Tx buffer len for 1 pixel (RGB * uint16_t * 2(Dual SPI))
#define DISPLAY_BLOCKS   (8 * 3) // Scan blocks number (24)

typedef struct LedDisplayDriver LedDisplayDriver;
typedef struct LedDisplayScan LedDisplayScan;

typedef void (*LedDisplayCallback)(void* context);

void led_display_scan_init(void);
void led_display_scan_start(void);
void led_display_scan_output_enable(bool enable);
void led_display_scan_data_sync_enable(void);

uint16_t led_display_gamma_apply(const uint16_t* gamma_lut, uint8_t in_val);
void led_display_gamma_lut_generate(uint16_t* gamma_lut, float gamma_val, uint8_t brightness);

void led_display_driver_init(uint8_t initial_brightness);
void led_display_driver_start(void);
void led_display_driver_set_update_callback(LedDisplayCallback callback, void* context);
void led_display_driver_send_buf_start(void);
void led_display_driver_send_frame(const uint8_t* frame_buf);
void led_display_driver_vsync_trig(void);
void led_display_driver_set_brightness(uint8_t brightness);
