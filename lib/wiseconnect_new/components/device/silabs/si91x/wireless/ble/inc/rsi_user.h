/*******************************************************************************
 * @file  rsi_user.h
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

#ifndef RSI_USER_H
#define RSI_USER_H
/******************************************************
 * *                      Macros
 * ******************************************************/

// Silicon Version 1.3
#define CHIP_VER_1P3 0

// Silicon Version 1.4 and above
#define CHIP_VER_1P4_AND_ABOVE 1

// To selcet Silicon version, select either of one from among two macros
#define RS9116_SILICON_CHIP_VER \
  CHIP_VER_1P4_AND_ABOVE // Set 1 for Silicon Chip Version 1p4 and Above, 0 for Silicon Chip Version 1p3

#define TX_WAIT_TIME 0

#define BT_WAIT_TIMEOUT_SF        1
#define BLE_WAIT_TIMEOUT_SF       1
#define BT_COMMON_WAIT_TIMEOUT_SF 1

/******************************************************
 * *                    Constants
 * ******************************************************/
/******************************************************
 * *                   Enumerations
 * ******************************************************/
/******************************************************
 * *                 Type Definitions
 * ******************************************************/
/******************************************************
 * *                    Structures
 * ******************************************************/
/******************************************************
 * *                 Global Variables
 * ******************************************************/
/******************************************************
 * *               Function Declarations
 * ******************************************************/
#endif
