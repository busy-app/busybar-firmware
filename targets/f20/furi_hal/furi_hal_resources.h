/**
 * @file furi_hal_resources.h
 * @brief Hardware resources API
 */
#pragma once

#include <furi.h>
#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    InputKeyUp,
    InputKeyDown,
    InputKeyRight,
    InputKeyLeft,
    InputKeyOk,
    InputKeyBack,
    InputKeyStart,
    InputKeyBusy,
    InputKeyStatus,
    InputKeyOff,
    InputKeyApps,
    InputKeySettings,
    InputKeyMAX, /**< Special value, don't use it */
} InputKey;

typedef struct {
    const GpioPin* pin;
    const char* name;
    const uint8_t number;
    const bool debug;
} GpioPinRecord;

extern const GpioPin gpio_swdio;
extern const GpioPin gpio_swclk;
extern const GpioPin gpio_log_usart_tx;
extern const GpioPin gpio_log_usart_rx;

extern const GpioPin gpio_usart1_tx;
extern const GpioPin gpio_usart1_rx;
extern const GpioPin gpio_usart1_rts;
extern const GpioPin gpio_usart1_cts;
extern const GpioPin gpio_lpuart1_tx;
extern const GpioPin gpio_lpuart1_rx;

extern const GpioPin gpio_917_pwr_en;
extern const GpioPin gpio_917_rst;
extern const GpioPin gpio_917_swo;
extern const GpioPin gpio_917_irq;

extern const GpioPin gpio_bq25798_qon;
extern const GpioPin gpio_bq25798_irq;

extern const GpioPin gpio_sd_card_d0;
extern const GpioPin gpio_sd_card_d1;
extern const GpioPin gpio_sd_card_d2;
extern const GpioPin gpio_sd_card_d3;
extern const GpioPin gpio_sd_card_ck;
extern const GpioPin gpio_sd_card_cmd;

extern const GpioPin gpio_usb_dm;
extern const GpioPin gpio_usb_dp;

extern const GpioPin gpio_ucpd_cc1;
extern const GpioPin gpio_ucpd_cc2;

extern const GpioPin gpio_oled_spi_sdin;
extern const GpioPin gpio_oled_spi_sclk;
extern const GpioPin gpio_oled_cs;
extern const GpioPin gpio_oled_dc;
extern const GpioPin gpio_oled_fr;
extern const GpioPin gpio_oled_vcc_en;

extern const GpioPin gpio_i2c_scl;
extern const GpioPin gpio_i2c_sda;

extern const GpioPin gpio_led_power_en;
extern const GpioPin gpio_led_scan_sdi;
extern const GpioPin gpio_led_scan_clk;
extern const GpioPin gpio_led_scan_latch;
extern const GpioPin gpio_led_gclk;
extern const GpioPin gpio_led_sdi_ospi_d0;
extern const GpioPin gpio_led_le_ospi_d1;
extern const GpioPin gpio_led_dclk_ospi_clk;

extern const GpioPin gpio_audio_dac;
extern const GpioPin gpio_pa8;

extern const GpioPinRecord gpio_pins[];
extern const size_t gpio_pins_count;

void furi_hal_resources_init_early(void);

void furi_hal_resources_deinit_early(void);

void furi_hal_resources_init(void);

#ifdef __cplusplus
}
#endif
