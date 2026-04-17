/***************************************************************************//**
 * # License
 * <b> Copyright 2021 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

/**
 * @file
 * Private defines a platform abstraction layer for the Z-Wave retention register.
 */
#ifndef ZPAL_RETENTION_REGISTER_PRIVATE_H_
#define ZPAL_RETENTION_REGISTER_PRIVATE_H_

#include <stdint.h>
#include "zpal_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ZPAL_RETENTION_REGISTER_PRIVATE_LOWSIDECAL      0 // Reserved for low side calibration. NO MORE USED
#define ZPAL_RETENTION_REGISTER_PRIVATE_HIGHSIDECAL     1 // Reserved for high side calibration. NO MORE USED
#define ZPAL_RETENTION_REGISTER_PRIVATE_DEEP_SLEEP_TICK 2

#define ZPAL_RETENTION_REGISTER_PRIVATE_COUNT 3

/**
 * @brief Reads a 32-bit value from the specified retention register.
 *
 * @param[in]   index Private retention register number (zero-based).
 * @param[out]  data  Pointer to a 32-bit variable where the value can be stored.
 * @return @ref ZPAL_STATUS_OK on success, @ref ZPAL_STATUS_INVALID_ARGUMENT on
 * invalid @p index or @p data and @ref ZPAL_STATUS_FAIL otherwise.
 */
zpal_status_t zpal_retention_register_read_private(uint32_t index, uint32_t *data);

/**
 * @brief Write a 32-bit value to the specified retention register.
 *
 * @param[in] index Private retention register number (zero-based).
 * @param[in] value 32-bit value to save in retention register.
 * @return @ref ZPAL_STATUS_OK on success, @ref ZPAL_STATUS_INVALID_ARGUMENT on
 * invalid @p index and @ref ZPAL_STATUS_FAIL otherwise.
 */
zpal_status_t zpal_retention_register_write_private(uint32_t index, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* ZPAL_RETENTION_REGISTER_PRIVATE_H_ */
