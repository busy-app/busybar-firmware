/***************************************************************************//**
 * @file
 * @brief This header maps the responsePrint APIs to its iternal counterparts
 *   to be used by RTOS based apps.
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

#ifndef RAILTEST_RESPONSE_PRINT_MAPPING_H
#define RAILTEST_RESPONSE_PRINT_MAPPING_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define responsePrintEnable railtest_responsePrintEnable
#define responsePrint railtest_responsePrint
#define responsePrintHeader railtest_responsePrintHeader
#define responsePrintMulti railtest_responsePrintMulti
#define responsePrintError railtest_responsePrintError
#define responsePrintStart railtest_responsePrintStart
#define responsePrintContinue railtest_responsePrintContinue
#define responsePrintEnd railtest_responsePrintEnd

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
void railtest_responsePrintEnable(bool enable);
bool railtest_responsePrintHeader(char *command, char *formatString, ...);
bool railtest_responsePrintMulti(char *formatString, ...);
bool railtest_responsePrint(char *command, char *formatString, ...);
bool railtest_responsePrintStart(char *command);
bool railtest_responsePrintContinue(char *formatString, ...);
bool railtest_responsePrintEnd(char *formatString, ...);
bool railtest_responsePrintError(char *command, uint8_t code, char *formatString, ...);
#ifdef __cplusplus
}
#endif

#endif // RAILTEST_RESPONSE_PRINT_MAPPING_H
