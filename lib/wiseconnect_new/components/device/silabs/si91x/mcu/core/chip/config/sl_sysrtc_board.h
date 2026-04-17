/******************************************************************************
* @file  sl_sysrtc_board.h
 ******************************************************************************
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

#ifndef SL_SYSRTC_BOARD_H
#define SL_SYSRTC_BOARD_H
/**
 * @defgroup  Board_Specific_Driver   Board specific drivers and support
 * functions
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup  Board_Drivers BOARD: Common board components used with board
 * drivers
 * @{
 */

#define SL_SI91X_SYSRTC_GROUP0_CAPTURE_PIN  0
#define SL_SI91X_SYSRTC_GROUP0_CAPTURE_MODE 3
#define SL_SI91X_SYSRTC_GROUP1_CAPTURE_PIN  1
#define SL_SI91X_SYSRTC_GROUP1_CAPTURE_MODE 4

#define SL_SI91X_SYSRTC_GROUP0_COMPARE_PIN1 3
#define SL_SI91X_SYSRTC_GROUP0_COMPARE_PIN2 4
#define SL_SI91X_SYSRTC_GROUP0_COMPARE_MODE 3

#define SL_SI91X_SYSRTC_GROUP0_COMPARE_NDEF_PIN1 4
#define SL_SI91X_SYSRTC_GROUP0_COMPARE_NDEF_PIN2 0
#define SL_SI91X_SYSRTC_GROUP0_COMPARE_NDEF_MODE 4

#define SL_SI91X_SYSRTC_GROUP1_COMPARE_PIN1 1
#define SL_SI91X_SYSRTC_GROUP1_COMPARE_PIN2 2
#define SL_SI91X_SYSRTC_GROUP1_COMPARE_MODE 5

#ifdef __cplusplus
}
#endif

#endif /*__SL_SYSRTC_BOARD_H__*/

/* @} end of Board_Drivers */
/* @} end of Board_Specific_Driver */
