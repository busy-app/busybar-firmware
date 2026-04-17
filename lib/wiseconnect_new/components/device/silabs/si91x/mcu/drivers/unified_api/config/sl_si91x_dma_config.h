/***************************************************************************/ /**
 * @file
 * @brief SL USART Config.
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

#ifndef SL_SI91X_DMA_CONFIG_H
#define SL_SI91X_DMA_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif

// <o SL_DMA0_CHANNEL_COUNT> DMA instance 0 channel count <1-32>
// <i> Default: 32
#define SL_DMA0_CHANNEL_COUNT 32

// <o SL_ULP_DMA_CHANNEL_COUNT> ULP DMA instance channel count <1-12>
// <i> Default: 12
#define SL_ULP_DMA_CHANNEL_COUNT 12

// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif
#endif //SL_SI91X_DMA_CONFIG_H
