/***************************************************************************/ /**
 * @file  sl_si91x_ssi_common_config.h
 * @brief SL SI91X Common Config.
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

#ifndef SL_COMMON_CONFIG_H
#define SL_COMMON_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>
// <h>DMA Configuration

// <q SL_SSI_PRIMARY_DMA_CONFIG_ENABLE> Primary DMA
// <i> Default: 0
#define SL_SSI_PRIMARY_DMA_CONFIG_ENABLE 0

// <q SL_SSI_SECONDARY_DMA_CONFIG_ENABLE> Secondary DMA
// <i> Default: 0
#define SL_SSI_SECONDARY_DMA_CONFIG_ENABLE 0

// <q SL_SSI_ULP_PRIMARY_DMA_CONFIG_ENABLE> ULP Primary DMA
// <i> Default: 0
#define SL_SSI_ULP_PRIMARY_DMA_CONFIG_ENABLE 0

// <o SL_SSI_TX_FIFO_THRESHOLD> TX FIFO Threshold <0-15>
// <i> Default: 0
#define SL_SSI_TX_FIFO_THRESHOLD 0

// <o SL_SSI_RX_FIFO_THRESHOLD> RX FIFO Threshold <0-15>
// <i> Default: 0
#define SL_SSI_RX_FIFO_THRESHOLD 0

// </h>
// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif
