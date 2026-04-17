/***************************************************************************//**
 * @file sl_hal_letimer.h
 *
 * @brief Low Energy Timer (LETIMER) peripheral API
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

#ifndef SL_HAL_LETIMER_H
#define SL_HAL_LETIMER_H

#include "em_device.h"
#if defined(LETIMER_COUNT) && (LETIMER_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_assert.h"
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup letimer LETIMER - Low Energy Timer
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Check if LETIMER module is valid.
#if defined(LETIMER_NUM)
#define SL_HAL_LETIMER_REF_VALID(letimer_ref) \
  (LETIMER_NUM(letimer_ref) != -1)
#else
#define SL_HAL_LETIMER_REF_VALID(letimer_ref) \
  (letimer_ref == LETIMER0)
#endif

/// Return LETIMER module max count.
#if defined(LETIMER_NUM)
#define SL_HAL_LETIMER_MAX_COUNT(letimer_ref) \
  ((uint32_t)((1ULL << LETIMER_CNT_WIDTH(LETIMER_NUM(letimer_ref))) - 1))
#else
#define SL_HAL_LETIMER_MAX_COUNT(letimer_ref) \
  ((letimer_ref == LETIMER0) ? (uint32_t)((1ULL << LETIMER0_CNT_WIDTH) - 1) : 0x0UL)
#endif

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Prescaler.
SL_ENUM(sl_hal_letimer_prescaler_t) {
  SL_HAL_LETIMER_PRESCALER_DIV1   = _LETIMER_CTRL_CNTPRESC_DIV1,   /// Divide by 1.
  SL_HAL_LETIMER_PRESCALER_DIV2   = _LETIMER_CTRL_CNTPRESC_DIV2,   /// Divide by 2.
  SL_HAL_LETIMER_PRESCALER_DIV4   = _LETIMER_CTRL_CNTPRESC_DIV4,   /// Divide by 4.
  SL_HAL_LETIMER_PRESCALER_DIV8   = _LETIMER_CTRL_CNTPRESC_DIV8,   /// Divide by 8.
  SL_HAL_LETIMER_PRESCALER_DIV16  = _LETIMER_CTRL_CNTPRESC_DIV16,  /// Divide by 16.
  SL_HAL_LETIMER_PRESCALER_DIV32  = _LETIMER_CTRL_CNTPRESC_DIV32,  /// Divide by 32.
  SL_HAL_LETIMER_PRESCALER_DIV64  = _LETIMER_CTRL_CNTPRESC_DIV64,  /// Divide by 64.
  SL_HAL_LETIMER_PRESCALER_DIV128 = _LETIMER_CTRL_CNTPRESC_DIV128, /// Divide by 128.
  SL_HAL_LETIMER_PRESCALER_DIV256 = _LETIMER_CTRL_CNTPRESC_DIV256, /// Divide by 256.
};

/// Repeat mode.
SL_ENUM(sl_hal_letimer_repeat_mode_t) {
  /// Count until stopped by SW.
  SL_HAL_LETIMER_REPEAT_MODE_FREE     = _LETIMER_CTRL_REPMODE_FREE,
  /// Count REP0 times.
  SL_HAL_LETIMER_REPEAT_MODE_ONESHOT  = _LETIMER_CTRL_REPMODE_ONESHOT,
  /// Count REP0 times, if REP1 has been written to, it is loaded into
  /// REP0 when REP0 is about to be decremented to 0.
  SL_HAL_LETIMER_REPEAT_MODE_BUFFERED = _LETIMER_CTRL_REPMODE_BUFFERED,
  /// Run as long as both REP0 and REP1 are not 0. Both REP0 and REP1
  /// are decremented when counter underflows.
  SL_HAL_LETIMER_REPEAT_MODE_DOUBLE   = _LETIMER_CTRL_REPMODE_DOUBLE
};

/// Underflow action on output.
SL_ENUM(sl_hal_letimer_underflow_output_action_t) {
  /// No output action.
  SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_NONE   = _LETIMER_CTRL_UFOA0_NONE,
  /// Toggle output when counter underflows.
  SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_TOGGLE = _LETIMER_CTRL_UFOA0_TOGGLE,
  /// Hold output one LETIMER clock cycle when counter underflows.
  SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_PULSE  = _LETIMER_CTRL_UFOA0_PULSE,
  /// Set output idle when counter underflows, and active when matching COMP1.
  SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_PWM    = _LETIMER_CTRL_UFOA0_PWM
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// LETIMER initialization structure.
typedef struct {
  /// Prescaling factor.
  sl_hal_letimer_prescaler_t               prescaler;
  /// Repeat mode.
  sl_hal_letimer_repeat_mode_t             repeat_mode;
  /// Underflow output 0 action.
  sl_hal_letimer_underflow_output_action_t underflow_output0_action;
  /// Underflow output 1 action.
  sl_hal_letimer_underflow_output_action_t underflow_output1_action;
  /// Counter shall keep running during debug halt.
  bool                                     debug_run;
  /// Load TOP into CNT when counter underflows.
  bool                                     enable_top;
  /// Load TOPBUFF into TOP when REP0 reaches 0, in BUFFERED mode only.
  bool                                     buffered_top;
  /// Idle value for output 0.
  bool                                     output0_polarity;
  /// Idle value for output 1.
  bool                                     output1_polarity;
} sl_hal_letimer_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_letimer_init_t sl_hal_letimer_config_t;
/** @endcond */

/// Default configuration for LETIMER initialization structure.
#define SL_HAL_LETIMER_INIT_DEFAULT                                          \
  {                                                                          \
    .prescaler                = SL_HAL_LETIMER_PRESCALER_DIV1,               \
    .repeat_mode              = SL_HAL_LETIMER_REPEAT_MODE_FREE,             \
    .underflow_output0_action = SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_NONE, \
    .underflow_output1_action = SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_NONE, \
    .debug_run                = false,                                       \
    .enable_top               = false,                                       \
    .buffered_top             = false,                                       \
    .output0_polarity         = false,                                       \
    .output1_polarity         = false                                        \
  }

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro name used for backward compatibility purposes.
#define SL_HAL_LETIMER_CONFIG_DEFAULT SL_HAL_LETIMER_INIT_DEFAULT
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize LETIMER.
 *
 * @param[in] letimer
 *   A pointer to the LETIMER peripheral register block.
 *
 * @param[in] init
 *   A pointer to the LETIMER initialization structure.
 ******************************************************************************/
void sl_hal_letimer_init(LETIMER_TypeDef *letimer,
                         const sl_hal_letimer_init_t *init);

/***************************************************************************//**
 * @brief
 *   Reset the LETIMER to the same state that it was in after a hardware reset.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 ******************************************************************************/
void sl_hal_letimer_reset(LETIMER_TypeDef *letimer);

/***************************************************************************//**
 * @brief
 *   Enable the LETIMER.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_letimer_enable(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  if ((letimer->EN & _LETIMER_EN_EN_MASK) == LETIMER_EN_EN) {
    return;
  }

  letimer->EN_SET = LETIMER_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable LETIMER.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_letimer_disable(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  if ((letimer->EN & _LETIMER_EN_EN_MASK) != LETIMER_EN_EN) {
    return;
  }

  letimer->EN_CLR = LETIMER_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Wait for resetting and disabling to finish.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_letimer_wait_ready(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

#if defined(_LETIMER_SWRST_RESETTING_MASK)
  while (letimer->SWRST & _LETIMER_SWRST_RESETTING_MASK) {
    // Wait for resetting to finish.
  }
#endif

#if defined(_LETIMER_EN_DISABLING_MASK)
  while (letimer->EN & _LETIMER_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_letimer_wait_sync(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  while (letimer->SYNCBUSY & _LETIMER_SYNCBUSY_MASK) {
    // Wait for the update to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Get status values for the LETIMER.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @return
 *   Current status values.
 ******************************************************************************/
__INLINE uint32_t sl_hal_letimer_get_status(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  return letimer->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more LETIMER interrupts.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] flags
 *   LETIMER interrupt source(s) to enable. Use one or more valid
 *   interrupt flags for the LETIMER module (LETIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_letimer_enable_interrupts(LETIMER_TypeDef *letimer,
                                               uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  letimer->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more LETIMER interrupts.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] flags
 *   LETIMER interrupt source(s) to disable. Use one or more valid
 *   interrupt flags for the LETIMER module (LETIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_letimer_disable_interrupts(LETIMER_TypeDef *letimer,
                                                uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  letimer->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending LETIMER interrupts.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] flags
 *   Pending LETIMER interrupt source(s) to clear. Use one or more valid
 *   interrupt flags for the LETIMER module (LETIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_letimer_clear_interrupts(LETIMER_TypeDef *letimer,
                                              uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  letimer->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending LETIMER interrupts from SW.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] flags
 *   LETIMER interrupt source(s) to set to pending. Use one or more valid
 *   interrupt flags for the LETIMER module (LETIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_letimer_set_interrupts(LETIMER_TypeDef *letimer,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  letimer->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending LETIMER interrupt flags.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @return
 *   LETIMER interrupt source(s) pending. Returns one or more valid
 *   interrupt flags for the LETIMER module (LETIMER_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_letimer_get_pending_interrupts(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  return letimer->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending LETIMER interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @note
 *   Interrupt flags are not cleared by this function.
 *
 * @return
 *   Pending and enabled LETIMER interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in LETIMERx_IEN_nnn
 *     register (LETIMERx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the LETIMER module
 *     (LETIMERx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_letimer_get_enabled_pending_interrupts(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  // Store LETIMER->IEN in temporary variable in order to define explicit order
  // of volatile accesses.
  uint32_t ien = letimer->IEN;

  return letimer->IF & ien;
}

#if defined(_LETIMER_LOCK_MASK)
/***************************************************************************//**
 * @brief
 *   Lock LETIMER registers.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @note
 *   When LETIMER registers are locked LETIMER_EN, LETIMER_SWRST, LETIMER_CTRL,
 *   LETIMER_CMD, LETIMER_CNT, LETIMER_COMP0, LETIMER_COMP1, LETIMER_TOP,
 *   LETIMER_TOPBUFF, LETIMER_REP0, LETIMER_REP1 and LETIMER_PRSMODE registers
 *   cannot be written to.
 ******************************************************************************/
__INLINE void sl_hal_letimer_lock(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  letimer->LOCK = ~LETIMER_LOCK_LETIMERLOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Unlock LETIMER registers.
 *
 * @param[in] letimer
 *   Pointer to theLETIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_letimer_unlock(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  letimer->LOCK = LETIMER_LOCK_LETIMERLOCKKEY_UNLOCK;
}
#endif

/***************************************************************************//**
 * @brief
 *   Start LETIMER counter.
 *
 * @details
 *   This function will send a start command to the LETIMER peripheral. The
 *   LETIMER peripheral will use some LF clock ticks before the command is
 *   executed. The @ref sl_hal_letimer_wait_sync() function can be used to wait
 *   for the start command to be executed.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_letimer_start(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));
  // Make sure module is enabled.
  EFM_ASSERT(letimer->EN & _LETIMER_EN_EN_MASK);

  sl_hal_letimer_wait_sync(letimer);
  letimer->CMD_SET = LETIMER_CMD_START;
}

/***************************************************************************//**
 * @brief
 *   Stop LETIMER counter.
 *
 * @details
 *   This function will send a stop command to the LETIMER peripheral. The
 *   LETIMER peripheral will use some LF clock ticks before the command is
 *   executed. The @ref sl_hal_letimer_wait_sync() function can be used to wait
 *   for the stop command to be executed.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_letimer_stop(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));
  // Make sure module is enabled.
  EFM_ASSERT(letimer->EN & _LETIMER_EN_EN_MASK);

  sl_hal_letimer_wait_sync(letimer);
  letimer->CMD_SET = LETIMER_CMD_STOP;
}

/***************************************************************************//**
 * @brief
 *   Set the LETIMER counter register value.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] value
 *   Value to set in counter register.
 ******************************************************************************/
__INLINE void sl_hal_letimer_set_counter(LETIMER_TypeDef *letimer,
                                         uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_LETIMER_MAX_COUNT(letimer));
  // Make sure module is enabled.
  EFM_ASSERT(letimer->EN & _LETIMER_EN_EN_MASK);

  sl_hal_letimer_wait_sync(letimer);
  letimer->CNT = value;
}

/***************************************************************************//**
 * @brief
 *   Get the LETIMER counter register value.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @return
 *   Current counter.
 ******************************************************************************/
__INLINE uint32_t sl_hal_letimer_get_counter(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  sl_hal_letimer_wait_sync(letimer);
  return letimer->CNT;
}

/***************************************************************************//**
 * @brief
 *   Set the LETIMER compare register value.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare register to set, either 0 or 1.
 *
 * @param[in] value
 *   Value to set in the compare register.
 ******************************************************************************/
void sl_hal_letimer_set_compare(LETIMER_TypeDef *letimer,
                                uint8_t channel,
                                uint32_t value);

/***************************************************************************//**
 * @brief
 *   Get the LETIMER compare register value.
 *
 * @param[in] letimer
 *   A pointer to the LETIMER peripheral register block.
 *
 * @param[in] channel
 *   A compare register to get, either 0 or 1.
 *
 * @return
 *   A compare register value, 0 if invalid register selected.
 ******************************************************************************/
uint32_t sl_hal_letimer_get_compare(LETIMER_TypeDef *letimer,
                                    uint8_t channel);

/***************************************************************************//**
 * @brief
 *   Set the LETIMER repeat register value.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] channel
 *   A repeat register to set, either 0 or 1.
 *
 * @param[in] value
 *   Value to set in the repeat register.
 ******************************************************************************/
void sl_hal_letimer_set_repeat(LETIMER_TypeDef *letimer,
                               uint8_t channel,
                               uint32_t value);

/***************************************************************************//**
 * @brief
 *   Get the LETIMER repeat register value.
 *
 * @param[in] letimer
 *   A pointer to the LETIMER peripheral register block.
 *
 * @param[in] channel
 *   A repeat register to get, either 0 or 1.
 *
 * @return
 *   A repeat register value, 0 if invalid register selected.
 ******************************************************************************/
uint32_t sl_hal_letimer_get_repeat(LETIMER_TypeDef *letimer,
                                   uint8_t channel);

/***************************************************************************//**
 * @brief
 *   Set the LETIMER top register value.
 *
 * @details
 *   The LETIMER is a down-counter, so when the counter reaches 0 then the top
 *   value will be loaded into the counter. This function can be used to set
 *   the top value. The LETIMER needs to be configured with @ref enable_top to
 *   use a top value.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value register.
 ******************************************************************************/
__INLINE void sl_hal_letimer_set_top(LETIMER_TypeDef *letimer,
                                     uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));
  // Make sure top value is valid.
  EFM_ASSERT(value <= SL_HAL_LETIMER_MAX_COUNT(letimer));
  // Make sure module is enabled.
  EFM_ASSERT(letimer->EN & _LETIMER_EN_EN_MASK);

  sl_hal_letimer_wait_sync(letimer);
  letimer->TOP = value;
}

/***************************************************************************//**
 * @brief
 *   Get the LETIMER top register value.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @return
 *   Current top value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_letimer_get_top(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  sl_hal_letimer_wait_sync(letimer);
  return letimer->TOP;
}

/***************************************************************************//**
 * @brief
 *   Set the LETIMER top register value buffer.
 *
 * @details
 *   When top value buffer register is updated, value is loaded into
 *   top value register the next time the counter reaches 0. This feature is
 *   useful to update top value safely when LETIMER is running. This won't
 *   happen if @ref buffered_top is not configured to true and the mode is not
 *   @ref SL_HAL_LETIMER_REPEAT_MODE_BUFFERED.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value buffer register.
 ******************************************************************************/
__INLINE void sl_hal_letimer_set_top_buffer(LETIMER_TypeDef *letimer,
                                            uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));
  // Make sure top buffer value is valid.
  EFM_ASSERT(value <= SL_HAL_LETIMER_MAX_COUNT(letimer));
  // Make sure module is enabled.
  EFM_ASSERT(letimer->EN & _LETIMER_EN_EN_MASK);

  sl_hal_letimer_wait_sync(letimer);
  letimer->TOPBUFF = value;
}

/***************************************************************************//**
 * @brief
 *   Get the LETIMER top register value buffer.
 *
 * @param[in] letimer
 *   Pointer to the LETIMER peripheral register block.
 *
 * @return
 *   Current top value buffer.
 ******************************************************************************/
__INLINE uint32_t sl_hal_letimer_get_top_buffer(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  sl_hal_letimer_wait_sync(letimer);
  return letimer->TOPBUFF;
}

/// @} (end addtogroup letimer)
#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup letimer LETIMER - Low Energy Timer
 * @{
 *
 * @n @section letimer_example Example
 *  The following example demonstrates how to configure and use the LETIMER for
 *  PWM generation with a configurable duty cycle.
 *
 * @code{.c}
 * // Global flag for COMP0 interrupt
 * volatile bool comp0_occurred = false;
 *
 * // LETIMER0 IRQ Handler
 * void LETIMER0_IRQHandler(void)
 * {
 *   // Get pending interrupts
 *   uint32_t flags = sl_hal_letimer_get_pending_interrupts(LETIMER0);
 *
 *   // Check for COMP0 interrupt
 *   if (flags & LETIMER_IF_COMP0) {
 *     // Clear the interrupt flag
 *     sl_hal_letimer_clear_interrupts(LETIMER0, LETIMER_IF_COMP0);
 *
 *     // Set our flag to indicate the event occurred
 *     comp0_occurred = true;
 *
 *     // Additional processing can be done here
 *   }
 * }
 *
 * // Example of configuring LETIMER for PWM generation
 * void letimer_pwm_example(void)
 * {
 *   // Enable bus clock for LETIMER0
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LETIMER0);
 *
 *   // Initialize with custom configuration for PWM generation
 *   sl_hal_letimer_init_t init = SL_HAL_LETIMER_INIT_DEFAULT;
 *
 *   // Set prescaler to divide clock by 4
 *   init.prescaler = SL_HAL_LETIMER_PRESCALER_DIV4;
 *
 *   // Free-running mode
 *   init.repeat_mode = SL_HAL_LETIMER_REPEAT_MODE_FREE;
 *
 *   // Configure PWM on output 0
 *   init.underflow_output0_action = SL_HAL_LETIMER_UNDERFLOW_OUTPUT_ACTION_PWM;
 *
 *   // Reload TOP on underflow
 *   init.enable_top = true;
 *
 *   // Initialize LETIMER with our configuration
 *   sl_hal_letimer_init(LETIMER0, &init);
 *
 *   // Enable LETIMER
 *   sl_hal_letimer_enable(LETIMER0);
 *
 *   // Set TOP value for desired frequency
 *   // For example, with 32.768 kHz clock, div4 prescaler, and TOP=1000:
 *   // Frequency = 32768 / 4 / 1000 = 8.192 Hz
 *   sl_hal_letimer_set_top(LETIMER0, 1000);
 *
 *   // Set compare values to control PWM duty cycle
 *   // COMP0 = 0 (active at start of period)
 *   sl_hal_letimer_set_compare(LETIMER0, 0, 0);
 *
 *   // COMP1 = 500 (50% duty cycle)
 *   sl_hal_letimer_set_compare(LETIMER0, 1, 500);
 *
 *   // Clear any pending interrupts
 *   sl_hal_letimer_clear_interrupts(LETIMER0, _LETIMER_IF_MASK);
 *
 *   // Enable COMP0 interrupt to trigger at the start of each period
 *   sl_hal_letimer_enable_interrupts(LETIMER0, LETIMER_IEN_COMP0);
 *
 *   // Enable NVIC interrupt for LETIMER
 *   sl_interrupt_manager_clear_irq_pending(LETIMER0_IRQn);
 *   sl_interrupt_manager_enable_irq(LETIMER0_IRQn);
 *
 *   // Reset the event flag
 *   comp0_occurred = false;
 *
 *   // Start LETIMER
 *   sl_hal_letimer_start(LETIMER0);
 *
 *   // Wait for first COMP0 interrupt to occur
 *   while (!comp0_occurred) {
 *     // Could enter energy mode here to save power while waiting
 *   }
 *
 *   // Change duty cycle to 25%
 *   sl_hal_letimer_set_compare(LETIMER0, 1, 750);  // 25% = (1000-750)/1000
 *
 *   // Application code would continue here
 *   // ...
 *
 *   // When done, clean up
 *   sl_hal_letimer_stop(LETIMER0);
 *   sl_hal_letimer_disable_interrupts(LETIMER0, _LETIMER_IEN_MASK);
 *   sl_interrupt_manager_disable_irq(LETIMER0_IRQn);
 *   sl_hal_letimer_disable(LETIMER0);
 *   sl_hal_letimer_wait_ready(LETIMER0);
 * }
 * @endcode
 * @} (end addtogroup letimer)
 ******************************************************************************/

#endif // defined(LETIMER_COUNT) && (LETIMER_COUNT > 0)
#endif // SL_HAL_LETIMER_H
