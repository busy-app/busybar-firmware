/***************************************************************************//**
 * @file
 * @brief OS abstraction layer primitives for the bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "btl_interface_lock.h"
#include "btl_errorcode.h"
#include "cmsis_os2.h"

static osMutexAttr_t mutex_attr = {
  .name      = "btlInterfaceLock",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0
};

static osMutexId_t mutex_id;

int32_t btl_osal_init_mutex(void)
{
  mutex_id = osMutexNew(&mutex_attr);
  return (mutex_id == NULL ? BOOTLOADER_ERROR_RTOS_MUTEX_FAILED : BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS);
}

int32_t btl_osal_free_mutex(void)
{
  osStatus_t status = osMutexDelete(mutex_id);
  return (status == osOK ? BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS : BOOTLOADER_ERROR_RTOS_MUTEX_FAILED);
}

int32_t btl_osal_lock_aquire(void)
{
  osStatus_t status = osMutexAcquire(mutex_id, osWaitForever);
  return (status == osOK ? BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS : BOOTLOADER_ERROR_RTOS_MUTEX_FAILED);
}

int32_t btl_osal_lock_release(void)
{
  osStatus_t status = osMutexRelease(mutex_id);
  return (status == osOK ? BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS : BOOTLOADER_ERROR_RTOS_MUTEX_FAILED);
}

#endif
