/*******************************************************************************
 * @file  malloc_thread_safety.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_common.h"
#include "sl_constants.h"
#include "cmsis_os2.h"
#include <malloc.h>

static osMutexId_t malloc_mutex = NULL;
static uint32_t mutex_object[20];

void __malloc_lock(struct _reent *reent)
{
  UNUSED_PARAMETER(reent);
  // Verify mutex has been created, if not create it
  if (malloc_mutex == NULL) {
    const osMutexAttr_t mutex_attr = {
      .name      = NULL,
      .attr_bits = 0,
      .cb_mem    = &mutex_object,
      .cb_size   = sizeof(mutex_object),
    };
    malloc_mutex = osMutexNew(&mutex_attr);
  }

  // Verify mutex was created
  if (osKernelGetState() == osKernelRunning && malloc_mutex != NULL) {
    osMutexAcquire(malloc_mutex, osWaitForever);
  }
}

void __malloc_unlock(struct _reent *reent)
{
  UNUSED_PARAMETER(reent);
  if (osKernelGetState() == osKernelRunning && malloc_mutex != NULL) {
    osMutexRelease(malloc_mutex);
  }
}
