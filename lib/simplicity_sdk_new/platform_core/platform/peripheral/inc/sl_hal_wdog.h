/***************************************************************************//**
 * @file
 * @brief Watchdog (WDOG) peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_HAL_WDOG_H
#define SL_HAL_WDOG_H

#include "em_device.h"
#if defined(WDOG_COUNT) && (WDOG_COUNT > 0)

#include <stdbool.h>
#include "sl_common.h"
#include "sl_enum.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup  wdog WDOG - Watchdog Timer
 * @brief Watchdog timer peripheral API
 *
 * @details The Watchdog API functions provide full support for the WDOG peripheral.
 *          The WDOG resets the system in case of a fault condition.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Check if WDOG instance is valid.
#define SL_HAL_WDOG_REF_VALID(ref)    (WDOG_NUM(ref) != -1)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Watchdog timeout period selection.
SL_ENUM(sl_hal_wdog_period_select_t) {
  SL_WDOG_PERIOD_9    = _WDOG_CFG_PERSEL_SEL0,  ///< 9 clock periods.
  SL_WDOG_PERIOD_17   = _WDOG_CFG_PERSEL_SEL1,  ///< 17 clock periods.
  SL_WDOG_PERIOD_33   = _WDOG_CFG_PERSEL_SEL2,  ///< 33 clock periods.
  SL_WDOG_PERIOD_65   = _WDOG_CFG_PERSEL_SEL3,  ///< 65 clock periods.
  SL_WDOG_PERIOD_129  = _WDOG_CFG_PERSEL_SEL4,  ///< 129 clock periods.
  SL_WDOG_PERIOD_257  = _WDOG_CFG_PERSEL_SEL5,  ///< 257 clock periods.
  SL_WDOG_PERIOD_513  = _WDOG_CFG_PERSEL_SEL6,  ///< 513 clock periods.
  SL_WDOG_PERIOD_1k   = _WDOG_CFG_PERSEL_SEL7,  ///< 1025 clock periods.
  SL_WDOG_PERIOD_2k   = _WDOG_CFG_PERSEL_SEL8,  ///< 2049 clock periods.
  SL_WDOG_PERIOD_4k   = _WDOG_CFG_PERSEL_SEL9,  ///< 4097 clock periods.
  SL_WDOG_PERIOD_8k   = _WDOG_CFG_PERSEL_SEL10, ///< 8193 clock periods.
  SL_WDOG_PERIOD_16k  = _WDOG_CFG_PERSEL_SEL11, ///< 16385 clock periods.
  SL_WDOG_PERIOD_32k  = _WDOG_CFG_PERSEL_SEL12, ///< 32769 clock periods.
  SL_WDOG_PERIOD_64k  = _WDOG_CFG_PERSEL_SEL13, ///< 65537 clock periods.
  SL_WDOG_PERIOD_128k = _WDOG_CFG_PERSEL_SEL14, ///< 131073 clock periods.
  SL_WDOG_PERIOD_256k = _WDOG_CFG_PERSEL_SEL15  ///< 262145 clock periods.
};

/// Select Watchdog warning timeout period as percentage of timeout.
SL_ENUM(sl_hal_wdog_warning_timeout_select_t) {
  SL_WDOG_WARNING_DISABLE = _WDOG_CFG_WARNSEL_DIS,  ///< Watchdog warning period is disabled.
  SL_WDOG_WARNING_TIME25  = _WDOG_CFG_WARNSEL_SEL1, ///< Watchdog warning period is 25% of the timeout.
  SL_WDOG_WARNING_TIME50  = _WDOG_CFG_WARNSEL_SEL2, ///< Watchdog warning period is 50% of the timeout.
  SL_WDOG_WARNING_TIME75  = _WDOG_CFG_WARNSEL_SEL3, ///< Watchdog warning period is 75% of the timeout.
};

///  Select Watchdog illegal window limit.
SL_ENUM(sl_hal_wdog_illegal_window_select_t) {
  SL_WDOG_ILLEGAL_WINDOW_DISABLE  = _WDOG_CFG_WINSEL_DIS,  ///< Watchdog illegal window disabled.
  SL_WDOG_ILLEGAL_WINDOW_TIME12_5 = _WDOG_CFG_WINSEL_SEL1, ///< Window timeout is 12.5% of the timeout.
  SL_WDOG_ILLEGAL_WINDOW_TIME25_0 = _WDOG_CFG_WINSEL_SEL2, ///< Window timeout is 25% of the timeout.
  SL_WDOG_ILLEGAL_WINDOW_TIME37_5 = _WDOG_CFG_WINSEL_SEL3, ///< Window timeout is 37.5% of the timeout.
  SL_WDOG_ILLEGAL_WINDOW_TIME50_0 = _WDOG_CFG_WINSEL_SEL4, ///< Window timeout is 50% of the timeout.
  SL_WDOG_ILLEGAL_WINDOW_TIME62_5 = _WDOG_CFG_WINSEL_SEL5, ///< Window timeout is 62.5% of the timeout.
  SL_WDOG_ILLEGAL_WINDOW_TIME75_0 = _WDOG_CFG_WINSEL_SEL6, ///< Window timeout is 75% of the timeout.
  SL_WDOG_ILLEGAL_WINDOW_TIME87_5 = _WDOG_CFG_WINSEL_SEL7, ///< Window timeout is 87.5% of the timeout.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// Watchdog initialization structure.
typedef struct {
  /// Counter keeps running during debug halt.
  bool                                 debug_run;

  /// Select WDOG clear source:
  /// False: Write to the clear bit will clear the WDOG counter.
  /// True: Rising edge on the PRS Source 0 will clear the WDOG counter.
  bool                                 clear_source;
#if defined(_WDOG_CFG_EM1RUN_MASK)
  /// Counter keeps running when in EM1.
  bool                                 em1_run;
#endif
#if defined(_WDOG_CFG_EM2RUN_MASK)
  /// Counter keeps running when in EM2.
  bool                                 em2_run;
#endif

#if defined(_WDOG_CFG_EM3RUN_MASK)
  /// Counter keeps running when in EM3.
  bool                                 em3_run;
#endif

  /// Block EMU from entering EM4.
  bool                                 em4_block;

  /// When set, a PRS Source 0 missing event will trigger a WDOG reset.
  bool                                 prs0_missing_reset_enable;

  /// When set, a PRS Source 1 missing event will trigger a WDOG reset.
  bool                                 prs1_missing_reset_enable;

  /// Block SW from modifying the configuration (a reset is needed to reconfigure).
  bool                                 lock;

  /// Watchdog timeout period.
  sl_hal_wdog_period_select_t          period_select;

  /// Select warning time as % of the Watchdog timeout.
  sl_hal_wdog_warning_timeout_select_t warning_time_select;

  /// Select illegal window time as % of the Watchdog timeout.
  sl_hal_wdog_illegal_window_select_t  window_time_select;

  /// Disable Watchdog reset output if true.
  bool                                 reset_disable;
} sl_hal_wdog_init_t;

#if !defined(_WDOG_CFG_EM2RUN_MASK) && !defined(_WDOG_CFG_EM3RUN_MASK) && defined(_WDOG_CFG_EM1RUN_MASK)
/// Suggested default configuration for WDOG initialization structure.
#define SL_HAL_WDOG_INIT_DEFAULT                                                                    \
  {                                                                                                 \
    false,                          /* WDOG is not counting during debug halt. */                   \
    false,                          /* The clear bit will clear the WDOG counter. */                \
    false,                          /* WDOG is not counting when in EM1. */                         \
    false,                          /* EM4 can be entered. */                                       \
    false,                          /* PRS Source 0 missing event will not trigger a WDOG reset. */ \
    false,                          /* PRS Source 1 missing event will not trigger a WDOG reset. */ \
    false,                          /* Do not lock WDOG configuration. */                           \
    SL_WDOG_PERIOD_256k,            /* Set longest possible timeout period. */                      \
    SL_WDOG_WARNING_DISABLE,        /* Disable warning interrupt. */                                \
    SL_WDOG_ILLEGAL_WINDOW_DISABLE, /* Disable illegal window interrupt. */                         \
    false                           /* Do not disable reset. */                                     \
  }
#elif defined(_WDOG_CFG_EM2RUN_MASK) && defined(_WDOG_CFG_EM3RUN_MASK) && defined(_WDOG_CFG_EM1RUN_MASK)
/// Suggested default configuration for WDOG initialization structure.
#define SL_HAL_WDOG_INIT_DEFAULT                                                                    \
  {                                                                                                 \
    false,                          /* WDOG is not counting during debug halt. */                   \
    false,                          /* The clear bit will clear the WDOG counter. */                \
    false,                          /* WDOG is not counting when in EM1. */                         \
    false,                          /* WDOG is not counting when in EM2. */                         \
    false,                          /* WDOG is not counting when in EM3. */                         \
    false,                          /* EM4 can be entered. */                                       \
    false,                          /* PRS Source 0 missing event will not trigger a WDOG reset. */ \
    false,                          /* PRS Source 1 missing event will not trigger a WDOG reset. */ \
    false,                          /* Do not lock WDOG configuration. */                           \
    SL_WDOG_PERIOD_256k,            /* Set longest possible timeout period. */                      \
    SL_WDOG_WARNING_DISABLE,        /* Disable warning interrupt. */                                \
    SL_WDOG_ILLEGAL_WINDOW_DISABLE, /* Disable illegal window interrupt. */                         \
    false                           /* Do not disable reset. */                                     \
  }
#elif defined(_WDOG_CFG_EM2RUN_MASK) && defined(_WDOG_CFG_EM3RUN_MASK) && !defined(_WDOG_CFG_EM1RUN_MASK)
/// Suggested default configuration for WDOG initialization structure.
#define SL_HAL_WDOG_INIT_DEFAULT                                                                    \
  {                                                                                                 \
    false,                          /* WDOG is not counting during debug halt. */                   \
    false,                          /* The clear bit will clear the WDOG counter. */                \
    false,                          /* WDOG is not counting when in EM2. */                         \
    false,                          /* WDOG is not counting when in EM3. */                         \
    false,                          /* EM4 can be entered. */                                       \
    false,                          /* PRS Source 0 missing event will not trigger a WDOG reset. */ \
    false,                          /* PRS Source 1 missing event will not trigger a WDOG reset. */ \
    false,                          /* Do not lock WDOG configuration. */                           \
    SL_WDOG_PERIOD_256k,            /* Set longest possible timeout period. */                      \
    SL_WDOG_WARNING_DISABLE,        /* Disable warning interrupt. */                                \
    SL_WDOG_ILLEGAL_WINDOW_DISABLE, /* Disable illegal window interrupt. */                         \
    false                           /* Do not disable reset. */                                     \
  }
#endif
/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Enable the watchdog peripheral.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @note
 *   This function modifies the WDOG CTRL register which requires
 *   synchronization into the low-frequency domain. If this register is
 *   modified before a previous update to the same register has completed,
 *   this function will stall until the previous synchronization has completed.
 ******************************************************************************/
void sl_hal_wdog_enable(WDOG_TypeDef *wdog);

/***************************************************************************//**
 * @brief
 *   Disable the watchdog peripheral.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @note
 *   This function modifies the WDOG CTRL register which requires
 *   synchronization into the low-frequency domain. If this register is
 *   modified before a previous update to the same register has completed,
 *   this function will stall until the previous synchronization has completed.
 ******************************************************************************/
void sl_hal_wdog_disable(WDOG_TypeDef *wdog);

/***************************************************************************//**
 * @brief
 *   Feed the watchdog peripheral.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @details
 *   When WDOG is activated, it must be fed (i.e., clearing the counter)
 *   before it reaches the defined timeout period. Otherwise, WDOG
 *   will generate a reset.
 *
 * @note
 *   Note that WDOG is an asynchronous peripheral and when calling the
 *   sl_hal_wdog_feed() function the hardware starts the process of clearing the
 *   counter. This process takes some time before it completes depending on the
 *   selected oscillator (up to 4 peripheral clock cycles). When using the
 *   ULFRCO for instance as the oscillator the watchdog runs on a 1 kHz clock
 *   and a watchdog clear operation might take up to 4 ms.
 *
 *   If the device supports EM2 or EM3 and it enters EM2 or EM3 while a command
 *   is in progress then that command will be aborted. An application can use
 *   @ref sl_hal_wdog_sync_wait() to wait for a command to complete.
 ******************************************************************************/
void sl_hal_wdog_feed(WDOG_TypeDef *wdog);

/***************************************************************************//**
 * @brief
 *   Initialize watchdog (assuming the watchdog configuration has not been locked).
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @param[in] init
 *   The structure holding the WDOG configuration.
 *                 A default setting #WDOG_INIT_DEFAULT is available
 *                 for initialization.
 *
 * @note
 *   This function modifies the WDOG CTRL register which requires
 *   synchronization into the low-frequency domain. If this register is modified
 *   before a previous update to the same register has completed, this function
 *   will stall until the previous synchronization has completed.
 ******************************************************************************/
void sl_hal_wdog_init(WDOG_TypeDef *wdog,
                      const sl_hal_wdog_init_t *init);

/***************************************************************************//**
 * @brief
 *   Lock the watchdog configuration.
 *
 * @param[in] wdog
 *   Pointer to WDOG peripheral register block.
 *
 * @details
 *   This prevents errors from overwriting the WDOG configuration, possibly
 *   disabling it. Only a reset can unlock the WDOG configuration once locked.
 *
 *   If the LFRCO or LFXO clocks are used to clock WDOG,
 *   consider using the option of inhibiting those clocks to be disabled.
 *   See the #WDOG_INIT_DEFAULT initialization structure.
 *
 * @note
 *   This function modifies the WDOG CTRL register which requires
 *   synchronization into the low-frequency domain. If this register is modified
 *   before a previous update to the same register has completed, this function
 *   will stall until the previous synchronization has completed.
 ******************************************************************************/
__INLINE void sl_hal_wdog_lock(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  wdog->LOCK = _WDOG_LOCK_LOCKKEY_LOCK;
}

/***************************************************************************//**
 * @brief
 *   Wait for the watchdog to complete all synchronization of register changes
 *   and commands.
 *
 * @param[in] wdog
 *   Pointer to WDOG peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_wdog_wait_sync(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  // Wait for synchronization to finish.
  while ((wdog->EN != 0U) && (wdog->SYNCBUSY != 0U)) ;
}

/***************************************************************************//**
 * @brief
 *   Wait for the watchdog to be completely disabled after calling
 *   sl_hal_wdog_disable().
 *
 * @param[in] wdog
 *   Pointer to WDOG peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_wdog_wait_ready(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

#if defined(_WDOG_EN_DISABLING_MASK)
  while (wdog->EN & _WDOG_EN_DISABLING_MASK) {
    // Wait for WDOG to be disabled.
  }
#else
  while (wdog->SYNCBUSY & WDOG_SYNCBUSY_CMD) {
    // Wait for WDOG to be ready.
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Unlock the watchdog configuration.
 *
 * @param[in] wdog
 *   Pointer to WDOG peripheral register block.
 *
 * @details
 *   Note that this function will have no effect on devices where a reset is
 *   the only way to unlock the watchdog.
 ******************************************************************************/
__INLINE void sl_hal_wdog_unlock(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  wdog->LOCK = _WDOG_LOCK_LOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending watchdog interrupts.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @param[in] flags
 *   WDOG interrupt sources to clear. Use a set of interrupt
 *   flags OR-ed together to clear multiple interrupt sources.
 ******************************************************************************/
__INLINE void sl_hal_wdog_clear_interrupts(WDOG_TypeDef *wdog,
                                           uint32_t flags)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  wdog->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more watchdog interrupts.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @param[in] flags
 *   WDOG interrupt sources to disable. Use a set of interrupt
 *   flags OR-ed together to disable multiple interrupt.
 ******************************************************************************/
__INLINE void sl_hal_wdog_disable_interrupts(WDOG_TypeDef *wdog,
                                             uint32_t flags)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  wdog->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more watchdog interrupts.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @param[in] flags
 *   WDOG interrupt sources to enable. Use a set of interrupt
 *   flags OR-ed together to set multiple interrupt.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   sl_hal_wdog_clear_interrupts() prior to enabling the interrupt.
 ******************************************************************************/
__INLINE void sl_hal_wdog_enable_interrupts(WDOG_TypeDef *wdog,
                                            uint32_t flags)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  wdog->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending watchdog interrupt flags.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @return
 *   Pending WDOG interrupt sources. Returns a set of interrupt flags OR-ed
 *   together for the interrupt sources set.
 *
 * @note
 *   The event bits are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_wdog_get_pending_interrupts(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  return wdog->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending watchdog interrupt flags.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @return
 *   Pending and enabled WDOG interrupt sources. Returns a set of interrupt
 *   flags OR-ed together for the interrupt sources set.
 *
 * @details
 *   Useful for handling more interrupt sources in the same interrupt handler.
 ******************************************************************************/
__INLINE uint32_t sl_hal_wdog_get_enabled_pending_interrupts(WDOG_TypeDef *wdog)
{
  uint32_t tmp;

  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  tmp = wdog->IEN;

  // Bitwise AND of pending and enabled interrupt flags.
  return wdog->IF & tmp;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending watchdog interrupts from SW.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @param[in] flags
 *   WDOG interrupt sources to set to pending.
 *   Use a set of interrupt flags (WDOG_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_wdog_set_interrupts(WDOG_TypeDef *wdog,
                                         uint32_t flags)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  wdog->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get enabled status of the watchdog.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @return
 *   True if Watchdog is enabled, false otherwise.
 ******************************************************************************/
__INLINE bool sl_hal_wdog_is_enabled(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  return (wdog->EN & _WDOG_EN_EN_MASK) == WDOG_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Get locked status of the watchdog.
 *
 * @param[in] wdog
 *   Pointer to the WDOG peripheral register block.
 *
 * @return
 *   True if Watchdog is locked, false otherwise.
 ******************************************************************************/
__INLINE bool sl_hal_wdog_is_locked(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  return (wdog->STATUS & _WDOG_STATUS_LOCK_MASK) == WDOG_STATUS_LOCK_LOCKED;
}

/** @} (end addtogroup wdog) */
#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup  wdog WDOG - Watchdog Timer
 * @{
 *
 *@n @section wdog_example Example
 *  The following example demonstrates how to configure and use the Watchdog Timer.
 *  This example shows how to initialize the watchdog with a custom configuration,
 *  enable interrupts, feed the watchdog, and properly clean up when done.
 *
 * @code{.c}
 * // WDOG warning interrupt handler
 * void WDOG0_IRQHandler(void)
 * {
 *   // Get pending interrupts
 *   uint32_t flags = sl_hal_wdog_get_pending_interrupts(WDOG0);
 *
 *   // Check for warning interrupt
 *   if (flags & WDOG_IF_WARN) {
 *     // Clear the warning interrupt flag
 *     sl_hal_wdog_clear_interrupts(WDOG0, WDOG_IF_WARN);
 *
 *     // Feed the watchdog before timeout occurs
 *     sl_hal_wdog_feed(WDOG0);
 *
 *     // Application can perform other actions here
 *     // ...
 *   }
 * }
 *
 * void wdog_example(void)
 * {
 *   // Enable bus clock for WDOG0
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_WDOG0);
 *
 *   // Initialize WDOG with default configuration
 *   sl_hal_wdog_init_t init = SL_HAL_WDOG_INIT_DEFAULT;
 *
 *   // Customize configuration
 *   init.period_select = SL_WDOG_PERIOD_64k;        // Set timeout period
 *   init.warning_time_select = SL_WDOG_WARNING_TIME75; // Get warning at 75% of timeout
 *
 *   // Initialize WDOG with our configuration
 *   sl_hal_wdog_init(WDOG0, &init);
 *
 *   // Enable warning interrupt
 *   sl_hal_wdog_clear_interrupts(WDOG0, WDOG_IF_WARN);
 *   sl_hal_wdog_enable_interrupts(WDOG0, WDOG_IF_WARN);
 *
 *   // Enable NVIC interrupt for WDOG
 *   sl_interrupt_manager_clear_irq_pending(WDOG0_IRQn);
 *   sl_interrupt_manager_enable_irq(WDOG0_IRQn);
 *
 *   // Enable WDOG - this starts the watchdog timer
 *   sl_hal_wdog_enable(WDOG0);
 *
 *   // Verify WDOG is enabled (optional)
 *   if (sl_hal_wdog_is_enabled(WDOG0)) {
 *     // WDOG is now running
 *
 *     // Feed the watchdog initially
 *     sl_hal_wdog_feed(WDOG0);
 *
 *     // Wait for feed operation to complete
 *     sl_hal_wdog_wait_sync(WDOG0);
 *
 *     // Main application loop would continue here
 *     // The watchdog will now trigger a warning interrupt at 75% of timeout
 *     // which will feed the watchdog in the interrupt handler
 *     // ...
 *   }
 *
 *   // When no longer needed, disable the watchdog (if not locked)
 *   if (!sl_hal_wdog_is_locked(WDOG0)) {
 *     sl_hal_wdog_disable(WDOG0);
 *     sl_hal_wdog_wait_ready(WDOG0);
 *
 *     // Disable NVIC interrupt for WDOG
 *     sl_interrupt_manager_disable_irq(WDOG0_IRQn);
 *   }
 * }
 * @endcode
 * @} (end addtogroup wdog)
 ******************************************************************************/

#endif /* defined(WDOG_COUNT) && (WDOG_COUNT > 0) */
#endif /* SL_HAL_WDOG_H */
