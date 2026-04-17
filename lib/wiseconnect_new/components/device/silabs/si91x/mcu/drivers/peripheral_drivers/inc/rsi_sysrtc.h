/******************************************************************************
* @file  rsi_sysrtc.h
* @brief System Real Time Counter (SYSRTC) peripheral API
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

#ifndef RSI_SYSRTC_H
#define RSI_SYSRTC_H

#define SYSRTC_CMP_DEF_PINS

#include "si91x_device.h"
#if defined(SI91X_SYSRTC_COUNT) && (SI91X_SYSRTC_COUNT > 0)
#include "rsi_sysrtc_headers.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/ /**
 * @addtogroup sysrtc
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Minimum compare channels for SYSRTC group.
#define SYSRTC_GROUP_MIN_CHANNEL_COMPARE 1u
/// Maximum compare channels for SYSRTC group.
#define SYSRTC_GROUP_MAX_CHANNEL_COMPARE 2u

/// Minimum capture channels for SYSRTC group.
#define SYSRTC_GROUP_MIN_CHANNEL_CAPTURE 0u
/// Maximum capture channels for SYSRTC group.
#define SYSRTC_GROUP_MAX_CHANNEL_CAPTURE 1u

/// Sysrtc group number.
#if defined(SLI_SI917B0)
#if !defined(SYSRTC_GROUP_NUMBER)
#define SYSRTC_GROUP_NUMBER 2u
#endif
#endif

/// Validation of valid SYSRTC group for assert statements.
#define SYSRTC_GROUP_VALID(group) ((unsigned)(group) < SYSRTC_GROUP_NUMBER)

/*NPSS INTERRUPT */
#ifndef NPSS_INTR_BASE
#define NPSS_INTR_BASE 0x12080000
#endif // NPSS_INTR_BASE
#ifndef NPSS_INTR_MASK_SET_REG
#define NPSS_INTR_MASK_SET_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x00))
#endif // NPSS_INTR_MASK_SET_REG
#ifndef NPSS_INTR_MASK_CLR_REG
#define NPSS_INTR_MASK_CLR_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x04))
#endif // NPSS_INTR_MASK_CLR_REG
#ifndef NPSS_INTR_CLEAR_REG
#define NPSS_INTR_CLEAR_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x08))
#endif // NPSS_INTR_CLEAR_REG
#ifndef NPSS_TO_MCU_SYSRTC_INTR
#define NPSS_TO_MCU_SYSRTC_INTR BIT(10)
#endif // NPSS_TO_MCU_SYSRTC_INTR

/*******************************************************************************
 *********************************   ENUM   ************************************
 ******************************************************************************/

/// Capture input edge select.
typedef enum {
  RSI_SYSRTC_CAPTURE_EDGE_RISING = 0, ///< Rising edges detected.
  RSI_SYSRTC_CAPTURE_EDGE_FALLING,    ///< Falling edges detected.
  RSI_SYSRTC_CAPTURE_EDGE_BOTH        ///< Both edges detected.
} rsi_sysrtc_capture_edge_t;

/// Compare match output action mode.
typedef enum {
  RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_CLEAR = 0, ///< Clear output.
  RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_SET,       ///< Set output.
  RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_PULSE,     ///< Generate a pulse.
  RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_TOGGLE,    ///< Toggle output.
  RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_CMPIF      ///< Export CMPIF.
} rsi_sysrtc_compare_match_out_action_t;

/// Capture input edge select.
typedef enum {
  RSI_SYSRTC_CLK_NONE       = 0, ///< No Clk select.
  RSI_SYSRTC_CLK_1kHz_clk   = 1, ///< 1 khz  clk select.
  RSI_SYSRTC_CLK_32KHz_RO   = 2, ///< 32 khz RO clk select.
  RSI_SYSRTC_CLK_32kHz_RC   = 4, ///< 32 khz RC clk select.
  RSI_SYSRTC_CLK_32kHz_Xtal = 8, ///< 32 khz XTAL or External Oscillator clk select.

} rsi_sysrtc_clk_inp_t;

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// SYSRTC configuration structure.
typedef struct {
  bool enable_debug_run; ///< Counter shall keep running during debug halt.
} rsi_sysrtc_config_t;

/// Suggested default values for SYSRTC configuration structure.
#define SYSRTC_CONFIG_DEFAULT                        \
  {                                                  \
    false, /* Disable updating during debug halt. */ \
  }

/// Compare channel configuration structure.
typedef struct {
  rsi_sysrtc_compare_match_out_action_t compare_match_out_action; ///< Compare mode channel match output action.
} rsi_sysrtc_group_channel_compare_config_t;

/// Capture channel configuration structure.
typedef struct {
  rsi_sysrtc_capture_edge_t capture_input_edge; ///< Capture mode channel input edge.
} rsi_sysrtc_group_channel_capture_config_t;

/// Group configuration structure.
typedef struct {
  bool compare_channel0_enable;                                               ///< Enable/Disable compare channel 0
  bool compare_channel1_enable;                                               ///< Enable/Disable compare channel 1
  bool capture_channel0_enable;                                               ///< Enable/Disable capture channel 0
  rsi_sysrtc_group_channel_compare_config_t const *p_compare_channel0_config; ///< Pointer to compare channel 0 config
  rsi_sysrtc_group_channel_compare_config_t const *p_compare_channel1_config; ///< Pointer to compare channel 1 config
  rsi_sysrtc_group_channel_capture_config_t const *p_capture_channel0_config; ///< Pointer to capture channel 0 config
} rsi_sysrtc_group_config_t;

/// Suggested default values for compare channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_DEFAULT \
  {                                                 \
    RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_PULSE       \
  }

/// Suggested default values for compare channel configuration structure, when using register input.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_DEFAULT_REGMODE \
  {                                                         \
    RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_SET                 \
  }

/// Suggested default values for capture channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_CAPTURE_CONFIG_DEFAULT \
  {                                                 \
    RSI_SYSRTC_CAPTURE_EDGE_RISING                  \
  }

/// Suggested default values for SYSRTC group configuration structure.
#define SYSRTC_GROUP_CONFIG_DEFAULT                                             \
  {                                                                             \
    true,    /* Enable compare channel 0. */                                    \
      false, /* Disable compare channel 1. */                                   \
      false, /* Disable capture channel 0. */                                   \
      NULL,  /* NULL Pointer to configuration structure for compare channel 0*/ \
      NULL,  /* NULL Pointer to configuration structure for compare channel 1*/ \
      NULL   /* NULL Pointer to configuration structure for capture channel 0*/ \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initializes SYSRTC module.
 *
 * Note that the compare values must be set separately with
 * (rsi_sysrtc_set_group_compare_channel_value()), which should probably be
 * done prior to the use of this function if configuring the SYSRTC to start
 * when initialization is completed.
 *
 * @param[in] p_config  A pointer to the SYSRTC initialization structure
 *                      variable.
 ******************************************************************************/
void rsi_sysrtc_init(const rsi_sysrtc_config_t *p_config);

/***************************************************************************/ /**
 * Enables SYSRTC Module
 ******************************************************************************/
void rsi_sysrtc_enable(void);

/***************************************************************************/ /**
 * Disables SYSRTC counting.
 ******************************************************************************/
void rsi_sysrtc_disable(void);

/***************************************************************************/ /**
 * Waits for the SYSRTC to complete all synchronization of register changes
 * and commands.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_wait_sync(void)
{

  while ((SYSRTC0->SYNCBUSY))
    ;
}

/***************************************************************************/ /**
 * Waits for the SYSRTC to complete reseting or disabling procedure.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_wait_ready(void)
{
  // Wait for all synchronizations to finish
  while ((SYSRTC0->SWRST & _SYSRTC_SWRST_RESETTING_MASK) || (SYSRTC0->EN & _SYSRTC_EN_SYNC_MASK)
         || (SYSRTC0->SYNCBUSY != 0U))
    ;
  return;
}

/***************************************************************************/ /**
 * Waits for the SYSRTC Group0 to complete all synchronization of register changes
 * and commands.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_grp0_sync()
{
  while ((SYSRTC0->GRP0_SYNCBUSY != 0))
    ;
  // Wait for all synchronizations to finish
}

/***************************************************************************/ /**
 * Waits for the SYSRTC Group to complete all synchronization of register changes
 * and commands.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_grp1_sync()
{
  while ((SYSRTC0->GRP1_SYNCBUSY != 0))
    ;
  // Wait for all synchronizations to finish
}

/***************************************************************************/ /**
 * Starts SYSRTC counter.
 *
 * This function will send a start command to the SYSRTC peripheral. The SYSRTC
 * peripheral will use some LF clock ticks before the command is executed.
 * The rsi_sysrtc_wait_sync() function can be used to wait for the start
 * command to be executed.
 *
 * @note  This function requires the SYSRTC to be enabled.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_start(void)
{

  SYSRTC0->CMD = SYSRTC_CMD_START;
  // Wait till START is synchronize with LF clk
  rsi_sysrtc_wait_sync();
}

/***************************************************************************/ /**
 * Stops the SYSRTC counter.
 *
 * This function will send a stop command to the SYSRTC peripheral. The SYSRTC
 * peripheral will use some LF clock ticks before the command is executed.
 * The rsi_sysrtc_wait_sync() function can be used to wait for the stop
 * command to be executed.
 *
 * @note  This function requires the SYSRTC to be enabled.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_stop(void)
{
  SYSRTC0->CMD = SYSRTC_CMD_STOP;
  // Wait till STOP is synchronize with LF clk
  while ((SYSRTC0->SYNCBUSY & _SYSRTC_SYNCBUSY_STOP_MASK))
    ;
}

/***************************************************************************/ /**
 * Restores SYSRTC to its reset state.
 ******************************************************************************/
void rsi_sysrtc_reset(void);

/***************************************************************************/ /**
 * Gets SYSRTC STATUS register value.
 *
 * @return  Current STATUS register value.
 ******************************************************************************/
__STATIC_INLINE uint32_t rsi_sysrtc_get_status(void)
{
  return SYSRTC0->STATUS;
}

/***************************************************************************/ /**
 * Locks SYSRTC registers.
 *
 * @note  When SYSRTC registers are locked SYSRTC_EN, SYSRTC_CFG, SYSRTC_CMD,
 *        SYSRTC_SWRST, SYSRTC_CNT and SYSRTC_TOPCNT registers cannot be written
 *        to.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_lock(void)
{
  SYSRTC0->LOCK = ~SYSRTC_LOCK_LOCKKEY_UNLOCK;
}

/***************************************************************************/ /**
 * Unlocks SYSRTC registers.
 *
 * @note  When SYSRTC registers are locked SYSRTC_EN, SYSRTC_CFG, SYSRTC_CMD,
 *        SYSRTC_SWRST, SYSRTC_CNT and SYSRTC_TOPCNT registers cannot be written
 *        to.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_unlock(void)
{
  SYSRTC0->LOCK = SYSRTC_LOCK_LOCKKEY_UNLOCK;
}

/***************************************************************************/ /**
 * Gets SYSRTC counter value.
 *
 * @return  Current SYSRTC counter value.
 ******************************************************************************/
__STATIC_INLINE uint32_t rsi_sysrtc_get_counter(void)
{
  return SYSRTC0->CNT;
}

/***************************************************************************/ /**
 * Sets the SYSRTC counter value.
 *
 * @param[in] value   The new SYSRTC counter value.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_set_counter(uint32_t value)
{
  SYSRTC0->CNT = value;
  while ((SYSRTC0->SYNCBUSY & _SYSRTC_SYNCBUSY_CNT_MASK))
    ;
}
/***************************************************************************/ /**
 * Sets the SYSRTC NPSS to MCU Interrupt bit.
 *
 * @param[in] void.
 ******************************************************************************/
__STATIC_INLINE void rsi_sysrtc_clear_npss_mcu_intr(void)
{
  MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR_b.SYSRTC_WAKEUP_CLEAR_b = 1U;
}

/**
 * @fn            STATIC INLINE void RSI_PS_NpssIntrUnMask(uint32_t mask)
 * @brief   This API is used to  un mask the NPSS interrupts
 * @param[in]     mask  is OR'ed value of the NPSS interrupt bits
 *                \ref NPSS_INTR_MASK_CLR_REG
 * @return    none
 */
__STATIC_INLINE void RSI_NpssIntrUnMask(uint32_t mask)
{
  NPSS_INTR_MASK_CLR_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_PS_NpssIntrMask(uint32_t mask)
 * @brief   This API is used to  mask the NPSS interrupts
 * @param[in]     mask  is OR'ed value of the NPSS interrupt bits
 *                \ref NPSS_INTR_MASK_SET_REG
 * @return    none
 */
__STATIC_INLINE void RSI_NpssIntrMask(uint32_t mask)
{
  NPSS_INTR_MASK_SET_REG = mask;
}

/**
 * @fn            STATIC INLINE void RSI_NpssIntrClear(uint32_t mask)
 * @brief   This API is used to  clear the NPSS interrupts
 * @param[in]     mask  is OR'ed value of the NPSS interrupt bits
 *                \ref NPSS_INTR_CLEAR_REG
 * @return    none
 */
__STATIC_INLINE void RSI_NpssIntrClear(uint32_t mask)
{
  NPSS_INTR_CLEAR_REG = mask;
}

/***************************************************************************/ /**
 * Initializes the selected SYSRTC group.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @param[in] p_group_config  Pointer to group configuration structure
 *                            variable.
 ******************************************************************************/
void rsi_sysrtc_init_group(uint8_t group_number, rsi_sysrtc_group_config_t const *p_group_config);

/***************************************************************************/ /**
 * Enables one or more SYSRTC interrupts for the given group.
 *
 * @note  Depending on the use, a pending interrupt may already be set prior to
 *        enabling the interrupt. To ignore a pending interrupt, consider using
 *        rsi_sysrtc_clear_group_interrupts() prior to enabling the interrupt.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @param[in] flags   SYSRTC interrupt sources to enable.
 *                    Use a set of interrupt flags OR-ed together to set
 *                    multiple interrupt sources for the given SYSRTC group.
 ******************************************************************************/
void rsi_sysrtc_enable_group_interrupts(uint8_t group_number, uint32_t flags);

/***************************************************************************/ /**
 * Disables one or more SYSRTC interrupts for the given group.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @param[in] flags   SYSRTC interrupt sources to disable.
 *                    Use a set of interrupt flags OR-ed together to disable
 *                    multiple interrupt sources for the given SYSRTC group.
 ******************************************************************************/
void rsi_sysrtc_disable_group_interrupts(uint8_t group_number, uint32_t flags);

/***************************************************************************/ /**
 * Clears one or more pending SYSRTC interrupts for the given group.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @param[in] flags   SYSRTC interrupt sources to clear.
 *                    Use a set of interrupt flags OR-ed together to clear
 *                    multiple interrupt sources for the given SYSRTC group.
 ******************************************************************************/
void rsi_sysrtc_clear_group_interrupts(uint8_t group_number, uint32_t flags);

/***************************************************************************/ /**
 * Gets pending SYSRTC interrupt flags for the given group.
 *
 * @note  Event bits are not cleared by using this function.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @return  Pending SYSRTC interrupt sources.
 *          Returns a set of interrupt flags OR-ed together for multiple
 *          interrupt sources in the SYSRTC group.
 ******************************************************************************/
uint32_t rsi_sysrtc_get_group_interrupts(uint8_t group_number);

/***************************************************************************/ /**
 * Gets enabled and pending SYSRTC interrupt flags.
 * Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note  Interrupt flags are not cleared by using this function.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @return  Pending and enabled SYSRTC interrupt sources.
 *          The return value is the bitwise AND of
 *          - the enabled interrupt sources in SYSRTC_GRPx_IEN and
 *          - the pending interrupt flags SYSRTC_GRPx_IF.
 ******************************************************************************/
uint32_t rsi_sysrtc_get_group_enabled_interrupts(uint8_t group_number);

/***************************************************************************/ /**
 * Sets one or more pending SYSRTC interrupts for the given group from Software.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @param[in] flags   SYSRTC interrupt sources to set to pending.
 *                    Use a set of interrupt flags OR-ed together to set
 *                    multiple interrupt sources for the SYSRTC group.
 ******************************************************************************/
void rsi_sysrtc_set_group_interrupts(uint8_t group_number, uint32_t flags);

/***************************************************************************/ /**
 * Gets SYSRTC compare register value for selected channel of given group.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @param[in] channel   Channel selector.
 *
 * @return  Compare register value.
 ******************************************************************************/
uint32_t rsi_sysrtc_get_group_compare_channel_value(uint8_t group_number, uint8_t channel);

/***************************************************************************/ /**
 * Sets SYSRTC compare register value for selected channel of given group.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @param[in] channel   Channel selector.
 *
 * @param[in] value   Compare register value.
 ******************************************************************************/
void rsi_sysrtc_set_compare_value(uint8_t group_number, uint8_t channel, uint32_t value);

/***************************************************************************/ /**
 * Gets SYSRTC input capture register value for capture channel of given group.
 *
 * @param[in] group_number  SYSRTC group number to use.
 *
 * @return  Capture register value.
 ******************************************************************************/
uint32_t rsi_sysrtc_get_capture_val(uint8_t group_number);

/***************************************************************************/ /**
 * Gets SYSRTC wakeup source
 *
 * @param[in] void
 *
 * @return  Value indicating if the wakeup by SYSRTC is set
 ******************************************************************************/
uint32_t rsi_sysrtc_get_sysrtc_wakeup(void);

/***************************************************************************/ /**
 * Clears SYSRTC wakeup status 
 * Clears the SYSRTC wakeup status bit
 *
 * @param[in] none.
 *
 * @return  none
 ******************************************************************************/
void rsi_sysrtc_clear_sysrtc_wakeup(void);

/***************************************************************************/ /**
 * Sets SYSRTC Compare GPIO Output Pin 
 *
 *
 * @param[in] none.
 *
 * @return  none
 ******************************************************************************/
void rsi_sysrtc_set_compare_gpio(const uint32_t group, const uint32_t chan);
/***************************************************************************/ /**
 * Sets the  SYSRTC clock source
 * @param[in] 
 *            sysrtc_clk - sysrtc clock as defined by sl_clk_clk_input_t structure
 *            div - Clock division factor for 32Khz clk (Used in SYSRTC and MCU WWD).
 *                  Note: Need to set 6'b010000 to generate 1 KHz clock
 *                  If 0th bit is set ? it divides by 2
 *                  Similarly, if 4th bit is set ? it divides by 32
 *
 * @return  none
 ******************************************************************************/
void rsi_sysrtc_clk_set(rsi_sysrtc_clk_inp_t sysrtc_clk, uint32_t div);

/***************************************************************************/ /**
 * Sets SYSRTC input output mode 
 * The input and out mode for capture and compare respectively can be set via 
 * MCUSYSRTC_REG1 or via GPIO. 
 *
 * @param[in] 
 *            set_gpio - a boolen which set if the IO is done via GPIO or not.
 *
 * @return  none
 ******************************************************************************/
void rsi_sysrtc_set_gpio(const bool set_gpio);
/***************************************************************************/ /**
 * Gets  SYSRTC output compare bit from register 
 * Read the output compare value which is set in MCUSYSRTC_REG1 
 *
 * @param[in] 
 *            group - group number
 *            channel - the compare channel
 *
 * @return  the status of the selected compare channel in the group
 ******************************************************************************/
uint32_t rsi_sysrtc_get_compare_bit(const uint32_t group, const uint32_t channel);

/***************************************************************************/ /**
 * Sets  SYSRTC input capture bit in register 
 * Write the input capture bit in MCUSYSRTC_REG1 
 *
 * @param[in] 
 *            group - group number
 *
 * @return  none
 ******************************************************************************/

void rsi_sysrtc_set_capture_reg(const uint32_t group);
void rsi_sysrtc_set_capture_gpio(const uint32_t group);
/**
 * @brief Enable SYSRTC based interrupts
 * 
 */
void rsi_sysrtc_enable_interrupts(void);

/**
 * @brief Disable SYSRTC based interrupts
 * 
 */
void rsi_sysrtc_disable_interrupts(void);

/** @} (end addtogroup sysrtc) */

#ifdef __cplusplus
}
#endif

#endif /* defined(SYSRTC_COUNT) && (SYSRTC_COUNT > 0) */
#endif /* PERIPHERAL_SYSRTC_H */
