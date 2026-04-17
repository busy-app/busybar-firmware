/***************************************************************************//**
 * @file  sl_system_compatibility.c
 * @brief  Retarget the sl_system function calls to sl_main.
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
#include "sl_main_process_action.h"
#include "sl_system_init.h"
#include "sl_system_process_action.h"

#if defined(__GNUC__)
// Wrapping a system function with GCC works by using the linker option '--wrap=symbol'.
// Any undefined reference to "symbol" will be resolved to "__wrap_symbol".
// Any undefined reference to "__real_symbol" will be resolved to "symbol".
#define STD_LIB_WRAPPER_SL_SYSTEM_INIT  __wrap_sl_system_init
#define STD_LIB_WRAPPER_SL_SYSTEM_PROCESS_ACTION  __wrap_sl_system_process_action

// The GNU gold linker has an issue with LTO and wrapping, where the symbol is
// stripped even if it is used in the source code, which leads to link-time errors.
// https://sourceware.org/bugzilla/show_bug.cgi?id=24415
// By marking the wrapper as externally_visible, the symbol will not be stripped
// from the final binary, regardless if it is referenced or not in the source code.
#define ATTR_EXT_VIS    __attribute__((externally_visible))

/*******************************************************************************
 ****************   PREVIOUS IMPLEMENTATION OF FUNCTIONS   *********************
 ******************************************************************************/
extern void sl_system_init();
extern void sl_system_process_action();

#elif defined(__IAR_SYSTEMS_ICC__)
// Wrapping a system function works with IAR by patching symbol definitions using $Super$$ and $Sub$$
// The $Super$$ special pattern identifies the original unpatched function used for calling
// the original function directly.
// The $Sub$$ special pattern identifies the new function that is called instead of the
// original function.

#define STD_LIB_WRAPPER_SL_SYSTEM_INIT  $Sub$$sl_system_init
#define STD_LIB_WRAPPER_SL_SYSTEM_PROCESS_ACTION  $Sub$$sl_system_process_action

// Since IAR does not use LTO, resolve the attribute as nothing.
#define ATTR_EXT_VIS

/*******************************************************************************
 ****************   PREVIOUS IMPLEMENTATION OF FUNCTIONS   *********************
 ******************************************************************************/
void sl_system_init()
{
  // Empty function to be overridden by the wrapper.
}

void sl_system_process_action()
{
  // Empty function to be overridden by the wrapper.
}

#else
#error Unsupported compiler for standard C memory functions retarget
#endif  // __GNUC__

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief sl_system_init() wrapper.
 *
 * @details Execute sl_main initialization
 ******************************************************************************/
ATTR_EXT_VIS void STD_LIB_WRAPPER_SL_SYSTEM_INIT()
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, the start task will be started and software component
  // configuration will take place there.
  sl_main_init();
}

/***************************************************************************//**
 * @brief sl_system_process_action() wrapper.
 *
 * @details Execute sl_main process action
 *
 * @note Original main function will be called by the start task
 ******************************************************************************/
ATTR_EXT_VIS void STD_LIB_WRAPPER_SL_SYSTEM_PROCESS_ACTION()
{
  // Silicon Labs components process action routine
  // must be called from the super loop.
  sl_main_process_action();
}
