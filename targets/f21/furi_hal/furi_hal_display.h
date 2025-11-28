#pragma once
#include "furi_hal_gpio.h"

/**
 * @brief Initialize the front display power pin.
 */
void furi_hal_display_power_pin_init(void);

/**
 * @brief Read the state of the front display power pin.
 * @return true if the power is enabled, false otherwise.
 * @note Pin can be pulled low by external temperature protection circuit. 
 */
bool furi_hal_display_power_pin_read(void);

/**
 * @brief Attach a callback to the front display power pin interrupt.
 * @param cb Callback function to be called on interrupt.
 * @param ctx Context to be passed to the callback.
 */
void furi_hal_display_power_pin_attach_callback(GpioExtiCallback cb, void* ctx);

/**
 * @brief Enable the front display power pin interrupt.
 */
void furi_hal_display_power_pin_interrupt_enable(void);

/**
 * @brief Disable the front display power pin interrupt.
 */
void furi_hal_display_power_pin_interrupt_disable(void);

/**
 * @brief Enable the front display power.
 */
void furi_hal_display_power_enable(void);

/**
 * @brief Disable the front display power.
 */
void furi_hal_display_power_disable(void);
