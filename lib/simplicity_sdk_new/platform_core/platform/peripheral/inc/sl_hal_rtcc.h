/***************************************************************************//**
 * @file sl_hal_rtcc.h
 * @brief Real Time Clock with Capture (RTCC) Peripheral API.
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
#ifndef SL_HAL_RTCC_H
#define SL_HAL_RTCC_H

#include "em_device.h"

#if defined(RTCC_COUNT) && (RTCC_COUNT == 1)

#include <stdbool.h>
#include "sl_assert.h"
#include "sl_enum.h"
#include "sl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup rtcc RTCC - Real Time Clock with Capture
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Validation of valid RTCC channel for assert statements.
#define SL_HAL_RTCC_CH_VALID(ch)    ((unsigned)(ch) < RTCC_CC_NUM)

/*******************************************************************************
 *********************************   ENUM   ************************************
 ******************************************************************************/

/// Counter prescaler selection.
SL_ENUM(sl_hal_rtcc_counter_prescaler_t) {
  SL_HAL_RTCC_COUNTER_PRESCALER_1     = _RTCC_CFG_CNTPRESC_DIV1,      ///< Divide clock by 1.
  SL_HAL_RTCC_COUNTER_PRESCALER_2     = _RTCC_CFG_CNTPRESC_DIV2,      ///< Divide clock by 2.
  SL_HAL_RTCC_COUNTER_PRESCALER_4     = _RTCC_CFG_CNTPRESC_DIV4,      ///< Divide clock by 4.
  SL_HAL_RTCC_COUNTER_PRESCALER_8     = _RTCC_CFG_CNTPRESC_DIV8,      ///< Divide clock by 8.
  SL_HAL_RTCC_COUNTER_PRESCALER_16    = _RTCC_CFG_CNTPRESC_DIV16,     ///< Divide clock by 16.
  SL_HAL_RTCC_COUNTER_PRESCALER_32    = _RTCC_CFG_CNTPRESC_DIV32,     ///< Divide clock by 32.
  SL_HAL_RTCC_COUNTER_PRESCALER_64    = _RTCC_CFG_CNTPRESC_DIV64,     ///< Divide clock by 64.
  SL_HAL_RTCC_COUNTER_PRESCALER_128   = _RTCC_CFG_CNTPRESC_DIV128,    ///< Divide clock by 128.
  SL_HAL_RTCC_COUNTER_PRESCALER_256   = _RTCC_CFG_CNTPRESC_DIV256,    ///< Divide clock by 256.
  SL_HAL_RTCC_COUNTER_PRESCALER_512   = _RTCC_CFG_CNTPRESC_DIV512,    ///< Divide clock by 512.
  SL_HAL_RTCC_COUNTER_PRESCALER_1024  = _RTCC_CFG_CNTPRESC_DIV1024,   ///< Divide clock by 1024.
  SL_HAL_RTCC_COUNTER_PRESCALER_2048  = _RTCC_CFG_CNTPRESC_DIV2048,   ///< Divide clock by 2048.
  SL_HAL_RTCC_COUNTER_PRESCALER_4096  = _RTCC_CFG_CNTPRESC_DIV4096,   ///< Divide clock by 4096.
  SL_HAL_RTCC_COUNTER_PRESCALER_8192  = _RTCC_CFG_CNTPRESC_DIV8192,   ///< Divide clock by 8192.
  SL_HAL_RTCC_COUNTER_PRESCALER_16384 = _RTCC_CFG_CNTPRESC_DIV16384,  ///< Divide clock by 16384.
  SL_HAL_RTCC_COUNTER_PRESCALER_32768 = _RTCC_CFG_CNTPRESC_DIV32768   ///< Divide clock by 32768.
};

/// Prescaler mode of the RTCC counter.
SL_ENUM(sl_hal_rtcc_prescaler_mode_t) {
  /// CNT register ticks according to prescaler value.
  SL_HAL_RTCC_COUNTER_TICK_PRESCALER   = _RTCC_CFG_CNTTICK_PRESC,

  /// CNT register ticks when PRECNT matches the 15 least significant bits of
  /// ch. 0 CCV register.
  SL_HAL_RTCC_COUNTER_TICK_CCV_0_MATCH = _RTCC_CFG_CNTTICK_CCV0MATCH
};

/// Capture/Compare channel mode.
SL_ENUM(sl_hal_rtcc_capture_compare_channel_mode_t) {
  SL_HAL_RTCC_CAPTURE_COMPARE_CHANNEL_MODE_OFF     = _RTCC_CC_CTRL_MODE_OFF,            ///< Capture/Compare channel turned off.
  SL_HAL_RTCC_CAPTURE_COMPARE_CHANNEL_MODE_CAPTURE = _RTCC_CC_CTRL_MODE_INPUTCAPTURE,   ///< Capture mode.
  SL_HAL_RTCC_CAPTURE_COMPARE_CHANNEL_MODE_COMPARE = _RTCC_CC_CTRL_MODE_OUTPUTCOMPARE,  ///< Compare mode.
};

/// Compare match output action mode.
SL_ENUM(sl_hal_rtcc_compare_match_out_action_t) {
  SL_HAL_RTCC_COMPARE_MATCH_OUT_ACTION_PULSE  = _RTCC_CC_CTRL_CMOA_PULSE,  ///< Generate a pulse.
  SL_HAL_RTCC_COMPARE_MATCH_OUT_ACTION_TOGGLE = _RTCC_CC_CTRL_CMOA_TOGGLE, ///< Toggle output.
  SL_HAL_RTCC_COMPARE_MATCH_OUT_ACTION_CLEAR  = _RTCC_CC_CTRL_CMOA_CLEAR,  ///< Clear output.
  SL_HAL_RTCC_COMPARE_MATCH_OUT_ACTION_SET    = _RTCC_CC_CTRL_CMOA_SET     ///< Set output.
};

/// PRS channel number. This type is used when configuring input capture mode on
/// a RTCC channel.
typedef uint8_t sl_hal_rtcc_prs_select_t;

/// Input edge select.
SL_ENUM(sl_hal_rtcc_in_edge_select_t) {
  SL_HAL_RTCC_IN_EDGE_RISING  = _RTCC_CC_CTRL_ICEDGE_RISING,  ///< Rising edges detected.
  SL_HAL_RTCC_IN_EDGE_FALLING = _RTCC_CC_CTRL_ICEDGE_FALLING, ///< Falling edges detected.
  SL_HAL_RTCC_IN_EDGE_BOTH    = _RTCC_CC_CTRL_ICEDGE_BOTH,    ///< Both edges detected.
  SL_HAL_RTCC_IN_EDGE_NONE    = _RTCC_CC_CTRL_ICEDGE_NONE     ///< No edge detection, signal is left as is.
};

/// Capture/Compare channel compare mode.
SL_ENUM(sl_hal_rtcc_compare_base_t) {
  SL_HAL_RTCC_COMPBASE_COUNTER_MODE      = _RTCC_CC_CTRL_COMPBASE_CNT,   ///< CCVx is compared with the CNT register.
  SL_HAL_RTCC_COMPARE_PRE_COUNTER_MODE   = _RTCC_CC_CTRL_COMPBASE_PRECNT ///< CCVx is compared with a CNT[16:0] and PRECNT[14:0].
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// RTCC initialization structure.
typedef struct {
  bool                            debug_run;                  ///< Enable/disable timer counting during debug halt.
  bool                            pre_counter_wrap_on_ccv_0;  ///< Enable/disable pre-counter wrap on ch. 0 CCV value.
  bool                            counter_wrap_on_ccv_1;      ///< Enable/disable counter wrap on ch. 1 CCV value.
  sl_hal_rtcc_counter_prescaler_t prescaler;                  ///< Counter prescaler.
  sl_hal_rtcc_prescaler_mode_t    prescaler_mode;             ///< Prescaler mode.
} sl_hal_rtcc_init_t;

/// RTCC capture/compare channel configuration structure.
typedef struct {
  sl_hal_rtcc_capture_compare_channel_mode_t channel_mode;              ///< Select mode of Capture/Compare channel.
  sl_hal_rtcc_compare_match_out_action_t     compare_match_out_action;  ///< Compare mode channel match output action.
  sl_hal_rtcc_prs_select_t                   prs_select;                ///< Capture mode channel PRS input channel selection.
  sl_hal_rtcc_in_edge_select_t               input_edge_select;         ///< Capture mode channel input edge selection.
  sl_hal_rtcc_compare_base_t                 compare_base;              ///<< Comparison base of channel in compare mode.
} sl_hal_rtcc_cc_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_rtcc_init_t sl_hal_rtcc_config_t;
typedef sl_hal_rtcc_cc_init_t sl_hal_rtcc_cc_config_t;
/** @endcond */

/*******************************************************************************
 ****************************  STRUCT INITIALIZERS  ***************************
 ******************************************************************************/

/// Default RTCC initialization structure.
#define SL_HAL_RTCC_INIT_DEFAULT                                                              \
  {                                                                                           \
    false,                              /* Disable RTCC during debug halt.                 */ \
    false,                              /* Disable precounter wrap on ch. 0 CCV value.     */ \
    false,                              /* Disable counter wrap on ch. 1 CCV value.        */ \
    SL_HAL_RTCC_COUNTER_PRESCALER_32,   /* 977 us per tick.                                */ \
    SL_HAL_RTCC_COUNTER_TICK_PRESCALER, /* Counter increments according to prescaler value.*/ \
  }

/// Default RTCC channel output compare initialization structure.
#define SL_HAL_RTCC_CH_INIT_COMPARE_DEFAULT                                                \
  {                                                                                        \
    SL_HAL_RTCC_CAPTURE_COMPARE_CHANNEL_MODE_COMPARE, /* Select output compare mode.    */ \
    SL_HAL_RTCC_COMPARE_MATCH_OUT_ACTION_PULSE,       /* Create pulse on compare match. */ \
    0,                                                /* Don't care.                    */ \
    SL_HAL_RTCC_IN_EDGE_NONE,                         /* Don't care.                    */ \
    SL_HAL_RTCC_COMPBASE_COUNTER_MODE,                /* Use CNT for comparison.        */ \
  }

/// Default RTCC channel input capture initialization structure.
#define SL_HAL_RTCC_CH_INIT_CAPTURE_DEFAULT                                               \
  {                                                                                       \
    SL_HAL_RTCC_CAPTURE_COMPARE_CHANNEL_MODE_CAPTURE, /* Select input capture mode.    */ \
    SL_HAL_RTCC_COMPARE_MATCH_OUT_ACTION_PULSE,       /* Don't care.                   */ \
    0,                                                /* Use PRS channel 0 as trigger. */ \
    SL_HAL_RTCC_IN_EDGE_RISING,                       /* Capture on rising edge.       */ \
    SL_HAL_RTCC_COMPBASE_COUNTER_MODE,                /* Don't care.                   */ \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize RTCC.
 *
 * @param[in] init
 *   A pointer to the RTCC initialization structure.
 *
 * @details
 *   Note that the compare values must be set separately with
 *   sl_hal_rtcc_set_channel_compare_value(), which should probably be done prior
 *   to the use of this function if configuring the RTCC to start when
 *   initialization is completed.
 ******************************************************************************/
void sl_hal_rtcc_init(const sl_hal_rtcc_init_t *init);

/***************************************************************************//**
 * @brief
 *   Configure the selected capture/compare channel of the RTCC.
 *
 * @param[in] channel
 *   A channel selector.
 *
 * @param[in] init
 *   A pointer to the RTCC channel initialization structure.
 *
 * @details
 *   Use this function to configure an RTCC channel. Select capture/compare mode,
 *   match output action, overflow output action, and PRS input configuration.
 *   See the configuration structure @ref sl_hal_rtcc_cc_init_t for more details.
 ******************************************************************************/
void sl_hal_rtcc_channel_init(uint32_t channel,
                              const sl_hal_rtcc_cc_init_t *init);

/***************************************************************************//**
 * @brief
 *   Restore RTCC to its reset state.
 ******************************************************************************/
void sl_hal_rtcc_reset(void);

/***************************************************************************//**
 * @brief
 *   Wait for the RTCC to complete all synchronization of register changes
 *   and commands.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_wait_sync(void)
{
  while ((RTCC->EN != 0U) && (RTCC->SYNCBUSY != 0U)) {
    // Wait for synchronization to finish
  }
}

/***************************************************************************//**
 * @brief
 *   Enable RTCC counting.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_enable(void)
{
  if (RTCC->EN != 0U) {
    // Modifying the enable bit while synchronization is active will BusFault
    sl_hal_rtcc_wait_sync();
  }
  RTCC->EN_SET = RTCC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable RTCC counting.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_disable(void)
{
  // Quick exit if we want to disable RTCC and it's already disabled.
  if (RTCC->EN == 0U) {
    return;
  }
  sl_hal_rtcc_wait_sync();
  RTCC->EN_CLR = RTCC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Start the RTCC counter.
 *
 * @details
 *   This function will send a start command to the RTCC peripheral. The RTCC
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_rtcc_wait_sync() function can be used to wait for the
 *   start command to be executed.
 *
 * @note
 *   This function requires the RTCC to be enabled.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_start(void)
{
  sl_hal_rtcc_wait_sync();
  RTCC->CMD = RTCC_CMD_START;
}

/***************************************************************************//**
 * @brief
 *   Stop the RTCC counter.
 *
 * @details
 *   This function will send a stop command to the RTCC peripheral. The RTCC
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_rtcc_wait_sync() function can be used to wait for the
 *   stop command to be executed.
 *
 * @note
 *   This function requires the RTCC to be enabled.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_stop(void)
{
  sl_hal_rtcc_wait_sync();
  RTCC->CMD = RTCC_CMD_STOP;
}

/***************************************************************************//**
 * @brief
 *   Get the RTCC compare register value for a selected channel.
 *
 * @param[in] channel
 *   Channel selector.
 *
 * @return
 *   Compare register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_channel_compare_value(uint32_t channel)
{
  EFM_ASSERT(SL_HAL_RTCC_CH_VALID(channel) );
  return RTCC->CC[channel].OCVALUE;
}

/***************************************************************************//**
 * @brief
 *   Set the RTCC compare register value for a selected channel.
 *
 * @param[in] channel
 *   Channel selector.
 *
 * @param[in] value
 *   Compare register value
 ******************************************************************************/
__INLINE void sl_hal_rtcc_set_channel_compare_value(uint32_t channel,
                                                    uint32_t value)
{
  EFM_ASSERT(SL_HAL_RTCC_CH_VALID(channel) );
  RTCC->CC[channel].OCVALUE = value;
}

/***************************************************************************//**
 * @brief
 *   Get the RTCC input capture register value for a selected channel.
 *
 * @param[in] channel
 *   Channel selector.
 *
 * @return Capture
 *   register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_channel_capture_value(uint32_t channel)
{
  EFM_ASSERT(SL_HAL_RTCC_CH_VALID(channel) );
  return RTCC->CC[channel].ICVALUE;
}

/***************************************************************************//**
 * @brief
 *   Get the RTCC capture/compare register value for  a selected channel.
 *   For parts with separate capture compare value registers, this function
 *   returns the compare value.
 *
 * @param[in] channel
 *   Channel selector.
 *
 * @return Capture/compare
 *   register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_channel_ccv(uint32_t channel)
{
  return sl_hal_rtcc_get_channel_compare_value(channel);
}

/***************************************************************************//**
 * @brief
 *   Set RTCC capture/compare register value for a selected channel.
 *   For parts with separate capture compare value registers, this function
 *   sets the compare value.
 *
 * @param[in] channel
 *   Channel selector.
 *
 * @param[in] value
 *   Capture/compare register value
 ******************************************************************************/
__INLINE void sl_hal_rtcc_set_channel_ccv(uint32_t channel,
                                          uint32_t value)
{
  sl_hal_rtcc_set_channel_compare_value(channel, value);
}

/***************************************************************************//**
 * @brief
 *   Get the combined CNT/PRECNT register content.
 *
 * @return
 *   CNT/PRECNT register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_combined_counter(void)
{
  while ((RTCC->SYNCBUSY & (RTCC_SYNCBUSY_CNT | RTCC_SYNCBUSY_PRECNT)) != 0U) {
    // Wait for CNT and PRECNT to synchronize
  }
  return RTCC->COMBCNT;
}

/***************************************************************************//**
 * @brief
 *   Get the RTCC counter value.
 *
 * @return
 *   Current RTCC counter value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_counter(void)
{
  while ((RTCC->SYNCBUSY & RTCC_SYNCBUSY_CNT) != 0U) {
    // Wait for CNT to synchronize before getting value
  }
  return RTCC->CNT;
}

/***************************************************************************//**
 * @brief
 *   Set the RTCC CNT counter.
 *
 * @param[in] value
 *   CNT value.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_set_counter(uint32_t value)
{
  while ((RTCC->SYNCBUSY & RTCC_SYNCBUSY_CNT) != 0U) {
    // Wait for CNT to synchronize before setting new value
  }
  RTCC->CNT = value;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending RTCC interrupts.
 *
 * @param[in] flags
 *   RTCC interrupt sources to clear. Use a set of interrupt flags OR-ed
 *   together to clear multiple interrupt sources.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_clear_interrupts(uint32_t flags)
{
  RTCC->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more RTCC interrupts.
 *
 * @param[in] flags
 *   RTCC interrupt sources to disable. Use a set of interrupt flags OR-ed
 *   together to disable multiple interrupt.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_disable_interrupts(uint32_t flags)
{
  RTCC->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more RTCC interrupts.
 *
 * @param[in] flags
 *   RTCC interrupt sources to enable. Use a set of interrupt flags OR-ed
 *   together to set multiple interrupt.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   sl_hal_rtcc_clear_interrupts() prior to enabling the interrupt.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_enable_interrupts(uint32_t flags)
{
  RTCC->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending RTCC interrupt flags.
 *
 * @return
 *   Pending RTCC interrupt sources. Returns a set of interrupt flags OR-ed
 *   together for the interrupt sources set.
 *
 * @note
 *   Event bits are not cleared by using this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_pending_interrupts(void)
{
  return RTCC->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending RTCC interrupt flags.
 *
 * @return
 *   Pending and enabled RTCC interrupt sources. Returns a set of interrupt
 *   flags OR-ed together for the interrupt sources set.
 *
 * @details
 *   Useful for handling more interrupt sources in the same interrupt handler.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_enabled_interrupts(void)
{
  uint32_t tmp;

  tmp = RTCC->IEN;

  /* Bitwise AND of pending and enabled interrupt flags. */
  return RTCC->IF & tmp;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending RTCC interrupts from SW.
 *
 * @param[in] flags
 *   RTCC interrupt sources to set to pending. Use a set of interrupt flags
 *   (RTCC_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_rtcc_set_interrupts(uint32_t flags)
{
  RTCC->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Lock RTCC registers.
 *
 * @note
 *   When RTCC registers are locked, RTCC_CTRL, RTCC_PRECNT, RTCC_CNT,
 *   RTCC_TIME, RTCC_DATE, RTCC_IEN, RTCC_POWERDOWN and RTCC_CCx_XXX registers
 *   cannot be written to.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_lock(void)
{
  RTCC->LOCK = ~RTCC_LOCK_LOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Unlock RTCC registers.
 *
 * @note
 *   When RTCC registers are locked, RTCC_CTRL, RTCC_PRECNT, RTCC_CNT,
 *   RTCC_TIME, RTCC_DATE, RTCC_IEN, RTCC_POWERDOWN and RTCC_CCx_XXX registers
 *   cannot be written to.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_unlock(void)
{
  RTCC->LOCK = RTCC_LOCK_LOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Get the RTCC pre-counter value.
 *
 * @return Current
 *   RTCC pre-counter value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_pre_counter(void)
{
  while ((RTCC->SYNCBUSY & RTCC_SYNCBUSY_PRECNT) != 0U) {
    /* Wait for PRECNT to synchronize */
  }
  return RTCC->PRECNT;
}

/***************************************************************************//**
 * @brief
 *   Set the RTCC pre-counter value.
 *
 * @param[in] pre_counter_value
 *   RTCC pre-counter value to be set.
 ******************************************************************************/
__INLINE void sl_hal_rtcc_set_pre_counter(uint32_t pre_counter_value)
{
  while ((RTCC->SYNCBUSY & RTCC_SYNCBUSY_PRECNT) != 0U) {
    /* Wait for PRECNT to synchronize */
  }
  RTCC->PRECNT = pre_counter_value;
}

/***************************************************************************//**
 * @brief
 *   Get the STATUS register value.
 *
 * @return Current
 *   STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_rtcc_get_status(void)
{
  /* Wait for synchronization. */
  sl_hal_rtcc_wait_sync();
  return RTCC->STATUS;
}

/** @} (end addtogroup rtcc) */

#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup rtcc RTCC - Real Time Clock with Capture
 * @{
 *
 * @n @section rtcc_example Example
 * This example demonstrates how to initialize the RTCC, configure channels for
 * compare mode, set compare values, and handle interrupts.
 *
 * @code
 *
 * #include "sl_hal_rtcc.h"
 *
 * // Flag to indicate compare match event occurred
 * volatile bool compare_match = false;
 *
 * // RTCC interrupt handler
 * void RTCC_IRQHandler(void)
 * {
 *   // Get enabled and pending interrupts
 *   uint32_t flags = sl_hal_rtcc_get_enabled_interrupts();
 *
 *   // Check if CC0 interrupt is triggered
 *   if (flags & RTCC_IF_CC0) {
 *     // Clear the interrupt flag
 *     sl_hal_rtcc_clear_interrupts(RTCC_IF_CC0);
 *
 *     // Set flag to indicate match
 *     compare_match = true;
 *   }
 * }
 *
 * void rtcc_example(void)
 * {
 *   // Enable bus clock for RTCC
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_RTCC);
 *
 *   // Initialize RTCC with default settings
 *   sl_hal_rtcc_init_t rtcc_init = SL_HAL_RTCC_INIT_DEFAULT;
 *   sl_hal_rtcc_init(&rtcc_init);
 *
 *   // Configure channel 0 for compare mode
 *   sl_hal_rtcc_cc_init_t cc_init = SL_HAL_RTCC_CH_INIT_COMPARE_DEFAULT;
 *   sl_hal_rtcc_channel_init(0, &cc_init);
 *
 *   // Set compare value for channel 0
 *   sl_hal_rtcc_set_channel_compare_value(0, 1000); // 1000 counts
 *
 *   // Reset counter
 *   sl_hal_rtcc_set_counter(0);
 *
 *   // Clear any pending interrupt flags
 *   sl_hal_rtcc_clear_interrupts(_RTCC_IF_MASK);
 *
 *   // Enable interrupts for CC0
 *   sl_hal_rtcc_enable_interrupts(RTCC_IEN_CC0);
 *
 *   // Enable NVIC interrupt for RTCC
 *   sl_interrupt_manager_enable_irq(RTCC_IRQn);
 *
 *   // Enable RTCC module
 *   sl_hal_rtcc_enable();
 *
 *   // Start RTCC counter
 *   sl_hal_rtcc_start();
 *
 *   // Wait for compare match
 *   while (!compare_match) {
 *     // Could enter low power mode here
 *   }
 *
 *   // Reset flag
 *   compare_match = false;
 *
 *   // Stop RTCC
 *   sl_hal_rtcc_stop();
 * }
 * @endcode
 *
 * @} (end addtogroup rtcc)
 ******************************************************************************/

#endif /* RTCC_COUNT && RTC_COUNT == 1 */
#endif /* SL_HAL_RTCC_H */
