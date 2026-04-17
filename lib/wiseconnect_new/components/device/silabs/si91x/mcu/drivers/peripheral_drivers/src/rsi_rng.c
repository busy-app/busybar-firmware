/******************************************************************************
* @file  rsi_rng.c
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

// Include Files

#include "rsi_rom_rng.h"

/*==============================================*/
/**
 * @fn           uint32_t rng_start(HWRNG_Type *pRNG, uint8_t rngMode)     
 * @brief        This API is used to start the Random Number Generation
 * @param[in]    pRNG        :   pointer to the control register
 * @param[in]    rngMode     :   mode of random number generation
 * @return       return RSI_OK if successful execution
 */
uint32_t rng_start(HWRNG_Type *pRNG, uint8_t rngMode)
{
  if (rngMode == 0 || rngMode == 1) {
    if (rngMode == RSI_RNG_TRUE_RANDOM) {
      pRNG->HWRNG_CTRL_REG_b.HWRNG_RNG_ST  = 1;
      pRNG->HWRNG_CTRL_REG_b.HWRNG_PRBS_ST = 0;
    } else {
      pRNG->HWRNG_CTRL_REG_b.HWRNG_PRBS_ST = 1;
      pRNG->HWRNG_CTRL_REG_b.HWRNG_RNG_ST  = 0;
    }
  } else {
    return ERROR_RNG_INVALID_ARG;
  }
  return RSI_OK;
}

void rng_config_lfsr(HWRNG_Type *pRNG, rng_lfsr_config_t lfsr_config_param)
{
  pRNG->HWRNG_CTRL_REG_b.TAP_LFSR_INPUT = lfsr_config_param;
}

/***************************************************************************/
/**
 * @brief   This API sets the soft reset bit of the HRNG peripheral.
 * @param[in]  pRNG  : Pointer to the control register of the hardware random number generator.
 * @return      none
 ******************************************************************************/
void rng_soft_reset_set(HWRNG_Type *pRNG)
{
  pRNG->HWRNG_CTRL_REG_b.SOFT_RESET = 1;
}

/***************************************************************************/
/**
 * @brief   This API clears the soft reset bit of the HRNG peripheral.
 * @param[in]  pRNG  : Pointer to the control register of the hardware random number generator.
 * @return      none
 ******************************************************************************/
void rng_soft_reset_clear(HWRNG_Type *pRNG)
{
  pRNG->HWRNG_CTRL_REG_b.SOFT_RESET = 0;
}

/*==============================================*/
/**
 * @fn           void rng_stop(HWRNG_Type *pRNG)    
 * @brief        This API is used to stop the Random Number Generation
 * @param[in]    pRNG        :   pointer to the control register
 * @return       none
 */
void rng_stop(HWRNG_Type *pRNG)
{
  //Disabling pseudo random number and true number generation bit.
  pRNG->HWRNG_CTRL_REG_b.HWRNG_PRBS_ST = 0;
  pRNG->HWRNG_CTRL_REG_b.HWRNG_RNG_ST  = 0;
}

/***************************************************************************/
/**
 * @brief
 *   Read random data from the HRNG using the LFSR input latch.
 *
 * @details
 *   This function reads random data from the HRNG using the LFSR input latch register.
 *   The data is valid only when the LFSR_32_BIT_INPUT_VALID bit is set. After reading
 *   the data from the LFSR_INPUT_LATCH_REG, the LFSR_32_BIT_INPUT_VALID bit will be
 *   cleared by hardware.
 *
 * @param[in]    pRNG        : Pointer to the control register of the hardware random number generator.
 * @param[out]   randomBytes : Pointer to an array where the random data will be stored.
 * @param[in]    numberOfBytes : Number of bytes of random data to read.
 *
 * @return
 *   - RSI_OK on success.
 *   - RSI_FAIL if the LFSR_32_BIT_INPUT_VALID bit is not set or if the TAP_LFSR_INPUT bit is not set.
 ******************************************************************************/
uint32_t rng_read_lfsr_input(HWRNG_Type *pRNG, uint32_t *randomBytes, uint32_t numberOfBytes)
{
  uint32_t status = RSI_FAIL, i;
  if (pRNG->HWRNG_CTRL_REG_b.TAP_LFSR_INPUT == 1) {
    for (i = 0; i < numberOfBytes; i++) {
      // Check if LFSR input data is valid
      if (pRNG->HWRNG_CTRL_REG_b.LFSR_32_BIT_INPUT_VALID != 0) {
        // Read from HWRNG_LFSR_INPUT_LATCH_REG
        randomBytes[i] = pRNG->HWRNG_LFSR_INPUT_LATCH_REG_REG_b.HWRNG_LFSR_INPUT_LATCH_REG;
        status         = RSI_OK;
      } else {
        status = RSI_FAIL;
        return status;
      }
    }
  } else {
    status = RSI_FAIL;
  }
  return status;
}
/*==============================================*/
/**
 * @fn           void rng_get_bytes(HWRNG_Type *pRNG, uint32_t *randomBytes, uint32_t numberOfBytes)      
 * @brief        This API is used to get the random number bytes
 * @param[in]    pRNG           :   pointer to the control register
 * @param[in]    randomBytes    :   random number register
 * @param[in]    numberOfBytes  :   number of bytes
 * @return       none
 */
void rng_get_bytes(HWRNG_Type *pRNG, uint32_t *randomBytes, uint32_t numberOfBytes)
{
  uint32_t i;

  for (i = 0; i < numberOfBytes; i++) {
    randomBytes[i] = pRNG->HWRNG_RAND_NUM_REG;
  }
}
const ROM_RNG_API_T rng_api = { &rng_start, &rng_stop, &rng_get_bytes };
