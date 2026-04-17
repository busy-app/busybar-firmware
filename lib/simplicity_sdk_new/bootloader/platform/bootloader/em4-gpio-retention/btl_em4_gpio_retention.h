/***************************************************************************//**
 * @file
 * @brief EM4 GPIO Retention header
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef BTL_EM4_GPIO_RETENTION_H
#define BTL_EM4_GPIO_RETENTION_H

#include "btl_interface.h"

/***************************************************************************//**
 * Validate and restore the EM4 GPIO retention configuration stored in BURAM.
 *
 * This function validates the stored configuration and restores the GPIO pin
 * configurations.
 *
 * @return BOOTLOADER_OK on success, BOOTLOADER_ERROR_GPIO_RETENTION_INVALID
 * on failure (e.g., invalid signature, invalid pin configuration).
 ******************************************************************************/
int32_t btl_em4GpioRetentionValidateAndRestore(void);

/***************************************************************************//**
 * Check if BURAM reset counter is enabled.
 *
 * @return true if reset counter is enabled, false otherwise.
 ******************************************************************************/
bool btl_em4GpioRetentionCounterEnabled(void);

/***************************************************************************//**
 * Get current BURAM reset counter value.
 *
 * @return Current reset counter value (0-15).
 ******************************************************************************/
uint8_t btl_em4GpioRetentionCounterGet(void);

/***************************************************************************//**
 * Enable BURAM reset counter.
 ******************************************************************************/
void btl_em4GpioRetentionCounterEnable(void);

/***************************************************************************//**
 * Increment BURAM reset counter.
 ******************************************************************************/
void btl_em4GpioRetentionCounterIncrement(void);

/***************************************************************************//**
 * Invalidate BURAM reset counter.
 ******************************************************************************/
void btl_em4GpioRetentionCounterInvalidate(void);

/***************************************************************************//**
 * Set BURAM reset reason.
 *
 * @param reason  The reset reason to set.
 ******************************************************************************/
void btl_em4GpioRetentionSetResetReason(uint8_t reason);

/***************************************************************************//**
 * Unlatch EM4 GPIO retention.
 ******************************************************************************/
void btl_em4GpioRetentionUnlatch(void);

/***************************************************************************//**
 * Check if BURAM signature is valid and reset reason indicates bootloader entry.
 *
 * @return true if signature is valid and reset reason indicates bootloader entry.
 ******************************************************************************/
bool btl_em4GpioRetentionEnterBootloader(void);

/***************************************************************************//**
 * Check if device woke up from EM4 sleep.
 *
 * This function checks if the device woke up from EM4 sleep by examining
 * the reset cause register.
 *
 * @return true if device woke up from EM4, false otherwise.
 ******************************************************************************/
bool btl_em4GpioRetentionIsWakeup(void);

#ifdef __cplusplus
}
#endif

#endif // BTL_EM4_GPIO_RETENTION_H
