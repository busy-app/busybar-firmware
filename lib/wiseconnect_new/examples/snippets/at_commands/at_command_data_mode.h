/***************************************************************************/ /**
 * @file at_command_data_mode.h
 * @brief AT Command Data Mode
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

#ifndef AT_COMMAND_DATA_MODE_H
#define AT_COMMAND_DATA_MODE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef sl_status_t (*at_data_mode_handler_t)(uint8_t *buffer, uint32_t length, void *user_data);

bool at_command_data_mode_is_enable(void);

bool at_command_data_mode_process(void);

void at_command_exit_data_mode(void);

sl_status_t at_command_goto_data_mode(at_data_mode_handler_t handler, uint32_t max_data, void *user_data);

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif // AT_COMMAND_DATA_MODE_H