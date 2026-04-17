/***************************************************************************/ /**
 * @file sl_si91x_ssi_primary_common_config.h
 * @brief SL SI91X SSI PRIMARY Common Config.
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

#ifndef SL_SI91X_SSI_PRIMARY_COMMON_CONFIG_H
#define SL_SI91X_SSI_PRIMARY_COMMON_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>
// <e> SSI Primary(Master) DMA Configuration

// <i> Default: 0
#define SL_SSI_PRIMARY_DMA_CONFIG_ENABLE 0

// <o SL_SSI_PRIMARY_TX_FIFO_THRESHOLD> TX FIFO Threshold <0-15>
// <i> Default: 0
#define SL_SSI_PRIMARY_TX_FIFO_THRESHOLD 0

// <o SL_SSI_PRIMARY_RX_FIFO_THRESHOLD> RX FIFO Threshold <0-15>
// <i> Default: 0
#define SL_SSI_PRIMARY_RX_FIFO_THRESHOLD 0
// </e>
// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_SSI_PRIMARY_COMMON_CONFIG_H
