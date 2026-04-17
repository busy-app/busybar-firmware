/***************************************************************************/ /**
 * @file
 * @brief CPC Instance implementation.
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

#include "sl_status.h"
#include "sli_cpc_assert.h"
#include "sli_cpc_debug.h"
#include "sli_cpc_instance.h"
#include "sl_cpc_instance_handles.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
#include "sli_cpc_fwu.h"
#endif

/***************************************************************************/ /**
 * Allocate kernel-related resources for a CPC instance.
 *
 * @retval  SL_STATUS_OK    Resources allocated successfully.
 * @retval  Other sl_status_t if error occurred.
 ******************************************************************************/
sl_status_t sli_cpc_instance_kernel_init(sli_cpc_instance_t *inst)
{
#if defined(SL_CATALOG_KERNEL_PRESENT)
  sl_status_t status = SL_STATUS_OK;

  // attributes for the task, mutex and semaphore
  osThreadAttr_t task_attr = {
    .name = "CPC Core",
    .priority = SL_CPC_TASK_PRIORITY,
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
    .stack_mem = &inst->thread_stack[0],
    .stack_size = THREAD_STACK_SIZE,
    .cb_mem = &inst->thread_cb[0],
    .cb_size = osThreadCbSize,
#else
    .stack_mem = NULL,
    .stack_size = 0U,
    .cb_mem = NULL,
    .cb_size = 0U,
#endif // SL_CATALOG_CMSIS_OS_COMMON_PRESENT
  };

  osMutexAttr_t mutex_attr = {
    .attr_bits = osMutexRecursive,
    .name = "CPC Endpoints List Lock",
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
    .cb_mem = inst->endpoints_list_mutex_cb,
    .cb_size = osMutexCbSize,
#else
    .cb_mem = NULL;
    .cb_size = 0U;
#endif // SL_CATALOG_CMSIS_OS_COMMON_PRESENT
  };

  osSemaphoreAttr_t event_signal_semaphore_attr = {
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
    .cb_mem = inst->event_semaphore_cb,
    .cb_size = osSemaphoreCbSize,
#else
    .cb_mem = NULL,
    .cb_size = 0U,
#endif // SL_CATALOG_CMSIS_OS_COMMON_PRESENT
  };

  inst->endpoints_list_lock = osMutexNew(&mutex_attr);
  if (inst->endpoints_list_lock == 0) {
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_ALLOCATION_FAILED;
    goto exit;
  }

  inst->event_signal = osSemaphoreNew(SLI_CPC_EVENT_SIGNAL_MAX_COUNT_INST(inst),
                                      0u,
                                      &event_signal_semaphore_attr);
  if (inst->event_signal == 0) {
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_ALLOCATION_FAILED;
    goto free_list_lock;
  }

  inst->thread_id = osThreadNew(&sl_cpc_task, (void*)inst, &task_attr);
  if (inst->thread_id == 0) {
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_ALLOCATION_FAILED;
    goto free_event_signal;
  }

  goto exit;

  free_event_signal:
  osSemaphoreDelete(inst->event_signal);

  free_list_lock:
  osMutexDelete(inst->endpoints_list_lock);

  exit:
  return status;
#else  // if not SL_CATALOG_KERNEL_PRESENT
  (void)inst;

  return SL_STATUS_OK;
#endif // SL_CATALOG_KERNEL_PRESENT
}

/***************************************************************************/ /**
 * Free kernel-related resources that were allocated in instance_kernel_init().
 ******************************************************************************/
static void instance_kernel_deinit(sli_cpc_instance_t *inst)
{
#if defined(SL_CATALOG_KERNEL_PRESENT)
  osThreadTerminate(inst->thread_id);
  osSemaphoreDelete(inst->event_signal);
  osMutexDelete(inst->endpoints_list_lock);
#endif // SL_CATALOG_KERNEL_PRESENT
  (void)inst;
}

/***************************************************************************//**
 * Hardware-initialization of the instance, sets up the driver.
 ******************************************************************************/
static sl_status_t instance_hw_init(sli_cpc_instance_t *inst)
{
  sl_status_t status;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  // retrieve driver associated with this instance
  inst->driver = sli_cpc_drv_get_driver();
#endif

  // get driver capabilities
  status = inst->driver->ops.get_capabilities(inst->driver,
                                              &inst->sli_cpc_driver_capabilities);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

  // call the driver hw initialization function
  status = inst->driver->ops.hw_init(inst->driver);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

  return status;
}

/***************************************************************************//**
 * Early init allocation, set up software variables and enable driver's hardware.
 ******************************************************************************/
static sl_status_t sli_cpc_instance_early_init(sli_cpc_instance_t *inst)
{
  sl_status_t status;

  SLI_CPC_DEBUG_CORE_INIT(inst);

  sl_slist_init(&inst->endpoints);
  sl_slist_init(&inst->defer_endpoint_free_list);
  sl_slist_init(&inst->transmit_queue);
  sl_slist_init(&inst->pending_on_security_ready_queue);
  sl_slist_init(&inst->write_complete_callback_list);
  sl_slist_init(&inst->process_queue);

  inst->event_counter = 0;
  inst->cpc_core_in_reset = false;
#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  inst->cpc_initialized = false;
#else
  inst->soft_reset = false;
#endif

  sli_cpc_dispatcher_init_handle(&inst->callback_dispatcher_handle, inst);
  sli_cpc_dispatcher_init_handle(&inst->retransmit_dispatcher_handle, inst);

  status = instance_hw_init(inst);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

  return status;
}

/***************************************************************************//**
 * Free resource reserved in sli_cpc_instance_early_init().
 ******************************************************************************/
static void sli_cpc_instance_early_deinit(sli_cpc_instance_t *inst)
{
  // drivers don't implement a hw_deinit operation, so skip that.

  // do not reset driver to null pointer. When running in primary mode, the
  // bootloader might still need to access the instance driver. This has to
  // be revisited at some point.
  // inst->driver = NULL;

  sli_cpc_dispatcher_cancel(&inst->callback_dispatcher_handle);
  sli_cpc_dispatcher_cancel(&inst->retransmit_dispatcher_handle);

  // lists don't have a deinit function

  instance_kernel_deinit(inst);
}

/***************************************************************************//**
 * Late initialization, finish initializing the driver and the system EP.
 ******************************************************************************/
static sl_status_t sli_cpc_instance_late_init(sli_cpc_instance_t *inst)
{
  sl_status_t status;

  status = inst->driver->ops.init(inst->driver, inst);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

  status = sli_cpc_system_init(&inst->system_ep);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

  return status;
}

/***************************************************************************//**
 * Free resource reserved in sli_cpc_instance_late_init().
 ******************************************************************************/
static void sli_cpc_instance_late_deinit(sli_cpc_instance_t *inst)
{
  (void)inst;

  // system endpoint doesn't have a deinit function
#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  if (inst->driver->ops.deinit) {
    inst->driver->ops.deinit(inst->driver);
  }
#endif
}

/***************************************************************************//**
 * Initialize an instance.
 ******************************************************************************/
sl_status_t sli_cpc_instance_init(sli_cpc_instance_t *inst)
{
  sl_status_t status;

  status = sli_cpc_instance_early_init(inst);
  if (status != SL_STATUS_OK) {
    goto exit;
  }

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  status = sli_cpc_fwu_probe_bootloader();
  if (status != SL_STATUS_OK) {
    goto early_deinit;
  }
#endif

  status = sli_cpc_instance_late_init(inst);
  if (status != SL_STATUS_OK) {
    goto early_deinit;
  }

  return SL_STATUS_OK;

  early_deinit:
  sli_cpc_instance_early_deinit(inst);

  exit:
  return status;
}

/***************************************************************************//**
 * Deinitialize an instance.
 ******************************************************************************/
void sli_cpc_instance_deinit(sli_cpc_instance_t *inst)
{
  sli_cpc_instance_late_deinit(inst);
  sli_cpc_instance_early_deinit(inst);
}

/***************************************************************************//**
 * Start instance, letting it process packets.
 ******************************************************************************/
sl_status_t sli_cpc_instance_start(sli_cpc_instance_t *inst)
{
  sl_status_t status;

  status = inst->driver->ops.start_rx(inst->driver);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

  status = sli_cpc_system_start(&inst->system_ep);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

  return status;
}

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************/ /**
 * Checks if CPC has been initialized
 ******************************************************************************/
bool sli_cpc_instance_is_initialized(sli_cpc_instance_t *inst)
{
  return inst->cpc_initialized;
}
#endif

/***************************************************************************/ /**
 * Get bus bitrate from driver.
 ******************************************************************************/
uint32_t sli_cpc_instance_bus_get_bitrate(sli_cpc_instance_t *inst)
{
  SLI_CPC_ASSERT(inst != NULL);
  SLI_CPC_ASSERT(inst->driver != NULL);

  return inst->driver->ops.get_bus_bitrate(inst->driver);
}

/***************************************************************************/ /**
 * Get maximum bus bitrate from driver.
 ******************************************************************************/
uint32_t sli_cpc_instance_bus_get_max_bitrate(sli_cpc_instance_t *inst)
{
  SLI_CPC_ASSERT(inst != NULL);
  SLI_CPC_ASSERT(inst->driver != NULL);

  return inst->driver->ops.get_bus_max_bitrate(inst->driver);
}
