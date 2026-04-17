/***************************************************************************/ /**
 * @file sl_si91x_hal_soc_soft_reset.h
 * @brief SoC soft reset API
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

#ifndef __SL_SI91X_HAL_SOC_SOFT_RESET__
#define __SL_SI91X_HAL_SOC_SOFT_RESET__

#include "rsi_qspi.h"

/** @cond DOXYGEN_IGNORE */
#define M4_QSPI_AES_CONFIG  *(volatile uint32 *)0x120000C8
#define AES_QSPI_KEY_SIZE   BIT(16)
#define AES_QSPI_KEY_LENGTH BIT(11)
/** @endcond */

/** 
 * \addtogroup SOFT_RESET_FUNCTIONS Soft Reset
 * \ingroup COMMON_SECTION_FUNCTIONS
 * @{ */

/**
 * @brief        To perform a Nested Vectored Interrupt Controller (NVIC) soft reset on the Si91X SoC.
 * 
 * @details      This function initiates a system reset request to reset the SoC.
 *               It resets the M4 core and the Network Processor (NWP) of the Si91X SoC, bringing the system back to its initial state.
 * 
 * @note         Ensure that all necessary data is saved before calling this function because it will reset the NVIC and potentially disrupt ongoing processes.
 */
void sl_si91x_soc_nvic_reset(void);

/** @} */

#endif
