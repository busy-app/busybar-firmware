/***************************************************************************//**
 * @file
 * @brief NVM3 utility functions
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "nvm3_utils.h"

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

void nvm3_utilsComputeBergerCode(uint8_t *pResult, void *pInput, uint8_t numberOfBits)
{
  uint8_t sum;
  uint32_t word = *((uint32_t *)pInput);
  uint32_t mask;

  // Clear bits that are outside the wanted bits
  if (numberOfBits < 32U) {
    mask = (1UL << numberOfBits) - 1U;
    word = word & mask;
  }

  // Count bits set:
  // From http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetNaive
  word = word - ((word >> 1) & 0x55555555U);
  word = (word & 0x33333333U) + ((word >> 2) & 0x33333333U);
  sum = (uint8_t)((((word + (word >> 4)) & 0xF0F0F0FU) * 0x1010101U) >> 24U);

  // Count bit cleared and accumulate
  *pResult = *pResult + (numberOfBits - sum);
}

/// @endcond
