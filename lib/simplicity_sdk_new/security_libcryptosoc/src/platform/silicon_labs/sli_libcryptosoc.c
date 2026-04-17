/***************************************************************************//**
 * @file
 * @brief Integration layer implementation for libcryptosoc internal APIs
 * @details
 * This file implements wrapper functions for internal libcryptosoc APIs.
 * It includes internal headers from libcryptosoc/src and provides a clean
 * interface for platform code to use these functions.
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

#include "cryptolib_def.h"
#include "sli_libcryptosoc.h"

// Include internal headers from libcryptosoc/src
#include "ba431_config.h"
#include "ba414ep_config.h"

//------------------------------------------------------------------------------
// BA431 Status Mask Constants
//------------------------------------------------------------------------------

uint32_t sli_ba431_stat_mask_state(void)
{
  return BA431_STAT_MASK_STATE;
}

uint32_t sli_ba431_stat_mask_startup_fail(void)
{
  return BA431_STAT_MASK_STARTUP_FAIL;
}

uint32_t sli_ba431_stat_mask_prealm_int(void)
{
  return BA431_STAT_MASK_PREALM_INT;
}

//------------------------------------------------------------------------------
// BA431 Control Register Constants
//------------------------------------------------------------------------------

uint32_t sli_ba431_ctrl_ndrng_enable(void)
{
  return BA431_CTRL_NDRNG_ENABLE;
}

//------------------------------------------------------------------------------
// BA431 Function Wrappers
//------------------------------------------------------------------------------

void sli_ba431_disable_ndrng(void)
{
  ba431_disable_ndrng();
}

uint32_t sli_ba431_read_fifolevel(void)
{
  return ba431_read_fifolevel();
}

uint32_t sli_ba431_read_status(void)
{
  return ba431_read_status();
}

uint32_t sli_ba431_read_controlreg(void)
{
  return ba431_read_controlreg();
}

void sli_ba431_read_conditioning_key(uint32_t *key)
{
  ba431_read_conditioning_key(key);
}

sli_ba431_state_t sli_ba431_get_state(void)
{
  ba431_state_t state = ba431_get_state();
  // Convert from internal ba431_state_t to sli_ba431_state_t
  switch (state) {
    case BA431_STATE_RESET:
      return SLI_BA431_STATE_RESET;
    case BA431_STATE_STARTUP:
      return SLI_BA431_STATE_STARTUP;
    case BA431_STATE_FIFOFULLON:
      return SLI_BA431_STATE_FIFOFULLON;
    case BA431_STATE_FIFOFULLOFF:
      return SLI_BA431_STATE_FIFOFULLOFF;
    case BA431_STATE_RUNNING:
      return SLI_BA431_STATE_RUNNING;
    case BA431_STATE_ERROR:
      return SLI_BA431_STATE_ERROR;
    default:
      return SLI_BA431_STATE_ERROR;
  }
}

//------------------------------------------------------------------------------
// BA414EP Function Wrappers
//------------------------------------------------------------------------------

#if PK_CM_ENABLED
void sli_ba414ep_set_rng(struct sx_rng rng)
{
  ba414ep_set_rng(rng);
}
#endif