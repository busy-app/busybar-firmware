/***************************************************************************/ /**
 * @file sl_si91x_power_manager_board_config.h
 * @brief Power Manager Board Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_POWER_MANAGER_BOARD_CONFIG_H
#define SL_POWER_MANAGER_BOARD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SLI_SI917)

#if defined(SLI_SI91X_MCU_COMMON_FLASH_MODE)

#define SL_SLEEP_STACK_USAGE_ADDRESS          0x1b000    // STACK_USAGE_ADDRESS for Si917 B0 Common flash
#define SL_SLEEP_RAM_USAGE_ADDRESS            0x24061EFC // RAM_USAGE_ADDRESS for Si917 B0 Common flash
#define SL_SLEEP_VECTOR_OFFSET                0x0 // Vector offset: sleep without retention for Si917 B0 Common flash
#define SL_SLEEP_VECTOR_OFFSET_WITH_RETENTION 0x8202000 // Vector offset: sleep with retention for Si917 B0 Common flash
#define SL_SLEEP_WAKEUP_CALLBACK_ADDRESS      0x1e000   // Wakeup callback address for Si917 B0 Common flash

#else

#define SL_SLEEP_STACK_USAGE_ADDRESS          0x1b000    // STACK_USAGE_ADDRESS for Si917 B0 Dual flash
#define SL_SLEEP_RAM_USAGE_ADDRESS            0x24061EFC // RAM_USAGE_ADDRESS for Si917 B0 Dual flash
#define SL_SLEEP_VECTOR_OFFSET                0x0 // Vector offset: sleep without retention for Si917 B0 Dual flash
#define SL_SLEEP_VECTOR_OFFSET_WITH_RETENTION 0x8012000 // Vector offset: sleep with retention for Si917 B0 Dual flash
#define SL_SLEEP_WAKEUP_CALLBACK_ADDRESS      0x1e000   // Wakeup callback address for Si917 B0 Dual flash

#endif // SLI_SI91X_MCU_COMMON_FLASH_MODE

#else

#if defined(SLI_SI91X_MCU_COMMON_FLASH_MODE)

#define SL_SLEEP_STACK_USAGE_ADDRESS          0x1b000    // STACK_USAGE_ADDRESS for Si917 A0 Common flash
#define SL_SLEEP_RAM_USAGE_ADDRESS            0x24061000 // RAM_USAGE_ADDRESS for Si917 A0 Common flash
#define SL_SLEEP_VECTOR_OFFSET                0x0 // Vector offset: sleep without retention for Si917 A0 Common flash
#define SL_SLEEP_VECTOR_OFFSET_WITH_RETENTION 0x8212000 // Vector offset: sleep with retention for Si917 A0 Common flash
#define SL_SLEEP_WAKEUP_CALLBACK_ADDRESS      0x1e000   // Wakeup callback address for Si917 A0 Common flash

#else

#define SL_SLEEP_STACK_USAGE_ADDRESS          0x1b000    // STACK_USAGE_ADDRESS for Si917 A0 Dual flash
#define SL_SLEEP_RAM_USAGE_ADDRESS            0x24061000 // RAM_USAGE_ADDRESS for Si917 A0 Dual flash
#define SL_SLEEP_VECTOR_OFFSET                0x0 // Vector offset: sleep without retention for Si917 A0 Dual flash
#define SL_SLEEP_VECTOR_OFFSET_WITH_RETENTION 0x8012000 // Vector offset: sleep with retention for Si917 A0 Dual flash
#define SL_SLEEP_WAKEUP_CALLBACK_ADDRESS      0x1e000   // Wakeup callback address for Si917 A0 Dual flash

#endif // SLI_SI91X_MCU_COMMON_FLASH_MODE

#endif // SLI_SI917

#ifdef __cplusplus
}
#endif

#endif /* SL_POWER_MANAGER_BOARD_CONFIG_H */