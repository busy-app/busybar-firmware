/***************************************************************************//**
 * @file
 * @brief I2C simple poll-based master mode driver
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

#include <stddef.h>
#include <string.h>
#include "em_device.h"
#include "sl_assert.h"
#include "sl_clock_manager.h"
#include "sl_device_peripheral.h"
#include "sl_udelay.h"
#include "sl_gpio.h"
#include "sl_i2c.h"
#include "sli_i2c.h"
#include "sl_i2cspm.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Transfer timeout (how many polls).
#define SL_I2CSPM_TRANSFER_TIMEOUT 300000

// SCL hold time in microseconds.
#ifndef SL_I2CSPM_SCL_HOLD_TIME_US
#define SL_I2CSPM_SCL_HOLD_TIME_US 100
#endif

// Clock cycles for bus recovery.
#define SL_I2CSPM_RECOVER_NUM_CLOCKS  10

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static sl_peripheral_t get_peripheral_from_i2c_port(I2C_TypeDef *port);
static sl_i2c_freq_mode_t map_frequency_to_mode(uint32_t frequency);

/*******************************************************************************
 *****************************   GLOBAL VARIABLES   ****************************
 ******************************************************************************/
// I2C handles for each I2CSPM instance - these use the sl_i2c driver internally
static sl_i2c_handle_t i2cDeviceHandle[I2C_COUNT];

/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
*******************************************************************************/

/*******************************************************************************
 *   Initalize I2C peripheral
 ******************************************************************************/
void I2CSPM_Init(I2CSPM_Init_TypeDef *init)
{
  sl_status_t status;
  int8_t i2cInstance;
  sl_i2c_init_params_t i2c_init_params;
  sl_i2c_handle_t *handle;

  EFM_ASSERT(init != NULL);

  // Get the instance from port number.
  i2cInstance = I2C_NUM(init->port);
  EFM_ASSERT(i2cInstance >= 0 && i2cInstance < I2C_COUNT);

  // Get the handle for this instance
  handle = &i2cDeviceHandle[i2cInstance];

  // Setup initialization parameters for sl_i2c_init
  i2c_init_params.i2c_peripheral = get_peripheral_from_i2c_port(init->port);

  // Check if we got a valid peripheral
  if (i2c_init_params.i2c_peripheral == NULL) {
    // Invalid I2C port - cannot initialize
    EFM_ASSERT(false);
    return;
  }

  i2c_init_params.operating_mode = SL_I2C_LEADER_MODE;
  i2c_init_params.frequency_mode = map_frequency_to_mode(init->i2cMaxFreq);
  i2c_init_params.scl_gpio.port = init->sclPort;
  i2c_init_params.scl_gpio.pin = init->sclPin;
  i2c_init_params.sda_gpio.port = init->sdaPort;
  i2c_init_params.sda_gpio.pin = init->sdaPin;

  // Initialize using the sl_i2c driver
  status = sl_i2c_init(handle, &i2c_init_params);

  // Handle the return status properly
  if (status == SL_STATUS_ALREADY_INITIALIZED) {
    // Already initialized, just return
    return;
  }
  EFM_ASSERT(status == SL_STATUS_OK);
}

/*******************************************************************************
 *   Perform I2C transfer
 ******************************************************************************/
I2C_TransferReturn_TypeDef I2CSPM_Transfer(I2C_TypeDef *i2c, I2C_TransferSeq_TypeDef *seq)
{
  sl_status_t status;
  int8_t i2cInstance;
  sl_i2c_handle_t *handle;
  uint32_t timeout_ms = SL_I2CSPM_TRANSFER_TIMEOUT;
  uint8_t follower_addr; // Store the shifted address

  // Validate input parameters
  if (i2c == NULL || seq == NULL) {
    return i2cTransferUsageFault;
  }

  // Get the instance from port number.
  i2cInstance = I2C_NUM(i2c);
  if (i2cInstance < 0 || i2cInstance >= I2C_COUNT) {
    return i2cTransferUsageFault;
  }

  // Get the handle for this instance
  handle = &i2cDeviceHandle[i2cInstance];

  // Check if this instance is initialized by examining the handle
  // If i2c_peripheral is NULL, the handle has not been initialized
  if (handle->i2c_peripheral == NULL) {
    return i2cTransferUsageFault;
  }

  // Compute follower address once (shift from 8-bit to 7-bit address)
  follower_addr = seq->addr >> 1;

  // Handle different transfer types using sl_i2c driver for all series
  switch (seq->flags) {
    case I2C_FLAG_WRITE:
      // Simple write operation
      if (seq->buf[0].len == 0) {
        return i2cTransferUsageFault;
      }
      status = sl_i2c_leader_send_blocking(handle, follower_addr, seq->buf[0].data, seq->buf[0].len, timeout_ms);
      break;

    case I2C_FLAG_READ:
      // Simple read operation
      if (seq->buf[0].len == 0) {
        return i2cTransferUsageFault;
      }
      status = sl_i2c_leader_receive_blocking(handle, follower_addr, seq->buf[0].data, seq->buf[0].len, timeout_ms);
      break;

    case I2C_FLAG_WRITE_READ:
      // Write followed by read (repeated start)
      if (seq->buf[0].len == 0 || seq->buf[1].len == 0) {
        return i2cTransferUsageFault;
      }
      status = sl_i2c_leader_transfer_blocking(handle, follower_addr,
                                               seq->buf[0].data, seq->buf[0].len,
                                               seq->buf[1].data, seq->buf[1].len,
                                               timeout_ms);
      break;

    case I2C_FLAG_WRITE_WRITE:
    {
      // Two buffers written in a single transaction: S+ADDR(W)+DATA0+DATA1+P
      if (seq->buf[0].len == 0 || seq->buf[1].len == 0) {
        return i2cTransferUsageFault;
      }

      // Use internal API for write-write without malloc
      status = sli_i2c_leader_write_no_stop_blocking(handle,
                                                     follower_addr,
                                                     seq->buf[0].data,
                                                     seq->buf[0].len,
                                                     seq->buf[1].data,
                                                     seq->buf[1].len,
                                                     timeout_ms);
      break;
    }

    default:
      return i2cTransferUsageFault;
  }

  // Convert sl_status_t to I2C_TransferReturn_TypeDef
  switch (status) {
    case SL_STATUS_OK:
      return i2cTransferDone;
    case SL_STATUS_NOT_FOUND:   // Address NACK
      return i2cTransferNack;
    case SL_STATUS_ABORT:       // Data NACK
      return i2cTransferNack;
    case SL_STATUS_TRANSMIT:
      return i2cTransferArbLost;
    case SL_STATUS_IO:
      return i2cTransferBusErr;
    case SL_STATUS_TIMEOUT:
      return i2cTransferBusErr;
    default:
      return i2cTransferUsageFault;
  }
}

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Map I2C port to peripheral enum
 ******************************************************************************/
static sl_peripheral_t get_peripheral_from_i2c_port(I2C_TypeDef *port)
{
  // Use I2C_NUM to get instance number, then switch on it
  int8_t i2c_instance = I2C_NUM(port);

  switch (i2c_instance) {
#if defined(I2C0)
    case 0:
      return SL_PERIPHERAL_I2C0;
#endif
#if defined(I2C1)
    case 1:
      return SL_PERIPHERAL_I2C1;
#endif
#if defined(I2C2)
    case 2:
      return SL_PERIPHERAL_I2C2;
#endif
#if defined(I2C3)
    case 3:
      return SL_PERIPHERAL_I2C3;
#endif
    default:
      // Invalid I2C instance - this should never happen with proper usage
      EFM_ASSERT(false);
      return NULL; // Return NULL to indicate invalid peripheral
  }
}

/***************************************************************************//**
 * @brief
 *   Map frequency value to frequency mode enum
 ******************************************************************************/
static sl_i2c_freq_mode_t map_frequency_to_mode(uint32_t frequency)
{
  if (frequency <= 100000) {
    return SL_I2C_FREQ_STANDARD_MODE;
  } else if (frequency <= 400000) {
    return SL_I2C_FREQ_FAST_MODE;
  } else {
    return SL_I2C_FREQ_FASTPLUS_MODE;
  }
}
