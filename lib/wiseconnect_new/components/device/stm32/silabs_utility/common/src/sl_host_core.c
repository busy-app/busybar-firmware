/***************************************************************************//**
 * @file
 * @brief Core interrupt handling API
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

#include "sl_core.h"
#include <stdio.h>

/******************************************************
 *               Function Definitions
 ******************************************************/
#include "cmsis_armclang.h" // For ARM Compiler
CORE_irqState_t CORE_EnterAtomic(void)
  {
    CORE_irqState_t irqState = __get_PRIMASK();
   __disable_irq();
    return irqState;
  }

void CORE_ExitAtomic(CORE_irqState_t irqState)
  {
  if (irqState == 0) {
      __enable_irq();
     }
  }

  CORE_irqState_t CORE_EnterCritical(void)
  {
    CORE_irqState_t irqState = __get_PRIMASK();
   __disable_irq();
    return irqState;
  }

void CORE_ExitCritical(CORE_irqState_t irqState)
  {
  if (irqState == 0) {
      __enable_irq();
     }
  }

void __aeabi_assert(const char *expr, const char *file, int line) {
  printf("Assert Failed at %s:%d -> %s\n", file, line, expr);
#ifdef __CC_ARM
__asm__("bkpt #0");
#else
__asm__("bkpt");
#endif
}