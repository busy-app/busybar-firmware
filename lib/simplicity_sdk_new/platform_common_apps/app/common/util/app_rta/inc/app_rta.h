/***************************************************************************//**
 * @file
 * @brief Application Runtime Adaptor (RTA) header
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
#ifndef APP_RTA_H
#define APP_RTA_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "sl_status.h"

/// Invalid RTA context
#define APP_RTA_INVALID_CONTEXT                       (NULL)
/// General context type
typedef void *app_rta_context_t;

/// Requirement type
typedef struct {
  uint8_t runtime          : 1; ///< This flag indicates that RTA should provide
                                ///  a runtime for the context, so the passed
                                ///  step function should be executed when
                                ///  required to proceed.
  uint8_t runtime_separate : 1; ///< This flag indicates the same as
                                ///  @ref runtime but the context will be held
                                ///  in a separate, exclusive runtime.
  uint8_t start_disabled   : 1; ///< Flag to start the context in disabled
                                ///  state.
  uint8_t guard            : 1; ///< Flag to require a guard for protecting
                                ///  internal variables. It is suggested to
                                ///  protect all internal state variables from
                                ///  accessing from different tasks.
                                ///  Guard must not be used from ISR context.
  uint8_t signal           : 1; ///< Require a signaling mechanism that can be
                                ///  set from any other context (including ISR)
                                ///  and checked from the step function.
                                ///  A semaphore will be created for signaling.
  uint8_t queue            : 1; ///< Include feature for passing messages from
                                /// any context (including ISR) to the step
                                /// function.
} app_rta_requirement_t;

/// General function type
typedef void (*app_rta_function_t)(void);

/// RTA error type for async errors
typedef enum {
  APP_RTA_ERROR_RUNTIME_INIT_FAILED = 0, /// Failed to initialize runtime.
  APP_RTA_ERROR_ACQUIRE_FAILED      = 1, /// Failed to acquire guard in runtime.
  APP_RTA_ERROR_RELEASE_FAILED      = 2, /// Failed to release guard in runtime.
} app_rta_error_t;

/**************************************************************************//**
 * Runtime error function type.
 * @param[in] location Location of the error event.
 * @param[in] result   Result/status code.
 *****************************************************************************/
typedef void (*app_rta_on_error_t)(app_rta_error_t error,
                                   sl_status_t     result);

/// Priority type
typedef enum {
  APP_RTA_PRIORITY_LOW          = 0,
  APP_RTA_PRIORITY_BELOW_NORMAL = 1,
  APP_RTA_PRIORITY_NORMAL       = 2,
  APP_RTA_PRIORITY_ABOVE_NORMAL = 3,
  APP_RTA_PRIORITY_HIGH         = 4,
} app_rta_priority_t;

/// RTA configuration type
typedef struct {
  app_rta_requirement_t requirement;        ///< Requirement flags.
  app_rta_function_t    step;               ///< Step function (process action).
  app_rta_on_error_t    error;              ///< Error callback.
  app_rta_priority_t    priority;           ///< Required priority.
                                            ///  Modules with the same priority
                                            ///  will share runtime.
  size_t                stack_size;         ///< Stack size requirement.
  uint16_t              wait_for_guard;     ///< Time amount to wait for guard.
  uint16_t              queue_size;         ///< Queue element count.
  size_t                queue_element_size; ///< Queue element size in bytes.
} app_rta_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Create Application Runtime Adaptor (RTA) context.
 *
 * Creates the context with the given configuration (in @ref config parameter).
 * Allocates the memory for the context dynamically and creates the objects for
 * the context if requirement.start_disabled is not specified in config.
 * At this point, the context is added to a list to be managed by the
 * Application Runtime Adaptor component.
 * The function is called from RTA init function using the auto-generated source
 * file autogen/app_rta_init.c.
 * After that, the RTA component starts the runtimes, so allocates memory for
 * the runtime and creates the runtime objects (stacks, tasks, semaphores).
 *
 * @param[in]  config  Reference to the Application RTA configuration structure.
 * @param[out] ctx_out Application RTA context handle output.
 * @return     status of the operation
 * @retval     SL_STATUS_OK                if enable/disable was successful.
 * @retval     SL_STATUS_INVALID_PARAMETER if configuration is invalid.
 * @retval     SL_STATUS_NULL_POINTER      if any of the arguments is NULL.
 * @retval     SL_STATUS_ALLOCATION_FAILED if failed to allocate context memory.
 *****************************************************************************/
sl_status_t app_rta_create_context(app_rta_config_t  *config,
                                   app_rta_context_t *ctx_out);

/**************************************************************************//**
 * Enable or disable Application Runtime Adaptor context.
 *
 * Enable or disable the context that has been previously created using the
 * @ref app_rta_create_context function.
 * When disabling the context, the context-specific runtime data will be freed
 * (e.g. mutex, semaphores, etc.) and the step function will not be executed
 * until context is enabled. If no more enabled context left in the given
 * runtime, the common runtime data is also deleted (e.g. common task is stopped
 * and the stack data is freed).
 * Enabling a context will create the context runtime data and objects. If the
 * required runtime is not initialized, the runtime data is reserved and objects
 * are created (stack, task, semaphores).
 * If guard was requested in @ref config (requirement.guard) and enabling
 * the context, the guard will be automatically acquired by this call, thus, it
 * is required to release the guard if the app_rta_enable_context(ctx, true)
 * call is succeeded (with SL_STATUS_OK).
 *
 * @note If enabling the context and guard is requested, the RTA automatically
 *       acquires guard to let module manipulate on its internal state in a safe
 *       way.
 *
 * @param[in] ctx    Application RTA context handle.
 * @param[in] enable Set true to enable or false to disable context.
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if enable/disable was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_enable_context(app_rta_context_t ctx, bool enable);

/**************************************************************************//**
 * Check if the given context is enabled.
 * @param[in] ctx Application RTA context handle.
 * @return true if the context exists and it is enabled.
 *****************************************************************************/
bool app_rta_is_enabled(app_rta_context_t ctx);

/**************************************************************************//**
 * Proceed with execution of runtime context (run step function).
 *
 * It can be used only when the context is valid and enabled and
 * requirement.runtime or requirement.runtime_separate is set in the config for
 * context creation.
 *
 * @param[in] ctx Application RTA context handle.
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if proceed was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if runtime is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 * @retval    SL_STATUS_FAIL           if proceed failed.
 *****************************************************************************/
sl_status_t app_rta_proceed(app_rta_context_t ctx);

/**************************************************************************//**
 * Acquire access to variables protected by the context.
 *
 * Acquire the guard to operate on the internal state variables.
 * Guard is implemented using mutexing (RTOS).
 * Guard must not be used from ISR context, use signal or queue instead.
 * It can be used only when the context is valid and enabled and
 * requirement.guard is set in the config for context creation.
 *
 * @param[in] ctx Application RTA context handle.
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if acquire was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if guard is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 * @retval    SL_STATUS_FAIL           if acquire failed (timed out).
 *****************************************************************************/
sl_status_t app_rta_acquire(app_rta_context_t ctx);

/**************************************************************************//**
 * Finish access to variables protected by the context.
 *
 * Release the guard to stop working on the internal state variables.
 * Guard is implemented using mutexing (RTOS).
 * Guard must not be used from ISR context, use ISR signal or queue instead.
 * It can be used only when the context is valid and enabled and
 * requirement.guard is set in the config for context creation.
 *
 * @param[in] ctx Application RTA context handle.
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if release was successful.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if guard is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_release(app_rta_context_t ctx);

/**************************************************************************//**
 * Signal the Application RTA context from ISR context.
 *
 * Send a signal (to the step function). It can be called from ISR or non-ISR
 * context also. The signal can be handled using @ref app_rta_signal_check.
 * It can be used only when the context is valid and enabled and
 * requirement.signal is set in the config for context creation.
 *
 * @param[in] ctx Application RTA context handle.
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if a signal was sent successfully.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if signal is not supported.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_signal(app_rta_context_t ctx);

/**************************************************************************//**
 * Check the signal coming from any context (including ISR).
 *
 * Check for the signal (in the step function). Signal can be emitted from ISR
 * or non-ISR context using @ref app_rta_signal.
 * It can be used only when the context is valid and enabled and
 * requirement.signal is set in the config for context creation.
 *
 * @param[in]  ctx Application RTA context handle.
 * @return     status of the operation
 * @retval     SL_STATUS_OK             if a signal was received.
 * @retval     SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval     SL_STATUS_NOT_SUPPORTED  if signal is not supported.
 * @retval     SL_STATUS_EMPTY          if no signal present.
 * @retval     SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_signal_check(app_rta_context_t ctx);

/**************************************************************************//**
 * Check the signal coming from any context (including ISR) and acquire guard.
 *
 * Check for the signal (in the step function). Signal can be emitted from ISR
 * or non-ISR context using @ref app_rta_signal.
 * It can be used only when the context is valid and enabled and
 * requirement.signal is set in the config for context creation.
 *
 * RTA automatically acquires guard to let module manipulate on its internal
 * state in a safe way, thus, releasing the guard is mandatory if this function
 * succeeds.
 *
 * @param[in]  ctx Application RTA context handle.
 * @return     status of the operation
 * @retval     SL_STATUS_OK             if a signal was received.
 * @retval     SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval     SL_STATUS_EMPTY          if no signal present.
 * @retval     SL_STATUS_NOT_SUPPORTED  if signal or guard is not supported.
 * @retval     SL_STATUS_FAIL           if failed to acquire the guard.
 * @retval     SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_signal_check_and_acquire(app_rta_context_t ctx);

/**************************************************************************//**
 * Push data into the Application RTA context queue from any context (including
 * ISR context).
 *
 * Send data (to the step function) through a FIFO queue. It can be called from
 * ISR or non-ISR context also. The data can be handled using
 * @ref app_rta_queue_read.
 * It can be used only when the context is valid and enabled and
 * requirement.queue is set in the config for context creation.
 *
 * @param[in] ctx  Application RTA context handle.
 * @param[in] data Data to send.
 * @param[in] size Size of the data.
 * @return    status of the operation
 * @retval    SL_STATUS_OK             if data was pushed to the queue
 *                                     successfully.
 * @retval    SL_STATUS_NULL_POINTER   if @ref data is NULL.
 * @retval    SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval    SL_STATUS_NOT_SUPPORTED  if queue is not supported.
 * @retval    SL_STATUS_FULL           if the queue is full.
 * @retval    SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_queue_push(app_rta_context_t ctx,
                               uint8_t           *data,
                               size_t            size);

/**************************************************************************//**
 * Read the queue coming from any context (including ISR).
 *
 * Read data from the FIFO queue (in the step function). Data can be emitted
 * from ISR or non-ISR context using @ref app_rta_queue_push.
 * It can be used only when the context is valid and enabled and
 * requirement.queue is set in the config for context creation.
 *
 * @param[in]  ctx  Application RTA context handle.
 * @param[in]  data Data arrived.
 * @param[out] size Size of the data.
 * @return     status of the operation
 * @retval     SL_STATUS_OK             if data was received from the queue.
 * @retval     SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval     SL_STATUS_NULL_POINTER   if @ref data or @ref size is NULL.
 * @retval     SL_STATUS_NOT_SUPPORTED  if queue is not supported.
 * @retval     SL_STATUS_EMPTY          if no data present in the queue.
 * @retval     SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_queue_read(app_rta_context_t ctx,
                               uint8_t           *data,
                               size_t            *size);

/**************************************************************************//**
 * Read the queue coming from any context (including ISR) and acquire guard.
 *
 * Read data from the FIFO queue (in the step function). Data can be emitted
 * from ISR or non-ISR context using @ref app_rta_queue_push.
 * It can be used only when the context is valid and enabled and
 * requirement.queue is set in the config for context creation.
 *
 * RTA automatically acquires guard to let module manipulate on its internal
 * state in a safe way, thus, releasing the guard is mandatory if this function
 * succeeds.
 *
 * @param[in]  ctx  Application RTA context handle.
 * @param[in]  data Data arrived.
 * @param[out] size Size of the data.
 * @return     status of the operation
 * @retval     SL_STATUS_OK             if data was received from the queue.
 * @retval     SL_STATUS_INVALID_HANDLE if context is not valid.
 * @retval     SL_STATUS_NULL_POINTER   if @ref data or @ref size is NULL.
 * @retval     SL_STATUS_NOT_SUPPORTED  if queue or guard is not supported.
 * @retval     SL_STATUS_EMPTY          if no data present in the queue.
 * @retval     SL_STATUS_FAIL           if failed to acquire the guard.
 * @retval     SL_STATUS_INVALID_STATE  if context is not enabled.
 *****************************************************************************/
sl_status_t app_rta_queue_read_and_acquire(app_rta_context_t ctx,
                                           uint8_t           *data,
                                           size_t            *size);

#ifdef __cplusplus
};
#endif

#endif // APP_RTA_H
