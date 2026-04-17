/***************************************************************************//**
 * @file
 * @brief General Purpose Cyclic Redundancy Check (GPCRC) API.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_hal_gpcrc.h"
#if defined(GPCRC_PRESENT) && (GPCRC_COUNT > 0)

#include "sl_common.h"

/*******************************************************************************
 ***************************   GLOBAL FUNCTIONS   ******************************
 ******************************************************************************/

extern __INLINE void sl_hal_gpcrc_reset(GPCRC_TypeDef *gpcrc);
extern __INLINE void sl_hal_gpcrc_enable(GPCRC_TypeDef *gpcrc);
extern __INLINE void sl_hal_gpcrc_disable(GPCRC_TypeDef *gpcrc);
extern __INLINE void sl_hal_gpcrc_start(GPCRC_TypeDef *gpcrc);
extern __INLINE void sl_hal_gpcrc_set_init_value(GPCRC_TypeDef *gpcrc,
                                                 uint32_t init_value);
extern __INLINE void sl_hal_gpcrc_write_input_32bit(GPCRC_TypeDef *gpcrc,
                                                    uint32_t data);
extern __INLINE void sl_hal_gpcrc_write_input_16bit(GPCRC_TypeDef *gpcrc,
                                                    uint16_t data);
extern __INLINE void sl_hal_gpcrc_write_input_8bit(GPCRC_TypeDef *gpcrc,
                                                   uint8_t data);
extern __INLINE uint32_t sl_hal_gpcrc_read_data(GPCRC_TypeDef *gpcrc);
extern __INLINE uint32_t sl_hal_gpcrc_read_data_bit_reversed(GPCRC_TypeDef *gpcrc);
extern __INLINE uint32_t sl_hal_gpcrc_read_data_byte_reversed(GPCRC_TypeDef *gpcrc);

/***************************************************************************//**
 * Initialize the General Purpose Cyclic Redundancy Check (GPCRC) module.
 ******************************************************************************/
void sl_hal_gpcrc_init(GPCRC_TypeDef *gpcrc,
                       const sl_hal_gpcrc_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));
  // Init structure must be provided.
  EFM_ASSERT(init);

  uint32_t poly_select;
  uint32_t rev_poly = 0;

  if (init->crc_poly == SL_HAL_GPCRC_COMMON_32_BIT_POLY) {
    poly_select = GPCRC_CTRL_POLYSEL_CRC32;
  } else {
    // If not using the fixed CRC-32 polynomial, use 16-bit.
    EFM_ASSERT((init->crc_poly & 0xFFFF0000UL) == 0U);

    poly_select = GPCRC_CTRL_POLYSEL_CRC16;
    rev_poly = (uint32_t)SL_RBIT16((uint16_t)init->crc_poly);
  }

  gpcrc->CTRL = (((uint32_t)init->auto_init << _GPCRC_CTRL_AUTOINIT_SHIFT)
                 | ((uint32_t)init->reverse_byte_order << _GPCRC_CTRL_BYTEREVERSE_SHIFT)
                 | ((uint32_t)init->reverse_bits << _GPCRC_CTRL_BITREVERSE_SHIFT)
                 | ((uint32_t)init->enable_byte_mode << _GPCRC_CTRL_BYTEMODE_SHIFT)
                 | poly_select);

  if (poly_select == GPCRC_CTRL_POLYSEL_CRC16) {
    // Set the CRC polynomial value.
    gpcrc->POLY = rev_poly & _GPCRC_POLY_POLY_MASK;
  }

  // Load the CRC initialization value to GPCRC_INIT.
  gpcrc->INIT = init->init_value;
}

#endif // defined(GPCRC_COUNT) && (GPCRC_COUNT > 0)
