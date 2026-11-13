/**
 * @file furi_hal_power.h
 * Power HAL API
 */
#pragma once

#include "furi_hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Super-early initialization of power HAL
 */
void furi_hal_power_init_super_early(void);

void furi_hal_power_reset(void);

/**
 * @brief Clears the list of wakeup sources.
 * 
 * Call this function prior to others in the `sleep_wakeup_*` family.
 * 
 * The system will always wake up from the reset signal.
 */
void furi_hal_power_sleep_wakeup_clear(void);

/**
 * @brief Adds a GPIO wakeup source for deep sleep.
 * 
 * Call this function in between `sleep_wakeup_clear` and `deep_sleep`.
 * 
 * @param[in] wakeup_pin GPIO pin that will trigger a wakeup. Must be in the
 *                       UULP power domain (`GpioTypeUulp` type).
 * @param[in] wakeup_condition The signal edge that the chip will listen for.
 */
void furi_hal_power_sleep_wakeup_gpio(const GpioPin* wakeup_pin, GpioCondition wakeup_condition);

/**
 * @brief Puts the chip into the lowest power sleep mode.
 * 
 * Use the `sleep_wakeup_*` function family to configure wakeup sources prior to
 * calling `deep_sleep`.
 * 
 * After one of the sources gets triggered, the chip wakes up and Cortex-M4
 * (our core) starts executing code at the reset vector.
 */
FURI_NORETURN void furi_hal_power_deep_sleep(void);

typedef enum {
    FuriHalPowerResetSourceUnknown, //<! Couldn't determine reset source
    FuriHalPowerResetSourcePowerOn, //<! Power-on reset
    FuriHalPowerResetSourceResetLine, //<! Reset via RESET line or NVIC
    FuriHalPowerResetSourceWakeup, //<! Wake up from deep sleep
} FuriHalPowerResetSource;

/**
 * @brief Gets the reset source
 * 
 * See @c FuriHalPowerResetSource for description
 * 
 * @returns reset source during this power cycle
 */
FuriHalPowerResetSource furi_hal_power_get_reset_source(void);

#ifdef __cplusplus
}
#endif
