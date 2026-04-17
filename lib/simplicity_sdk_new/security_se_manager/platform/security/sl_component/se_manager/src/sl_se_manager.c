/***************************************************************************//**
 * @file
 * @brief Silicon Labs Secure Engine Manager API.
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

#include "sl_se_manager.h"

#if defined(SLI_MAILBOX_COMMAND_SUPPORTED) || defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
#if !defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)

#include "sli_se_manager_internal.h"
#include "sli_se_manager_mailbox.h"
#include "sl_assert.h"
#if defined(_CMU_CLKEN1_SEMAILBOXHOST_MASK)
#if defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_hal_bus.h"
#else
#include "em_bus.h"
#endif
#endif
#if !defined(SLI_SE_MANAGER_HOST_SYSTEM)
#include "sli_psec_osal.h"
#endif

#include <string.h>

/// @addtogroup sl_se_managers
/// @{

// -----------------------------------------------------------------------------
// Locals

#if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)
  #if defined(SL_SE_MANAGER_THREADING)
/// Priority to use for SEMBRX IRQ
    #if defined(SE_MANAGER_USER_SEMBRX_IRQ_PRIORITY)
      #if (SE_MANAGER_USER_SEMBRX_IRQ_PRIORITY >= (1U << __NVIC_PRIO_BITS) )
        #error Illegal SEMBRX priority level.
      #endif
      #if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
        #if (SE_MANAGER_USER_SEMBRX_IRQ_PRIORITY < (configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8U - __NVIC_PRIO_BITS) ) )
          #error Illegal SEMBRX priority level.
        #endif
      #else
        #if (SE_MANAGER_USER_SEMBRX_IRQ_PRIORITY < CORE_ATOMIC_BASE_PRIORITY_LEVEL)
          #error Illegal SEMBRX priority level.
        #endif
      #endif
      #define SE_MANAGER_SEMBRX_IRQ_PRIORITY SE_MANAGER_USER_SEMBRX_IRQ_PRIORITY
    #else
      #if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
        #define SE_MANAGER_SEMBRX_IRQ_PRIORITY (configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8U - __NVIC_PRIO_BITS) )
      #else
        #define SE_MANAGER_SEMBRX_IRQ_PRIORITY (CORE_ATOMIC_BASE_PRIORITY_LEVEL)
      #endif
    #endif
  #else  // defined(SL_SE_MANAGER_THREADING)
/// Priority to use for SEMBRX IRQ
    #if defined(SE_MANAGER_USER_SEMBRX_IRQ_PRIORITY)
      #define SE_MANAGER_SEMBRX_IRQ_PRIORITY SE_MANAGER_USER_SEMBRX_IRQ_PRIORITY
    #else
      #define SE_MANAGER_SEMBRX_IRQ_PRIORITY (0)
    #endif
  #endif  // defined(SL_SE_MANAGER_THREADING)
#endif  // defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

#if defined(SL_SE_MANAGER_THREADING) \
  || defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

// Flag to indicate that the SE Manager is initialized or not.
static volatile bool se_manager_initialized = false;

  #if defined(SL_SE_MANAGER_THREADING)
// Lock mutex for synchronizing multiple threads calling into the
// SE Manager API.
static sli_psec_osal_lock_t se_lock = { 0 };

  #endif // SL_SE_MANAGER_THREADING

  #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)
// SE command completion.
static volatile sli_psec_osal_completion_t se_command_completion;
// SE mailbox command response code. This value is read from the SEMAILBOX
// in ISR in order to clear the command complete interrupt condition.
static volatile sli_se_mailbox_response_t se_manager_command_response = SLI_SE_RESPONSE_INTERNAL_ERROR;
  #endif // SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION

#endif // #if defined (SL_SE_MANAGER_THREADING)
//   || defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

#if defined(_SILICON_LABS_32B_SERIES_3)

/// Flag to indicate that a flash write or erase command is in progress
volatile bool flash_wr_command_in_progress = false;

#endif // #if defined(_SILICON_LABS_32B_SERIES_3)

// -----------------------------------------------------------------------------
// Local functions

#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
/***************************************************************************//**
 *   Configure L1 cache enablement and return previous state
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static bool configure_l1_cache(bool enable)
 {
  // Disable interrupts while modifying CLKENs
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // Read state of clock enable for L1 cache
  bool l1_cache_clock_was_enabled = ((CMU->CLKEN1 & CMU_CLKEN1_ICACHE0) != 0);
  CMU->CLKEN1_SET = CMU_CLKEN1_ICACHE0;

  // Get CACHEDIS state
  bool cache_was_enabled = ((L1ICACHE0->CTRL & ICACHE_CTRL_CACHEDIS) == 0);

  if (enable) {
    L1ICACHE0->CTRL_CLR = ICACHE_CTRL_CACHEDIS;
  } else {
    L1ICACHE0->CTRL_SET = ICACHE_CTRL_CACHEDIS;
  }

  // Restore clock enable for L1 cache
  if (!l1_cache_clock_was_enabled) {
    CMU->CLKEN1_CLR = CMU_CLKEN1_ICACHE0;
  }

  // Restore interrupts
  CORE_EXIT_CRITICAL();

  return cache_was_enabled;
}

#endif // #if defined(_SILICON_LABS_32B_SERIES_3)

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 * Initialize the SE Manager.
 ******************************************************************************/
sl_status_t sl_se_init(void)
{
  sl_status_t ret = SL_STATUS_OK;
  #if defined (SL_SE_MANAGER_THREADING) \
  || defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

  #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)
  (void)se_manager_command_response;
  #endif

  #if defined(SL_SE_MANAGER_THREADING)
  SLI_PSEC_OSAL_KERNEL_CRITICAL_SECTION_START
  #endif

  if ( !se_manager_initialized ) {
      #if defined(SL_SE_MANAGER_THREADING)
    // Initialize SE lock
    ret = sli_psec_osal_init_lock(&se_lock);
      #endif
      #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)
    if (ret == SL_STATUS_OK) {
      // Initialize command completion object.
      ret = sli_psec_osal_init_completion((sli_psec_osal_completion_t *)&se_command_completion);
      if (ret == SL_STATUS_OK) {
        // Enable SE RX mailbox interrupt in NVIC, but not in SEMAILBOX
        // which will be enabled if the yield parameter in
        // sli_se_execute_and_wait is true.
        NVIC_SetPriority(SEMBRX_IRQn, SE_MANAGER_SEMBRX_IRQ_PRIORITY);
        NVIC_EnableIRQ(SEMBRX_IRQn);
      }
    }
      #endif // SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION
    if (ret == SL_STATUS_OK) {
      se_manager_initialized = true;
    }
  }

  #if defined(SL_SE_MANAGER_THREADING)
  SLI_PSEC_OSAL_KERNEL_CRITICAL_SECTION_END
  #endif

  #endif // #if defined (SL_SE_MANAGER_THREADING)
  //   || defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

  return ret;
}

/***************************************************************************//**
 * Denitialize the SE Manager.
 ******************************************************************************/
sl_status_t sl_se_deinit(void)
{
  sl_status_t ret = SL_STATUS_OK;

  #if defined (SL_SE_MANAGER_THREADING) \
  || defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

  #if defined(SL_SE_MANAGER_THREADING)
  SLI_PSEC_OSAL_KERNEL_CRITICAL_SECTION_START
  #endif

  if ( se_manager_initialized ) {
    // We need to exit the critical section in case the SE lock is held by a
    // thread, and we want to take it before de-initializing.
    #if defined(SL_SE_MANAGER_THREADING)
    SLI_PSEC_OSAL_KERNEL_CRITICAL_SECTION_END
    #endif

    // Acquire the SE lock to make sure no thread is executing SE commands
    // when we de-initialize.
    ret = sli_se_lock_acquire();
    if (ret != SL_STATUS_OK) {
      return ret;
    }

      #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)
    // Disable SE RX mailbox interrupt in NVIC.
    NVIC_ClearPendingIRQ(SEMBRX_IRQn);
    NVIC_DisableIRQ(SEMBRX_IRQn);
    // Free command completion object.
    ret = sli_psec_osal_free_completion((sli_psec_osal_completion_t *)&se_command_completion);
      #endif // SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION

      #if defined(SL_SE_MANAGER_THREADING)
    if (ret == SL_STATUS_OK) {
      // Free the SE lock mutex
      ret = sli_psec_osal_free_lock(&se_lock);
    }
      #endif

    // Mark the SE Manager as un-initialized.
    se_manager_initialized = false;
  }
  #if defined(SL_SE_MANAGER_THREADING)
  else {
    SLI_PSEC_OSAL_KERNEL_CRITICAL_SECTION_END
  }
  #endif

  #endif // #if defined (SL_SE_MANAGER_THREADING)
  //   || defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

  return ret;
}

/***************************************************************************//**
 * @brief
 *   Translate SE response codes to sl_status_t codes.
 *
 * @return
 *   Converted status code, their meaning is documented here @ref sl_status.h,
 *   Asserts and returns @c SL_STATUS_FAIL on unexpected response.
 ******************************************************************************/
sl_status_t sli_se_to_sl_status(sli_se_mailbox_response_t res)
{
  switch (res) {
    case SLI_SE_RESPONSE_OK:
      return SL_STATUS_OK;
    case SLI_SE_RESPONSE_INVALID_COMMAND:
      return SL_STATUS_COMMAND_IS_INVALID;
    case SLI_SE_RESPONSE_AUTHORIZATION_ERROR:
      return SL_STATUS_INVALID_CREDENTIALS;
    case SLI_SE_RESPONSE_INVALID_SIGNATURE:
      return SL_STATUS_INVALID_SIGNATURE;
    case SLI_SE_RESPONSE_BUS_ERROR:
      return SL_STATUS_BUS_ERROR;
    case SLI_SE_RESPONSE_INTERNAL_ERROR:
      return SL_STATUS_FAIL;
    case SLI_SE_RESPONSE_CRYPTO_ERROR:
      return SL_STATUS_FAIL;
    case SLI_SE_RESPONSE_INVALID_PARAMETER:
      return SL_STATUS_INVALID_PARAMETER;
    case SLI_SE_RESPONSE_ABORT:
      return SL_STATUS_ABORT;
    case SLI_SE_RESPONSE_SELFTEST_ERROR:
      return SL_STATUS_INITIALIZATION;
    case SLI_SE_RESPONSE_NOT_INITIALIZED:
      return SL_STATUS_NOT_INITIALIZED;
#if defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
    case SLI_SE_RESPONSE_MAILBOX_INVALID:
      return SL_STATUS_COMMAND_IS_INVALID;
#endif
    default:
      // Assert we do not get a bad SE response code.
      EFM_ASSERT(false);
      return SL_STATUS_FAIL;
  }
}

/***************************************************************************//**
 * Acquire the SE lock for exclusive access if necessary (thread mode).
 * Enable the SEMAILBOX clock if necessary.
 ******************************************************************************/
sl_status_t sli_se_lock_acquire(void)
{
  #if defined(SL_SE_MANAGER_THREADING)
  sl_status_t status = sli_psec_osal_take_lock(&se_lock);
  #else
  sl_status_t status = SL_STATUS_OK;
  #endif
  #if defined(_CMU_CLKEN1_SEMAILBOXHOST_MASK)
  if (status == SL_STATUS_OK) {
  #if defined(_SILICON_LABS_32B_SERIES_3)
    sl_hal_bus_reg_write_bit(&CMU->CLKEN1, _CMU_CLKEN1_SEMAILBOXHOST_SHIFT, 1);
  #else
    BUS_RegBitWrite(&CMU->CLKEN1, _CMU_CLKEN1_SEMAILBOXHOST_SHIFT, 1);
  #endif
    // Make sure the write to CMU->CLKEN1 is finished.
    __DSB();
  }
  #endif
  return status;
}

/***************************************************************************//**
 * Release the SE lock if necessary (thread mode).
 * Disable the SEMAILBOX clock if necessary.
 ******************************************************************************/
sl_status_t sli_se_lock_release(void)
{
  #if defined(_CMU_CLKEN1_SEMAILBOXHOST_MASK)
  #if defined(_SILICON_LABS_32B_SERIES_3)
  sl_hal_bus_reg_write_bit(&CMU->CLKEN1, _CMU_CLKEN1_SEMAILBOXHOST_SHIFT, 0);
  #else
  BUS_RegBitWrite(&CMU->CLKEN1, _CMU_CLKEN1_SEMAILBOXHOST_SHIFT, 0);
  #endif
  #endif
  #if defined(SL_SE_MANAGER_THREADING)
  return sli_psec_osal_give_lock(&se_lock);
  #else
  return SL_STATUS_OK;
  #endif
}

#if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

/***************************************************************************//**
 * @brief
 *   SE Mailbox Interrupt Service Routine
 ******************************************************************************/
void SEMBRX_IRQHandler(void)
{
  sl_status_t status;
  // Check if the SE mailbox is the source of the interrupt.
  if (SEMAILBOX_HOST->RX_STATUS & SEMAILBOX_RX_STATUS_RXINT) {
    // Signal SE mailbox completion.
    status = sli_psec_osal_complete((sli_psec_osal_completion_t *)&se_command_completion);
    EFM_ASSERT(status == SL_STATUS_OK);
  }
  // Get command response and clear interrupt condition in SEMAILBOX peripheral
  se_manager_command_response = sli_se_mailbox_handle_response();
  // Clear interrupt condition in NVIC
  NVIC_ClearPendingIRQ(SEMBRX_IRQn);
}

#endif // #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

/***************************************************************************//**
 * Set the yield attribute of the SE command context object.
 ******************************************************************************/
sl_status_t sl_se_set_yield(sl_se_command_context_t *cmd_ctx,
                            bool yield)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)
  cmd_ctx->yield = yield;
  return SL_STATUS_OK;
  #else
  if (yield) {
    return SL_STATUS_NOT_AVAILABLE;
  } else {
    (void) cmd_ctx;
    return SL_STATUS_OK;
  }
  #endif
}

/***************************************************************************//**
 * @brief
 *   Execute and wait for SE mailbox command to complete.
 *
 * @return
 *   One of the following status code, any other status codes relates to internal
 *   function errors see @ref sl_status.h for their meaning.
 *   - @c SL_STATUS_OK
 *   - @c SL_STATUS_INVALID_PARAMETER
 ******************************************************************************/
#if defined(SLI_MAILBOX_COMMAND_SUPPORTED) && !defined(SLI_SE_MANAGER_HOST_SYSTEM)
sl_status_t sli_se_execute_and_wait(sl_se_command_context_t *cmd_ctx)
{
  sl_status_t status = SL_STATUS_FAIL;
  sli_se_mailbox_response_t command_response = SLI_SE_RESPONSE_INTERNAL_ERROR;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Try to acquire SE lock
  status = sli_se_lock_acquire();
  if (status != SL_STATUS_OK) {
    return status;
  }

  #if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  bool l1_cache_was_enabled = false; // Track if L1 cache was enabled
  if (cmd_ctx->flash_wr) {
    // Disable L1 cache during flash command execution
    l1_cache_was_enabled = configure_l1_cache(false);
    flash_wr_command_in_progress = true;
  }
  #endif // #if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)

  // Execute SE mailbox command
  sli_se_mailbox_execute_command(&cmd_ctx->command);

  #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)
  if (cmd_ctx->yield) {
    // Enable SEMAILBOX RXINT interrupt
    sli_se_mailbox_enable_interrupt(SEMAILBOX_CONFIGURATION_RXINTEN);

    // Yield and Wait for the command completion signal
    status = sli_psec_osal_wait_completion((sli_psec_osal_completion_t *)&se_command_completion,
                                           SLI_PSEC_OSAL_WAIT_FOREVER);

    // Disable SEMAILBOX RXINT interrupt.
    sli_se_mailbox_disable_interrupt(SEMAILBOX_CONFIGURATION_RXINTEN);

    if (status != SL_STATUS_OK) {
      #if (_SILICON_LABS_32B_SERIES == 3)
      // Read the command handle word ( not used ) from the SEMAILBOX FIFO
      SEMAILBOX_HOST->FIFO;
      #endif // #if (_SILICON_LABS_32B_SERIES == 3)
      sli_se_lock_release();
      return status;
    }

    // Get response which is read in the ISR to clear interrupt condition.
    command_response = se_manager_command_response;
    // Default to an error.
    se_manager_command_response = SLI_SE_RESPONSE_INTERNAL_ERROR;
  } else {
    // Wait for command completion and get command response
    command_response = sli_se_mailbox_handle_response();
  }

  #else // #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

  // Wait for command completion and get command response
  command_response = sli_se_mailbox_handle_response();

  #endif // #if defined(SL_SE_MANAGER_YIELD_WHILE_WAITING_FOR_COMMAND_COMPLETION)

  #if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  if (cmd_ctx->flash_wr) {
    // Restore L1 cache if it was enabled
    if (l1_cache_was_enabled) {
      (void)configure_l1_cache(true);
    }
    flash_wr_command_in_progress = false;
  }
  #endif // #if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)

  // Release SE lock
  status = sli_se_lock_release();

  // Return sl_status_t code.
  if (command_response == SLI_SE_RESPONSE_OK) {
    return status;
  } else {
    // Convert from sli_se_mailbox_response_t to sl_status_t code and return.
    return sli_se_to_sl_status(command_response);
  }
}

#elif defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED) // SLI_MAILBOX_COMMAND_SUPPORTED

sl_status_t sli_se_execute_and_wait(sl_se_command_context_t *cmd_ctx)
{
  sl_status_t status;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Try to acquire SE lock
  status = sli_se_lock_acquire();
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Execute SE mailbox command
  sli_se_mailbox_execute_command(&cmd_ctx->command);

  return SL_STATUS_FAIL; // Should never get to this point
}

/***************************************************************************//**
 * From VSE mailbox read which command, if any, was executed.
 ******************************************************************************/
sl_status_t sl_se_read_executed_command(sl_se_command_context_t *cmd_ctx)
{
  sl_status_t status;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Try to acquire SE lock
  status = sli_se_lock_acquire();
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Read command
  cmd_ctx->command.command = sli_vse_mailbox_read_executed_command();

  // Release SE lock
  status = sli_se_lock_release();

  // Return sl_status_t code.
  if (cmd_ctx->command.command == SLI_SE_RESPONSE_MAILBOX_INVALID) {
    // Convert from sli_se_mailbox_response_t to sl_status_t code and return.
    return sli_se_to_sl_status(SLI_SE_RESPONSE_MAILBOX_INVALID);
  } else {
    return status;
  }
}

/***************************************************************************//**
 * Acknowledge and get status and output data of a completed command.
 ******************************************************************************/
sl_status_t sl_se_ack_command(sl_se_command_context_t *cmd_ctx)
{
  sl_status_t status;
  sl_status_t command_response;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Try to acquire SE lock
  status = sli_se_lock_acquire();
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Acknowledge VSE mailbox command
  command_response = sli_vse_mailbox_ack_command(&cmd_ctx->command);

  // Release SE lock
  status = sli_se_lock_release();

  // Return sl_status_t code.
  if (command_response == SLI_SE_RESPONSE_OK) {
    return status;
  } else {
    // Convert from SE_Response_t to sl_status_t code and return.
    return sli_se_to_sl_status(command_response);
  }
}

#endif // SLI_VSE_MAILBOX_COMMAND_SUPPORTED

#endif // !SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT

/***************************************************************************//**
 * Initialize an SE command context object
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sl_se_init_command_context(sl_se_command_context_t *cmd_ctx)
{
  sl_se_command_context_t v = SL_SE_COMMAND_CONTEXT_INIT;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  *cmd_ctx = v;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * De-initialize an SE command context object
 ******************************************************************************/
sl_status_t sl_se_deinit_command_context(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return sl_se_init_command_context(cmd_ctx);
}

/** @} (end addtogroup sl_se) */

#endif // defined(SLI_MAILBOX_COMMAND_SUPPORTED) || defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
