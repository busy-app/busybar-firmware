#pragma once

#include "front_display.h"

#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi_hal_bus.h>
#include <furi_hal_gpio.h>
#include <furi_hal_dma.h>

// LED current per color channel
#define CUR_GAIN_R (255)
#define CUR_GAIN_G (255)
#define CUR_GAIN_B (255)

#define DISPLAY_GAMMA     (2.8f) // Default gamma value
#define DISPLAY_GAMMA_MIN (2.5f) // Minimum brightness-adjusted gamma value

#define BRIGHTNESS_VAL_MIN (0)
#define BRIGHTNESS_VAL_MAX (100)

#define LED_DRIVER_CHAIN (3)
#define PIXEL_BUF_LEN    (3 * 2 * 2) // Tx buffer len for 1 pixel (RGB * uint16_t * 2(Dual SPI))
#define DISPLAY_BLOCKS   (8 * 3) // Scan blocks number (24)
#define TRANSFER_COUNT   (FRONT_DISPLAY_H * FRONT_DISPLAY_W / LED_DRIVER_CHAIN)

#define DISPLAY_GPIO_SPEED GpioSpeedMedium

typedef struct FrontDisplayDriver FrontDisplayDriver;
typedef struct FrontDisplayScan FrontDisplayScan;

typedef void (*FrontDisplayCallback)(void* context);

void front_display_scan_init(void);
void front_display_scan_start(void);
void front_display_scan_output_enable(bool enable);
void front_display_scan_data_sync_enable(void);

void front_display_driver_init(uint8_t initial_brightness);
void front_display_driver_start(void);
void front_display_driver_set_update_callback(FrontDisplayCallback callback, void* context);
void front_display_driver_send_buf_start(void);
void front_display_driver_send_frame(const uint8_t* frame_buf);
void front_display_driver_vsync_trig(void);

// Valid brightness values: [0, BRIGHTNESS_VAL_MAX]
void front_display_driver_set_brightness(uint8_t brightness);

void front_display_scan_deinit(void);
void front_display_driver_deinit(void);
