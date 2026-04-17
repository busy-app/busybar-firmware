/********************************************************************************
 * @file  x_bit.h
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef COMMON_INCLUDE_X_BIT_H_
#define COMMON_INCLUDE_X_BIT_H_
#include <stdint.h>
#include <stdbool.h>
#include "x_compat.h"

typedef uint32_t x_bitmap_t;
typedef uint8_t x_bit_t;

/**
 * Maximum bit which can be used in bitmap
 */
#define X_BIT_MAX (sizeof(x_bitmap_t) * 8 - 1)

/**
 * Invalid bit index
 */
#define X_BIT_INVALID (X_BIT_MAX + 1)

/**
 * Convert bit index to bit mask
 * @param bit bit to convert
 * @return bitmask
 */
#define BIT(bit) (1U << (bit))

/**
 * Check if any bit is set in bitmap
 *
 * @param bitmap bitmap to check against
 * @param bits bitmap of bits to compare
 * @return x_bitmap_t bitmap of matching bits
 */
static inline x_bitmap_t x_bitmap_isset(x_bitmap_t bitmap, x_bitmap_t bits)
{
  return bitmap & bits;
}

/**
 * Check if bit is set in bitmask
 * @param bitmask bitmask to check
 * @param bit bit index to check
 * @return nonzero if bit is set
 */
static inline x_bitmap_t x_bit_isset(x_bitmap_t bitmask, x_bit_t bit)
{
  return bitmask & BIT(bit);
}

/**
 * Check bitmap state
 *
 * @param bitmask bitmap to check against
 * @param set these bits must be 1 in bitmap
 * @param clr these bits must be 0 in bitmap, overlapping bits with set are ignored
 *
 * @return true if only bits in set-variable are 1
 */
static inline bool x_bit_check_mask(x_bitmap_t bitmap, x_bitmap_t set, x_bitmap_t clr)
{
  return (bitmap & (set | clr)) == set;
}

/**
 * Set bit in bitmask
 * @param bitmask bitmask to modify
 * @param bit bit index to set
 */
static inline void x_bit_set(x_bitmap_t *bitmask, x_bit_t bit)
{
  //cstat !ATH-shift-bounds
  *bitmask |= BIT(bit);
}

/**
 * Clear bit in bitmask
 * @param bitmask bitmask to modify
 * @param bitnum bit index to clear
 */
static inline void x_bit_clr(x_bitmap_t *bitmask, x_bit_t bit)
{
  *bitmask &= ~BIT(bit);
}

/**
 * Find lowest bit not set in bitmask
 * @param bitmap
 * @return index, or X_BIT_INVALID if not found
 */
static inline x_bit_t x_bit_lowest_not_set(x_bitmap_t bitmap)
{
  return X_COUNT_LEADING_ZEROS(X_INVERT_WORD(~bitmap));
}

/**
 * Find lowest bit set in bitmask
 * @param bitmap
 * @return index, or X_BIT_INVALID if not found
 */
static inline x_bit_t x_bit_lowest_set(x_bitmap_t bitmap)
{
  return X_COUNT_LEADING_ZEROS(X_INVERT_WORD(bitmap));
}

/**
 * Set bit in bitmask atomically
 * @param bitmask bitmask to modify
 * @param bit bit index to set
 */
static inline void x_bit_set_atomic(x_bitmap_t *bitmask, x_bit_t bit)
{
  X_ATOMIC_BIT_SET(bitmask, bit);
}

/**
 * Clear bit in bitmask atomically
 * @param bitmask bitmask to modify
 * @param bitnum bit index to clear
 */
static inline void x_bit_clr_atomic(x_bitmap_t *bitmask, x_bit_t bit)
{
  X_ATOMIC_BIT_CLR(bitmask, bit);
}

/**
 * Set multiple bits at same time using exclusive access
 * @param bitmap bitmap to set
 * @param bits to set
 */
static inline void x_bit_set_exclusive(x_bitmap_t *bitmap, x_bitmap_t bits)
{
  X_BIT_SET_EXCLUSIVE((uint32_t *)bitmap, (uint32_t)bits);
}

/**
 * Clear multiple bits at same time using exclusive access
 * @param bitmap bitmap to clear
 * @param bits to clear
 */
static inline void x_bit_clr_exclusive(x_bitmap_t *bitmap, x_bitmap_t bits)
{
  X_BIT_CLR_EXCLUSIVE((uint32_t *)bitmap, (uint32_t)bits);
}

/**
 * Swap bitmap and return old using exclusive access
 * @param bitmap bitmap to exchange
 * @param bits new bitmap to set to
 * @return old bitmap
 */
static inline x_bitmap_t x_bit_swap_exclusive(x_bitmap_t *bitmap, x_bitmap_t bits)
{
  X_SWAP_EXCLUSIVE((uint32_t *)bitmap, (uint32_t)bits);
}

/**
 * Calculate the number of set bits from given bitmap
 * @param examine bitmap
 * @return number of set bits
 */
unsigned int x_bit_popcount(x_bitmap_t bitmap);

#endif /* COMMON_INCLUDE_X_BIT_H_ */
