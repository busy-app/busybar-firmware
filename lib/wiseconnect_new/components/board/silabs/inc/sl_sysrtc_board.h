/*******************************************************************************
 * @file  sl_sysrtc_board.h
 * @brief sl sysrtc board specific configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
