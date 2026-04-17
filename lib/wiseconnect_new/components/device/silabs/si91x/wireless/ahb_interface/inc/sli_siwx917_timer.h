/***************************************************************************/ /**
 * @file  sli_siwx917_timer.h
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

/******************************************************
 * *                      Macros
 * ******************************************************/
#define sl_si91x_timer_NODE_0 0

/******************************************************
 * *                 Type Definitions
 * ******************************************************/

typedef struct {
  uint32_t start_time;
  uint32_t timeout;
} sli_si91x_timer_t;

/******************************************************
 * *               Function Declarations
 * ******************************************************/
void sli_si91x_timer_expiry_interrupt_handler(void);
uint32_t sli_si91x_timer_read_counter(void);
void sli_si91x_timer_init(sli_si91x_timer_t *timer, uint32_t duration);
int32_t sli_si91x_timer_expired(const sli_si91x_timer_t *timer);
uint32_t sli_si91x_timer_left(const sli_si91x_timer_t *timer);

uint32_t rsi_hal_gettickcount(void);
