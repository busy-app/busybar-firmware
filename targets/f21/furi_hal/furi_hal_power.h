/**
 * @file furi_hal_power.h
 * Power HAL API
 */
#pragma once

#include <stdint.h>
#include "furi_hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Super-early initialization of power HAL
 */
void furi_hal_power_init_super_early(void);

void furi_hal_power_reset(void);

void furi_hal_power_reset_917(bool to_dfu);

/** Get current insomnia level
 *
 * @return     insomnia level: 0 - no insomnia, >0 - insomnia, bearer count.
 */
uint16_t furi_hal_power_insomnia_level(void);

/** Enter insomnia mode Prevents device from going to sleep
  * @warning    Internally increases insomnia level Must be paired with
  *             furi_hal_power_insomnia_exit
  */
void furi_hal_power_insomnia_enter(void);

/** Exit insomnia mode Allow device to go to sleep
  * @warning    Internally decreases insomnia level. Must be paired with
  *             furi_hal_power_insomnia_enter
  */
void furi_hal_power_insomnia_exit(void);

/** Check if sleep available
  *
  * @return     true if available
  */
bool furi_hal_power_sleep_available(void);

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
 * @param[in] wakeup_pin GPIO pin that will trigger a wakeup. Must be one of the
 *                       `WKUP` pins.
 * @param[in] wakeup_condition The signal edge that the chip will listen for.
 *                             Must be one of the `GpioModeInterruptX` values.
 */
void furi_hal_power_sleep_wakeup_gpio(const GpioPin* wakeup_pin, GpioMode wakeup_condition);

/**
 * @brief Puts the chip into the lowest power sleep mode.
 * 
 * Use the `sleep_wakeup_*` function family to configure wakeup sources prior to
 * calling `deep_sleep`.
 * 
 * After one of the sources gets triggered, the chip wakes up and the
 * processor core starts executing code at the reset vector.
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
