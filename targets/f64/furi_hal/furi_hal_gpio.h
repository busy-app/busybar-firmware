/**
 * @file furi_hal_gpio.h
 * @brief GPIO HAL library for Si917
 *
 * @note If a pin is configured as open drain output, use furi_hal_gpio_write_open_drain()
 *       instead of furi_hal_gpio_write()
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interrupt callback prototype
 */
typedef void (*GpioExtiCallback)(void* ctx);

/**
 * Gpio modes
 */
typedef enum {
    GpioModeInput,
    GpioModeOutputPushPull,
    GpioModeOutputOpenDrain,
} GpioMode;

/**
 * Gpio pull modes
 */
typedef enum {
    GpioPullNo,
    GpioPullUp,
    GpioPullDown,
} GpioPull;

/**
 * Gpio speed modes
 */
typedef enum {
    GpioSpeedLow,
    GpioSpeedHigh,
} GpioSpeed;

/**
 * Gpio alternate functions
 */
typedef enum {
    GpioAltFnUnused = 0,
    GpioAltFn1 = 1,

    GpioAltFn2USART0_CLK = 2, /**< USART0_CLK on GPIO_52 */
    GpioAltFn2USART0_RTS = 2, /** USART0_RTS on GPIO_53 */
    GpioAltFn2USART0_TX = 2, /**< USART0_TX on GPIO_54 */
    GpioAltFn2USART0_RX = 2, /**< USART0_RX on GPIO_55 */
    GpioAltFn2USART0_CTS = 2, /** USART0_CTS on GPIO_56 */

    GpioAltFn3ULP_UART_RX = 3, /**< ULP_UART_RX on ULP_GPIO_2 */
    GpioAltFn3ULP_UART_TX = 3, /**< ULP_UART_TX on ULP_GPIO_3 (via mux only) */
    GpioAltFn3QEI_PHA = 3, /**< QEI via SOCPERH_ON_ULP_GPIO_9 */
    GpioAltFn3QEI_PHB = 3, /**< QEI via SOCPERH_ON_ULP_GPIO_10 */

    GpioAltFn4 = 4,
    GpioAltFn5 = 5,

    GpioAltFn6SOCPERH_ON_ULP_GPIO_8 = 6, /**< Multiplexed HP Peripheral on ULP_GPIO_8 */
    GpioAltFn6SOCPERH_ON_ULP_GPIO_9 = 6, /**< Multiplexed HP Peripheral on ULP_GPIO_9 */
    GpioAltFn6SOCPERH_ON_ULP_GPIO_10 = 6, /**< Multiplexed HP Peripheral on ULP_GPIO_10 */
    GpioAltFn6SOCPERH_ON_ULP_GPIO_11 = 6, /**< Multiplexed HP Peripheral on ULP_GPIO_11 */

    GpioAltFn6UART1_RX = 6, /**< UART1_RX via SOCPERH_ON_ULP_GPIO_8 */

    GpioAltFn7 = 7,
    GpioAltFn8 = 8,

    GpioAltFn9ULPPERH_ON_SOC_GPIO_2 = 9, /**< Multiplexed ULP Peripheral on GPIO_8 */
    GpioAltFn9ULPPERH_ON_SOC_GPIO_3 = 9, /**< Multiplexed ULP Peripheral on GPIO_9 */
    GpioAltFn9UART1_TX = 9, /**< UART1_TX via SOCPERH_ON_ULP_GPIO_11 */

    GpioAltFn10 = 10,
    GpioAltFn10PWM_0H = 10, /**< PWM_0H on GPIO_7 */
    GpioAltFn10PWM_2H = 10, /**< PWM_2H on GPIO_11 */
    GpioAltFn10PWM_3H = 10, /**< PWM_3H on GPIO_15 */
    GpioAltFn11 = 11,
    GpioAltFn12 = 12,
    GpioAltFn13 = 13,
    GpioAltFn14 = 14,
    GpioAltFn15 = 15,
} GpioAltFn;

typedef enum {
    GpioTypeHp,
    GpioTypeUlp,
    GpioTypeUulp,
} GpioType;

typedef enum {
    GpioConditionRise,
    GpioConditionFall,
    GpioConditionRiseFall,
} GpioCondition;

typedef struct {
    GpioType type;
    uint8_t pin;
} GpioPin;

/**
 * GPIO initialization function, simple version
 * @param gpio  GpioPin
 * @param mode  GpioMode
 */
void furi_hal_gpio_init_simple(const GpioPin* gpio, const GpioMode mode);

/**
 * GPIO initialization function, normal version
 * @param gpio  GpioPin
 * @param mode  GpioMode
 * @param pull  GpioPull
 * @param speed GpioSpeed
 */
void furi_hal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

/**
 * GPIO initialization function, extended version
 * @param gpio  GpioPin
 * @param mode  GpioMode
 * @param pull  GpioPull
 * @param speed GpioSpeed
 * @param alt_fn GpioAltFn
 */
void furi_hal_gpio_init_ex(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed,
    const GpioAltFn alt_fn);

/**
 * Enable ULP to HP GPIO multiplexer
 * @param ulp_gpio GpioPin (must belong to ULP domain)
 * @param alt_fn GpioAltFn
 */
void furi_hal_gpio_enable_ulp_on_hp(const GpioPin* ulp_gpio, const GpioAltFn alt_fn);

/**
 * Add and enable interrupt
 * @param gpio GpioPin
 * @param cond GpioCondition for triggering the interrupt
 * @param cb   GpioExtiCallback
 * @param ctx  context for callback
 */
void furi_hal_gpio_add_int_callback(
    const GpioPin* gpio,
    const GpioCondition cond,
    GpioExtiCallback cb,
    void* ctx);

/**
 * Enable interrupt
 * @param gpio GpioPin
 */
void furi_hal_gpio_enable_int_callback(const GpioPin* gpio);

/**
 * Disable interrupt
 * @param gpio GpioPin
 */
void furi_hal_gpio_disable_int_callback(const GpioPin* gpio);

/**
 * Remove interrupt
 * @param gpio GpioPin
 */
void furi_hal_gpio_remove_int_callback(const GpioPin* gpio);

/**
 * GPIO write pin
 * @param gpio  GpioPin
 * @param state true / false
 */
void furi_hal_gpio_write(const GpioPin* gpio, const bool state);

/**
 * GPIO write pin, open drain mode
 * @param gpio  GpioPin
 * @param state true / false
 */
void furi_hal_gpio_write_open_drain(const GpioPin* gpio, const bool state);

/**
 * GPIO read pin
 * @param gpio GpioPin
 * @return true / false
 */
bool furi_hal_gpio_read(const GpioPin* gpio);

#ifdef __cplusplus
}
#endif
