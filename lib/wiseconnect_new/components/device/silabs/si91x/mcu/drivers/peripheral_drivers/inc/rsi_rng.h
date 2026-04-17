/***************************************************************************/ /**
* @file  rsi_rng.h
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

//Include Files

#include "rsi_ccp_common.h"
#include "base_types.h"

#ifndef RSI_RNG_H
#define RSI_RNG_H

#ifdef __cplusplus
extern "C" {
#endif

#define RSI_RNG_TRUE_RANDOM   0 ///< Define for true random number generation mode.
#define RSI_RNG_PSEUDO_RANDOM 1 ///< Define for pseudo-random number generation mode.
#define RSI_RNG_LFSR_32_BIT_INPUT_VALID \
  0x10                            ///< Define for 32-bit input valid in LFSR (Linear Feedback Shift Register) mode.
#define HWRNG_CLK_ENABLE 0x400000 ///< Define to enable the clock for the hardware random number generator.

typedef enum rng_lfsr_config { RNG_LFSR_DISABLE = 0, RNG_LFSR_ENABLE = 1 } rng_lfsr_config_t;

uint32_t rng_start(HWRNG_Type *pRNG, uint8_t rng_mode);
void rng_stop(HWRNG_Type *pRNG);
void rng_get_bytes(HWRNG_Type *pRNG, uint32_t *random_bytes, uint32_t number_of_bytes);
uint32_t rng_read_lfsr_input(HWRNG_Type *pRNG, uint32_t *randomBytes, uint32_t numberOfBytes);
void rng_config_lfsr(HWRNG_Type *pRNG, rng_lfsr_config_t lfsr_config_param);
void rng_soft_reset_set(HWRNG_Type *pRNG);
void rng_soft_reset_clear(HWRNG_Type *pRNG);

#ifdef __cplusplus
}
#endif

#endif // RSI_RNG_H
