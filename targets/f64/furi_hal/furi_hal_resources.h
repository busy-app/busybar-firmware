/**
 * @file furi_hal_resources.h
 * @brief Hardware resources API
 *
 * @warning GPIO_0...5 are reserved and MUST NOT be used
 */
#pragma once

#include <furi.h>
#include <furi_hal_gpio.h>

#include <input_common/input_common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const GpioPin* gpio;
    const char* name;
    const GpioCondition cond;
    const InputDevice device;
    union {
        const InputButton button;
        const InputSwitchPosition pos;
    };
} InputPin;

/* HP GPIO pins */
extern const GpioPin gpio_6;
extern const GpioPin gpio_pwm_red;
extern const GpioPin gpio_ulp_uart_rx;
extern const GpioPin gpio_ulp_uart_tx;
extern const GpioPin gpio_10;
extern const GpioPin gpio_pwm_green;
extern const GpioPin gpio_12;
extern const GpioPin gpio_pwm_blue;
extern const GpioPin gpio_25;
extern const GpioPin gpio_26;
extern const GpioPin gpio_27;
extern const GpioPin gpio_28;
extern const GpioPin gpio_29;
extern const GpioPin gpio_30;
extern const GpioPin gpio_46;
extern const GpioPin gpio_47;
extern const GpioPin gpio_48;
extern const GpioPin gpio_49;
extern const GpioPin gpio_sw_busy;
extern const GpioPin gpio_sw_settings;
extern const GpioPin gpio_52;
extern const GpioPin gpio_usart0_rts;
extern const GpioPin gpio_usart0_tx;
extern const GpioPin gpio_usart0_rx;
extern const GpioPin gpio_usart0_cts;
extern const GpioPin gpio_57;

/* Internal HP GPIO pins */
extern const GpioPin gpio_i_64; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_65; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_66; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_67; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_68; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_69; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_70; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_71; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_uart1_rx; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_encoder_a; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_encoder_b; /**< Not available on the package, internal use only */
extern const GpioPin gpio_i_uart1_tx; /**< Not available on the package, internal use only */

/* ULP GPIO pins */
extern const GpioPin gpio_ulp_0;
extern const GpioPin gpio_irq;
extern const GpioPin gpio_ulp_2;
extern const GpioPin gpio_ulp_i_3; /**< Not available on the package, internal use only */
extern const GpioPin gpio_ulp_4;
extern const GpioPin gpio_ulp_5;
extern const GpioPin gpio_sw_apps;
extern const GpioPin gpio_sw_status;
extern const GpioPin gpio_uart1_rx;
extern const GpioPin gpio_encoder_a;
extern const GpioPin gpio_encoder_b;
extern const GpioPin gpio_uart1_tx;

/* UULP GPIO pins */
extern const GpioPin gpio_sw_off;
extern const GpioPin gpio_sw_back;
extern const GpioPin gpio_sw_start_pause;
extern const GpioPin gpio_sw_ok;

extern const InputPin input_pins[];
extern const size_t input_pins_count;

void furi_hal_resources_init_early(void);

void furi_hal_resources_deinit_early(void);

void furi_hal_resources_init(void);

#ifdef __cplusplus
}
#endif
