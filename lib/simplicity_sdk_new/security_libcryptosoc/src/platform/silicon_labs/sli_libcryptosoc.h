/***************************************************************************//**
 * @file
 * @brief Integration layer for libcryptosoc internal APIs
 * @details
 * This file provides wrapper functions for internal libcryptosoc APIs that
 * are used by platform code. This allows the platform code to use these
 * functions without directly including internal headers from libcryptosoc/src.
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

#ifndef SLI_LIBCRYPTOSOC_H
#define SLI_LIBCRYPTOSOC_H

#include "cryptolib_def.h"
#include "sx_rng.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// BA431 State Enumeration
//------------------------------------------------------------------------------

/**
 * @brief BA431 state enumeration type
 */
typedef enum sli_ba431_state_e
{
  SLI_BA431_STATE_RESET       = 0x00000000,    /**< Reset */
  SLI_BA431_STATE_STARTUP     = 0x00000002,    /**< Start-up */
  SLI_BA431_STATE_FIFOFULLON  = 0x00000004,    /**< FIFO full - on */
  SLI_BA431_STATE_FIFOFULLOFF = 0x00000006,    /**< FIFO full - off */
  SLI_BA431_STATE_RUNNING     = 0x00000008,    /**< Running */
  SLI_BA431_STATE_ERROR       = 0x0000000A     /**< Error */
} sli_ba431_state_t;

//------------------------------------------------------------------------------
// BA431 State Enumeration Wrapper Functions
//------------------------------------------------------------------------------

/**
 * @brief Get the BA431_STATE_RESET enum value
 * @return The reset state value
 */
static inline sli_ba431_state_t sli_ba431_state_reset(void)
{
  return SLI_BA431_STATE_RESET;
}

/**
 * @brief Get the BA431_STATE_STARTUP enum value
 * @return The startup state value
 */
static inline sli_ba431_state_t sli_ba431_state_startup(void)
{
  return SLI_BA431_STATE_STARTUP;
}

/**
 * @brief Get the BA431_STATE_FIFOFULLON enum value
 * @return The FIFO full on state value
 */
static inline sli_ba431_state_t sli_ba431_state_fifofullon(void)
{
  return SLI_BA431_STATE_FIFOFULLON;
}

/**
 * @brief Get the BA431_STATE_FIFOFULLOFF enum value
 * @return The FIFO full off state value
 */
static inline sli_ba431_state_t sli_ba431_state_fifofulloff(void)
{
  return SLI_BA431_STATE_FIFOFULLOFF;
}

/**
 * @brief Get the BA431_STATE_RUNNING enum value
 * @return The running state value
 */
static inline sli_ba431_state_t sli_ba431_state_running(void)
{
  return SLI_BA431_STATE_RUNNING;
}

/**
 * @brief Get the BA431_STATE_ERROR enum value
 * @return The error state value
 */
static inline sli_ba431_state_t sli_ba431_state_error(void)
{
  return SLI_BA431_STATE_ERROR;
}

//------------------------------------------------------------------------------
// BA431 Status Mask Constants
//------------------------------------------------------------------------------

/**
 * @brief Get the BA431_STAT_MASK_STATE mask value
 * @return The state mask value
 */
uint32_t sli_ba431_stat_mask_state(void);

/**
 * @brief Get the BA431_STAT_MASK_STARTUP_FAIL mask value
 * @return The startup fail mask value
 */
uint32_t sli_ba431_stat_mask_startup_fail(void);

/**
 * @brief Get the BA431_STAT_MASK_PREALM_INT mask value
 * @return The preliminary alarm interrupt mask value
 */
uint32_t sli_ba431_stat_mask_prealm_int(void);

//------------------------------------------------------------------------------
// BA431 Control Register Constants
//------------------------------------------------------------------------------

/**
 * @brief Get the BA431_CTRL_NDRNG_ENABLE control register value
 * @return The NDRNG enable control value
 */
uint32_t sli_ba431_ctrl_ndrng_enable(void);

//------------------------------------------------------------------------------
// BA431 Function Wrappers
//------------------------------------------------------------------------------

/**
 * @brief Disable BA431 configured as NDRNG source
 */
void sli_ba431_disable_ndrng(void);

/**
 * @brief Returns the current BA431 fifolevel
 * @return fifolevel value
 */
uint32_t sli_ba431_read_fifolevel(void);

/**
 * @brief Returns the current BA431 status
 * @return BA431 status
 */
uint32_t sli_ba431_read_status(void);

/**
 * @brief Returns the current BA431 ControlReg
 * @return BA431 ControlReg
 */
uint32_t sli_ba431_read_controlreg(void);

/**
 * @brief Read the current conditioning key
 * @param key pointer to an array of 4 32b words
 */
void sli_ba431_read_conditioning_key(uint32_t *key);

/**
 * @brief Get the current state of BA431
 * @return The current BA431 state
 */
sli_ba431_state_t sli_ba431_get_state(void);

//------------------------------------------------------------------------------
// BA414EP Function Wrappers
//------------------------------------------------------------------------------

#if PK_CM_ENABLED
/**
 * @brief Set the random number generator to be used for the PK countermeasures.
 * @param rng a structure containing a pointer to a function to generate
 *          the mandatory random for the countermeasures.
 */
void sli_ba414ep_set_rng(struct sx_rng rng);
#endif

#ifdef __cplusplus
}
#endif

#endif // SLI_LIBCRYPTOSOC_H

