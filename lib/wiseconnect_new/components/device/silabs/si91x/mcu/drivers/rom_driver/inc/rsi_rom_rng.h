/******************************************************************************
* @file  rsi_rom_rng.h
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

#ifndef __RSI_ROM_RNG_H__
#define __RSI_ROM_RNG_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RNG_DRIVERS 
 *  @{
 *
 */
#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn           STATIC INLINE uint32_t  RSI_RNG_Start(HWRNG_Type *pRNG, uint8_t rngMode)
 * @brief        This API is used to start the Random Number Generation
 * @param[in]    pRNG		 : Random Number Generator handler
 * @param[in]    rngMode : mode of the Random Number Generator
 *               			- \ref RNG_TRUE_RANDOM   - For True RNG
 *               			- \ref RNG_PSEUDO_RANDOM - For Psudo RNG
 * @return       returns 0 \ref RSI_OK on success ,non zero on failure
 *
 */
STATIC INLINE uint32_t RSI_RNG_Start(HWRNG_Type *pRNG, uint8_t rngMode)
{
#if defined(RNG_ROMDRIVER_PRESENT)
  return ROMAPI_RNG_API->rng_start(pRNG, rngMode);
#else
  return rng_start(pRNG, rngMode);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_RNG_Stop(HWRNG_Type *pRNG)
 * @brief        This API is used to stop the Random Number Generation
 * @param[in]    pRNG  : Random Number Generator handler
 * @return       none
 */
STATIC INLINE void RSI_RNG_Stop(HWRNG_Type *pRNG)
{
#if defined(RNG_ROMDRIVER_PRESENT)
  ROMAPI_RNG_API->rng_stop(pRNG);
#else
  rng_stop(pRNG);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_RNG_GetBytes(HWRNG_Type *pRNG, uint32_t *randomBytes, uint32_t numberOfBytes)
 * @brief        This API is used to get the random number bytes
 * @param[in]    pRNG	         : Random Number Generator handler
 * @param[in]    numberOfBytes : Number of bytes to generate
 * @param[out]   randomBytes	 : variable or array to store generated random bytes
 * @return       none
 */
STATIC INLINE void RSI_RNG_GetBytes(HWRNG_Type *pRNG, uint32_t *randomBytes, uint32_t numberOfBytes)
{
#if defined(RNG_ROMDRIVER_PRESENT)
  ROMAPI_RNG_API->rng_get_bytes(pRNG, randomBytes, numberOfBytes);
#else
  rng_get_bytes(pRNG, randomBytes, numberOfBytes);
#endif
}

#ifdef __cplusplus
}
#endif

#endif

/* @} end of RSI_RNG_DRIVERS */
