/********************************************************************************
 * @file  x_compat.c
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
#include "x_compat.h"

uint32_t x_compat_reverse32(uint32_t x)
{
  x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
  x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
  x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
  x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
  x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);
  return x;
}

uint32_t x_compat_reverse32_bytes(uint32_t x)
{
  uint32_t r;
  r = (x >> 24) & 0x000000ff;
  r |= (x >> 8) & 0x0000ff00;
  r |= (x << 8) & 0x00ff0000;
  r |= (x << 24) & 0xff000000;

  return r;
}

//Weak helper functions to hook to external test libraries
WEAK void x_compat_int_disable(uint8_t *state, const char *file, int line)
{
  (void)state;
  (void)file;
  (void)line;
}

WEAK void x_compat_int_enable(uint8_t *state, const char *file, int line)
{
  (void)state;
  (void)file;
  (void)line;
}

WEAK bool x_compat_int_disabled()
{
  return false;
}

WEAK bool x_compat_int_in_irq()
{
  return false;
}
