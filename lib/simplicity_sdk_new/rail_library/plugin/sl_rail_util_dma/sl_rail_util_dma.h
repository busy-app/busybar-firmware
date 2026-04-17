/***************************************************************************//**
 * @file
 * @brief
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

#ifndef SL_RAIL_UTIL_DMA_H
#define SL_RAIL_UTIL_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the RAIL DMA Utility.
 *
 * @note: This function should be called during application initialization.
 *
 * @warning If TrustZone is enabled and LDMA is configured as secure
 *   peripheral, this function must be called by the secure application.
 *   Non-secure application call is unnecessary because RAIL knows it must work
 *   with secure LDMA when \ref RAIL_TZ_Config_t::radioPerformM2mLdmaCallback
 *   is provided to \ref RAIL_TZ_InitNonSecure().
 */
void sl_rail_util_dma_init(void);

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTIL_DMA_H
