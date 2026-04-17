/***************************************************************************//**
 * @file
 * @brief simulation files for the system timer part of the HAL
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
// _POSIX_C_SOURCE >= 199309L for clock_gettime with CLOCK_MONOTONIC clockId
// Link with -lrt (only for glibc versions before 2.17).
#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <assert.h>

bool halUseRealtime = false; // needed in ncp-common

uint32_t halCommonGetInt32uMillisecondTick(void)
{
  // Get monotonic time and derive the milliseconds tick.
  struct timespec ts;
  // Assert the call for success(0)
  assert(0 == clock_gettime(CLOCK_MONOTONIC, &ts));
  uint32_t now = (uint32_t)((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
  return now;
}

uint16_t halCommonGetInt16uMillisecondTick(void)
{
  return (uint16_t)halCommonGetInt32uMillisecondTick();
}

uint16_t halCommonGetInt16uQuarterSecondTick(void)
{
  return (uint16_t)(halCommonGetInt32uMillisecondTick() >> 8);
}
