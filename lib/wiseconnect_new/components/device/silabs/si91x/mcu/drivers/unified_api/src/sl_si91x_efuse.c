/******************************************************************************
* @file sl_si91x_efuse.c
* @brief EFUSE API implementation
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
#include "sl_si91x_efuse.h"
/*******************************************************************************
 ***************************  LOCAL MACROS   ***********************************
 ******************************************************************************/

#define MAX_ADDRESS           31                     // Max possible value of address
#define MAX_BIT_POS           7                      // Max possible value of bit position
#define EFUSE_CLK_ENABLE      BIT(5)                 // Enables EFUSE_CLK_ENABLE
#define EFUSE_PCLK_ENABLE     BIT(19)                // Enables EFUSE_PCLK_ENABLE
#define EFUSE_CLK_DISABLE     ((uint32_t)0U << (5))  // Disables EFUSE_CLK_ENABLE
#define EFUSE_PCLK_DISABLE    ((uint32_t)0U << (19)) // Disables EFUSE_PCLK_ENABLE
#define EFUSE_RELEASE_VERSION 0                      // EFUSE Release version
#define EFUSE_SQA_VERSION     0                      // EFUSE SQA version
#define EFUSE_DEV_VERSION     1                      // EFUSE Developer

// Converts clock frequency (Hz) to clock period in nanoseconds
#define CLK_PERIOD_NS(freq_hz) (1000000000.0 / (freq_hz))
//  STROBE pulse width in nanoseconds
#define TPGM_DELAY_NS 2000

/*******************************************************************************
 ***************************  Global  VARIABLES ********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error);
/*******************************************************************************
 **************************   Global function Definitions   ********************
 ******************************************************************************/

/*******************************************************************************
 * To get the release, sqa and dev version of EFUSE.
 * It returns the structure for efuse version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_efuse_version_t sl_si91x_efuse_get_version(void)
{
  sl_efuse_version_t version;
  version.release = EFUSE_RELEASE_VERSION;
  version.major   = EFUSE_SQA_VERSION;
  version.minor   = EFUSE_DEV_VERSION;
  return version;
}

/*******************************************************************************
 * This API Is Used for enabling efuse clock.
 ******************************************************************************/
sl_status_t sl_si91x_efuse_enable_clock(void)
{
  sl_status_t status;
  rsi_error_t error_status;
  error_status = RSI_CLK_PeripheralClkEnable3(M4CLK, (EFUSE_CLK_ENABLE | EFUSE_PCLK_ENABLE));
  status       = convert_rsi_to_sl_error_code(error_status);
  return status;
}

/*******************************************************************************
 * This API Is Used for disabling the efuse clock.
 ******************************************************************************/
sl_status_t sl_si91x_efuse_disable_clock(void)
{
  sl_status_t status;
  rsi_error_t error_status;
  error_status = RSI_CLK_PeripheralClkDisable3(M4CLK, (EFUSE_CLK_DISABLE | EFUSE_PCLK_DISABLE));
  status       = convert_rsi_to_sl_error_code(error_status);
  return status;
}

/*******************************************************************************
 * This API is used to Initialize the EFUSE.
 * Efuse Clock is enabled.
 * Efuse is enabled.
 ******************************************************************************/
sl_status_t sl_si91x_efuse_init(void)
{
  sl_status_t status;
  // Function to enable the efuse clock.
  status = sl_si91x_efuse_enable_clock();
  // If the status is equal to SL_STATUS_OK, enable the efuse.
  if (status == SL_STATUS_OK) {
    // Static function to enable the efuse.
    sl_si91x_efuse_enable();
  }
  return status;
}

/*******************************************************************************
 * This API is used to Un-initialize the EFUSE.
 * Efuse is disabled.
 * Efuse Clock is disabled.
 * The actions to be performed before Un-initializing the EFUSE:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 *    - Set the eFUSE address for read and write operations using @ref
 *      sl_si91x_efuse_set_address() API
 *    - Write the data in the specified address using @ref
 *      sl_si91x_efuse_write_bit_v2() API
 *    - Read the data from efuse address using @ref
 *      sl_si91x_efuse_memory_mapped_read_byte() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_deinit(void)
{
  sl_status_t status;
  // Static function to disable the efuse.
  sl_si91x_efuse_disable();
  // Function to disable the efuse clock.
  status = sl_si91x_efuse_disable_clock();
  return status;
}

/*******************************************************************************
 * This API is used to set the eFUSE address for read and write operations.
 * The actions to be performed before setting the eFUSE address for read and
 * write operations is:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_set_address(uint16_t address)
{
  sl_status_t status;
  /* Check for valid parameters */
  if (address > MAX_ADDRESS) {
    // Returns invalid parameter status code if address > MAX_ADDRESS
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    efuse_write_address(EFUSE, address);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * This API is used to get the eFUSE address for read and write operations.
 * The actions to be performed before getting the eFUSE address for read and
 * write operations is:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 *    - Set the eFUSE address for read and write operation using
 *      @ref sl_si91x_efuse_set_address() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_get_address(uint16_t *read_address)
{
  sl_status_t status;
  rsi_error_t error_status;
  if (read_address == NULL) {
    // To validate the read_byte pointer, if the parameters is NULL,
    // it returns an error code
    status = SL_STATUS_NULL_POINTER;
  } else {
    error_status = efuse_get_addr(EFUSE, read_address);
    status       = convert_rsi_to_sl_error_code(error_status);
  }
  return status;
}

/*******************************************************************************
 * This API is used to write the eFUSE data in the specified address.
 * The actions to be performed before writing the eFUSE data is:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 *    - Set the eFUSE address for read and write operations using @ref
 *      sl_si91x_efuse_set_address() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_write_bit(uint16_t address, uint8_t bit_pos, uint32_t hold_time)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    /* Check for valid parameters */
    if (address > MAX_ADDRESS) {
      // Returns invalid parameter status code if address > MAX_ADDRESS
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (bit_pos > MAX_BIT_POS) {
      // Returns invalid parameter status code if u8bitpos > BIT_POS_MAX
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_EFUSE_WriteBit(EFUSE, address, bit_pos, hold_time);
    status       = convert_rsi_to_sl_error_code(error_status);
  } while (false);
  return status;
}
/*******************************************************************************
 * This API is used to write the eFUSE data in the specified address.
 * The actions to be performed before writing the eFUSE data is:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 *    - Set the eFUSE address for read and write operations using @ref
 *      sl_si91x_efuse_set_address() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_write_bit_v2(uint16_t address, uint8_t bit_pos)
{
  sl_status_t status;
  uint32_t sli_si91x_m4_core_clk_freq = 0;
  uint32_t sli_si91x_hold_time        = 0;
  double sli_si91x_clk_period_ns      = 0;
  rsi_error_t error_status;
  do {
    /* Check for valid parameters */
    if (address > MAX_ADDRESS) {
      // Returns invalid parameter status code if address > MAX_ADDRESS
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (bit_pos > MAX_BIT_POS) {
      // Returns invalid parameter status code if u8bitpos > BIT_POS_MAX
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Get the clock frequency of the M4 core
    sl_si91x_clock_manager_m4_get_core_clk_src_freq(&sli_si91x_m4_core_clk_freq);

    // Get clock period in nanoseconds
    sli_si91x_clk_period_ns = CLK_PERIOD_NS(sli_si91x_m4_core_clk_freq);

    // Calculate hold time
    sli_si91x_hold_time = (uint32_t)(TPGM_DELAY_NS / sli_si91x_clk_period_ns);

    error_status = RSI_EFUSE_WriteBit(EFUSE, address, bit_pos, sli_si91x_hold_time);
    status       = convert_rsi_to_sl_error_code(error_status);
  } while (false);
  return status;
}
/*******************************************************************************
 * This API is used to Read the 1 word(16 bits) of data from 32x8 byte eFUSE
 * memory(OTP) in memory mapped mode.
 * The actions to be performed before reading the eFUSE data in memory
 * mapped mode is:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 *    - Set the eFUSE address for read and write operations using @ref
 *      sl_si91x_efuse_set_address() API
 *    - Write the data in the specified address using @ref
 *      sl_si91x_efuse_write_bit_v2() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_memory_mapped_read_word(uint16_t address, uint16_t *read_word, uint32_t soc_clk)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    /* Check for valid parameters */
    if (address > MAX_ADDRESS) {
      // Returns invalid parameter status code if address > MAX_ADDRESS
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (read_word == NULL) {
      // To validate the read_word pointer, if the parameters is NULL,
      // it returns an error code
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    error_status = RSI_EFUSE_MemMapReadWord(EFUSE, address, read_word, soc_clk);
    status       = convert_rsi_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to read the data from 32x8 byte eFUSE memory(OTP) in memory
 * mapped mode.
 * The actions to be performed before reading the eFUSE data in memory mapped
 * mode is:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 *    - Set the eFUSE address for read and write operations using @ref
 *      sl_si91x_efuse_set_address() API
 *    - Write the data in the specified address using @ref
 *      sl_si91x_efuse_write_bit_v2() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_memory_mapped_read_byte(uint16_t address, uint8_t *read_byte, uint32_t soc_clk)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    /* Check for valid parameters */
    if (address > MAX_ADDRESS) {
      // Returns invalid parameter status code if address > MAX_ADDRESS
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (read_byte == NULL) {
      // To validate the read_byte pointer, if the parameters is NULL,
      // it returns an error code
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    error_status = RSI_EFUSE_MemMapReadByte(EFUSE, address, read_byte, soc_clk);
    status       = convert_rsi_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is used to read the data from 32x8 byte eFUSE memory(OTP) in fsm
 * mode.
 * The actions to be performed before reading the eFUSE data in fsm or
 * indirect mode is:
 *    - Initialize the EFUSE using @ref sl_si91x_efuse_init() API
 *    - Set the eFUSE address for read and write operations using @ref
 *      sl_si91x_efuse_set_address() API
 *    - Write the data in the specified address using @ref
 *      sl_si91x_efuse_write_bit_v2() API
 ******************************************************************************/
sl_status_t sl_si91x_efuse_fsm_read_byte(uint16_t address, uint8_t *read_byte, uint32_t soc_clk)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    /* Check for valid parameters */
    if (address > MAX_ADDRESS) {
      // Returns invalid parameter status code if address > MAX_ADDRESS
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (read_byte == NULL) {
      // To validate the read_byte pointer, if the parameters is NULL,
      // it returns an error code
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    error_status = RSI_EFUSE_FsmReadByte(EFUSE, address, read_byte, soc_clk);
    status       = convert_rsi_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * This API is to validate the RSI error code.
 * While calling the RSI APIs, it returns the RSI Error codes.
 * This function converts the RSI error codes into SL error codes.
 * It takes argument as RSI error type and returns the SL error type.
 * It has a single switch statement which is mapped with the SL error code and
 * after successful conversion it breaks the switch statement.
 * If the error code is not listed, by default is SL_STATUS_FAIL.
 ******************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error)
{
  sl_status_t status;
  switch (error) {
    case RSI_OK:
      status = SL_STATUS_OK;
      break;
    case INVALID_PARAMETERS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_INVALID_INPUT_FREQUENCY:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_CLOCK_NOT_ENABLED:
      status = SL_STATUS_NOT_INITIALIZED;
      break;
    case ERROR_EFUSE_INVALID_PARAMETERS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_EFUSE_INVALID_WRITE_ADDRESS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_EFUSE_INVALID_WRITE_BIT_POSITION:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }
  return status;
}
