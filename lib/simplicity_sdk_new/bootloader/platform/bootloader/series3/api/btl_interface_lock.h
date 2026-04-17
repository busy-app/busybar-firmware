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
#ifndef BTL_INTERFACE_LOCK_H
#define BTL_INTERFACE_LOCK_H

/***************************************************************************//**
 * @addtogroup InterfaceS3
 * @{
 * @addtogroup CommonInterfaceS3
 * @{
 * @addtogroup ResourceLockingS3 Resource locking for Application Interface
 * @brief Locking uses mutex, for synchronizing multiple threads
 * @details
 * @{
 ******************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "stdint.h"
/***************************************************************************//**
 * @brief          Initialize a given mutex
 *
 * @param mutex    Pointer to the mutex needing initialization
 *
 * @return         BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS on Success, BOOTLOADER_ERROR_RTOS_MUTEX_FAILED on Failure
 *****************************************************************************/
int32_t btl_osal_init_mutex(void);

/***************************************************************************//**
 * @brief          Free a given mutex
 *
 * @param mutex    Pointer to the mutex being freed
 *
 * @return         BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS on Success, BOOTLOADER_ERROR_RTOS_MUTEX_FAILED on Failure
 *****************************************************************************/
int32_t btl_osal_free_mutex(void);

/***************************************************************************//**
 * @brief          Pend on a mutex
 *
 * @param mutex    Pointer to the mutex being pended on
 *
 * @return         BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS on Success, BOOTLOADER_ERROR_RTOS_MUTEX_FAILED on Failure
 *****************************************************************************/
int32_t btl_osal_lock_aquire(void);

/***************************************************************************//**
 * @brief          Release a mutex
 *
 * @param mutex    Pointer to the mutex being released
 *
 * @return         BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS on Success, BOOTLOADER_ERROR_RTOS_MUTEX_FAILED on Failure
 *****************************************************************************/
int32_t btl_osal_lock_release(void);
/** @} (end addtogroup ResourceLockingS3) */
/** @} (end addtogroup CommonInterface) */
/** @} (end addtogroup Interface) */
#endif  // SL_CATALOG_KERNEL_PRESENT
#endif  // BTL_INTERFACE_LOCK_H
