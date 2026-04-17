/***************************************************************************//**
 * @file
 * @brief Back-Up Real Time Counter (BURTC) peripheral API
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

#ifndef SL_HAL_BURTC_H
#define SL_HAL_BURTC_H

#include "em_device.h"

#if defined(BURTC_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/***************************************************************************//**
 * @addtogroup burtc BURTC - Back-Up Real Time Counter
 * @brief Back-up Real Time Counter (BURTC) Peripheral API
 *
 * @details
 *  This module contains functions to control the BURTC peripheral.
 *  The Backup Real Time Counter allows timekeeping in all energy modes.
 *  The Backup RTC is also available when the system is in backup mode.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// BURTC initialization structure.
typedef struct {
  uint32_t             clock_divider;   ///< Clock divider. Supported range is 1-32768
  bool                 debug_run;       ///< If true, counter will keep running under debug halt
  bool                 compare0_top;    ///< Set if Compare Value 0 is also top value (counter restart)
  bool                 em4_comparator;  ///< Enable EM4 wakeup on compare match.
  bool                 em4_overflow;    ///< Enable EM4 wakeup on counter overflow.
} sl_hal_burtc_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_burtc_init_t sl_hal_burtc_init_config_t;
/** @endcond */

/*******************************************************************************
 **************************   STRUCT INITIALIZERS   ****************************
 ******************************************************************************/

/**
 * @brief
 *   Default configuration for BURTC init structure.
 */
#define SL_HAL_BURTC_INIT_DEFAULT \
  {                               \
    1,                            \
    false,                        \
    false,                        \
    false,                        \
    false,                        \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize the Back-Up RTC.
 *
 * @param[in] init
 *    A pointer to the initialization structure used to configure the BURTC.
 ******************************************************************************/
void sl_hal_burtc_init(const sl_hal_burtc_init_t *init);

/***************************************************************************//**
 * @brief
 *   Reset the Back-Up RTC.
 ******************************************************************************/
void sl_hal_burtc_reset(void);

/***************************************************************************//**
 * @brief
 *   Enable the Back-Up RTC.
 ******************************************************************************/
void sl_hal_burtc_enable(void);

/***************************************************************************//**
 * @brief
 *   Disable the Back-Up RTC.
 ******************************************************************************/
void sl_hal_burtc_disable(void);

/***************************************************************************//**
 * @brief
 *   Reset the Back-Up RTC counter.
 ******************************************************************************/
void sl_hal_burtc_reset_counter(void);

/***************************************************************************//**
 * @brief
 *   Set the Back-Up RTC compare register.
 *
 * @param[in] value
 *   Back-Up RTC compare register value.
 ******************************************************************************/
void sl_hal_burtc_set_compare(uint32_t value);

/***************************************************************************//**
 * @brief
 *   Get the Back-Up RTC compare register value.
 *
 * @return
 *   return the Back-Up RTC compare register value.
 ******************************************************************************/
uint32_t sl_hal_burtc_get_compare(void);

/***************************************************************************//**
 * @brief
 *   Clear one or more pending BURTC interrupts.
 *
 * @param[in] flags
 *   BURTC interrupt sources to clear. Use a set of interrupt flags OR-ed
 *   together to clear multiple interrupt sources for the BURTC module
 *   (BURTC_IF_CLR_nnn).
 ******************************************************************************/
__INLINE void sl_hal_burtc_clear_interrupts(uint32_t flags)
{
  BURTC->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more BURTC interrupts.
 *
 * @param[in] flags
 *   BURTC interrupt sources to disable. Use a set of interrupt flags OR-ed
 *   together to disable multiple interrupt sources for the BURTC module.
 ******************************************************************************/
__INLINE void sl_hal_burtc_disable_interrupts(uint32_t flags)
{
  BURTC->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more BURTC interrupts.
 *
 * @note
 *   Depending on use, a pending interrupt may already be set prior to
 *   enabling the interrupt. Consider using sl_hal_burtc_clear_interrupts()
 *   prior to enabling, if a pending interrupt should be ignored.
 *
 * @param[in] flags
 *   BURTC interrupt sources to enable. Use a set of interrupt flags OR-ed
 *   together to set multiple interrupt sources for the BURTC module.
 ******************************************************************************/
__INLINE void sl_hal_burtc_enable_interrupts(uint32_t flags)
{
  BURTC->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending BURTC interrupt flags.
 *
 * @note
 *   This function does not clear the event bits.
 *
 * @return
 *   Pending BURTC interrupt sources. Returns a set of interrupt flags OR-ed
 *   together for multiple interrupt sources in the BURTC module.
 ******************************************************************************/
__INLINE uint32_t sl_hal_burtc_get_pending_interrupts(void)
{
  return BURTC->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending BURTC interrupt flags.
 *
 * @note
 *   The event bits are not cleared by the use of this function.
 *
 * @return
 *   Pending BURTC interrupt sources that is also enabled. Returns a set of
 *   interrupt flags OR-ed together for multiple interrupt sources in the
 *   BURTC module.
 ******************************************************************************/
__INLINE uint32_t sl_hal_burtc_get_enabled_pending_interrupts(void)
{
  uint32_t tmp;

  /* Get enabled interrupts */
  tmp = BURTC->IEN;

  /* Return set interrupts */
  return BURTC->IF & tmp;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending BURTC interrupts from SW.
 *
 * @param[in] flags
 *   BURTC interrupt sources to set to pending. Use a set of interrupt flags
 *   OR-ed together to set multiple interrupt sources for the BURTC module.
 ******************************************************************************/
__INLINE void sl_hal_burtc_set_interrupts(uint32_t flags)
{
  BURTC->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Status of BURTC RAM, timestamp and LP Mode.
 *
 * @return A mask logically OR-ed status bits.
 ******************************************************************************/
__INLINE uint32_t sl_hal_burtc_get_status(void)
{
  return BURTC->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Wait for the BURTC to complete all synchronization of register changes
 *   and commands.
 ******************************************************************************/
__INLINE void sl_hal_burtc_wait_sync(void)
{
  while ((BURTC->EN != 0U) && (BURTC->SYNCBUSY != 0U)) {
    /* Wait for previous synchronization to finish */
  }
}

/***************************************************************************//**
 * @brief
 *   Start BURTC counter.
 *
 *   This function will send a start command to the BURTC peripheral. The BURTC
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_burtc_wait_sync() function can be used to wait for the
 *   start command to be executed.
 *
 * @note
 *   This function requires the BURTC to be enabled.
 ******************************************************************************/
__INLINE void sl_hal_burtc_start(void)
{
  sl_hal_burtc_wait_sync();
  BURTC->CMD = BURTC_CMD_START;
}

/***************************************************************************//**
 * @brief
 *   Stop the BURTC counter.
 *
 *   This function will send a stop command to the BURTC peripheral. The BURTC
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_burtc_wait_sync() function can be used to wait for the stop
 *   command to be executed.
 *
 * @note
 *   This function requires the BURTC to be enabled.
 ******************************************************************************/
__INLINE void sl_hal_burtc_stop(void)
{
  sl_hal_burtc_wait_sync();
  BURTC->CMD = BURTC_CMD_STOP;
}

/***************************************************************************//**
 * @brief Get BURTC counter.
 *
 * @return
 *   BURTC counter value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_burtc_get_counter(void)
{
  return BURTC->CNT;
}

/***************************************************************************//**
 * @brief
 *   Lock BURTC registers, which will protect from writing new config settings.
 ******************************************************************************/
__INLINE void sl_hal_burtc_lock(void)
{
  BURTC->LOCK = 0x0;
}

/***************************************************************************//**
 * @brief
 *   Unlock BURTC registers, which will enable write access to change
 *   configuration.
 ******************************************************************************/
__INLINE void sl_hal_burtc_unlock(void)
{
  BURTC->LOCK = BURTC_LOCK_LOCKKEY_UNLOCK;
}

/** @} (end addtogroup burtc) */

#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup burtc BURTC - Back-Up Real Time Counter
 * @{
 *
 *@n @section burtc_example Example
 *  This example demonstrates initialization, basic configuration, and usage of the
 *  BURTC peripheral. It shows how to:
 *  - Initialize the BURTC with default settings
 *  - Start and stop the counter
 *  - Configure and read the compare register value
 *
 * @code{.c}
 * #include "sl_hal_burtc.h"
 *
 * void burtc_example(void)
 * {
 *   // Enable bus clock for BURTC
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);
 *
 *   // Initialize configuration structure with default settings
 *   sl_hal_burtc_init_t init = SL_HAL_BURTC_INIT_DEFAULT;
 *
 *   // Initialize BURTC with default settings
 *   sl_hal_burtc_init(&init);
 *
 *   // Enable and start the BURTC counter
 *   sl_hal_burtc_enable();
 *   sl_hal_burtc_start();
 *
 *   // Set compare value for generating an interrupt
 *   sl_hal_burtc_set_compare(1000);  // Set to 1000 counts
 *
 *   // Get the current compare value
 *   uint32_t compare_value = sl_hal_burtc_get_compare();
 *
 *   // Later, when done using the BURTC
 *   sl_hal_burtc_stop();
 *   sl_hal_burtc_disable();
 * }
 * @endcode
 *
 * @} (end addtogroup burtc)
 ******************************************************************************/

#endif /* defined( BURTC_PRESENT ) && ( BURTC_COUNT == 1 ) */
#endif /* SL_HAL_BURTC_H */
