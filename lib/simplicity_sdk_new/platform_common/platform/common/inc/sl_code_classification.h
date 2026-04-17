/***************************************************************************//**
 * @file
 * @brief Code Classification API
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef _SL_CODE_CLASSIFICATION_H_
#define _SL_CODE_CLASSIFICATION_H_

#include "sli_code_classification.h"

// NOTE: This API is for use by applications only.

/**************************************************************************//**
 * @addtogroup code_placement Code Placement
 * @brief Code Placement API
 * @details
 *  Provides support for executing code from RAM rather than flash memory.
 * @note
 *  Inlined functions are placed wherever the calling function resides.
 * @{
 * @n @section ramfunc_use_cases Use Cases
 *
 * Consider placing code in RAM when:
 * - The function is performance-critical and called frequently
 * - The function must execute with minimal latency
 * - The function is called during flash operations (erase/write)
 * - Deterministic timing is required (eliminates flash wait states)
 *
 * @n @section ramfunc_linker Linker Requirements
 *
 * The linker script must define the appropriate RAM section "text_application_ram" for code placement.
 *
 * The "text_application_ram" section is pre-configured in Silicon Labs linker scripts,
 * but custom linker scripts must include the "text_application_ram" section.
 *
 * @n @section ramfunc_usage Usage
 * In your .h file:
 * @code{.c}
 * #include "sl_code_classification.h"
 *
 * SL_CODE_RAM
 * void MyPrint(const char* string);
 * @endcode
 *
 *****************************************************************************/

/******************************************************************************/
/* Macro API                                                                  */
/******************************************************************************/
#if defined(__GNUC__) && !defined(__MACH__)

// With GCC and non-mach-o llvm, __attribute__ can be used to specify the input 
// section of functions.

/// Prepend a function definition with this macro to place it in RAM.
#define SL_CODE_RAM \
  __attribute__((section("text_application_ram")))

#elif defined(__ICCARM__)

// With IAR, _Pragma can be used to specify the input section of
// functions.

/// Prepend a function definition with this macro to place it in RAM.
#define SL_CODE_RAM \
  _Pragma("location =\"text_application_ram\"")

#elif defined(__GNUC__) && defined(__MACH__)

#define SL_CODE_RAM

#else
  #error "(sl_code_classification.h): Code classification does not support \
  the chosen compiler."
#endif // __GNUC__

/** @} (end addtogroup code_placement) */
#endif // _SL_CODE_CLASSIFICATION_H_
