/***************************************************************************//**
 * @file
 * @brief Part of Silicon Labs's platform abstraction layer for SxSymCrypt.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "../../hw.h"

#include "em_device.h"
#include "sl_assert.h"
#include "sl_clock_manager.h"
#include "sli_psec_osal.h"
#include "sl_status.h"
#include "sl_code_classification.h"
#include "sxsymcrypt/interrupts.h"

#include <stdbool.h>

// -----------------------------------------------------------------------------
// Defines
#define SLI_CRYPTOMASTER_ENGINES (2)

// -----------------------------------------------------------------------------
// Static Data
#if defined(SLI_PSEC_THREADING)
static sli_psec_osal_lock_t      cryptomaster_locks[SLI_CRYPTOMASTER_ENGINES] = { 0 };
static sli_psec_osal_lock_t      cryptomaster_selection = { 0 };
static volatile bool             cryptomaster_locks_initialized = false;
sli_psec_osal_completion_t       cryptomaster_hostcrypto_completion;
#endif

unsigned int requested_cryptomaster_index
  = SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO;

volatile sl_status_t sli_crypto_preempted_status = SL_STATUS_NOT_INITIALIZED;

extern struct sx_regs cryptomaster_structs[];
// -----------------------------------------------------------------------------
// Global Function Definitions

// -------------------------------------
// CryptoMaster Selection

sl_status_t sli_sxsymcrypt_init_locks(void)
{
  sl_status_t ret = SL_STATUS_OK;
#if defined(SLI_PSEC_THREADING)
  if (!cryptomaster_locks_initialized) {
    int32_t kernel_lock_state = 0;
    osKernelState_t kernel_state = sli_psec_osal_kernel_get_state();
    if (kernel_state != osKernelInactive && kernel_state != osKernelReady) {
      kernel_lock_state = sli_psec_osal_kernel_lock();
      if (kernel_lock_state < 0) {
        return SL_STATUS_SUSPENDED;
      }
    }

    // Check the flag again after entering the critical section. Now that we're
    // in the critical section, we can be sure that we are the only ones looking
    // at the flag and no-one is interrupting us during its manipulation.
    if (!cryptomaster_locks_initialized) {
      int i;
      ret = sli_psec_osal_init_lock(&cryptomaster_selection);
      if (ret != SL_STATUS_OK) {
        return ret;
      }
      for (i = 0; (ret == SL_STATUS_OK) && (i < SLI_CRYPTOMASTER_ENGINES); i++) {
        ret = sli_psec_osal_init_lock(&cryptomaster_locks[i]);
        if (ret != SL_STATUS_OK) {
          return ret;
        }
      }
      ret = sli_psec_osal_init_completion(&cryptomaster_hostcrypto_completion);
      if (ret != SL_STATUS_OK) {
        return ret;
      }
      cryptomaster_locks_initialized = true;
    }

    if (kernel_state != osKernelInactive && kernel_state != osKernelReady) {
      if (sli_psec_osal_kernel_restore_lock(kernel_lock_state) < 0) {
        return SL_STATUS_INVALID_STATE;
      }
    }
  }
#endif
  EFM_ASSERT(sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_SYMCRYPTO) == SL_STATUS_OK);
  EFM_ASSERT(sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LPWAES) == SL_STATUS_OK);
  sx_interrupts_enable();
  EFM_ASSERT(sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_SYMCRYPTO) == SL_STATUS_OK);
  EFM_ASSERT(sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_LPWAES) == SL_STATUS_OK);
  return ret;
}

sl_status_t sli_sxsymcrypt_lock_cryptomaster_selection(unsigned int instance, bool yield)
{
  sl_status_t ret = SL_STATUS_OK;

  if (instance >= SLI_CRYPTOMASTER_ENGINES) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if called from ISR
  if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0U) {
    // Not called from ISR
#if defined(SLI_PSEC_THREADING)
    ret = sli_psec_osal_take_lock(&cryptomaster_selection);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
#endif
    // Select engine if not in ISR
    requested_cryptomaster_index = instance;

#if defined(SLI_PSEC_THREADING)
    ret = sli_psec_osal_take_lock(&cryptomaster_locks[requested_cryptomaster_index]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
#endif
    if (requested_cryptomaster_index == SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO
        && yield == true) {
      // Enable SYMCRYPTO IRQ if yield is set
      sl_interrupt_manager_enable_irq(SYMCRYPTO_IRQn);
      cryptomaster_structs[SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO].yield = true;
    }
  }

  // Enabling clocks here as sx_cmdma_list_compatible() executes before
  // sx_cmdma_find_available() is called when creating operation object.
  if (instance == SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO) {
    EFM_ASSERT(sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_SYMCRYPTO)
               == SL_STATUS_OK);
  } else if (instance == SLI_SXSYMCRYPT_CRYPTOMASTER_LPWAES) {
    EFM_ASSERT(sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LPWAES)
               == SL_STATUS_OK);
  }

  // Check if called from ISR
  if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U) {
    // Called from ISR
    if (instance == SLI_SXSYMCRYPT_CRYPTOMASTER_LPWAES) {
      // IRQ: need to store & restore LPWAES registers
      while (LPWAES->STATUS & (SYMCRYPTO_STATUS_FETCHERBSY | SYMCRYPTO_STATUS_PUSHERBSY | SYMCRYPTO_STATUS_SOFTRSTBSY)) {
        // Wait for completion of the previous operation, since the LPWAES
        // peripheral does not support preemption of an operation in progress.
      }
      // Check if preempted operation was successful
      if (LPWAES->IF & (SYMCRYPTO_IF_FETCHERERROR | SYMCRYPTO_IF_PUSHERERROR)) {
        sli_crypto_preempted_status = SL_STATUS_FAIL;
      } else {
        sli_crypto_preempted_status = SL_STATUS_OK;
      }
      return SL_STATUS_ISR;
    }
    if (instance == SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO) {
      // Accessing Hostcrypto from ISR is not supported
      return SL_STATUS_NOT_SUPPORTED;
    }
  }

  return ret;
}

sl_status_t sli_sxsymcrypt_unlock_cryptomaster_selection(void)
{
  if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U) {
    return SL_STATUS_OK;
  }
#if defined(SLI_PSEC_THREADING)
  return sli_psec_osal_give_lock(&cryptomaster_selection);
#else
  return SL_STATUS_OK;
#endif
}

// -------------------------------------
// CryptoMaster Locking and Clocking

// This function is called when a crypto operation is finished or aborted. This
// is the moment where we are intended to release all locks put on the
// CryptoMaster instance.
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
void sx_cmdma_release_hw(struct sx_regs *regs)
{
#if defined(SLI_PSEC_THREADING)
  // If selection is not locked then we can disable clocks
  if (sli_psec_osal_lock_is_accessible(&cryptomaster_selection) == SL_STATUS_OK)
#endif
  {
    if (regs->instance_index == SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO) {
      if (cryptomaster_structs[SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO].yield == true) {
        // Disable SYMCRYPTO IRQ
        sl_interrupt_manager_disable_irq(SYMCRYPTO_IRQn);
        cryptomaster_structs[SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO].yield = false;
      }
      EFM_ASSERT(sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_SYMCRYPTO)
                 == SL_STATUS_OK);
    }
    if (regs->instance_index == SLI_SXSYMCRYPT_CRYPTOMASTER_LPWAES) {
      if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0U) {
        // Not in ISR
        if (sli_crypto_preempted_status == SL_STATUS_NOT_INITIALIZED) {
          // Earlier operation was not preempted. Therefore we can disable the clocks
          EFM_ASSERT(sl_clock_manager_disable_bus_clock(SL_BUS_CLOCK_LPWAES)
                     == SL_STATUS_OK);
        }
      }
    }
  }
#if defined(SLI_PSEC_THREADING)
  sli_psec_osal_give_lock(&cryptomaster_locks[regs->instance_index]);
#endif
}

// This function is called when a crypto operation is created. This is the
// moment where we are intended to decide what CryptoMaster engine to use.
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SXSYMCRYPT, SL_CODE_CLASS_TIME_CRITICAL)
struct sx_regs *sx_cmdma_find_available(unsigned int compatible)
{
  // The compatibility bitmask is not very important for our implementation
  // where there's only two CryptoMaster instances, and what engine to use comes
  // more down to the consumer rather than just the engine availability. We will
  // therefore ignore this argument instead and throw every operation at CM
  // instance that was selected in the call to
  // sli_sxsymcrypt_lock_cryptomaster_selection().
  if (!(compatible & (1 << requested_cryptomaster_index))) {
    return NULL;
  }
  if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U) {
    // IRQ: need to store & restore LPWAES registers
    while (LPWAES->STATUS & (SYMCRYPTO_STATUS_FETCHERBSY | SYMCRYPTO_STATUS_PUSHERBSY | SYMCRYPTO_STATUS_SOFTRSTBSY)) {
      // Wait for completion of the previous operation, since the LPWAES
      // peripheral does not support preemption of an operation in progress.
    }
    // This return statement will not actually do anything. It is added only
    // code readability.
    return sx_hw_find_regs(SLI_SXSYMCRYPT_CRYPTOMASTER_LPWAES);
  }
  return sx_hw_find_regs(requested_cryptomaster_index);
}

void sli_crypto_lpwaes_save_state(sli_cryptomaster_state_t *state)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  state->FETCHADDR = LPWAES->FETCHADDR;
  state->PUSHADDR = LPWAES->PUSHADDR;
  CORE_EXIT_CRITICAL();
}

void sli_crypto_lpwaes_restore_state(sli_cryptomaster_state_t *state)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  LPWAES->FETCHADDR = state->FETCHADDR;
  LPWAES->PUSHADDR = state->PUSHADDR;
  CORE_EXIT_CRITICAL();
}

void sli_cmdma_release_hw(struct sx_regs *regs)
{
  sx_cmdma_release_hw(regs);
}
