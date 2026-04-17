/***************************************************************************//**
 * @file
 * @brief General Purpose Cyclic Redundancy Check (GPCRC) API.
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

#ifndef SL_HAL_GPCRC_H
#define SL_HAL_GPCRC_H

#include "em_device.h"
#if defined(GPCRC_PRESENT) && (GPCRC_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sl_assert.h"

/***************************************************************************//**
 * @addtogroup gpcrc GPCRC - General Purpose CRC
 * @brief General Purpose Cyclic Redundancy Check (GPCRC) API
 *
 * @details
 * The GPCRC API functions provide full support for the GPCRC peripheral.
 *
 * The GPCRC module is a peripheral that implements a Cyclic Redundancy Check
 * (CRC) function. It supports a fixed 32-bit polynomial and a user
 * configurable 16-bit polynomial. The fixed 32-bit polynomial is the commonly
 * used IEEE 802.3 polynomial 0x04C11DB7.
 *
 * When using a 16-bit polynomial it is up to the user to choose a polynomial
 * that fits the application. Commonly used 16-bit polynomials are 0x1021
 * (CCITT-16), 0x3D65 (IEC16-MBus), and 0x8005 (ZigBee, 802.15.4, and USB).
 * See this link for other polynomials:
 * https://en.wikipedia.org/wiki/Cyclic_redundancy_check
 *
 * Before a CRC calculation can begin, call the
 * @ref sl_hal_gpcrc_start function. This function will reset CRC calculation
 * by copying the configured initialization value over to the CRC data register.
 *
 * There are two ways of sending input data to the GPCRC. Either write
 * the input data into the input data register using input functions
 * @ref sl_hal_gpcrc_write_input_32bit, @ref sl_hal_gpcrc_write_input_16bit
 * and @ref sl_hal_gpcrc_write_input_8bit, or the user can configure @ref ldma
 * to transfer data directly to one of the GPCRC
 * input data registers.
 *
 *@{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/
/// The fixed 32-bit polynomial commonly used by IEEE 802.3.
#define SL_HAL_GPCRC_COMMON_32_BIT_POLY 0x04C11DB7UL
/// Check if GPCRC instance is valid.
#define SL_HAL_GPCRC_REF_VALID(ref)     (GPCRC_NUM(ref) != -1)

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// CRC initialization structure.
typedef struct {
  /**
   * Reverse byte order. This has an effect when sending a 32-bit word or
   * 16-bit half word input to the CRC calculation. When set to true, the input
   * bytes are reversed before entering the CRC calculation. When set to
   * false, the input bytes stay in the same order.
   */
  bool                       reverse_byte_order;

  /**
   * Reverse bits within each input byte. This setting enables or disables byte
   * level bit reversal. When byte-level bit reversal is enabled, then each byte
   * of input data will be reversed before entering CRC calculation.
   */
  bool                       reverse_bits;

  /**
   * Enable/disable byte mode. When byte mode is enabled, then all input
   * is treated as single byte input even though the input is a 32-bit word
   * or a 16-bit half word. Only the least significant byte of the data-word
   * will be used for CRC calculation for all writes.
   */
  bool                       enable_byte_mode;

  /**
   * Enable automatic initialization by re-seeding the CRC result based on
   * the init value after reading one of the CRC data registers.
   */
  bool                       auto_init;

  /**
   * CRC polynomial value. GPCRC supports either a fixed 32-bit polynomial
   * or a user-configurable 16 bit polynomial. The fixed 32-bit polynomial
   * is the one used in IEEE 802.3, which has the value 0x04C11DB7. To use the
   * 32-bit fixed polynomial, assign 0x04C11DB7 to the crc_poly field.
   * To use a 16-bit polynomial, assign a value to crc_poly where the upper 16
   * bits are zero.
   *
   * The polynomial should be written in normal bit order. For instance,
   * to use the CRC-16 polynomial X^16 + X^15 + X^2 + 1, first convert
   * it to hex representation and remove the highest order term
   * of the polynomial. This will give 0x8005 as the value to write into
   * crcPoly.
   */
  uint32_t                   crc_poly;

  /**
   * CRC initialization value. This value is assigned to the GPCRC_INIT register.
   * The initValue is loaded into the data register when calling the
   * @ref sl_hal_gpcrc_start function or when one of the data registers are read
   * while @ref auto_init is enabled.
   */
  uint32_t                   init_value;
} sl_hal_gpcrc_init_t;

/// Default configuration for sl_hal_gpcrc_init_t structure.
#define SL_HAL_GPCRC_INIT_DEFAULT                                                         \
  {                                                                                       \
    false,                           /* Byte order is normal. */                          \
    false,                           /* Bit order is not reversed on output. */           \
    false,                           /* Disable byte mode. */                             \
    false,                           /* Disable automatic initialization on data read. */ \
    SL_HAL_GPCRC_COMMON_32_BIT_POLY, /* CRC32 Polynomial value. */                        \
    0x00000000UL,                    /* Initialization value. */                          \
  }

/*******************************************************************************
 ******************************   PROTOTYPES   *********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize the General Purpose Cyclic Redundancy Check (GPCRC) module.
 *
 * @details
 *   Use this function to configure the operational parameters of the GPCRC,
 *   such as the polynomial to use and how the input should be preprocessed
 *   before entering the CRC calculation.
 *
 * @note
 *   This function will not copy the initialization value to the data register
 *   to prepare for a new CRC calculation. Either call
 *   @ref sl_hal_gpcrc_start before each calculation or by use the
 *   autoInit functionality.
 *
 * @param[in] gpcrc
 *   A pointer to the GPCRC peripheral register block.
 *
 * @param[in] init
 *   A pointer to the initialization structure used
 *   to configure the GPCRC.
 ******************************************************************************/
void sl_hal_gpcrc_init(GPCRC_TypeDef *gpcrc,
                       const sl_hal_gpcrc_init_t *init);

/***************************************************************************//**
 * @brief
 *   Reset GPCRC registers to the hardware reset state.
 *
 * @note
 *   The data registers are not reset by this function.
 *
 * @param[in] gpcrc
 *   A pointer to the GPCRC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_reset(GPCRC_TypeDef *gpcrc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  gpcrc->CTRL = _GPCRC_CTRL_RESETVALUE;
  gpcrc->POLY = _GPCRC_POLY_RESETVALUE;
  gpcrc->INIT = _GPCRC_INIT_RESETVALUE;
}

/***************************************************************************//**
 * @brief
 *   Enable GPCRC.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_enable(GPCRC_TypeDef *gpcrc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  // Enable peripheral.
  gpcrc->EN_SET = GPCRC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable GPCRC.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_disable(GPCRC_TypeDef *gpcrc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  // Disable peripheral.
  gpcrc->EN_CLR = GPCRC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Issue a command to initialize the CRC calculation.
 *
 * @details
 *   Issues the command INIT in GPCRC_CMD that initializes the
 *   CRC calculation by writing the initial values to the DATA register.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_start(GPCRC_TypeDef *gpcrc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  gpcrc->CMD_SET = GPCRC_CMD_INIT;
}

/***************************************************************************//**
 * @brief
 *   Set the initialization value of the CRC.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 *
 * @param[in] init_value
 *   Value to use to initialize a CRC calculation.
 *   This value is moved into the data register when
 *   calling @ref sl_hal_gpcrc_start.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_set_init_value(GPCRC_TypeDef *gpcrc,
                                          uint32_t init_value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  gpcrc->INIT = init_value;
}

/***************************************************************************//**
 * @brief
 *   Write a 32-bit value to the input data register of the CRC.
 *
 * @details
 *   Use this function to write a 32-bit input data to the CRC. CRC
 *   calculation is based on the provided input data using the configured
 *   CRC polynomial.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 *
 * @param[in] data
 *   Data to be written to the input data register.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_write_input_32bit(GPCRC_TypeDef *gpcrc,
                                             uint32_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  gpcrc->INPUTDATA = data;
}

/***************************************************************************//**
 * @brief
 *   Write a 16-bit value to the input data register of the CRC.
 *
 * @details
 *   Use this function to write a 16 bit input data to the CRC. CRC
 *   calculation is based on the provided input data using the configured
 *   CRC polynomial.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 *
 * @param[in] data
 *   Data to be written to the input data register.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_write_input_16bit(GPCRC_TypeDef *gpcrc,
                                             uint16_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  gpcrc->INPUTDATAHWORD = data;
}

/***************************************************************************//**
 * @brief
 *   Write an 8-bit value to the CRC input data register.
 *
 * @details
 *   Use this function to write an 8-bit input data to the CRC. CRC
 *   calculation is based on the provided input data using the configured
 *   CRC polynomial.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 *
 * @param[in] data
 *   Data to be written to the input data register.
 ******************************************************************************/
__INLINE void sl_hal_gpcrc_write_input_8bit(GPCRC_TypeDef *gpcrc,
                                            uint8_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  gpcrc->INPUTDATABYTE = data;
}

/***************************************************************************//**
 * @brief
 *   Read the CRC data register.
 *
 * @details
 *   Use this function to read the calculated CRC value.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 *
 * @return
 *   Content of the CRC data register.
 ******************************************************************************/
__INLINE uint32_t sl_hal_gpcrc_read_data(GPCRC_TypeDef *gpcrc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  return gpcrc->DATA;
}

/***************************************************************************//**
 * @brief
 *   Read the data register of the CRC bit reversed.
 *
 * @details
 *   Use this function to read the calculated CRC value bit reversed. When
 *   using a 32-bit polynomial, bits [31:0] are reversed, when using a
 *   16-bit polynomial, bits [15:0] are reversed.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 *
 * @return
 *   Content of the CRC data register bit reversed.
 ******************************************************************************/
__INLINE uint32_t sl_hal_gpcrc_read_data_bit_reversed(GPCRC_TypeDef *gpcrc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  return gpcrc->DATAREV;
}

/***************************************************************************//**
 * @brief
 *   Read the data register of the CRC byte reversed.
 *
 * @details
 *   Use this function to read the calculated CRC value byte reversed.
 *
 * @param[in] gpcrc
 *   Pointer to GPCRC peripheral register block.
 *
 * @return
 *   Content of the CRC data register byte reversed.
 ******************************************************************************/
__INLINE uint32_t sl_hal_gpcrc_read_data_byte_reversed(GPCRC_TypeDef *gpcrc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_GPCRC_REF_VALID(gpcrc));

  return gpcrc->DATABYTEREV;
}

/** @} (end addtogroup gpcrc) */

#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup gpcrc GPCRC - General Purpose CRC
 * @{
 *
 *@n @section gpcrc_example Example
 *  The following sections provide examples on how to use the GPCRC module for
 *  various CRC calculation standards.
 *
 * @code{.c}
 * // Example 1: CRC-32 calculation
 * void gpcrc_example_crc32(void)
 * {
 *   uint32_t checksum;
 *
 *   // Enable bus clock for GPCRC0
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPCRC0);
 *
 *   // Initialize GPCRC, 32-bit fixed polynomial is default
 *   sl_hal_gpcrc_init_t init = SL_HAL_GPCRC_INIT_DEFAULT;
 *   init.init_value = 0xFFFFFFFF; // Standard CRC-32 init value
 *   sl_hal_gpcrc_init(GPCRC0, &init);
 *
 *   sl_hal_gpcrc_start(GPCRC0);
 *   sl_hal_gpcrc_write_input_8bit(GPCRC0, 0xC5);
 *   // According to the CRC-32 specification, the end result should be inverted
 *   checksum = ~sl_hal_gpcrc_read_data(GPCRC0);
 *   // The checksum is now 0x390CD9B2
 * }
 *
 * // Example 2: CRC-16 calculation with polynomial 0x8005
 * void gpcrc_example_crc16(void)
 * {
 *   uint16_t checksum;
 *
 *   // Enable bus clock for GPCRC0
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPCRC0);
 *
 *   // Initialize GPCRC with the 16-bit polynomial 0x8005
 *   sl_hal_gpcrc_init_t init = SL_HAL_GPCRC_INIT_DEFAULT;
 *   init.crc_poly = 0x8005;
 *   sl_hal_gpcrc_init(GPCRC0, &init);
 *
 *   sl_hal_gpcrc_start(GPCRC0);
 *   sl_hal_gpcrc_write_input_8bit(GPCRC0, 0xAB);
 *   checksum = (uint16_t) sl_hal_gpcrc_read_data(GPCRC0);
 *   // The checksum is now 0xBF41
 * }
 *@endcode
 * @} (end addtogroup gpcrc)
 ******************************************************************************/

 #endif // defined(GPCRC_COUNT) && (GPCRC_COUNT > 0)
 #endif // SL_HAL_GPCRC_H
