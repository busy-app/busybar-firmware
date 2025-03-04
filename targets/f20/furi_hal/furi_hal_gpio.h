#pragma once
#include "stdbool.h"
#include <stm32u5xx_ll_gpio.h>
#include <stm32u5xx_safe.h>
// #include <stm32u5xx_ll_system.h>
// #include <stm32u5xx_ll_exti.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Number of gpio on one port
 */
#define GPIO_NUMBER (16U)

/**
 * Interrupt callback prototype
 */
typedef void (*GpioExtiCallback)(void* ctx);

/**
 * Gpio interrupt type
 */
typedef struct {
    GpioExtiCallback callback;
    void* context;
} GpioInterrupt;

/**
 * Gpio modes
 */
typedef enum {
    GpioModeInput,
    GpioModeOutputPushPull,
    GpioModeOutputOpenDrain,
    GpioModeAltFunctionPushPull,
    GpioModeAltFunctionOpenDrain,
    GpioModeAnalog,
    GpioModeInterruptRise,
    GpioModeInterruptFall,
    GpioModeInterruptRiseFall,
    GpioModeEventRise,
    GpioModeEventFall,
    GpioModeEventRiseFall,
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
    GpioSpeedMedium,
    GpioSpeedHigh,
    GpioSpeedVeryHigh,
} GpioSpeed;

/**
 * Gpio alternate functions
 * https://www.st.com/resource/en/datasheet/stm32u5g7vj.pdf
 * DS14102 "Table 28. Alternate function", Page 130
 */
typedef enum {
    GpioAltFn0RTC_50HZ = 0x00, /*!< RTC_50Hz Alternate Function mapping */
    GpioAltFn0MCO = 0x00, /*!< MCO (MCO1 and MCO2) Alternate Function mapping */
    GpioAltFn0SWJ = 0x00, /*!< SWJ (SWD and JTAG) Alternate Function mapping */
    GpioAltFn0TRACE = 0x00, /*!< TRACE Alternate Function mapping */
    GpioAltFn0LPTIM1 = 0x00, /*!< LPTIM1 Alternate Function mapping */
    GpioAltFn0CSLEEP = 0x00, /*!< CSLEEP Alternate Function mapping */
    GpioAltFn0CSTOP = 0x00, /*!< CSTOP Alternate Function mapping */
    GpioAltFn0CRS = 0x00, /*!< CRS Alternate Function mapping */
    GpioAltFn0SRDSTOP = 0x00, /*!< SRDSTOP Alternate Function mapping */

    GpioAltFn1TIM1 = 0x01, /*!< TIM1 Alternate Function mapping */
    GpioAltFn1TIM2 = 0x01, /*!< TIM2 Alternate Function mapping */
    GpioAltFn1TIM5 = 0x01, /*!< TIM5 Alternate Function mapping */
    GpioAltFn1TIM8 = 0x01, /*!< TIM8 Alternate Function mapping */
    GpioAltFn1LPTIM1 = 0x01, /*!< LPTIM1 Alternate Function mapping */
    GpioAltFn1IR = 0x01, /*!< IR Alternate Function mapping */

    GpioAltFn2TIM1 = 0x02, /*!< TIM1 Alternate Function mapping */
    GpioAltFn2TIM2 = 0x02, /*!< TIM2 Alternate Function mapping */
    GpioAltFn2TIM3 = 0x02, /*!< TIM3 Alternate Function mapping */
    GpioAltFn2TIM4 = 0x02, /*!< TIM4 Alternate Function mapping */
    GpioAltFn2TIM5 = 0x02, /*!< TIM5 Alternate Function mapping */
    GpioAltFn2LPTIM1 = 0x02, /*!< LPTIM1 Alternate Function mapping */
    GpioAltFn2LPTIM2 = 0x02, /*!< LPTIM2 Alternate Function mapping */
    GpioAltFn2LPTIM3 = 0x02, /*!< LPTIM3 Alternate Function mapping */
    GpioAltFn2I2C5 = 0x02, /*!< I2C5 Alternate Function mapping */
    GpioAltFn2I2C6 = 0x02, /*!< I2C6 Alternate Function mapping */
    GpioAltFn2GFXTIM = 0x02, /*!< GFXTIM Alternate Function mapping */

    GpioAltFn3I2C4 = 0x03, /*!< I2C4 Alternate Function mapping */
    GpioAltFn3OCTOSPI1 = 0x03, /*!< OCTOSPI1 Alternate Function mapping */
    GpioAltFn3SAI1 = 0x03, /*!< SAI1 Alternate Function mapping */
    GpioAltFn3SPI2 = 0x03, /*!< SPI2 Alternate Function mapping */
    GpioAltFn3TIM1 = 0x03, /*!< TIM1 Alternate Function mapping */
    GpioAltFn3TIM8 = 0x03, /*!< TIM8 Alternate Function mapping */
    GpioAltFn3TIM8_COMP1 = 0x03, /*!< TIM8/COMP1 Break in Alternate Function mapping */
    GpioAltFn3TIM8_COMP2 = 0x03, /*!< TIM8/COMP2 Break in Alternate Function mapping */
    GpioAltFn3TIM1_COMP1 = 0x03, /*!< TIM1/COMP1 Break in Alternate Function mapping */
    GpioAltFn3TIM1_COMP2 = 0x03, /*!< TIM1/COMP2 Break in Alternate Function mapping */
    GpioAltFn3USART2 = 0x03, /*!< USART2 Alternate Function mapping */
    GpioAltFn3ADF1 = 0x03, /*!< ADF1 Alternate Function mapping */
    GpioAltFn3USB_HS = 0x03, /*!< USB_HS Alternate Function mapping */

    GpioAltFn4I2C1 = 0x04, /*!< I2C1 Alternate Function mapping */
    GpioAltFn4I2C2 = 0x04, /*!< I2C2 Alternate Function mapping */
    GpioAltFn4I2C3 = 0x04, /*!< I2C3 Alternate Function mapping */
    GpioAltFn4I2C4 = 0x04, /*!< I2C4 Alternate Function mapping */
    GpioAltFn4PSSI = 0x04, /*!< PSSI Alternate Function mapping */
    GpioAltFn4DCMI = 0x04, /*!< DCMI Alternate Function mapping */
    GpioAltFn4LPTIM3 = 0x04, /*!< LPTIM3 Alternate Function mapping */
    GpioAltFn4I2C5 = 0x04, /*!< I2C5 Alternate Function mapping */

    GpioAltFn5I2C4 = 0x05, /*!< I2C4 Alternate Function mapping */
    GpioAltFn5OCTOSPI1 = 0x05, /*!< OCTOSPI1 Alternate Function mapping */
    GpioAltFn5OCTOSPI2 = 0x05, /*!< OCTOSPI2 Alternate Function mapping */
    GpioAltFn5SPI1 = 0x05, /*!< SPI1 Alternate Function mapping */
    GpioAltFn5SPI2 = 0x05, /*!< SPI2 Alternate Function mapping */
    GpioAltFn5SPI3 = 0x05, /*!< SPI3 Alternate Function mapping */
    GpioAltFn5DCMI = 0x05, /*!< DCMI Alternate Function mapping */
    GpioAltFn5MDF1 = 0x05, /*!< MDF1 Alternate Function mapping */
    GpioAltFn5PSSI = 0x05, /*!< PSSI Alternate Function mapping */
    GpioAltFn5GFXTIM = 0x05, /*!< GFXTIM Alternate Function mapping */

    GpioAltFn6OCTOSPI1 = 0x06, /*!< OCTOSPI1 Alternate Function mapping */
    GpioAltFn6OCTOSPI2 = 0x06, /*!< OCTOSPI2 Alternate Function mapping */
    GpioAltFn6MDF1 = 0x06, /*!< MDF1 Alternate Function mapping */
    GpioAltFn6SPI3 = 0x06, /*!< SPI3 Alternate Function mapping */
    GpioAltFn6I2C3 = 0x06, /*!< I2C3 Alternate Function mapping */

    GpioAltFn7USART1 = 0x07, /*!< USART1 Alternate Function mapping */
    GpioAltFn7USART2 = 0x07, /*!< USART2 Alternate Function mapping */
    GpioAltFn7USART3 = 0x07, /*!< USART3 Alternate Function mapping */
    GpioAltFn7USART6 = 0x07, /*!< USART6 Alternate Function mapping */
    GpioAltFn7LTDC = 0x07, /*!< LTDC Alternate Function mapping */

    GpioAltFn8LPUART1 = 0x08, /*!< LPUART1 Alternate Function mapping */
    GpioAltFn8UART4 = 0x08, /*!< UART4 Alternate Function mapping */
    GpioAltFn8UART5 = 0x08, /*!< UART5 Alternate Function mapping */
    GpioAltFn8SDMMC1 = 0x08, /*!< SDMMC1 Alternate Function mapping */
    GpioAltFn8SDMMC2 = 0x08, /*!< SDMMC2 Alternate Function mapping */
    GpioAltFn8LTDC = 0x08, /*!< LTDC Alternate Function mapping */
    GpioAltFn8HSPI1 = 0x08, /*!< HSPI1 Alternate Function mapping */

    GpioAltFn9FDCAN1 = 0x09, /*!< FDCAN1 Alternate Function mapping */
    GpioAltFn9TSC = 0x09, /*!< TSC Alternate Function mapping */

    GpioAltFn10DCMI = 0x0A, /*!< DCMI Alternate Function mapping */
    GpioAltFn10PSSI = 0x0A, /*!< PSSI Alternate Function mapping */
    GpioAltFn10USB = 0x0A, /*!< USB Alternate Function mapping */
    GpioAltFn10OCTOSPI1 = 0x0A, /*!< OCTOSPI1 Alternate Function mapping */
    GpioAltFn10OCTOSPI2 = 0x0A, /*!< OCTOSPI2 Alternate Function mapping */
    GpioAltFn10CRS = 0x0A, /*!< CRS Alternate Function mapping */
    GpioAltFn10USB_HS = 0x0A, /*!< USB_HS Alternate Function mapping */
    GpioAltFn10DSI = 0x0A, /*!< DSI Alternate Function mapping */
    GpioAltFn10GFXTIM = 0x0A, /*!< GFXTIM Alternate Function mapping */

    GpioAltFn11UCPD1 = 0x0B, /*!< UCPD1 Alternate Function mapping */
    GpioAltFn11SDMMC2 = 0x0B, /*!< SDMMC2 Alternate Function mapping */
    GpioAltFn11LPGPIO1 = 0x0B, /*!< LPGPIO1 Alternate Function mapping */
    GpioAltFn11FMC = 0x0B, /*!< FMC Alternate Function mapping */
    GpioAltFn11DSI = 0x0B, /*!< DSI Alternate Function mapping */
    GpioAltFn11GFXTIM = 0x0B, /*!< GFXTIM Alternate Function mapping */

    GpioAltFn12COMP1 = 0x0C, /*!< COMP1 Alternate Function mapping */
    GpioAltFn12COMP2 = 0x0C, /*!< COMP2 Alternate Function mapping */
    GpioAltFn12FMC = 0x0C, /*!< FMC Alternate Function mapping */
    GpioAltFn12TIM1_COMP1 = 0x0C, /*!< TIM1/COMP1 Break in Alternate Function mapping */
    GpioAltFn12TIM1_COMP2 = 0x0C, /*!< TIM1/COMP2 Break in Alternate Function mapping */
    GpioAltFn12TIM8_COMP2 = 0x0C, /*!< TIM8/COMP2 Break in Alternate Function mapping */
    GpioAltFn12SDMMC1 = 0x0C, /*!< SDMMC1 Alternate Function mapping */
    GpioAltFn12SDMMC2 = 0x0C, /*!< SDMMC2 Alternate Function mapping */

    GpioAltFn13SAI1 = 0x0D, /*!< SAI1 Alternate Function mapping */
    GpioAltFn13SAI2 = 0x0D, /*!< SAI2 Alternate Function mapping */
    GpioAltFn13LPTIM4 = 0x0D, /*!< LPTIM4 Alternate Function mapping */
    GpioAltFn13LPTIM2 = 0x0D, /*!< LPTIM2 Alternate Function mapping */
    GpioAltFn13GFXTIM = 0x0D, /*!< GFXTIM Alternate Function mapping */

    GpioAltFn14LPTIM2 = 0x0E, /*!< LPTIM2 Alternate Function mapping */
    GpioAltFn14LPTIM3 = 0x0E, /*!< LPTIM3 Alternate Function mapping */
    GpioAltFn14TIM2 = 0x0E, /*!< TIM2 Alternate Function mapping */
    GpioAltFn14TIM15 = 0x0E, /*!< TIM15 Alternate Function mapping */
    GpioAltFn14TIM15_COMP1 = 0x0E, /*!< TIM15/COMP1 Alternate Function mapping */
    GpioAltFn14TIM16 = 0x0E, /*!< TIM16 Alternate Function mapping */
    GpioAltFn14TIM16_COMP1 = 0x0E, /*!< TIM16/COMP1 Alternate Function mapping */
    GpioAltFn14TIM17 = 0x0E, /*!< TIM17 Alternate Function mapping */
    GpioAltFn14TIM17_COMP1 = 0x0E, /*!< TIM17/COMP1 Alternate Function mapping */
    GpioAltFn14FMC = 0x0E, /*!< FMC Alternate Function mapping */

    GpioAltFn15EVENTOUT = 0x0F, /*!< EVENTOUT Alternate Function mapping */
    GpioAltFnUnused = 16, /*!< just dummy value */
} GpioAltFn;

/**
 * Gpio structure
 */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
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
 * Add and enable interrupt
 * @param gpio GpioPin
 * @param cb   GpioExtiCallback
 * @param ctx  context for callback
 */
void furi_hal_gpio_add_int_callback(const GpioPin* gpio, GpioExtiCallback cb, void* ctx);

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
static inline void furi_hal_gpio_write(const GpioPin* gpio, const bool state) {
    // writing to BSSR is an atomic operation
    if(state == true) {
        gpio->port->BSRR = gpio->pin;
    } else {
        gpio->port->BSRR = (uint32_t)gpio->pin << GPIO_NUMBER;
    }
}

/**
 * Compatibility macro for systems that require a different
 * open drain GPIO procedure
 */
#define furi_hal_gpio_write_open_drain(gpio, state) furi_hal_gpio_write(gpio, state)

/**
 * GPIO read pin
 * @param port GPIO port
 * @param pin pin mask
 * @return true / false
 */
static inline void
    furi_hal_gpio_write_port_pin(GPIO_TypeDef* port, uint16_t pin, const bool state) {
    // writing to BSSR is an atomic operation
    if(state == true) {
        port->BSRR = pin;
    } else {
        port->BSRR = pin << GPIO_NUMBER;
    }
}

/**
 * GPIO read pin
 * @param gpio GpioPin
 * @return true / false
 */
static inline bool furi_hal_gpio_read(const GpioPin* gpio) {
    if((gpio->port->IDR & gpio->pin) != 0x00U) {
        return true;
    } else {
        return false;
    }
}

/**
 * GPIO read pin
 * @param port GPIO port
 * @param pin pin mask
 * @return true / false
 */
static inline bool furi_hal_gpio_read_port_pin(GPIO_TypeDef* port, uint16_t pin) {
    if((port->IDR & pin) != 0x00U) {
        return true;
    } else {
        return false;
    }
}

#ifdef __cplusplus
}
#endif
