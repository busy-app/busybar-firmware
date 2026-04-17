/*******************************************************************************
* @file  sl_i2c_board.h
* @brief sl i2c board specific configuration
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

#ifndef SL_I2C_BOARD_H
#define SL_I2C_BOARD_H
/**
 * @defgroup  Board_Specific_Driver   Board specific drivers and support functions
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup  Board_Drivers BOARD: Common board components used with board drivers
 * @{
 */
#include "em_device.h"

#define SL_SI91X_I2C0_SCL_PORT RTE_I2C0_SCL_PORT
#define SL_SI91X_I2C0_SCL_PIN  RTE_I2C0_SCL_PIN
#define SL_SI91X_I2C0_SCL_MUX  RTE_I2C0_SCL_MUX
#define SL_SI91X_I2C0_SCL_PAD  RTE_I2C0_SCL_PAD
#define SL_SI91X_I2C0_SCL_REN  RTE_I2C0_SCL_I2C_REN

#define SL_SI91X_I2C0_SDA_PORT RTE_I2C0_SDA_PORT
#define SL_SI91X_I2C0_SDA_PIN  RTE_I2C0_SDA_PIN
#define SL_SI91X_I2C0_SDA_MUX  RTE_I2C0_SDA_MUX
#define SL_SI91X_I2C0_SDA_PAD  RTE_I2C0_SDA_PAD
#define SL_SI91X_I2C0_SDA_REN  RTE_I2C0_SDA_I2C_REN

#define SL_SI91X_I2C1_SCL_PORT RTE_I2C1_SCL_PORT
#define SL_SI91X_I2C1_SCL_PIN  RTE_I2C1_SCL_PIN
#define SL_SI91X_I2C1_SCL_MUX  RTE_I2C1_SCL_MUX
#define SL_SI91X_I2C1_SCL_PAD  RTE_I2C1_SCL_PAD
#define SL_SI91X_I2C1_SCL_REN  RTE_I2C1_SCL_REN

#define SL_SI91X_I2C1_SDA_PORT RTE_I2C1_SDA_PORT
#define SL_SI91X_I2C1_SDA_PIN  RTE_I2C1_SDA_PIN
#define SL_SI91X_I2C1_SDA_MUX  RTE_I2C1_SDA_MUX
#define SL_SI91X_I2C1_SDA_PAD  RTE_I2C1_SDA_PAD
#define SL_SI91X_I2C1_SDA_REN  RTE_I2C1_SDA_REN

#define SL_SI91X_I2C2_SCL_PORT RTE_I2C2_SCL_PORT
#define SL_SI91X_I2C2_SCL_PIN  RTE_I2C2_SCL_PIN
#define SL_SI91X_I2C2_SCL_MUX  RTE_I2C2_SCL_MUX
#define SL_SI91X_I2C2_SCL_PAD  RTE_I2C2_SCL_PAD
#define SL_SI91X_I2C2_SCL_REN  RTE_I2C2_SCL_REN

#define SL_SI91X_I2C2_SDA_PORT RTE_I2C2_SDA_PORT
#define SL_SI91X_I2C2_SDA_PIN  RTE_I2C2_SDA_PIN
#define SL_SI91X_I2C2_SDA_MUX  RTE_I2C2_SDA_MUX
#define SL_SI91X_I2C2_SDA_PAD  RTE_I2C2_SDA_PAD
#define SL_SI91X_I2C2_SDA_REN  RTE_I2C2_SDA_REN

#ifdef __cplusplus
}
#endif

#endif /*__SL_I2C_BOARD_H__*/

/* @} end of Board_Drivers */
/* @} end of Board_Specific_Driver */
