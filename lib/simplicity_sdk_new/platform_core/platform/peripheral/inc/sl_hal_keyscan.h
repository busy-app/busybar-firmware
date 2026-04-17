/***************************************************************************//**
 * @file
 * @brief Keyscan (KEYSCAN) peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_HAL_KEYSCAN_H
#define SL_HAL_KEYSCAN_H

#include "em_device.h"

#if defined(KEYSCAN_COUNT) && (KEYSCAN_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_hal_keyscan_compat.h"
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup keyscan KEYSCAN
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   ENUM   ************************************
 ******************************************************************************/

/// KEYSCAN configuration delay values
SL_ENUM(sl_hal_keyscan_delay_t) {
  SL_HAL_KEYSCAN_DELAY_2MS = 0, ///< 2 ms delay.
  SL_HAL_KEYSCAN_DELAY_4MS,   ///< 4 ms delay.
  SL_HAL_KEYSCAN_DELAY_6MS,   ///< 6 ms delay.
  SL_HAL_KEYSCAN_DELAY_8MS,   ///< 8 ms delay.
  SL_HAL_KEYSCAN_DELAY_10MS,  ///< 10 ms delay.
  SL_HAL_KEYSCAN_DELAY_12MS,  ///< 12 ms delay.
  SL_HAL_KEYSCAN_DELAY_14MS,  ///< 14 ms delay.
  SL_HAL_KEYSCAN_DELAY_16MS,  ///< 16 ms delay.
  SL_HAL_KEYSCAN_DELAY_18MS,  ///< 18 ms delay.
  SL_HAL_KEYSCAN_DELAY_20MS,  ///< 20 ms delay.
  SL_HAL_KEYSCAN_DELAY_22MS,  ///< 22 ms delay.
  SL_HAL_KEYSCAN_DELAY_24MS,  ///< 24 ms delay.
  SL_HAL_KEYSCAN_DELAY_26MS,  ///< 26 ms delay.
  SL_HAL_KEYSCAN_DELAY_28MS,  ///< 28 ms delay.
  SL_HAL_KEYSCAN_DELAY_30MS,  ///< 30 ms delay.
  SL_HAL_KEYSCAN_DELAY_32MS   ///< 32 ms delay.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// KEYSCAN configuration structure.
typedef struct {
  uint32_t            clock_divider;       ///< Clock divider value.
  uint8_t             column_number;       ///< Number of columns to set for keyscan (maximum 8).
  uint8_t             row_number;          ///< Number of rows  to set for keyscan (maximum 6).
  sl_hal_keyscan_delay_t  scan_delay;      ///< Scan delay.
  sl_hal_keyscan_delay_t  debounce_delay;  ///< Debounce delay.
  sl_hal_keyscan_delay_t  stable_delay;    ///< Stable delay.
  bool                single_press_enable; ///< Enable Single Press feature.
  bool                auto_start_enable;   ///< Enable auto-start feature.
} sl_hal_keyscan_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_keyscan_init_t sl_hal_keyscan_config_t;
/** @endcond */

/// Suggested default values for KEYSCAN configuration structure.
#define SL_HAL_KEYSCAN_INIT_DEFAULT                                      \
  {                                                                      \
    0x1387F,                  /* Clock divider default value = 79999. */ \
    3u,                       /* 3 columns by default. */                \
    6u,                       /* 6 rows by default. */                   \
    SL_HAL_KEYSCAN_DELAY_2MS, /* value 0 = 2ms Scan Delay*/              \
    SL_HAL_KEYSCAN_DELAY_2MS, /* value 0 = 2ms Debounce Delay */         \
    SL_HAL_KEYSCAN_DELAY_2MS, /* value 0 = 2ms Row Stable Delay */       \
    false,                    /* Multi-press by default.*/               \
    false,                    /* No auto-start by default. */            \
  }

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro name used for backward compatibility purposes.
#define KEYSCAN_CONFIG_DEFAULT SL_HAL_KEYSCAN_INIT_DEFAULT
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initializes KEYSCAN module.
 *
 * @param[in] init
 *  A pointer to the KEYSCAN initialization structure variable.
 ******************************************************************************/
void sl_hal_keyscan_init(const sl_hal_keyscan_init_t *init);

/***************************************************************************//**
 * @brief
 *   Enables KEYSCAN module.
 ******************************************************************************/
void sl_hal_keyscan_enable(void);

/***************************************************************************//**
 * @brief
 *   Disables KEYSCAN module.
 *
 * @note
 *   The disabling of the module could take some time. This function will
 *   not wait for the disabling to finish before returning. Use the function
 *   sl_hal_keyscan_wait_ready to wait for the module to be fully disable.
 ******************************************************************************/
void sl_hal_keyscan_disable(void);

/***************************************************************************//**
 * @brief
 *    Waits for the KEYSCAN to complete resetting or disabling procedure.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_wait_ready(void)
{
  while ((KEYSCAN->SWRST & _KEYSCAN_SWRST_RESETTING_MASK) || (KEYSCAN->EN & _KEYSCAN_EN_DISABLING_MASK) || (KEYSCAN->STATUS & _KEYSCAN_STATUS_SYNCBUSY_MASK)) {
    // Wait for resetting, for disabling or for all synchronizations to finish
  }
}

/***************************************************************************//**
 * @brief
 *   Waits for the KEYSCAN to complete all synchronization of register changes
 *   and commands.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_wait_sync(void)
{
  while ((KEYSCAN->EN != 0U) && (KEYSCAN->STATUS & KEYSCAN_STATUS_SYNCBUSY)) {
    // Wait for all synchronizations to finish
  }
}

/***************************************************************************//**
 * @brief
 *   Starts KEYSCAN scan.
 *
 * @note
 *   This function will send a start command to the KEYSCAN peripheral.
 *   The sl_hal_keyscan_wait_sync function can be used to wait for the start
 *   command to be executed.
 *
 * @note
 *   This function requires the KEYSCAN to be enabled.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_start_scan(void)
{
  sl_hal_keyscan_wait_sync();
  KEYSCAN->CMD = KEYSCAN_CMD_KEYSCANSTART;
  sl_hal_keyscan_wait_ready();
}

/***************************************************************************//**
 * @brief
 *   Stops the KEYSCAN scan.
 *
 * @note
 *   This function will send a stop command to the KEYSCAN peripheral.
 *   The sl_hal_keyscan_wait_sync function can be used to wait for the stop
 *   command to be executed.
 *
 * @note
 *   This function requires the KEYSCAN to be enabled.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_stop_scan(void)
{
  sl_hal_keyscan_wait_sync();
  KEYSCAN->CMD = KEYSCAN_CMD_KEYSCANSTOP;
  sl_hal_keyscan_wait_ready();
}

/***************************************************************************//**
 * @brief
 *   Restores KEYSCAN to its reset state.
 *
 * @note
 *   The resetting of the module could take some time. This function will
 *   not wait for the resetting to finish before returning. Use the function
 *   sl_hal_keyscan_wait_ready to wait for the module to be fully reset.
 ******************************************************************************/
void sl_hal_keyscan_reset(void);

/***************************************************************************//**
 * @brief
 *   Gets KEYSCAN STATUS register value.
 *
 * @return
 *   Current STATUS register value.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_hal_keyscan_get_status(void)
{
  return KEYSCAN->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Enables one or more KEYSCAN interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *        enabling the interrupt. To ignore a pending interrupt, consider using
 *        sl_hal_keyscan_clear_interrupts prior to enabling the interrupt.
 *
 * @param[in] flags
 *   KEYSCAN interrupt sources to enable. Use a set of interrupt flags
 *   OR-ed together to set multiple interrupt sources.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_enable_interrupts(uint32_t flags)
{
  KEYSCAN->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disables one or more KEYSCAN interrupts.
 *
 * @param[in] flags
 *   KEYSCAN interrupt sources to disable.
 *   Use a set of interrupt flags OR-ed together to disable
 *   multiple interrupt sources.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_disable_interrupts(uint32_t flags)
{
  KEYSCAN->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Clears one or more pending KEYSCAN interrupts.
 *
 * @param[in] flags
 *   KEYSCAN interrupt sources to clear.
 *   Use a set of interrupt flags OR-ed together to clear
 *   multiple interrupt sources.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_clear_interrupts(uint32_t flags)
{
  KEYSCAN->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Gets pending KEYSCAN interrupt flags.
 *
 * @note
 *   Event bits are not cleared by using this function.
 *
 * @return
 *   Pending KEYSCAN interrupt sources.
 *   Returns a set of interrupt flags OR-ed together for multiple
 *   interrupt sources.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_hal_keyscan_get_interrupts(void)
{
  return KEYSCAN->IF;
}

/***************************************************************************//**
 * @brief
 *   Gets enabled and pending KEYSCAN interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note
 *   Interrupt flags are not cleared by using this function.
 *
 * @return
 *   Pending and enabled KEYSCAN interrupt sources.
 *   The return value is the bitwise AND of
 *   - the enabled interrupt sources in KEYSCAN_IEN and
 *   - the pending interrupt flags KEYSCAN_IF.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_hal_keyscan_get_enabled_interrupts(void)
{
  return KEYSCAN->IF & KEYSCAN->IEN;
}

/***************************************************************************//**
 * @brief
 *   Sets one or more pending KEYSCAN interrupts from Software.
 *
 * @param[in] flags
 *   KEYSCAN interrupt sources to set to pending.
 *   Use a set of interrupt flags OR-ed together to set
 *   multiple interrupt sources.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_keyscan_set_interrupts(uint32_t flags)
{
  KEYSCAN->IF_SET = flags;
}

/** @} (end addtogroup keyscan) */

#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup keyscan KEYSCAN
 * @{
 *
 * @n @section keyscan_example Example
 * The following example demonstrates how to configure and use the KEYSCAN module
 * to detect key presses on a keypad matrix.
 *
 * @code{.c}
 * // Global variables for key press tracking
 * volatile bool key_pressed = false;
 * volatile uint8_t detected_key = 0xFF;
 *
 * // Interrupt handler for key press detection
 * void KEYSCAN_IRQHandler(void)
 * {
 *   uint32_t flags = sl_hal_keyscan_get_interrupts();
 *
 *   if (flags & KEYSCAN_IF_KEY) {
 *     // Read the pressed key data
 *     uint32_t key_data = KEYSCAN->KEYDATA;
 *     uint8_t row = (key_data & _KEYSCAN_KEYDATA_ROW_MASK) >> _KEYSCAN_KEYDATA_ROW_SHIFT;
 *     uint8_t col = (key_data & _KEYSCAN_KEYDATA_COL_MASK) >> _KEYSCAN_KEYDATA_COL_SHIFT;
 *
 *     // Calculate key index (assuming 3 columns)
 *     detected_key = row * 3 + col;
 *     key_pressed = true;
 *
 *     // Clear the interrupt flag
 *     sl_hal_keyscan_clear_interrupts(KEYSCAN_IF_KEY);
 *   }
 * }
 *
 * // Example of configuring and using KEYSCAN
 * void keyscan_example(void)
 * {
 *   // Enable bus clock for KEYSCAN
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_KEYSCAN);
 *
 *   // Configure KEYSCAN with custom settings
 *   sl_hal_keyscan_init_t keyscan_init = SL_HAL_KEYSCAN_INIT_DEFAULT;
 *
 *   // Configure for a 3x4 keypad
 *   keyscan_init.column_number = 3;
 *   keyscan_init.row_number = 4;
 *
 *   // Increase debounce delay to 20ms for improved reliability
 *   keyscan_init.debounce_delay = SL_HAL_KEYSCAN_DELAY_20MS;
 *
 *   // Initialize KEYSCAN with our configuration
 *   sl_hal_keyscan_init(&keyscan_init);
 *
 *   // Enable KEY interrupt
 *   sl_hal_keyscan_enable_interrupts(KEYSCAN_IEN_KEY);
 *
 *   // Enable NVIC interrupt for KEYSCAN
 *   sl_interrupt_manager_enable_irq(KEYSCAN_IRQn);
 *
 *   // Enable KEYSCAN peripheral
 *   sl_hal_keyscan_enable();
 *   sl_hal_keyscan_wait_sync();
 *
 *   // Start keypad scanning
 *   sl_hal_keyscan_start_scan();
 *
 *   // At this point, the KEYSCAN peripheral will automatically scan the keypad
 *   // and generate interrupts when keys are pressed. The interrupt handler will
 *   // process these events.
 *
 *   // Main application code would go here
 *   // ...
 *
 *   // When done with scanning, clean up
 *   sl_hal_keyscan_stop_scan();
 *   sl_hal_keyscan_disable_interrupts(KEYSCAN_IEN_KEY);
 *   sl_hal_keyscan_clear_interrupts(KEYSCAN_IF_KEY);
 *   sl_hal_keyscan_disable();
 *   sl_hal_keyscan_wait_ready();
 *   sl_hal_keyscan_reset();
 *   sl_hal_keyscan_wait_ready();
 * }
 * @endcode
 * @} (end addtogroup keyscan)
 ******************************************************************************/

#endif /* defined(KEYSCAN_COUNT) && (KEYSCAN_COUNT > 0) */
#endif /* SL_HAL_KEYSCAN_H */
