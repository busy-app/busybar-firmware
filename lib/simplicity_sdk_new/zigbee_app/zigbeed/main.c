/***************************************************************************//**
 * @file main.c
 * @brief main() function.
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
#include "sl_component_catalog.h"
#include "sl_main_init.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_main_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_main_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#include PLATFORM_HEADER
#include "sl_zigbee.h"

#if defined(SL_ZIGBEE_TEST) || defined(ZIGBEE_STACK_ON_HOST)
#include "serial/serial.h"
#endif

#ifdef UNIX_HOST
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
extern void app_process_args(int argc, char *argv[]);
#ifdef __linux__
#include <sys/prctl.h>
#include <stdio.h>
#endif

jmp_buf gResetJump;

void __gcov_flush();
#endif //UNIX_HOST

// From platform/base/phy/simulation/security.h:
extern uint8_t simulatorEncryptionType;

void app_init(void)
{
#if defined(SL_ZIGBEE_TEST) || defined(ZIGBEE_STACK_ON_HOST)
  // In simulation we still rely on the ember serial driver.
  assert(sli_legacy_serial_init((uint8_t)APP_SERIAL,
                                (SerialBaudRate)APP_BAUD_RATE,
                                (SerialParity)PARITY_NONE,
                                1) == SL_STATUS_OK);
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_init();
#endif

#if defined(UNIX_HOST)
  simulatorEncryptionType = 0;   // USE_REAL_ENCRYPTION
#endif
}

#ifdef UNIX_HOST
int main(int argc, char *argv[])
#elif SL_ZIGBEE_TEST
int nodeMain(void)
#else
int main(void)
#endif
{
#ifdef UNIX_HOST
#if defined(SL_CATALOG_ZIGBEE_MSLA_CHECK_PRESENT)
  if ( access("/accept_silabs_msla", F_OK) != 0 ) {
    fprintf(stderr, "In order to run this software, you must accept the Silicon Labs MSLA found at https://www.silabs.com/about-us/legal/master-software-license-agreement by creating a file called /accept_silabs_msla.\n");
    return -1;
  }
#endif
  fprintf(stdout, "By using this software, you are agreeing to the Silicon Labs MSLA found at https://www.silabs.com/about-us/legal/master-software-license-agreement.\n");

  app_process_args(argc, argv);

  if (setjmp(gResetJump)) {
    fprintf(stderr, "Restarting\n");
    execvp(argv[0], argv);
  }
#endif //UNIX_HOST
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, the start task will be started and software component
  // configuration will take place there.
  sl_main_init();

#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Start the kernel. The start task will be executed (Highest priority) to complete
  // the Simplicity SDK components initialization and the user app_init() hook function will be called.
  sl_main_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT

  // User provided code.
  app_init();

  while (1) {
    // Silicon Labs components process action routine
    // must be called from the super loop.
    sl_main_process_action();

    // User provided code. Application process.
    app_process_action();

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#endif
  }
#endif // SL_CATALOG_KERNEL_PRESENT
}
