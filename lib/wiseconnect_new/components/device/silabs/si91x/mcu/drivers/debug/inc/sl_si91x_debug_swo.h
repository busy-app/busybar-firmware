/***************************************************************************/ /**
 * @file
 * @brief SI91x SWO header 
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_SI91X_DEBUG_SWO_H
#define SL_SI91X_DEBUG_SWO_H

#include "sl_status.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
****************************  DEFINES   ********************************
*******************************************************************************/

#define PAD_SEL_MCU_CLK      7  ///< Pad selection MCU Clock
#define GPIO_MCU_CLK_OUT     12 ///< GPIO for MCU Clock
#define PAD_SEL_TRACE_CLK_IN 8  ///< Pad Selection For Trace Clock In
#define GPIO_TRACE_CLK_IN    15 ///< GPIO for Trace Clock In

/*******************************************************************************
****************************  FUNCTION PROTOTYPES   ********************************
*******************************************************************************/

/**
 * @addtogroup si91x_debug Debug
 * @{
 * @addtogroup si91x_debug_swo SWO Debug
 * @brief Serial Wire Output Debug Configuration
 * @details
 * Initializes Serial Wire Output using the DWT and ITM peripherals of
 * Cortex-M processors. Configures output of Program Counter samples,
 * interrupt events, and data over any ITM channel.
 * @{
 */
/**
 * Initialize SWO Debug
 *
 * @details
 * Initializes SWO debug using configuration parameters for frequency, as well
 * as whether PC sample events and interrupt events should be emitted.
 *
 * @note
 * This function does not enable any software stimulus ports in the ITM. This
 * can be done by calling @ref sl_si91x_debug_swo_enable_itm or by using
 * @ref sl_si91x_debug_swo_write which enables the necessary ports automatically.
 *
 * @return Status code
 * @retval SL_STATUS_OK SWO initialized successfully
 */
sl_status_t sl_si91x_debug_swo_init(void);

/**
 * Enable an ITM channel
 *
 * @details
 * Enables the given ITM channel in the Trace Enable Register.
 *
 * @param[in] channel ITM channel number
 *
 * @return Status code
 * @retval SL_STATUS_OK ITM channel enabled successfully
 */
sl_status_t sl_si91x_debug_swo_enable_itm(uint32_t channel);

/**
 * Disable an ITM channel
 *
 * @details
 * Disables the given ITM channel in the Trace Enable Register.
 *
 * @param[in] channel ITM channel number
 *
 * @return Status code
 * @retval SL_STATUS_INVALID_PARAMETER Invalid channel number
 * @retval SL_STATUS_OK ITM channel disabled successfully
 */
sl_status_t sl_si91x_debug_swo_disable_itm(uint32_t channel);

/**
 * Write a byte to an ITM channel
 *
 * @details
 * Writes the given byte to the given ITM channel
 *
 * @note
 * This function ensures that the ITM channel is enabled, and enables it if
 * it wasn't already. Note that even if the debug port is locked, the byte
 * will still be written on the SWO.
 *
 * @param[in] channel ITM channel number
 * @param[in] byte Byte to send
 *
 * @return Status code
 * @retval SL_STATUS_OK Byte sent successfully
 * @retval SL_STATUS_INVALID_PARAMETER Invalid channel number
 * @retval SL_STATUS_NOT_INITIALIZED ITM has not been enabled, call
 *                                   @ref sl_si91x_debug_swo_init first.
 */
sl_status_t sl_si91x_debug_swo_write_u8(uint32_t channel, uint8_t byte);

/**
 * Write a half-word to an ITM channel
 *
 * @details
 * Writes the given half-word to the given ITM channel
 *
 * @note
 * This function ensures that the ITM channel is enabled, and enables it if
 * it wasn't already.
 *
 * @param[in] channel ITM channel number
 * @param[in] half_word Half-word to send
 *
 * @return Status code
 * @retval SL_STATUS_OK half-word sent successfully
 * @retval SL_STATUS_INVALID_PARAMETER Invalid channel number
 * @retval SL_STATUS_NOT_INITIALIZED ITM has not been enabled, call
 *                                   @ref sl_si91x_debug_swo_init first.
 */
sl_status_t sl_si91x_debug_swo_write_u16(uint32_t channel, uint16_t half_word);

/**
 * Write a word to an ITM channel
 *
 * @details
 * Writes the given word to the given ITM channel
 *
 * @note
 * This function ensures that the ITM channel is enabled, and enables it if
 * it wasn't already.
 *
 * @param[in] channel ITM channel number
 * @param[in] word Word to send
 *
 * @return Status code
 * @retval SL_STATUS_OK half-word sent successfully
 * @retval SL_STATUS_INVALID_PARAMETER Invalid channel number
 * @retval SL_STATUS_NOT_INITIALIZED ITM has not been enabled, call
 *                                   @ref sl_si91x_debug_swo_init first.
 */
sl_status_t sl_si91x_debug_swo_write_u32(uint32_t channel, uint32_t word);

/** @} end si91x_debug_swo */
/** @} end si91x_debug */

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_DEBUG_SWO_H
