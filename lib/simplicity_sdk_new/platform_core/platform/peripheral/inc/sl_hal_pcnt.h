/***************************************************************************//**
 * @file  sl_hal_pcnt.h
 *
 * @brief Pulse Counter (PCNT) peripheral API
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

#ifndef SL_HAL_PCNT_H
#define SL_HAL_PCNT_H

#include "em_device.h"
#if defined(PCNT_COUNT) && (PCNT_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_assert.h"
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup pcnt PCNT - Pulse Counter
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Check if PCNT module is valid.
#define SL_HAL_PCNT_REF_VALID(pcnt_ref) \
  (PCNT_NUM(pcnt_ref) != -1)

/// Return PCNT module max count.
#define SL_HAL_PCNT_MAX_COUNT(pcnt_ref) \
  ((uint32_t)((1ULL << PCNT_PCNT_WIDTH(PCNT_NUM(pcnt_ref))) - 1))

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// PCNT mode.
SL_ENUM(sl_hal_pcnt_mode_t) {
  /// Single input LFACLK oversampling mode (available in EM0-EM3).
  SL_HAL_PCNT_MODE_OVS_SINGLE     = _PCNT_CFG_MODE_OVSSINGLE,

  /// Externally clocked single input counter mode (available in EM0-EM3).
  SL_HAL_PCNT_MODE_EXT_CLK_SINGLE = _PCNT_CFG_MODE_EXTCLKSINGLE,

  /// Externally clocked quadrature decoder mode (available in EM0-EM3).
  SL_HAL_PCNT_MODE_EXT_CLK_QUAD   = _PCNT_CFG_MODE_EXTCLKQUAD,

  /// LFACLK oversampling quadrature decoder 1X mode (available in EM0-EM3).
  SL_HAL_PCNT_MODE_OVS_QUAD_1X    = _PCNT_CFG_MODE_OVSQUAD1X,

  /// LFACLK oversampling quadrature decoder 2X mode (available in EM0-EM3).
  SL_HAL_PCNT_MODE_OVS_QUAD_2X    = _PCNT_CFG_MODE_OVSQUAD2X,

  /// LFACLK oversampling quadrature decoder 4X mode (available in EM0-EM3).
  SL_HAL_PCNT_MODE_OVS_QUAD_4X    = _PCNT_CFG_MODE_OVSQUAD4X
};

/// Counter event selection.
SL_ENUM(sl_hal_pcnt_count_event_t) {
  /// Counts up on up-count and down on down-count events.
  SL_HAL_PCNT_COUNT_EVENT_BOTH = _PCNT_CTRL_CNTEV_BOTH,

  /// Only counts up on up-count events.
  SL_HAL_PCNT_COUNT_EVENT_UP   = _PCNT_CTRL_CNTEV_UP,

  /// Only counts down on down-count events.
  SL_HAL_PCNT_COUNT_EVENT_DOWN = _PCNT_CTRL_CNTEV_DOWN
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// PCNT initialization structure.
typedef struct {
  /// Mode to operate in.
  sl_hal_pcnt_mode_t        mode;

  /// Selects whether the main counter responds to up-count events,
  /// down-count events or both.
  sl_hal_pcnt_count_event_t main_count_event;

  /// Selects whether the auxiliary counter responds to up-count events,
  ///  down-count events or both.
  sl_hal_pcnt_count_event_t aux_count_event;

  /// Polarity of incoming edge. @n
  /// In EXT_CLK_SINGLE mode: if false, positive edges are counted,
  /// otherwise negative edges. @n
  /// In EXT_CLK_QUAD mode: if true, counting direction is inverted.
  bool                      negative_edge;

  /// Counting direction, only applicable for OVS_SINGLE and EXT_CLK_SINGLE modes.
  /// If true, counting up, otherwise counting down.
  bool                      count_down;

  /// Set to true to enable S1 to determine the direction of counting in
  /// OVS_SINGLE or EXT_CLK_SINGLE modes. @n
  /// When S1 is high, the count direction is given by @ref count_down, and when
  /// S1 is low, the count direction is the opposite.
  bool                      s1_count_direction;

  /// Enable/disable PCNT counting during debug halt. Only in OVS_SINGLE and
  /// OVS_QUAD modes.
  bool                      debug_halt;

  /// Set to true to enable hysteresis. When enabled, PCNT will always
  /// overflow and underflow to TOP/2.
  bool                      hysteresis_enable;

  /// Set to true if a PRS channel is used as input to S0IN in PCNTx_INPUT
  ///  register.
  bool                      s0_prs_enable;

  /// Set to true if a PRS channel is used as input to S1IN in PCNTx_INPUT
  /// register.
  bool                      s1_prs_enable;

  /// Enable digital pulse width filter, only available in OVS_SINGLE and
  /// OVS_QUAD modes.
  bool                      filter_enable;

  /// When set, removes flutter from Quad decoder inputs S0IN and S1IN.
  /// Available only in OVS_SINGLE and OVS_QUAD modes.
  bool                      flutter_remove;

  /// Used only in OVS_SINGLE and OVS_QUAD modes. To use this, enable filter
  /// by setting filter_enable to true. @n
  /// Filter length = (filter_lenght + 5) LFACLK cycles.
  uint8_t                   filter_lenght;
} sl_hal_pcnt_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_pcnt_init_t sl_hal_pcnt_config_t;
/** @endcond */

/// Default configuration for PCNT initialization structure.
#define SL_HAL_PCNT_INIT_DEFAULT(init_mode)             \
  {                                                     \
    .mode               = init_mode,                    \
    .main_count_event   = SL_HAL_PCNT_COUNT_EVENT_BOTH, \
    .aux_count_event    = SL_HAL_PCNT_COUNT_EVENT_BOTH, \
    .negative_edge      = false,                        \
    .count_down         = false,                        \
    .s1_count_direction = false,                        \
    .debug_halt         = false,                        \
    .hysteresis_enable  = false,                        \
    .s0_prs_enable      = false,                        \
    .s1_prs_enable      = false,                        \
    .filter_enable      = false,                        \
    .flutter_remove     = false,                        \
    .filter_lenght      = 0                             \
  }

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro name used for backward compatibility purposes.
#define SL_HAL_PCNT_CONFIG_DEFAULT(init_mode) SL_HAL_PCNT_INIT_DEFAULT(init_mode)
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize the pulse counter (PCNT).
 *
 * @details
 *   Notice that the EM23GRPACLK must be enabled in all modes, since some basic
 *   setup is done with this clock even if the external pin clock usage mode is
 *   chosen. The pulse counter clock for the selected instance must also be
 *   enabled prior to initialization.
 *
 *   Notice that pins used by the PCNT module must be properly configured
 *   by the user explicitly through setting the ROUTE register for
 *   the PCNT to work as intended.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] init
 *   Pointer to the initialization structure.
 ******************************************************************************/
void sl_hal_pcnt_init(PCNT_TypeDef *pcnt,
                      const sl_hal_pcnt_init_t *init);

/***************************************************************************//**
 * @brief
 *   Reset the PCNT to the same state that it was in after a hardware reset.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
void sl_hal_pcnt_reset(PCNT_TypeDef *pcnt);

/***************************************************************************//**
 * @brief
 *   Enable the PCNT.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_enable(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  if ((pcnt->EN & _PCNT_EN_EN_MASK) == PCNT_EN_EN) {
    return;
  }

  pcnt->EN_SET = PCNT_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable PCNT.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_disable(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  if ((pcnt->EN & _PCNT_EN_EN_MASK) != PCNT_EN_EN) {
    return;
  }

  pcnt->EN_CLR = PCNT_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Returns if the PCNT module is enabled or not.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @return Returns true if the module is enabled.
 ******************************************************************************/
__INLINE bool sl_hal_pcnt_is_enabled(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  return ((pcnt->EN & _PCNT_EN_EN_MASK) >> _PCNT_EN_EN_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_wait_ready(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  while (pcnt->SWRST & PCNT_SWRST_RESETTING) {
    // Wait for resetting to finish.
  }

  while (pcnt->EN & _PCNT_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_wait_sync(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  while (pcnt->SYNCBUSY & _PCNT_SYNCBUSY_MASK) {
    // Wait for the update to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Get status values for the PCNT.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @return
 *   Current status values.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pcnt_get_status(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  return pcnt->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more PCNT interrupts.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] flags
 *   PCNT interrupt source(s) to enable. Use one or more valid
 *   interrupt flags for the PCNT module (PCNT_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_enable_interrupts(PCNT_TypeDef *pcnt,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  pcnt->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more PCNT interrupts.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] flags
 *   PCNT interrupt source(s) to disable. Use one or more valid
 *   interrupt flags for the PCNT module (PCNT_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_disable_interrupts(PCNT_TypeDef *pcnt,
                                             uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  pcnt->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending PCNT interrupts.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] flags
 *   Pending PCNT interrupt source(s) to clear. Use one or more valid
 *   interrupt flags for the PCNT module (PCNT_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_clear_interrupts(PCNT_TypeDef *pcnt,
                                           uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  pcnt->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending PCNT interrupts from SW.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] flags
 *   PCNT interrupt source(s) to set to pending. Use one or more valid
 *   interrupt flags for the PCNT module (PCNT_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_set_interrupts(PCNT_TypeDef *pcnt,
                                         uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  pcnt->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending PCNT interrupt flags.
 *
 * @note
 *   Event bits are not cleared by this function.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @return
 *   PCNT interrupt source(s) pending. Returns one or more valid
 *   interrupt flags for the PCNT module (PCNT_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pcnt_get_pending_interrupts(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  return pcnt->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending PCNT interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @note
 *   Interrupt flags are not cleared by this function.
 *
 * @return
 *   Pending and enabled PCNT interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in PCNTx_IEN_nnn
 *     register (PCNTx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the PCNT module
 *     (PCNTx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_pcnt_get_enabled_pending_interrupts(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  // Store PCNT->IEN in temporary variable in order to define explicit order
  // of volatile accesses.
  uint32_t ien = pcnt->IEN;

  return pcnt->IF & ien;
}

/***************************************************************************//**
 * @brief
 *   Lock PCNT registers.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @note When PCNT registers are locked PCNT_CFG, PCNT_EN, PCNT_SWRST, PCNT_CMD,
 *       PCNT_CTRL, PCNT_OVSCTRL, PCNT_CNT, PCNT_TOP, and PCNT_TOPB registers
 *       cannot be written to.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_lock(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  pcnt->LOCK = ~PCNT_LOCK_PCNTLOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Unlock PCNT registers.
 *
 * @param[in] pcnt
 *   Pointer to thePCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_unlock(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  pcnt->LOCK = PCNT_LOCK_PCNTLOCKKEY_UNLOCK;
}

/***************************************************************************//**
 * @brief
 *   Set the top value buffer.
 *
 * @details
 *   When top value buffer register is updated, value is loaded into
 *   top value register at the next wrap around. This feature is useful
 *   in order to update top value safely when PCNT is running. This won't
 *   happen if @ref SL_HAL_PCNT_COUNT_EVENT_BOTH is used.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value buffer register.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_set_top_buffer(PCNT_TypeDef *pcnt,
                                         uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_PCNT_MAX_COUNT(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->TOPB = value;
}

/***************************************************************************//**
 * @brief
 *   Get the top value buffer.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @return
 *   Current top value buffer.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pcnt_get_top_buffer(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  sl_hal_pcnt_wait_sync(pcnt);
  return pcnt->TOPB;
}

/***************************************************************************//**
 * @brief
 *   Set the top value.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] value
 *   Value to set in top value register.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_set_top(PCNT_TypeDef *pcnt,
                                  uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_PCNT_MAX_COUNT(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->TOP = value;
}

/***************************************************************************//**
 * @brief
 *   Get the top value.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @return
 *   Current top value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pcnt_get_top(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  sl_hal_pcnt_wait_sync(pcnt);
  return pcnt->TOP;
}

/***************************************************************************//**
 * @brief
 *   Start PCNT main counter.
 *
 * @details
 *   This function will send a start command to the PCNT peripheral. The PCNT
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_pcnt_wait_sync() function can be used to wait for the start
 *   command to be executed.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_start_main_counter(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_STARTCNT;
}

/***************************************************************************//**
 * @brief
 *   Stop PCNT main counter.
 *
 * @details
 *   This function will send a stop command to the PCNT peripheral. The PCNT
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_pcnt_wait_sync() function can be used to wait for the stop
 *   command to be executed.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_stop_main_counter(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_STOPCNT;
}

/***************************************************************************//**
 * @brief
 *   Set a counter value.
 *
 * @note
 *   This function could stall until synchronization to low-frequency domain is
 *   completed. For that reason, it should normally not be used when
 *   an external clock is used for the PCNT module, since stall time may be
 *   undefined.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @param[in] value
 *   Value to set in counter register.
 ******************************************************************************/
void sl_hal_pcnt_set_main_counter(PCNT_TypeDef *pcnt,
                                  uint32_t value);

/***************************************************************************//**
 * @brief
 *   Get the main counter value.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @return
 *   Current main counter value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pcnt_get_main_counter(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  sl_hal_pcnt_wait_sync(pcnt);
  return pcnt->CNT;
}

/***************************************************************************//**
 * @brief
 *   Start PCNT auxiliary counter.
 *
 * @details
 *   This function will send a start command to the PCNT peripheral. The PCNT
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_pcnt_wait_sync() function can be used to wait for the start
 *   command to be executed.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_start_aux_counter(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_STARTAUXCNT;
}

/***************************************************************************//**
 * @brief
 *   Stop PCNT auxiliary counter.
 *
 * @details
 *   This function will send a stop command to the PCNT peripheral. The PCNT
 *   peripheral will use some LF clock ticks before the command is executed.
 *   The @ref sl_hal_pcnt_wait_sync() function can be used to wait for the stop
 *   command to be executed.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_stop_aux_counter(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_STOPAUXCNT;
}

/***************************************************************************//**
 * @brief
 *   Get the auxiliary counter value.
 *
 * @param[in] pcnt
 *   Pointer to the PCNT peripheral register block.
 *
 * @return
 *   Current auxiliary counter value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_pcnt_get_aux_counter(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  sl_hal_pcnt_wait_sync(pcnt);
  return pcnt->AUXCNT;
}

/***************************************************************************//**
 * @brief
 *   Reset PCNT counters.
 *
 * @param[in] pcnt
 *   A pointer to the PCNT peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_pcnt_reset_counters(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_CNTRST | PCNT_CMD_AUXCNTRST;
}

/// @} (end addtogroup pcnt)
#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup pcnt PCNT - Pulse Counter
 * @{
 *
 * @n @section pcnt_example Example
 *  The following example demonstrates basic usage of the PCNT peripheral in
 *  oversampling single input mode.
 *
 * @code{.c}
 * void pcnt_basic_example(void)
 * {
 *   // Enable bus clock for PCNT0
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PCNT0);
 *
 *   // Configure PCNT with default settings for single input oversampling mode
 *   sl_hal_pcnt_init_t init = SL_HAL_PCNT_INIT_DEFAULT(SL_HAL_PCNT_MODE_OVS_SINGLE);
 *
 *   // Initialize PCNT with our configuration
 *   sl_hal_pcnt_init(PCNT0, &init);
 *
 *   // Enable the PCNT peripheral
 *   sl_hal_pcnt_enable(PCNT0);
 *
 *   // Start the main counter
 *   sl_hal_pcnt_start_main_counter(PCNT0);
 *
 *   // Wait for synchronization to complete
 *   sl_hal_pcnt_wait_sync(PCNT0);
 *
 *   // Check if counter is running by reading status
 *   uint32_t status = sl_hal_pcnt_get_status(PCNT0);
 *   bool is_running = status & PCNT_STATUS_CNTRUNNING;
 *
 *   if (is_running) {
 *     // Read current counter value
 *     uint32_t count_initial = sl_hal_pcnt_get_main_counter(PCNT0);
 *
 *     // Optionally set counter to a specific value
 *     uint32_t count_new = count_initial + 10;
 *     sl_hal_pcnt_set_main_counter(PCNT0, count_new);
 *
 *     // Read the updated counter value
 *     uint32_t count_after = sl_hal_pcnt_get_main_counter(PCNT0);
 *
 *     // Application processing...
 *
 *     // Reset the counter when needed
 *     sl_hal_pcnt_reset_counters(PCNT0);
 *
 *     // Counter value should now be 0
 *     count_after = sl_hal_pcnt_get_main_counter(PCNT0);
 *   }
 *
 *   // Stop the counter when done
 *   sl_hal_pcnt_stop_main_counter(PCNT0);
 *   sl_hal_pcnt_wait_sync(PCNT0);
 *
 *   // Verify counter is stopped
 *   status = sl_hal_pcnt_get_status(PCNT0);
 *   is_running = status & PCNT_STATUS_CNTRUNNING;
 *
 *   // Clean up - disable PCNT when no longer needed
 *   sl_hal_pcnt_disable(PCNT0);
 * }
 *
 * @endcode
 *
 * @} (end addtogroup pcnt)
 ******************************************************************************/

#endif // defined(PCNT_COUNT) && (PCNT_COUNT > 0)
#endif // SL_HAL_PCNT_H
