/***************************************************************************//**
 * @file
 * @brief System Real Time Counter (SYSRTC) peripheral API for Sub-Systems
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

#ifndef SL_HAL_SYSRTC_SUBSYSTEM_H
#define SL_HAL_SYSRTC_SUBSYSTEM_H

#include "em_device.h"

#if defined(SYSRTC_COUNT) && (SYSRTC_COUNT > 0)
#include <stdbool.h>
#include "sl_code_classification.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_hal_sysrtc_compat.h"
#include "sl_enum.h"
#include "sl_assert.h"
#if defined(SL_CATALOG_HAL_SYSRTC_INTERNAL_PRESENT)
#include "sli_hal_sysrtc_subsystem.h"
#endif

/***************************************************************************//**
 * @addtogroup sysrtc SYSRTC - System RTC
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Minimum compare channels for SYSRTC group.
#define SYSRTC_GROUP_MIN_CHANNEL_COMPARE  1u
/// Maximum compare channels for SYSRTC group.
#define SYSRTC_GROUP_MAX_CHANNEL_COMPARE  3u

/// Minimum capture channels for SYSRTC group.
#define SYSRTC_GROUP_MIN_CHANNEL_CAPTURE  0u
/// Maximum capture channels for SYSRTC group.
#define SYSRTC_GROUP_MAX_CHANNEL_CAPTURE  1u

/// SYSRTC instance.
#if !defined(SYSRTC_INST)
#define SYSRTC_INST   SYSRTC0
#endif

/// Number of SYSRTC groups.
#if !defined(SYSRTC_GROUP_NUMBER)
#define SYSRTC_GROUP_NUMBER   1u
#endif

/// Validation of valid SYSRTC group for assert statements.
#define SYSRTC_GROUP_VALID(group)    ((unsigned)(group) < SYSRTC_GROUP_NUMBER)

/*******************************************************************************
 *********************************   ENUM   ************************************
 ******************************************************************************/

/// Capture input edge select.
SL_ENUM(sl_hal_sysrtc_capture_edge_t) {
  SL_HAL_SYSRTC_CAPTURE_EDGE_RISING = 0,  ///< Rising edges detected.
  SL_HAL_SYSRTC_CAPTURE_EDGE_FALLING,     ///< Falling edges detected.
  SL_HAL_SYSRTC_CAPTURE_EDGE_BOTH         ///< Both edges detected.
};

/// Compare match output action mode.
SL_ENUM(sl_hal_sysrtc_compare_match_out_action_t) {
  SL_HAL_SYSRTC_COMPARE_MATCH_OUT_ACTION_CLEAR = 0, ///< Clear output.
  SL_HAL_SYSRTC_COMPARE_MATCH_OUT_ACTION_SET,       ///< Set output.
  SL_HAL_SYSRTC_COMPARE_MATCH_OUT_ACTION_PULSE,     ///< Generate a pulse.
  SL_HAL_SYSRTC_COMPARE_MATCH_OUT_ACTION_TOGGLE,    ///< Toggle output.
  SL_HAL_SYSRTC_COMPARE_MATCH_OUT_ACTION_CMPIF      ///< Export CMPIF.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// Compare channel configuration structure.
typedef struct {
  sl_hal_sysrtc_compare_match_out_action_t  compare_match_out_action; ///< Compare mode channel match output action.
} sl_hal_sysrtc_group_channel_compare_config_t;

/// Capture channel configuration structure.
typedef struct {
  sl_hal_sysrtc_capture_edge_t  capture_input_edge; ///< Capture mode channel input edge.
} sl_hal_sysrtc_group_channel_capture_config_t;

/// Group configuration structure.
typedef struct {
  bool compare_channel0_enable;                                                  ///< Enable/Disable compare channel 0.
  bool compare_channel1_enable;                                                  ///< Enable/Disable compare channel 1.
  bool compare_channel2_enable;                                                  ///< Enable/Disable compare channel 2.
  bool capture_channel0_enable;                                                  ///< Enable/Disable capture channel 0.
  sl_hal_sysrtc_group_channel_compare_config_t const *p_compare_channel0_config; ///< Pointer to compare channel 0 config.
  sl_hal_sysrtc_group_channel_compare_config_t const *p_compare_channel1_config; ///< Pointer to compare channel 1 config.
  sl_hal_sysrtc_group_channel_compare_config_t const *p_compare_channel2_config; ///< Pointer to compare channel 2 config.
  sl_hal_sysrtc_group_channel_capture_config_t const *p_capture_channel0_config; ///< Pointer to capture channel 0 config.
} sl_hal_sysrtc_group_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_sysrtc_group_init_t sl_hal_sysrtc_group_config_t;
/** @endcond */

/// Suggested default values for compare channel configuration structure.
#define SL_HAL_SYSRTC_GROUP_CHANNEL_COMPARE_INIT_DEFAULT \
  {                                                      \
    SL_HAL_SYSRTC_COMPARE_MATCH_OUT_ACTION_PULSE         \
  }

/// Compare channel configuration for starting HFXO using PRS.
#define SL_HAL_SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_EARLY_WAKEUP \
  {                                                             \
    SL_HAL_SYSRTC_COMPARE_MATCH_OUT_ACTION_CMPIF                \
  }

/// Suggested default values for capture channel configuration structure.
#define SL_HAL_SYSRTC_GROUP_CHANNEL_CAPTURE_INIT_DEFAULT \
  {                                                      \
    SL_HAL_SYSRTC_CAPTURE_EDGE_RISING                    \
  }

/// Suggested default values for SYSRTC group configuration structure.
#define SL_HAL_SYSRTC_GROUP_INIT_DEFAULT                                        \
  {                                                                             \
    true,  /* Enable compare channel 0. */                                      \
    false, /* Disable compare channel 1. */                                     \
    false, /* Disable compare channel 2. */                                     \
    false, /* Disable capture channel 0. */                                     \
    NULL,  /* NULL Pointer to configuration structure for compare channel 0. */ \
    NULL,  /* NULL Pointer to configuration structure for compare channel 1. */ \
    NULL,  /* NULL Pointer to configuration structure for compare channel 2. */ \
    NULL   /* NULL Pointer to configuration structure for capture channel 0. */ \
  }

/// Pre-trigger configuration structure.
typedef struct {
  bool enable;   ///< Enable/Disable the pre-trigger.
  uint8_t ticks; ///< Number of LF clock ticks the pre-trigger occurs before the counter reach the compare channel.
} sl_hal_sysrtc_pretrigger_config_t;

/// Group pre-trigger configuration structure.
typedef struct {
  sl_hal_sysrtc_pretrigger_config_t emu_wakeup; ///< Pre-trigger to wake up EMU.
  sl_hal_sysrtc_pretrigger_config_t hfxo_start; ///< Pre-trigger to start HFXO.
  uint8_t compare_channel;                      ///< Compare channel used for the pre-triggers.
} sl_hal_sysrtc_group_pretrigger_config_t;

/// Suggested default values for pre-trigger configuration structure.
#define SL_HAL_SYSRTC_PRETRIGGER_INIT_DEFAULT                          \
  {                                                                    \
    false, /* Disable the pre-trigger. */                              \
    0,     /* The pre-trigger occurs 0 LF ticks before the compare. */ \
  }

/// Suggested default values for group pre-trigger configuration structure.
#define SL_HAL_SYSRTC_GROUP_PRETRIGGER_INIT_DEFAULT                                                \
  {                                                                                                \
    SL_HAL_SYSRTC_PRETRIGGER_INIT_DEFAULT, /* Default pre-trigger configuration to wake up EMU. */ \
    SL_HAL_SYSRTC_PRETRIGGER_INIT_DEFAULT, /* Default pre-trigger configuration to start HFXO. */  \
    0                                      /* Compare channel used by the pre-triggers. */         \
  }

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Alias for deprecated macro names used for backward compatibility purposes.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_DEFAULT       SL_HAL_SYSRTC_GROUP_CHANNEL_COMPARE_INIT_DEFAULT
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_EARLY_WAKEUP  SL_HAL_SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_EARLY_WAKEUP
#define SYSRTC_GROUP_CHANNEL_CAPTURE_CONFIG_DEFAULT       SL_HAL_SYSRTC_GROUP_CHANNEL_CAPTURE_INIT_DEFAULT
#define SYSRTC_GROUP_CONFIG_DEFAULT                       SL_HAL_SYSRTC_GROUP_INIT_DEFAULT
#define SYSRTC_PRETRIGGER_CONFIG_DEFAULT                  SL_HAL_SYSRTC_PRETRIGGER_INIT_DEFAULT
#define SYSRTC_GROUP_PRETRIGGER_CONFIG_DEFAULT            SL_HAL_SYSRTC_GROUP_PRETRIGGER_INIT_DEFAULT
/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Waits for the SYSRTC to complete all synchronization of register changes
 *   and commands.
 *
 * @brief
 *   Wait for synchronization of register changes to low frequency domain.
 *
 * @note
 *   This function will stall execution until the synchronization is complete.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
__INLINE void sl_hal_sysrtc_wait_sync(void)
{
#if defined(_SYSRTC_EN_MASK)
  while ((SYSRTC_INST->EN & SYSRTC_EN_EN) && (SYSRTC_INST->SYNCBUSY != 0U)) {
    // Wait for all synchronizations to finish.
  }
#else
  while (SYSRTC_INST->SYNCBUSY != 0U) {
    // Wait for all synchronizations to finish.
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Gets SYSRTC STATUS register value.
 *
 * @return
 *   Current STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_sysrtc_get_status(void)
{
  return SYSRTC_INST->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Gets SYSRTC counter value.
 *
 * @return
 *   Current SYSRTC counter value.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
__INLINE uint32_t sl_hal_sysrtc_get_counter(void)
{
  // Wait for Counter to synchronize before getting the value.
  sl_hal_sysrtc_wait_sync();

  return SYSRTC_INST->CNT;
}

/***************************************************************************//**
 * @brief
 *   Waits for the SYSRTC group to complete all synchronization of register
 *   changes and commands.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
void sl_hal_sysrtc_wait_sync_group(uint8_t group_number);

/***************************************************************************//**
 * @brief
 *   Initializes the selected SYSRTC group.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] group_init
 *   Pointer to group initialization structure variable.
 ******************************************************************************/
void sl_hal_sysrtc_init_group(uint8_t group_number,
                              sl_hal_sysrtc_group_init_t const *group_init);

/***************************************************************************//**
 * @brief
 *   Enables one or more SYSRTC interrupts for the given group.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   sl_hal_sysrtc_clear_group_interrupts() prior to enabling the interrupt.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] flags
 *   SYSRTC interrupt sources to enable.
 *   Use a set of interrupt flags OR-ed together to set
 *   multiple interrupt sources for the given SYSRTC group.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
void sl_hal_sysrtc_enable_group_interrupts(uint8_t group_number,
                                           uint32_t flags);

/***************************************************************************//**
 * @brief
 *   Disables one or more SYSRTC interrupts for the given group.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] flags
 *   SYSRTC interrupt sources to disable.
 *   Use a set of interrupt flags OR-ed together to disable
 *   multiple interrupt sources for the given SYSRTC group.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
void sl_hal_sysrtc_disable_group_interrupts(uint8_t group_number,
                                            uint32_t flags);

/***************************************************************************//**
 * @brief
 *   Clears one or more pending SYSRTC interrupts for the given group.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] flags
 *   SYSRTC interrupt sources to clear.
 *   Use a set of interrupt flags OR-ed together to clear
 *   multiple interrupt sources for the given SYSRTC group.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
void sl_hal_sysrtc_clear_group_interrupts(uint8_t group_number,
                                          uint32_t flags);

/***************************************************************************//**
 * @brief
 *   Gets pending SYSRTC interrupt flags for the given group.
 *
 * @note
 *   Event bits are not cleared by using this function.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @return
 *   Pending SYSRTC interrupt sources.
 *   Returns a set of interrupt flags OR-ed together for multiple
 *   interrupt sources in the SYSRTC group.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t sl_hal_sysrtc_get_group_interrupts(uint8_t group_number);

/***************************************************************************//**
 * @brief
 *   Gets enabled and pending SYSRTC interrupt flags for the given group.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note
 *   Interrupt flags are not cleared by using this function.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @return
 *   Pending and enabled SYSRTC interrupt sources.
 *   The return value is the bitwise AND of
 *   - the enabled interrupt sources in SYSRTC_GRPx_IEN and
 *   - the pending interrupt flags SYSRTC_GRPx_IF.
 ******************************************************************************/
uint32_t sl_hal_sysrtc_get_group_enabled_interrupts(uint8_t group_number);

/***************************************************************************//**
 * @brief
 *   Sets one or more pending SYSRTC interrupts for the given group from Software.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] flags
 *   SYSRTC interrupt sources to set to pending. Use a set of interrupt flags
 *   OR-ed together to set multiple interrupt sources for the SYSRTC group.
 ******************************************************************************/
void sl_hal_sysrtc_set_group_interrupts(uint8_t group_number,
                                        uint32_t flags);

/***************************************************************************//**
 * @brief
 *   Gets SYSRTC compare register value for selected channel of given group.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] channel
 *   Channel selector.
 *
 * @return
 *   Compare register value.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t sl_hal_sysrtc_get_group_compare_channel_value(uint8_t group_number,
                                                       uint8_t channel);

/***************************************************************************//**
 * @brief
 *   Sets SYSRTC compare register value for selected channel of given group.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] channel
 *   Channel selector.
 *
 * @param[in] value
 *   Compare register value.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
void sl_hal_sysrtc_set_group_compare_channel_value(uint8_t group_number,
                                                   uint8_t channel,
                                                   uint32_t value);

/***************************************************************************//**
 * @brief
 *   Gets SYSRTC input capture register value for capture channel of given group.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @return
 *   Capture register value.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_HAL_SYSRTC, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t sl_hal_sysrtc_get_group_capture_channel_value(uint8_t group_number);

/***************************************************************************//**
 * @brief
 *   Sets the pre-triggers for a given group.
 *
 * @param[in] group_number
 *   SYSRTC group number to use.
 *
 * @param[in] group_pretrigger_config
 *   Pointer to group pre-trigger configuration structure variable.
 ******************************************************************************/
void sl_hal_sysrtc_set_group_pretrigger(uint8_t group_number,
                                        sl_hal_sysrtc_group_pretrigger_config_t const *group_pretrigger_config);

/***************************************************************************//**
 * @brief
 *   Gets the status of the pre-triggers for a given group.
 *
 * @param[in]  group_number
 *   SYSRTC group number to use.
 *
 * @return
 *   Pre-trigger status flags for a given group.
 ******************************************************************************/
uint32_t sl_hal_sysrtc_get_group_pretrigger_status(uint8_t group_number);

/***************************************************************************//**
 * @brief
 *   Clears the status of the pre-triggers for a given group.
 *
 * @details
 *   The ACTIVE status must be cleared from Software.
 *
 * @param[in]  group_number
 *   SYSRTC group number to use.
 *
 * @param[out] flags
 *   Pre-trigger status flags for a given group.
 ******************************************************************************/
void sl_hal_sysrtc_clear_group_pretrigger_status(uint8_t group_number,
                                                 uint32_t flags);

/** @} (end addtogroup sysrtc) */

#ifdef __cplusplus
}
#endif

#endif /* defined(SYSRTC_COUNT) && (SYSRTC_COUNT > 0) */
#endif /* SL_HAL_SYSRTC_SUBSYSTEM_H */
