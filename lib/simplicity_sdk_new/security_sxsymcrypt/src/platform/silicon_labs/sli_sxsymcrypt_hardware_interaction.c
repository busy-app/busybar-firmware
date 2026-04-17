/***************************************************************************//**
 * @file
 * @brief Part of Silicon Labs's platform abstraction layer for SxSymCrypt.
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

#include "sli_sxsymcrypt.h"

#include "em_device.h"
#include "sl_code_classification.h"
#include "sli_psec_osal.h"

#include "../../membarriers.h"
#include "../../hw.h"

#include <stddef.h>

// -----------------------------------------------------------------------------
// Constant Data

struct sx_regs cryptomaster_structs[] = {
  {
    .instance_index = SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO,
    .base_address = (uint8_t *)SYMCRYPTO,
    .yield = false
  },
  {
    .instance_index = SLI_SXSYMCRYPT_CRYPTOMASTER_LPWAES,
    .base_address = (uint8_t *)LPWAES,
    .yield = false
  },
};

#if defined(SLI_PSEC_THREADING)
extern sli_psec_osal_completion_t cryptomaster_hostcrypto_completion;
#endif

// -----------------------------------------------------------------------------
// Global Function Definitions

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void SYMCRYPTO_IRQHandler(void)
{
  if (SYMCRYPTO->IF & (SYMCRYPTO_IF_FETCHERERROR | SYMCRYPTO_IF_PUSHERERROR
      | SYMCRYPTO_IF_PUSHERSTOPPED)) {
      SYMCRYPTO->IF_CLR = SYMCRYPTO_IF_CLR_FETCHERERRORIFC | SYMCRYPTO_IF_CLR_PUSHERERRORIFC
                      | SYMCRYPTO_IF_CLR_PUSHERSTOPPEDIFC;
  }
#if defined(SLI_PSEC_THREADING)
  if (cryptomaster_structs[SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO].yield == true) {
    sli_psec_osal_complete(&cryptomaster_hostcrypto_completion);
  }
#endif
}
// -------------------------------------
// Wait for Completion

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sx_cmdma_wait(struct sx_regs *regs)
{
  (void)regs;
#if defined(SLI_PSEC_THREADING)
  if (cryptomaster_structs[SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO].yield == true) {
    sli_psec_osal_wait_completion(&cryptomaster_hostcrypto_completion, SLI_PSEC_OSAL_WAIT_FOREVER);
  }
#endif
}

// -------------------------------------
// CryptoMaster Instance Look-Up

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
struct sx_regs *sx_hw_find_regs(unsigned int idx)
{
  // As crypto engine will be selected by the user, we will directly
  // return the base address for that instance.
  if (idx >= SLI_SXSYMCRYPT_CRYPTOMASTER_N_INSTANCES) {
    return NULL;
  }

  if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U) {
    return (struct sx_regs *) &cryptomaster_structs[SLI_SXSYMCRYPT_CRYPTOMASTER_LPWAES];
  } else {
    return (struct sx_regs *) &cryptomaster_structs[requested_cryptomaster_index];
  }
}

// -------------------------------------
// Register Reads and Writes

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t sx_rdreg(struct sx_regs *regs, uint32_t addr)
{
  volatile uint32_t *p = (uint32_t*)(regs->base_address + addr);

  __asm__ volatile ("dsb");
  uint32_t v = *p;
  __asm__ volatile ("dsb");

  return v;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sx_wrreg_addr(struct sx_regs *regs, uint32_t addr, struct sxdesc *p)
{
  volatile size_t *d = (volatile size_t*)(regs->base_address + addr);

  __asm__ volatile ("dsb");
  *d = (size_t)p;
  __asm__ volatile ("dsb");
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sx_wrreg(struct sx_regs *regs, uint32_t addr, uint32_t val)
{
  volatile uint32_t *p = (uint32_t*)(regs->base_address + addr);

  __asm__ volatile ("dsb");
  *p = val;
  __asm__ volatile ("dsb");
}

// This function is always called right before the first write to a
// CryptoMaster. Should we want to implement resource locking at a  higher
// granularity, we could lock access to a CryptoMaster here, and then release it
// in sx_flush_fromhw();
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sx_flush_tohw(struct sx_regs *regs, char *cpumem, size_t sz)
{
  (void)regs;
  (void)cpumem;
  (void)sz;
}

// This function is always called right after the final read from a
// CryptoMaster. Should we want to implement resource locking at a  higher
// granularity, we could unlock access to a CryptoMaster here.
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sx_flush_fromhw(struct sx_regs *regs,
                     char *cpumem,
                     size_t offset,
                     size_t sz)
{
  (void)regs;
  (void)cpumem;
  (void)offset;
  (void)sz;
}

// -------------------------------------
// Memory Mapping

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
char *sx_map_internal(struct sx_regs *regs, char *dma)
{
  (void)regs;

  return (char*)dma;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
char *sx_map_usrdatain(char *s, size_t sz)
{
  (void) sz;

  return s;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
char *sx_map_usrdataout(char *s, size_t sz)
{
  (void) sz;

  return s;
}

/*****************************************************************************
 ** Bus error handling
 *****************************************************************************/

/** @brief Trigger hardfault by access to invalid memory address */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sx_trigger_hardfault(void) {
   uint32_t *bad_mem_addr = (uint32_t*) 0xFFFFFFFC;
   *bad_mem_addr = 1;
}
