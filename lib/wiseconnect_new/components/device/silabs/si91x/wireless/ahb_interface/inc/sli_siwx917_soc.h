/***************************************************************************/ /**
 * @file  sli_siwx917_soc.h
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

#pragma once

#include <stdint.h>

// Upgrade images
#define RSI_JUMP_TO_PC 'J'

//! Check CRC
#define CHECK_NWP_INTEGRITY 'K'
#define RSI_UPGRADE_BL      '#'

#define HOST_INTF_REG_OUT 0x4105003C
#define HOST_INTF_REG_IN  0x41050034
#define BOARD_READY       0xABCD
#define REG_READ          0xD1
#define REG_WRITE         0xD2
#define PONG_WRITE        0xD4
#define PING_WRITE        0xD5
#define GPIO_RESET        0xD7
#define LOAD_BOOTLOADER   0xD8
#ifdef RS9116
#define HOST_INTERACT_REG_VALID      (0xA0 << 8)
#define HOST_INTERACT_REG_VALID_READ (0xAB << 8)
#else
#define HOST_INTERACT_REG_VALID      (0xAB << 8)
#define HOST_INTERACT_REG_VALID_READ (0xAB << 8)
#endif

#define RSI_RESET_LOOP_COUNTER(X)    X = 0
#define RSI_WHILE_LOOP(X, Y)         while ((X++) < (uint32_t)Y)
#define RSI_LOOP_COUNT_UPGRADE_IMAGE 0xFFFF
#define RSI_LOOP_COUNT_WAKEUP_REQ    0xFFFFFFFF
#define RSI_LOOP_COUNT_WAKEUP_WAIT   0xFFFFFFFF

// It takes more time than usual to set the default firmware image
#define RSI_LOOP_COUNT_SET_IMAGE_WAIT 0xFFFFFFF
#define RSI_LOOP_COUNT_UPGRADE_REQ    0xFFFF
#define RSI_LOOP_COUNT_UPGRADE_CHUNK  0xFFFF
#define RSI_LOOP_COUNT_UPGRADE_STATUS 0xFFFF
#define RSI_LOOP_COUNT_SELECT_OPTION  0xFFFF
#define RSI_CHECK_LOOP_COUNTER(X, Y) \
  do {                               \
    if (X >= Y)                      \
      return -1;                     \
  } while (0)

void sli_siwx917_update_system_core_clock(void);
void RSI_Set_Cntrls_To_M4(void);
void RSI_Set_Cntrls_To_TA(void);

void sli_si91x_platform_init(void);
int16_t sli_si91x_send_boot_instruction(uint8_t type, uint16_t *data);
int16_t rsi_waitfor_boardready(void);
int16_t rsi_select_option(uint8_t cmd, uint8_t fw_image_number);
int16_t rsi_bl_select_option(uint8_t cmd);
int16_t rsi_boot_insn(uint8_t type, uint16_t *data);
int16_t rsi_mem_rd(uint32_t addr, uint16_t len, uint8_t *dBuf);
void sli_si91x_ulp_wakeup_init(void);
int16_t sli_si91x_set_default_nwp_fw(const uint8_t fw_image_number);
