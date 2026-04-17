/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <setjmp.h>
#include "stack/include/sl_zigbee.h"
#include "em2xx-reset-defs.h"

extern jmp_buf gResetJump;

//This hasn't been defined elsewhere in MP container's zigbeed build
//since moving to package manager.
#ifndef UNUSED_VAR

/**
 * @description Useful macro for avoiding compiler warnings related to unused
 * function arguments or unused variables.
 */
#define UNUSED_VAR(x) (void)(x)
#endif

// platform/base/hal/micro/unix/simulation/micro.c
void halStackProcessBootCount(void)
{
  // Note:  We need to add the increment call in order to test the lighting
  // sample applications.

#if defined(COMMON_TOKEN_STACK_BOOT_COUNTER)
#ifndef SL_ZIGBEE_SCRIPTED_TEST
  (void)sl_token_manager_increment_counter(COMMON_TOKEN_STACK_BOOT_COUNTER);
#endif
#endif
}

// Referenced from processEzspCommand in command-handlers.c
uint16_t halGetStandaloneBootloaderVersion(void)
{
  return 0xFFFF;
}

// Referenced from checkBootloaderLaunch in ncp-common.c
sl_status_t halLaunchStandaloneBootloader(uint8_t mode)
{
  UNUSED_VAR(mode);
  return SL_STATUS_FAIL;
}

// Removed platform/base/hal/micro/unix/host/micro.c so placing stubs here
void halInternalResetWatchDog(void)
{
}

uint8_t halGetResetInfo(void)
{
  return EM2XX_RESET_SOFTWARE;
  //return EM2XX_RESET_POWERON;
}

void halReboot(void)
{
  longjmp(gResetJump, 1);
  assert(false); //Should never reach here
}

// Referenced from sli_zigbee_stack_stop_scan in zigbee-scan.c
// Stub in platform/radio/rail_lib/chip/simulation/simulation_stub.c
void halStackSymbolDelayAIsr(void)
{
}

// Referenced from sli_zigbee_network_check_incoming_queue in network.c
// Stub in platform/base/hal/micro/generic/led-stub.c
void halStackIndicateActivity(bool turnOn)
{
  UNUSED_VAR(turnOn);
}

void halResetWatchdog(void)
{
  //stub
}

void halInternalAssertFailed(const char * filename, int linenumber)
{
  //stub
  UNUSED_VAR(filename);
  UNUSED_VAR(linenumber);
}
