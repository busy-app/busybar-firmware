/******************************************************************************
* @file sl_si91x_hrng.c
* @brief HRNG API implementation
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

#include "sl_si91x_hrng.h"
#include "rsi_rom_clks.h"
#include "si91x_device.h"

/* Initialize the HRNG instance pointer */
HWRNG_Type *pHRNG = ((HWRNG_Type *)HWRNG_BASE);
typedef rng_lfsr_config_t sl_hrng_lfsr_config_t;

/*******************************************************************************/
/**
 * @brief Validates the parameters for the hardware random number generator.
 *
 * This function checks if the provided number of bytes is within the valid range
 * and if the pointer to the random bytes buffer is not NULL.
 *
 * @param[in] number_of_bytes The number of random bytes requested.
 * @param[in] random_bytes Pointer to the buffer where the random bytes will be stored.
 *
 * @return SL_STATUS_OK if the parameters are valid.
 * @return SL_STATUS_INVALID_PARAMETER if the parameters are invalid.
 ********************************************************************************/
static sl_status_t validate_parameters(uint8_t number_of_bytes, uint32_t *random_bytes)
{
  return (random_bytes != NULL && number_of_bytes >= SL_SI91X_HWRNG_MIN_BYTES
          && number_of_bytes <= SL_SI91X_HWRNG_MAX_BYTES)
           ? SL_STATUS_OK
           : SL_STATUS_INVALID_PARAMETER;
}

/*******************************************************************************/
/**
 * Converts an RSI error code to an SL error code.
 * This function takes an error code of type `uint32_t` and converts it 
 * to a corresponding error code of type `sl_status_t`.
 ******************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(uint32_t error)
{
  switch (error) {
    case ARM_DRIVER_OK:
      return SL_STATUS_OK;
    case ARM_DRIVER_ERROR:
    case ARM_DRIVER_ERROR_SPECIFIC:
      return SL_STATUS_FAIL;
    case ARM_DRIVER_ERROR_BUSY:
      return SL_STATUS_BUSY;
    case ARM_DRIVER_ERROR_TIMEOUT:
      return SL_STATUS_TIMEOUT;
    case ARM_DRIVER_ERROR_UNSUPPORTED:
      return SL_STATUS_NOT_SUPPORTED;
    case ARM_DRIVER_ERROR_PARAMETER:
    case ARM_SPI_ERROR_SS_MODE:
      return SL_STATUS_INVALID_PARAMETER;
    case ARM_SPI_ERROR_MODE:
      return SL_STATUS_INVALID_MODE;
    case ARM_SPI_ERROR_FRAME_FORMAT:
      return SL_STATUS_INVALID_TYPE;
    case ARM_SPI_ERROR_DATA_BITS:
    case ARM_SPI_ERROR_BIT_ORDER:
      return SL_STATUS_INVALID_RANGE;
    default:
      return SL_STATUS_FAIL;
  }
}

/***************************************************************************/
/**
 *    This function enables the peripheral clocks for the HRNG by accessing
 *    the SET1 register. It ensures the clock is turned on before using the HRNG.
 ******************************************************************************/
sl_status_t sl_si91x_hrng_init(void)
{
  int32_t error_code = RSI_CLK_PeripheralClkEnable1(M4CLK, HWRNG_PCLK_ENABLE);
  return convert_rsi_to_sl_error_code(error_code);
}

/***************************************************************************/
/**
 *    This function disables the peripheral clocks for the HRNG, turning it off
 *    when no longer needed, thereby saving power.
 ******************************************************************************/
sl_status_t sl_si91x_hrng_deinit(void)
{
  int32_t error_code = RSI_CLK_PeripheralClkDisable1(M4CLK, HWRNG_PCLK_ENABLE);
  return convert_rsi_to_sl_error_code(error_code);
}

/***************************************************************************/
/**
 *    This function starts the HRNG with the specified mode, either
 *    true random or pseudo-random generation.
 ******************************************************************************/
sl_status_t sl_si91x_hrng_start(sl_si91x_hrng_mode_t hrng_mode)
{
  int32_t error_code = rng_start(pHRNG, hrng_mode);
  return convert_rsi_to_sl_error_code(error_code);
}
/***************************************************************************/
/**
 * This function sets the soft reset bit of the HRNG peripheral.
 ***************************************************************************/
void sl_si91x_hrng_soft_reset_set(void)
{
  rng_soft_reset_set(pHRNG);
}

/***************************************************************************/
/**
 * This function clears the soft reset bit of the HRNG peripheral.
 ***************************************************************************/
void sl_si91x_hrng_soft_reset_clear(void)
{
  rng_soft_reset_clear(pHRNG);
}
/***************************************************************************/
/**
 * This function reads random data from the HRNG using the LFSR input latch register.
 ******************************************************************************/
static void sl_si91x_hrng_config_lfsr(sl_hrng_lfsr_config_t lfsr_config_param)
{
  rng_config_lfsr(pHRNG, lfsr_config_param);
}

/***************************************************************************/
/**
 *    This function reads random data from the HRNG using the LFSR input latch register.
 *    It first enables the LFSR, reads the specified number of bytes, and then disables
 *    the LFSR. The data is valid only when the LFSR_32_BIT_INPUT_VALID bit is set.
 *
 ******************************************************************************/
sl_status_t sl_si91x_hrng_read_lfsr_input(uint8_t number_of_bytes, uint32_t *random_bytes)
{
  sl_status_t status = validate_parameters(number_of_bytes, random_bytes);
  if (status != SL_STATUS_OK) {
    return status;
  }

  sl_hrng_lfsr_config_t lfsr_config = RNG_LFSR_ENABLE;
  sl_si91x_hrng_config_lfsr(lfsr_config);

  int32_t error_code = rng_read_lfsr_input(pHRNG, random_bytes, number_of_bytes);
  status             = convert_rsi_to_sl_error_code(error_code);
  if (status != SL_STATUS_OK) {
    return status;
  }

  lfsr_config = RNG_LFSR_DISABLE;
  rng_config_lfsr(pHRNG, lfsr_config);

  return status;
}

/***************************************************************************/
/**
 *    This function stops the HRNG and ends any ongoing random number generation.
 *
 ******************************************************************************/
sl_status_t sl_si91x_hrng_stop(void)
{
  rng_stop(pHRNG);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 *    This function retrieves the specified number of random bytes generated by
 *    the HRNG and stores them in the provided buffer.
 *
 *******************************************************************************/
sl_status_t sl_si91x_hrng_get_bytes(uint32_t *random_bytes, uint8_t number_of_bytes)
{
  sl_status_t status = validate_parameters(number_of_bytes, random_bytes);
  if (status != SL_STATUS_OK) {
    return status;
  }

  rng_get_bytes(pHRNG, random_bytes, number_of_bytes);
  return SL_STATUS_OK;
}
