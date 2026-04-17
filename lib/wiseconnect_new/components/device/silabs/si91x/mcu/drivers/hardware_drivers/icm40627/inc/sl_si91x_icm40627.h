/**
 * @file sl_si91x_icm40627.h
 * @brief Driver for the Invensense ICM40627 6-axis motion sensor
 *
 * # License
 * Copyright 2024 Silicon Laboratories Inc. www.silabs.com
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
 */

#ifndef SL_SI91X_ICM40627_H
#define SL_SI91X_ICM40627_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_si91x_ssi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/**
 * @addtogroup ICM40627 ICM40627 Motion Sensor
 * @ingroup SI91X_HARDWARE_DRIVER_APIS
 * @{
 * 
 ******************************************************************************/
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
/**************************************************************************/ /**
* @name ICM40627 Register banks
* @{
******************************************************************************/
#define SL_ICM40627_BANK_0 (0 << 7)                                          /**< Register bank 0 */
#define SL_ICM40627_BANK_1 (1 << 7)                                          /**< Register bank 1 */
#define SL_ICM40627_BANK_2 (2 << 7)                                          /**< Register bank 2 */
#define SL_ICM40627_BANK_3 (3 << 7)                                          /**< Register bank 3 */
#define SL_ICM40627_BANK_4 (4 << 7)                                          /**< Register bank 4 */
/**@}*/

/**************************************************************************/ /**
* @name Register and associated bit definitions
* @{
******************************************************************************/
/***********************/
/* Bank 0 Register map */
/***********************/
#define SL_ICM40627_REG_DEVICE_CONFIG              (SL_ICM40627_BANK_0 | 0x11) /**< Device Configuration Register */
#define SL_ICM40627_BIT_SPI_MODE                   0x10                        /**< SPI mode selection            */
#define SL_ICM40627_BIT_SOFT_RESET_CONFIG_EN_RESET 0x01                        /**< Software reset configuration  */

#define SL_ICM40627_REG_DRIVE_CONFIG      (SL_ICM40627_BANK_0 | 0x13) /**< Drive Config Register to control slew rate */
#define SL_ICM40627_REG_INT_CONFIG        (SL_ICM40627_BANK_0 | 0x14) /**< Interrupt Configuration Register */
#define SL_ICM40627_REG_FIFO_CONFIG       (SL_ICM40627_BANK_0 | 0x16) /**< FIFO Configuration Register */
#define SL_ICM40627_REG_TEMP_DATA1        (SL_ICM40627_BANK_0 | 0x1D) /**< Temperature Data Register */
#define SL_ICM40627_REG_TEMP_DATA0        (SL_ICM40627_BANK_0 | 0x1E) /**< Temperature Data Register */
#define SL_ICM40627_REG_ACCEL_DATA_X1     (SL_ICM40627_BANK_0 | 0x1F) /**< Upper Byte of Accel X-axis Data Register */
#define SL_ICM40627_REG_ACCEL_DATA_X0     (SL_ICM40627_BANK_0 | 0x20) /**< Lower Byte of Accel X-axis Data Register */
#define SL_ICM40627_REG_ACCEL_DATA_Y1     (SL_ICM40627_BANK_0 | 0x21) /**< Upper Byte of Accel Y-axis Data Register */
#define SL_ICM40627_REG_ACCEL_DATA_Y0     (SL_ICM40627_BANK_0 | 0x22) /**< Lower Byte of Accel Y-axis Data Register */
#define SL_ICM40627_REG_ACCEL_DATA_Z1     (SL_ICM40627_BANK_0 | 0x23) /**< Upper Byte of Accel Z-axis Data Register */
#define SL_ICM40627_REG_ACCEL_DATA_Z0     (SL_ICM40627_BANK_0 | 0x24) /**< Lower Byte of Accel Z-axis Data Register */
#define SL_ICM40627_REG_GYRO_DATA_X1      (SL_ICM40627_BANK_0 | 0x25) /**< Upper Byte of Gyro X-axis Data Register */
#define SL_ICM40627_REG_GYRO_DATA_X0      (SL_ICM40627_BANK_0 | 0x26) /**< Lower Byte of Gyro X-axis Data Register */
#define SL_ICM40627_REG_GYRO_DATA_Y1      (SL_ICM40627_BANK_0 | 0x27) /**< Upper Byte of Gyro Y-axis Data Register */
#define SL_ICM40627_REG_GYRO_DATA_Y0      (SL_ICM40627_BANK_0 | 0x28) /**< Lower Byte of Gyro Y-axis Data Register */
#define SL_ICM40627_REG_GYRO_DATA_Z1      (SL_ICM40627_BANK_0 | 0x29) /**< Upper Byte of Gyro Z-axis Data Register */
#define SL_ICM40627_REG_GYRO_DATA_Z0      (SL_ICM40627_BANK_0 | 0x2A) /**< Lower Byte of Gyro Z-axis Data Register */
#define SL_ICM40627_REG_TMST_FSYNCH       (SL_ICM40627_BANK_0 | 0x2B) /**< Upper Byte of the Time Delta */
#define SL_ICM40627_REG_TMST_FSYNCL       (SL_ICM40627_BANK_0 | 0x2C) /**< Lower Byte of the Time Delta */
#define SL_ICM40627_REG_INT_STATUS        (SL_ICM40627_BANK_0 | 0x2D) /**< Interrupt Status Register */
#define SL_ICM40627_REG_FIFO_COUNTH       (SL_ICM40627_BANK_0 | 0x2E) /**< High Bits count of number of records in FIFO */
#define SL_ICM40627_REG_FIFO_COUNTL       (SL_ICM40627_BANK_0 | 0x2F) /**< Low Bits count of number of records in FIFO */
#define SL_ICM40627_REG_FIFO_DATA         (SL_ICM40627_BANK_0 | 0x30) /**< FIFO Data Port Register */
#define SL_ICM40627_REG_APEX_DATA0        (SL_ICM40627_BANK_0 | 0x31) /**< Pedometer Output: Lower Byte */
#define SL_ICM40627_REG_APEX_DATA1        (SL_ICM40627_BANK_0 | 0x32) /**< Pedometer Output: Upper Byte */
#define SL_ICM40627_REG_APEX_DATA2        (SL_ICM40627_BANK_0 | 0x33) /**< Pedometer Output: Walk/run cadency */
#define SL_ICM40627_REG_APEX_DATA3        (SL_ICM40627_BANK_0 | 0x34) /**< Pedometer Output: Detected activity */
#define SL_ICM40627_REG_APEX_DATA4        (SL_ICM40627_BANK_0 | 0x35) /**< Tap Detection Output Register */
#define SL_ICM40627_REG_APEX_DATA5        (SL_ICM40627_BANK_0 | 0x36) /**< DOUBLE_TAP_TIMING Register */
#define SL_ICM40627_REG_INT_STATUS2       (SL_ICM40627_BANK_0 | 0x37) /**< Interrupt Status Register */
#define SL_ICM40627_REG_INT_STATUS3       (SL_ICM40627_BANK_0 | 0x38) /**< Interrupt Status Register */
#define SL_ICM40627_REG_SIGNAL_PATH_RESET (SL_ICM40627_BANK_0 | 0x4B) /**< Signal Path Reset Register */

#define SL_ICM40627_REG_INTF_CONFIG0               (SL_ICM40627_BANK_0 | 0x4C) /**< FIFO/Sense Configuration Register */
#define SL_ICM40627_INTF_CONFIG0_SHIFT_UI_SIFS_CFG (0x00)

#define SL_ICM40627_BIT_UI_FSYNC_INT \
  6 /**< This bit automatically sets to 1 when a UI FSYNC interrupt is generated. The bit clears to 0 after the register has been read. >**/
#define SL_ICM40627_BIT_PLL_RDY_INT \
  5 /**< This bit automatically sets to 1 when a PLL Ready interrupt is generated. The bit clears to 0 after the register has been read. >**/
#define SL_ICM40627_BIT_RESET_DONE_INT \
  4 /**< This bit automatically sets to 1 when software reset is complete. The bit clears to 0 after the register has been read. >**/
#define SL_ICM40627_BIT_DATA_RDY_INT \
  3 /**< This bit automatically sets to 1 when a Data Ready interrupt is generated.The bit clears to 0 after the register has been read. >**/
#define SL_ICM40627_BIT_FIFO_THS_INT \
  2 /**< This bit automatically sets to 1 when the FIFO buffer reaches the thresholdvalue. The bit clears to 0 after the register has been read. >**/
#define SL_ICM40627_BIT_FIFO_FULL_INT \
  1 /**< This bit automatically sets to 1 when the FIFO buffer is full. The bit clears to 0after the register has been read. >**/
#define SL_ICM40627_BIT_AGC_RDY_INT \
  0 /**< This bit automatically sets to 1 when an AGC Ready interrupt is generated.The bit clears to 0 after the register has been read. >**/
#define SL_ICM40627_MASK_DATA_RDY_INT   (1 << SL_ICM40627_BIT_DATA_RDY_INT)
#define SL_ICM40627_MASK_RESET_DONE_INT (1 << SL_ICM40627_BIT_RESET_DONE_INT)

#define SL_ICM40627_INTF_CONFIG0_MASK_UI_SIFS_CFG (0x03)

#define SL_ICM40627_INTF_CONFIG0_BIT_UI_SIFS_CFG_DISABLE_SPI (0x02)
#define SL_ICM40627_INTF_CONFIG0_BIT_UI_SIFS_CFG_DISABLE_I2C (0x03)

#define FIFO_SAMPLE_COUNT (12 * 42)

#define SL_ICM40627_REG_INTF_CONFIG1 \
  (SL_ICM40627_BANK_0                \
   | 0x4D) /**< Configures the clock selection for the accelerometer low-power mode and clock source */

#define SL_ICM40627_INT_CONFIG1_BIT_CLKSEL_SEL_PLL (0x01) /**< Select PLL when available, else select RC oscillator */

#define SL_ICM40627_REG_PWR_MGMT0              (SL_ICM40627_BANK_0 | 0x4E) /**< Power Management Register */
#define SL_ICM40627_PWR_MGMT0_SHIFT_ACCEL_MODE 0
#define SL_ICM40627_PWR_MGMT0_MASK_ACCEL_MODE  0x1F /**< Power management for accelerometer bitmask */

#define SL_ICM40627_PWR_MGMT0_ACCEL_MODE_LN \
  (0x03 << SL_ICM40627_PWR_MGMT0_SHIFT_ACCEL_MODE) /**< Enable accelerometer standby mode */
#define SL_ICM40627_PWR_MGMT0_ACCEL_MODE_DIS \
  (0x3C << SL_ICM40627_PWR_MGMT0_SHIFT_ACCEL_MODE) /**< Disable accelerometer */

#define SL_ICM40627_PWR_MGMT0_SHIFT_GYRO_MODE 0
#define SL_ICM40627_PWR_MGMT0_MASK_GYRO_MODE  0x1F /**< Power management for gyroscope bitmask */

#define SL_ICM40627_PWR_MGMT0_GYRO_MODE_LN  (0x0C << SL_ICM40627_PWR_MGMT0_SHIFT_GYRO_MODE) /**< Enable gyroscope */
#define SL_ICM40627_PWR_MGMT0_GYRO_MODE_DIS (0x33 << SL_ICM40627_PWR_MGMT0_SHIFT_GYRO_MODE) /**< Disable gyroscope */

#define SL_ICM40627_PWR_MGMT0_SHIFT_TEMP_MODE 0
#define SL_ICM40627_PWR_MGMT0_MASK_TEMP_MODE  0x3F /**< Temperature sensor bitmask */

#define SL_ICM40627_PWR_MGMT0_TEMP_MODE_EN \
  (0x1F << SL_ICM40627_PWR_MGMT0_SHIFT_TEMP_MODE) /**< Enable temperature sensor */
#define SL_ICM40627_PWR_MGMT0_TEMP_MODE_DIS \
  (0x20 << SL_ICM40627_PWR_MGMT0_SHIFT_TEMP_MODE) /**< Disable temperature sensor */

#define SL_ICM40627_REG_GYRO_CONFIG0       (SL_ICM40627_BANK_0 | 0x4F) /**< Gyro Configuration Register */
#define SL_ICM40627_REG_ACCEL_CONFIG0      (SL_ICM40627_BANK_0 | 0x50) /**< Accel Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG1       (SL_ICM40627_BANK_0 | 0x51) /**< Gyro Configuration Register */
#define SL_ICM40627_REG_GYRO_ACCEL_CONFIG0 (SL_ICM40627_BANK_0 | 0x52) /**< Gyro & Accel Configuration Register */
#define SL_ICM40627_REG_ACCEL_CONFIG1      (SL_ICM40627_BANK_0 | 0x53) /**< Accel Configuration Register */
#define SL_ICM40627_REG_TMST_CONFIG        (SL_ICM40627_BANK_0 | 0x54) /**< Time Stamp Resolution Register */
#define SL_ICM40627_REG_APEX_CONFIG0       (SL_ICM40627_BANK_0 | 0x56) /**< Apex Configuration Register */
#define SL_ICM40627_REG_SMD_CONFIG         (SL_ICM40627_BANK_0 | 0x57) /**< SMD Configuration Register */
#define SL_ICM40627_REG_FIFO_CONFIG1       (SL_ICM40627_BANK_0 | 0x5F) /**< FIFO Configuration Register */
#define SL_ICM40627_REG_FIFO_CONFIG2       (SL_ICM40627_BANK_0 | 0x60) /**< FIFO Configuration Register */
#define SL_ICM40627_REG_FIFO_CONFIG3       (SL_ICM40627_BANK_0 | 0x61) /**< FIFO Configuration Register */
#define SL_ICM40627_REG_FSYNC_CONFIG       (SL_ICM40627_BANK_0 | 0x62) /**< FSync Configuration Register */
#define SL_ICM40627_REG_INT_CONFIG0        (SL_ICM40627_BANK_0 | 0x63) /**< Interrupt Configuration Register */
#define SL_ICM40627_REG_INT_CONFIG1        (SL_ICM40627_BANK_0 | 0x64) /**< Interrupt Configuration Register */
#define SL_ICM40627_REG_INT_SOURCE0        (SL_ICM40627_BANK_0 | 0x65) /**< Interrupt Source Register */
#define SL_ICM40627_REG_INT_SOURCE1        (SL_ICM40627_BANK_0 | 0x66) /**< Interrupt Source Register */
#define SL_ICM40627_REG_INT_SOURCE3        (SL_ICM40627_BANK_0 | 0x68) /**< Interrupt Source Register */
#define SL_ICM40627_REG_INT_SOURCE4        (SL_ICM40627_BANK_0 | 0x69) /**< Interrupt Source Register */
#define SL_ICM40627_REG_FIFO_LOST_PKT0     (SL_ICM40627_BANK_0 | 0x6C) /**< No. of packets lost in the FIFO (Low) */
#define SL_ICM40627_REG_FIFO_LOST_PKT1     (SL_ICM40627_BANK_0 | 0x6D) /**< No. of packets lost in the FIFO (High) */
#define SL_ICM40627_REG_SELF_TEST_CONFIG   (SL_ICM40627_BANK_0 | 0x70) /**< Device Self-Test Register */
#define SL_ICM40627_REG_WHO_AM_I           (SL_ICM40627_BANK_0 | 0x75) /**< Device Identity Register */
#define SL_ICM40627_REG_BANK_SEL           (SL_ICM40627_BANK_0 | 0x76) /**< Register Bank Selection Register */

#define SL_ICM40627_REG_INT_SOURCE0        (SL_ICM40627_BANK_0 | 0x65) /**< INT_SOURCE0 >**/
#define SL_ICM40627_BIT_UI_FSYNC_INT1_EN   6                           /**< UI FSYNC interrupt >**/
#define SL_ICM40627_BIT_PLL_RDY_INT1_EN    5                           /**< PLL ready interrupt >**/
#define SL_ICM40627_BIT_RESET_DONE_INT1_EN 4                           /**< Reset done interrupt >**/
#define SL_ICM40627_BIT_UI_DRDY_INT1_EN    3                           /**<  UI data ready interrupt >**/
#define SL_ICM40627_MASK_UI_DRDY_INT1_EN \
  (1 << SL_ICM40627_BIT_UI_DRDY_INT1_EN)     /**<  UI data ready interrupt Bit Mask >**/
#define SL_ICM40627_BIT_FIFO_THS_INT1_EN   2 /**<  FIFO threshold interrupt >**/
#define SL_ICM40627_BIT_FIFO_FULL_INT1_EN  1 /**< FIFO full interrupt >**/
#define SL_ICM40627_BIT_UI_AGC_RDY_INT1_EN 0 /**< UI AGC ready interrupt >**/

#define SL_ICM40627_REG_INT_SOURCE1   (SL_ICM40627_BANK_0 | 0x66) /**< INT_SOURCE1 >**/
#define SL_ICM40627_BIT_SMD_INT1_EN   3                           /**< SMD interrupt >**/
#define SL_ICM40627_BIT_WOM_Z_INT1_EN 2                           /**< Z-axis WOM interrupt >**/
#define SL_ICM40627_BIT_WOM_Y_INT1_EN 1                           /**< Y-axis WOM interrupt >**/
#define SL_ICM40627_BIT_WOM_X_INT1_EN 0                           /**< X-axis WOM interrupt >**/
#define SL_ICM40627_BIT_MASK_SMD_INT1_EN \
  (1 << SL_ICM40627_BIT_SMD_INT1_EN) /**< SMD Bitmask interrupt routed to INT1 >**/
#define SL_ICM40627_BIT_MASK_WOM_Z_INT1_EN \
  (1 << SL_ICM40627_BIT_WOM_Z_INT1_EN) /**< Z-axis WOM interrupt routed to INT1 >**/
#define SL_ICM40627_BIT_MASK_WOM_Y_INT1_EN \
  (1 << SL_ICM40627_BIT_WOM_Y_INT1_EN) /**< Y-axis WOM interrupt routed to INT1 >**/
#define SL_ICM40627_BIT_MASK_WOM_X_INT1_EN \
  (1 << SL_ICM40627_BIT_WOM_X_INT1_EN) /**< X-axis WOM interrupt routed to INT1 >**/

#define SL_ICM40627_SHIFT_GYRO_ODR      0 /**< Gyroscope ODR selection for UI interface output >**/
#define SL_ICM40627_MASK_GYRO_ODR_VALUE 0x0F
#define SL_ICM40627_GYRO_ODR_VALUE_0    (0 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< Reserved >**/
#define SL_ICM40627_GYRO_ODR_VALUE_1    (1 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< Reserved >**/
#define SL_ICM40627_GYRO_ODR_VALUE_10   (2 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< Reserved >**/
#define SL_ICM40627_GYRO_ODR_VALUE_8000 (3 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< 8 kHz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_4000 (4 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< 4 kHz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_2000 (5 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< 2 kHz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_1000 (6 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< 1 kHz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_200  (7 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< 200 Hz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_100  (8 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< 100 Hz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_50   (9 << SL_ICM40627_SHIFT_GYRO_ODR)    /**< 50 Hz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_25   (0x0A << SL_ICM40627_SHIFT_GYRO_ODR) /**< 25 Hz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_12_5 (0x0B << SL_ICM40627_SHIFT_GYRO_ODR) /**< 12.5 Hz >**/
#define SL_ICM40627_GYRO_ODR_VALUE_1100 (0x0C << SL_ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define SL_ICM40627_GYRO_ODR_VALUE_1101 (0x0D << SL_ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define SL_ICM40627_GYRO_ODR_VALUE_1110 (0x0E << SL_ICM40627_SHIFT_GYRO_ODR) /**< Reserved >**/
#define SL_ICM40627_GYRO_ODR_VALUE_500  (0x0F << SL_ICM40627_SHIFT_GYRO_ODR) /**< 500 Hz >**/

#define SL_ICM40627_SHIFT_ACCEL_ODR        0 /**< Accelerometer ODR selection for UI interface output >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_0      (0 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< Reserved >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_1      (1 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< Reserved >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_10     (2 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< Reserved >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_8000   (3 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< 8 kHz (LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_4000   (4 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< 4 kHz (LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_2000   (5 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< 2 kHz (LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_1000   (6 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< 1 kHz (LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_200    (7 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< 200 Hz (LP or LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_100    (8 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< 100 Hz (LP or LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_50     (9 << SL_ICM40627_SHIFT_ACCEL_ODR)    /**< 50 Hz (LP or LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_25     (0x0A << SL_ICM40627_SHIFT_ACCEL_ODR) /**< 25 Hz (LP or LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_12_5   (0x0B << SL_ICM40627_SHIFT_ACCEL_ODR) /**< 12.5 Hz (LP or LN mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_6_25   (0x0C << SL_ICM40627_SHIFT_ACCEL_ODR) /**< 6.25 Hz (LP mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_3_125  (0x0D << SL_ICM40627_SHIFT_ACCEL_ODR) /**< 3.125 Hz (LP mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_1_5625 (0x0E << SL_ICM40627_SHIFT_ACCEL_ODR) /**< 1.5625 Hz (LP mode) >**/
#define SL_ICM40627_ACCEL_ODR_VALUE_500    (0x0F << SL_ICM40627_SHIFT_ACCEL_ODR) /**< 500 Hz (LP or LN mode) >**/
#define SL_ICM40627_MASK_ACCEL_ODR_VALUE   0x0F

#define SL_ICM40627_BIT_STEP_DET_INT     5    /**< Step Detection Interrupt, clears on read >**/
#define SL_ICM40627_BIT_STEP_CNT_OVF_INT 4    /**< Step Count Overflow Interrupt, clears on read >**/
#define SL_ICM40627_BIT_TILT_DET_INT     3    /**< Tilt Detection Interrupt, clears on read >**/
#define SL_ICM40627_BIT_WAKE_INT         2    /**< Wake Event Interrupt, clears on read >**/
#define SL_ICM40627_BIT_SLEEP_INT        1    /**< Sleep Event Interrupt, clears on read >**/
#define SL_ICM40627_BIT_TAP_DET_INT      0    /**< Tap Detection Interrupt, clears on read >**/
#define SL_ICM40627_BIT_MASK_WAKE_INT    0x04 /**< Wake Event Interrupt, clears on read >**/
#define SL_ICM40627_BIT_MASK_SLEEP_INT   0x02 /**< Sleep Event Interrupt, clears on read >**/

#define SL_ICM40627_MASK_GYRO_UI_FILT_BW  0x0F /*Gyro band-width select bit-mask */
#define SL_ICM40627_MASK_ACCEL_UI_FILT_BW 0xF0 /*Accelerometer band-width select bit-mask */

/***********************/
/* Bank 1 Register map */
/***********************/
#define SL_ICM40627_REG_SENSOR_CONFIG0       (SL_ICM40627_BANK_1 | 0x03) /**< Sensor Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC2  (SL_ICM40627_BANK_1 | 0x0B) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC3  (SL_ICM40627_BANK_1 | 0x0C) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC4  (SL_ICM40627_BANK_1 | 0x0D) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC5  (SL_ICM40627_BANK_1 | 0x0E) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC6  (SL_ICM40627_BANK_1 | 0x0F) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC7  (SL_ICM40627_BANK_1 | 0x10) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC8  (SL_ICM40627_BANK_1 | 0x11) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC9  (SL_ICM40627_BANK_1 | 0x12) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_GYRO_CONFIG_STATIC10 (SL_ICM40627_BANK_1 | 0x13) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_XG_ST_DATA           (SL_ICM40627_BANK_1 | 0x5F) /**< X-Gyro Self-Test Data Register */
#define SL_ICM40627_REG_YG_ST_DATA           (SL_ICM40627_BANK_1 | 0x60) /**< Y-Gyro Self-Test Data Register */
#define SL_ICM40627_REG_ZG_ST_DATA           (SL_ICM40627_BANK_1 | 0x61) /**< Z-Gyro Self-Test Data Register */
#define SL_ICM40627_REG_TMSTVAL0             (SL_ICM40627_BANK_1 | 0x62) /**< Latched Internal Counter Register */
#define SL_ICM40627_REG_TMSTVAL1             (SL_ICM40627_BANK_1 | 0x63) /**< Latched Internal Counter Register */
#define SL_ICM40627_REG_TMSTVAL2             (SL_ICM40627_BANK_1 | 0x64) /**< Latched Internal Counter Register */
#define SL_ICM40627_REG_INTF_CONFIG4         (SL_ICM40627_BANK_1 | 0x7A) /**< SPI AP Interface Configuration Register */
#define SL_ICM40627_REG_INTF_CONFIG5         (SL_ICM40627_BANK_1 | 0x7B) /**< Pin-9 Function Configuration Register */

/***********************/
/* Bank 2 Register map */
/***********************/
#define SL_ICM40627_REG_ACCEL_CONFIG_STATIC2 (SL_ICM40627_BANK_2 | 0x03) /**< Accelerometer Configuration Register */
#define SL_ICM40627_REG_ACCEL_CONFIG_STATIC3 (SL_ICM40627_BANK_2 | 0x04) /**< Accelerometer Configuration Register */
#define SL_ICM40627_REG_ACCEL_CONFIG_STATIC4 (SL_ICM40627_BANK_1 | 0x05) /**< Accelerometer Configuration Register */
#define SL_ICM40627_REG_XA_ST_DATA           (SL_ICM40627_BANK_1 | 0x3B) /**< X-Accel Self-Test Data Register */
#define SL_ICM40627_REG_YA_ST_DATA           (SL_ICM40627_BANK_1 | 0x3C) /**< Y-Accel Self-Test Data Register */
#define SL_ICM40627_REG_ZA_ST_DATA           (SL_ICM40627_BANK_1 | 0x3D) /**< Z-Accel Self-Test Data Register */

#define SL_ICM40627_SHIFT_ACCEL_FS       5
#define SL_ICM40627_MASK_ACCEL_FULLSCALE (0x06 << SL_ICM40627_SHIFT_ACCEL_FS) /**< Accel Full Scale Select bitmask */

#define SL_ICM40627_ACCEL_FULLSCALE_2G  (0x03 << SL_ICM40627_SHIFT_ACCEL_FS) /**< Accel Full Scale = 2 g  */
#define SL_ICM40627_ACCEL_FULLSCALE_4G  (0x02 << SL_ICM40627_SHIFT_ACCEL_FS) /**< Accel Full Scale = 4 g  */
#define SL_ICM40627_ACCEL_FULLSCALE_8G  (0x01 << SL_ICM40627_SHIFT_ACCEL_FS) /**< Accel Full Scale = 8 g  */
#define SL_ICM40627_ACCEL_FULLSCALE_16G (0x00 << SL_ICM40627_SHIFT_ACCEL_FS) /**< Accel Full Scale = 16 g */
#define SL_ICM40627_MASK_ACCEL_FS_SEL   0xE0

#define SL_ICM40627_MASK_ACCEL_BW (0x39)

#define SL_ICM40627_ACCEL_BW_8000HZ (0x03) /**< Accel Bandwidth = 8 kHz   */
#define SL_ICM40627_ACCEL_BW_200HZ  (0x07) /**< Accel Bandwidth = 200 Hz  */
#define SL_ICM40627_ACCEL_BW_50HZ   (0x09) /**< Accel Bandwidth = 50 Hz   */
#define SL_ICM40627_ACCEL_BW_25HZ   (0x0A) /**< Accel Bandwidth = 25 Hz   */
#define SL_ICM40627_ACCEL_BW_12_5HZ (0x0B) /**< Accel Bandwidth = 12.5 Hz */

#define SL_ICM40627_SHIFT_GYRO_FS_SEL   5                                       /**< Gyro Full Scale Select bit shift */
#define SL_ICM40627_MASK_GYRO_FULLSCALE (0x06 << SL_ICM40627_SHIFT_GYRO_FS_SEL) /**< Gyro Full Scale Select bitmask */

#define SL_ICM40627_MASK_GYRO_FS_SEL      0xE0 /**< Bitmask for Full scale select for gyroscope  >**/
#define SL_ICM40627_GYRO_FULLSCALE_250DPS (0x03 << SL_ICM40627_SHIFT_GYRO_FS_SEL) /**< Gyro Full Scale = 250 deg/sec  */
#define SL_ICM40627_GYRO_FULLSCALE_500DPS (0x02 << SL_ICM40627_SHIFT_GYRO_FS_SEL) /**< Gyro Full Scale = 500 deg/sec  */
#define SL_ICM40627_GYRO_FULLSCALE_1000DPS \
  (0x01 << SL_ICM40627_SHIFT_GYRO_FS_SEL) /**< Gyro Full Scale = 1000 deg/sec */
#define SL_ICM40627_GYRO_FULLSCALE_2000DPS \
  (0x00 << SL_ICM40627_SHIFT_GYRO_FS_SEL) /**< Gyro Full Scale = 2000 deg/sec */

#define SL_ICM40627_SHIFT_GYRO_DLPCFG 3 /**< Gyro DLPF Config bit shift */

#define SL_ICM40627_MASK_GYRO_BW (0x39)

#define SL_ICM40627_GYRO_BW_8000HZ (0x03) /**< Gyro Bandwidth = 8 kHz   */
#define SL_ICM40627_GYRO_BW_200HZ  (0x07) /**< Gyro Bandwidth = 200 Hz  */
#define SL_ICM40627_GYRO_BW_50HZ   (0x09) /**< Gyro Bandwidth = 50 Hz   */
#define SL_ICM40627_GYRO_BW_25HZ   (0x0A) /**< Gyro Bandwidth = 25 Hz   */
#define SL_ICM40627_GYRO_BW_12_5HZ (0x0B) /**< Gyro Bandwidth = 12.5 Hz */

/***********************/
/* Bank 4 Register map */
/***********************/
#define SL_ICM40627_REG_GYRO_ON_OFF_CONFIG (SL_ICM40627_BANK_4 | 0x0E) /**< Sensor Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG1       (SL_ICM40627_BANK_4 | 0x40) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG2       (SL_ICM40627_BANK_4 | 0x41) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG3       (SL_ICM40627_BANK_4 | 0x42) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG4       (SL_ICM40627_BANK_4 | 0x43) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG5       (SL_ICM40627_BANK_4 | 0x44) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG6       (SL_ICM40627_BANK_4 | 0x45) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG7       (SL_ICM40627_BANK_4 | 0x46) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG8       (SL_ICM40627_BANK_4 | 0x47) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_APEX_CONFIG9       (SL_ICM40627_BANK_4 | 0x48) /**< Gyroscope Configuration Register */
#define SL_ICM40627_REG_ACCEL_WOM_X_THR \
  (SL_ICM40627_BANK_4 | 0x4A) /**< Threshold for X-Accel Wake-on Motion Interrupt */
#define SL_ICM40627_REG_ACCEL_WOM_Y_THR \
  (SL_ICM40627_BANK_4 | 0x4B) /**< Threshold for Y-Accel Wake-on Motion Interrupt */
#define SL_ICM40627_REG_ACCEL_WOM_Z_THR \
  (SL_ICM40627_BANK_4 | 0x4C) /**< Threshold for Z-Accel Wake-on Motion Interrupt */

#define SL_ICM40627_REG_INT_SOURCE6               (SL_ICM40627_BANK_4 | 0x4D) /**< Interrupt Source Register */
#define SL_ICM40627_INT_SOURCE6_SHIFT_UI_SIFS_CFG (0x00)

#define SL_ICM40627_INT_SOURCE6_MASK_UI_SIFS_CFG (0x03)

#define SL_ICM40627_INT_SOURCE6_BIT_CLKSEL_SEL_PLL (0x01) /**< Select PLL when available, else select RC oscillator */

#define SL_ICM40627_REG_INT_SOURCE7  (SL_ICM40627_BANK_4 | 0x4E) /**< Interrupt Source Register */
#define SL_ICM40627_REG_OFFSET_USER0 (SL_ICM40627_BANK_4 | 0x77) /**< Lower Bits of X-Gyro Offset Register */
#define SL_ICM40627_REG_OFFSET_USER1 (SL_ICM40627_BANK_4 | 0x78) /**< Upper Bits of X-Gyro & Y-Gyro Offset Register */
#define SL_ICM40627_REG_OFFSET_USER2 (SL_ICM40627_BANK_4 | 0x79) /**< Lower Bits of Y-Gyro Offset Register */
#define SL_ICM40627_REG_OFFSET_USER3 (SL_ICM40627_BANK_4 | 0x7A) /**< Lower Bits of Z-Gyro Offset Register */
#define SL_ICM40627_REG_OFFSET_USER4 (SL_ICM40627_BANK_4 | 0x7B) /**< Upper Bits of X-Accel & Z-Gyro Offset Register */
#define SL_ICM40627_REG_OFFSET_USER5 (SL_ICM40627_BANK_4 | 0x7C) /**< Lower Bits of X-Accel Offset Register */
#define SL_ICM40627_REG_OFFSET_USER6 (SL_ICM40627_BANK_4 | 0x7D) /**< Lower Bits of Y-Accel Offset Register */
#define SL_ICM40627_REG_OFFSET_USER7 (SL_ICM40627_BANK_4 | 0x7E) /**< Upper Bits of Y-Accel & Z-Accel Offset Register */
#define SL_ICM40627_REG_OFFSET_USER8 (SL_ICM40627_BANK_4 | 0x7F) /**< Lower Bits of Z-Accel Offset Register */

#define ICM40627_DEVICE_ID (0x4E) /**< ICM40627 Device ID value    */

// Constants for sensor calibration
#define SL_ICM40627_FIFO_PACKET_SIZE           16 // Each FIFO packet: Header(1)+Accel(6)+Gyro(6)+Temp(1)+Timestamp(2)
#define SL_ICM40627_FIFO_DATA_ARRAY_SIZE       17 // FIFO data array size (1 register address + 16 data bytes)
#define SL_ICM40627_FIFO_ACCEL_X_MSB_INDEX     2  // Index of accel X MSB in FIFO data
#define SL_ICM40627_FIFO_ACCEL_X_LSB_INDEX     3  // Index of accel X LSB in FIFO data
#define SL_ICM40627_FIFO_ACCEL_Y_MSB_INDEX     4  // Index of accel Y MSB in FIFO data
#define SL_ICM40627_FIFO_ACCEL_Y_LSB_INDEX     5  // Index of accel Y LSB in FIFO data
#define SL_ICM40627_FIFO_ACCEL_Z_MSB_INDEX     6  // Index of accel Z MSB in FIFO data
#define SL_ICM40627_FIFO_ACCEL_Z_LSB_INDEX     7  // Index of accel Z LSB in FIFO data
#define SL_ICM40627_FIFO_GYRO_X_MSB_INDEX      8  // Index of gyro X MSB in FIFO data
#define SL_ICM40627_FIFO_GYRO_X_LSB_INDEX      9  // Index of gyro X LSB in FIFO data
#define SL_ICM40627_FIFO_GYRO_Y_MSB_INDEX      10 // Index of gyro Y MSB in FIFO data
#define SL_ICM40627_FIFO_GYRO_Y_LSB_INDEX      11 // Index of gyro Y LSB in FIFO data
#define SL_ICM40627_FIFO_GYRO_Z_MSB_INDEX      12 // Index of gyro Z MSB in FIFO data
#define SL_ICM40627_FIFO_GYRO_Z_LSB_INDEX      13 // Index of gyro Z LSB in FIFO data
#define SL_ICM40627_GYRO_OFFSET_RESOLUTION_DPS 32.0f   // Gyro offset resolution: 1/32 dps per LSB
#define SL_ICM40627_GYRO_OFFSET_MAX_LSB        2047    // Maximum gyro offset value in LSB (±64 dps)
#define SL_ICM40627_GYRO_OFFSET_MIN_LSB        -2048   // Minimum gyro offset value in LSB (±64 dps)
#define SL_ICM40627_ACCEL_OFFSET_RESOLUTION_MG 2000.0f // Accel offset resolution: 0.5mg per LSB (1g/0.5mg = 2000)
#define SL_ICM40627_ACCEL_OFFSET_MAX_LSB       2000    // Maximum accel offset value in LSB (±1g)
#define SL_ICM40627_ACCEL_OFFSET_MIN_LSB       -2000   // Minimum accel offset value in LSB (±1g)
#define SL_ICM40627_BYTE_MASK_LOW_NIBBLE       0x0F    // Mask for lower 4 bits
#define SL_ICM40627_BYTE_MASK_HIGH_NIBBLE      0xF0    // Mask for upper 4 bits
#define SL_ICM40627_BYTE_SHIFT_8               8       // Shift by 8 bits
#define SL_ICM40627_BYTE_SHIFT_4               4       // Shift by 4 bits
#define SL_ICM40627_REG_BANK_SHIFT             7       // Shift value to extract bank number from register address
#define SL_ICM40627_AXIS_X                     0       // X-axis index
#define SL_ICM40627_AXIS_Y                     1       // Y-axis index
#define SL_ICM40627_AXIS_Z                     2       // Z-axis index
#define SL_ICM40627_FIFO_DISABLE_VALUE         0x00    // Value to disable FIFO

/** @endcond */

/***************************************************************************/ /**
 * @brief
 *    Select the desired Register bank.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] bank
 *    The address of the Register bank (0..3)
 ******************************************************************************/
sl_status_t icm40627_select_register_bank(sl_ssi_handle_t ssi_driver_handle, uint8_t bank);

/***************************************************************************/ /**
 * @brief
 *    Perform soft reset on the ICM40627 chip.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_software_reset(sl_ssi_handle_t ssi_driver_handle);

/***************************************************************************/ /**
 * @brief
 *    Set the bandwidth of the gyroscope.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] gyro_ODR
 *    The desired ODR value. Use the ICM40627_GYRO_BW_xHZ macros, which
 *    are defined in the icm40627.h file. The value of x can be
 *    6, 12, 24, 51, 120, 150, 200, 360 or 12100.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_set_gyro_bandwidth(sl_ssi_handle_t ssi_driver_handle, uint8_t gyro_ODR);

/***************************************************************************/ /**
 * @brief
 *    Set the bandwidth of the accelerometer.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] accel_ODR
 *    The desired ODR value. Use the ICM40627_ACCEL_BW_yHZ macros, which
 *    are defined in the icm40627.h file. The value of y can be
 *    6, 12, 24, 50, 111, 246, 470 or 1210.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_set_accel_bandwidth(sl_ssi_handle_t ssi_driver_handle, uint8_t accel_ODR);

/***************************************************************************/ /**
 * @brief
 *    Enable or disable the sleep mode of the ICM40627 device.
 *
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor.
 *
 * @param[in] enable
 *    If true, sleep mode is enabled to reduce power consumption by disabling
 *    the accelerometer and gyroscope while retaining essential functions
 *    for quick wake-up. Set to false to disable sleep mode and resume
 *    normal operation.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_enable_sleep_mode(sl_ssi_handle_t ssi_driver_handle, bool enable);

/***************************************************************************/ /**
 * @brief
 *    Sets the sample rate for both the accelerometer and the gyroscope.
 *
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor.
 *
 * @param[in] sample_rate
 *    The desired sample rate in Hz. The function will set the closest available
 *    sample rate to the requested value if an exact match is not possible.
 *    Available sample rates (in Hz) are:
 *      - 8000.0, 4000.0, 2000.0, 1000.0, 500.0, 200.0, 100.0, 50.0, 25.0, 12.5
 *      - 6.25 (accelerometer only)
 *      - 3.125 (accelerometer only)
 *
 *    The accelerometer supports rates down to 3.125 Hz, while the gyroscope
 *    supports rates down to 12.5 Hz. Requests below 12.5 Hz will set only the
 *    accelerometer rate, with the gyroscope defaulting to 12.5 Hz.
 *
 * @return
 *    The actual sample rate set, which may differ from the requested rate due
 *    to available sample rate options.
 ******************************************************************************/
float sl_si91x_icm40627_set_sample_rate(sl_ssi_handle_t ssi_driver_handle, float sample_rate);

/***************************************************************************/ /**
 * @brief
 *    Set the sample rate(Output Data Rate) of the accelerometer.
 *
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] accel_ODR
 *    The desired sample rate value. Use the sl_accel_ODR_t enum,
 *    which are defined in the icm40627.h file. The value can be
 *    8 kHz, 4 kHz, 2 kHz, 1 kHz, 500 Hz, 200 Hz, 100 Hz, 50 Hz
 *    25 Hz, 12.5 Hz, 6.25 Hz, 3.125 Hz, 1.5625 Hz.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_accel_set_sample_rate(sl_ssi_handle_t ssi_driver_handle, uint8_t accel_ODR);

/***************************************************************************/ /**
 * @brief
 *    Set the sample rate(Output Data Rate) of the gyroscope.
 *
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] gyro_ODR
 *    The desired sample rate value. Use the sl_gyro_ODR_t enum,
 *    which are defined in the icm40627.h file. The value can be
 *    8 kHz, 4 kHz, 2 kHz, 1 kHz, 500 Hz, 200 Hz, 100 Hz, 50 Hz
 *    25 Hz, 12.5 Hz.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_gyro_set_sample_rate(sl_ssi_handle_t ssi_driver_handle, uint8_t gyro_ODR);

/***************************************************************************/ /**
 * @brief
 *    Read the raw acceleration value and convert to g value based on
 *    the actual resolution.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] accel
 *    A 3-element array of float numbers containing the acceleration values
 *    for the x, y and z axes in g units.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_get_accel_data(sl_ssi_handle_t ssi_driver_handle, float accel[3]);

/***************************************************************************/ /**
 * @brief
 *    Read the raw gyroscope value and convert to deg/sec value based on
 *    the actual resolution.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] gyro
 *    A 3-element array of float numbers containing the gyroscope values
 *    for the x, y and z axes in deg/sec units.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_get_gyro_data(sl_ssi_handle_t ssi_driver_handle, float gyro[3]);

/***************************************************************************/ /**
 * @brief
 *    Get the actual resolution of the accelerometer.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] accel_res
 *    The resolution in g/bit units
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_get_accel_resolution(sl_ssi_handle_t ssi_driver_handle, float *accel_res);

/***************************************************************************/ /**
 * @brief
 *    Get the actual resolution of the gyroscope.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] gyro_res
 *    The actual resolution in (deg/sec)/bit units
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_get_gyro_resolution(sl_ssi_handle_t ssi_driver_handle, float *gyro_res);

/***************************************************************************/ /**
 * @brief
 *    Set the full scale value of the accelerometer.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] accelFs
 *    The desired full scale value. Use the ICM40627_ACCEL_FULLSCALE_xG
 *    macros, which are defined in the icm40627.h file. The value of x can be
 *    2, 4, 8 or 16.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_set_accel_full_scale(sl_ssi_handle_t ssi_driver_handle, uint8_t accelFs);

/***************************************************************************/ /**
 * @brief
 *    Set the full scale value of the gyroscope.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] accelFs
 *    The desired full scale value. Use the ICM40627_GYRO_FULLSCALE_yDPS
 *    macros, which are defined in the icm40627.h file. The value of y can be
 *    250, 500, 1000 or 2000.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_set_gyro_full_scale(sl_ssi_handle_t ssi_driver_handle, uint8_t accelFs);

/***************************************************************************/ /**
 * @brief
 *    Enable or disable the interrupts in the ICM40627 chip.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] dataReadyEnable
 *    If true, enables the Raw Data Ready interrupt, otherwise disables.
 *
 * @param[in] womEnable
 *    If true, enables the Wake-up On Motion interrupt, otherwise disables.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_enable_interrupt(sl_ssi_handle_t ssi_driver_handle, bool dataReadyEnable, bool womEnable);

/***************************************************************************/ /**
 * @brief
 *    Read the interrupt status registers of the ICM40627 chip.
 *
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] int_status
 *    A 32-bit value that represents the combined status of the three interrupt
 *    registers: INT_STATUS, INT_STATUS2, and INT_STATUS3. Each bit in this value
 *    corresponds to a specific interrupt flag, as follows:
 *      - Bits 0-7  : INT_STATUS - Primary interrupt flags.
 *      - Bits 8-15 : INT_STATUS2 - Secondary interrupt flags.
 *      - Bits 16-23: INT_STATUS3 - Additional interrupt flags.
 *      - Bits 24-31: Reserved for future use (set to 0).
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_read_interrupt_status(sl_ssi_handle_t ssi_driver_handle, uint32_t *int_status);

/***************************************************************************/ /**
 * @brief
 *    Check if new data is available to read.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @return
 *    Returns true if the Raw Data Ready interrupt bit is set, false otherwise.
 ******************************************************************************/
bool sl_si91x_icm40627_is_data_ready(sl_ssi_handle_t ssi_driver_handle);

/***************************************************************************/ /**
 * @brief
 *    Enable or disable the sensors in the ICM40627 chip.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[in] accel
 *    If true, enables the acceleration sensor
 *
 * @param[in] gyro
 *    If true, enables the gyroscope sensor
 *
 * @param[in] temp
 *    If true, enables the temperature sensor
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_enable_sensor(sl_ssi_handle_t ssi_driver_handle, bool accel, bool gyro, bool temp);

/***************************************************************************/ /**
 * @brief
 *    Initialize the ICM40627 sensor. Enable the power supply and SPI lines,
 *    set up the host SPI controller, configure the chip control interface,
 *    clock generator, and interrupt line.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_init(sl_ssi_handle_t ssi_driver_handle);

/***************************************************************************/ /**
 * @brief Initializes the SSI interface to enable communication with ICM40627 sensor.
 *
 * This API configures the SSI peripheral with the required settings for the ICM40627 sensor,
 * including bit width, device mode, clock mode, baud rate, and sample delay. It also initializes
 * the SSI driver, sets the configuration, registers the event callback, and sets the slave number.
 *
 * @param[out] ssi_driver_handle Pointer to the SSI driver handle to be initialized.
 * @param[in]  ssi_slave_number  SSI slave number to be used for communication.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_ssi_interface_init(sl_ssi_handle_t *ssi_driver_handle, uint32_t ssi_slave_number);

/***************************************************************************/ /**
 * @brief
 *    De-initialize the ICM40627 sensor by disconnecting the supply and SPI
 *    lines.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_deinit(void);

/***************************************************************************/ /**
 * @brief
 *    Accelerometer and gyroscope calibration function. Reads the gyroscope
 *    and accelerometer values while the device is at rest and in level. The
 *    resulting values are loaded to the accel and gyro bias Registers to cancel
 *    the static offset error.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] accel_bias_scaled
 *    The mesured acceleration sensor bias in mg
 *
 * @param[out] gyro_bias_scaled
 *    The mesured gyro sensor bias in deg/sec
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_calibrate_accel_and_gyro(sl_ssi_handle_t ssi_driver_handle,
                                                       float *accel_bias_scaled,
                                                       float *gyro_bias_scaled);

/***************************************************************************/ /**
 * @brief
 *    Read the temperature sensor raw value and convert to Celsius.
 * 
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] temp_data
 *    The mesured temperature in Celsius
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_get_temperature_data(sl_ssi_handle_t ssi_driver_handle, float *temp_data);

/***************************************************************************/ /**
 * @brief
 *    Read the device ID of the ICM40627.
 *
 * @param[in] ssi_driver_handle
 *    The handle to the SSI driver used for communication with the ICM40627 sensor
 *
 * @param[out] dev_id
 *    The ID of the device read from the WHO_AM_I Register. Expected value 0xE0.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_icm40627_get_device_id(sl_ssi_handle_t ssi_driver_handle, uint8_t *dev_id);

/// @} end group ICM40627 ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup ICM40627 ICM40627 Motion Sensor
* @{
*
* @details
*
* @section ICM40627 Introduction
*
* The ICM40627 motion sensor driver provides a set of functions for interacting with the ICM40627 6-axis motion sensor over the SPI interface. It enables integration of the sensor by offering APIs for initialization, configuration, data acquisition, and calibration. The driver supports reading acceleration, gyroscope, and temperature data, as well as configuring output data rates, full-scale ranges, and interrupt handling.
*
* @section ICM40627_Use Usage
*
* After initializing the SSI (SPI) peripheral and configuring the ICM40627 sensor, the following common operations are available:
*
* 1. *Initialize the ICM40627 sensor:* @ref sl_si91x_icm40627_init
* 2. *Read the device ID to verify sensor presence:* @ref sl_si91x_icm40627_get_device_id
* 3. *Read acceleration data (in g):* @ref sl_si91x_icm40627_get_accel_data
* 4. *Read gyroscope data (in deg/sec):* @ref sl_si91x_icm40627_get_gyro_data
* 5. *Read temperature data (in Celsius):* @ref sl_si91x_icm40627_get_temperature_data
* 6. *Set output data rates and bandwidths for accelerometer and gyroscope:* @ref sl_si91x_icm40627_set_sample_rate, @ref sl_si91x_icm40627_set_accel_bandwidth, @ref sl_si91x_icm40627_set_gyro_bandwidth
* 7. *Configure full-scale ranges for accelerometer and gyroscope:* @ref sl_si91x_icm40627_set_accel_full_scale, @ref sl_si91x_icm40627_set_gyro_full_scale
* 8. *Enable or disable sensor interrupts:* @ref sl_si91x_icm40627_enable_interrupt
* 9. *Calibrate accelerometer and gyroscope biases:* @ref sl_si91x_icm40627_calibrate_accel_and_gyro
*
* @li Typical usage sequence:
*   - *Initialize the sensor:* Use @ref sl_si91x_icm40627_init before other operations.
*   - *Verify sensor presence:* Optionally read the device ID using @ref sl_si91x_icm40627_get_device_id.
*   - *Configure sensor settings:* Set sample rates, bandwidths, and full-scale ranges as needed.
*   - *Enable required interrupts:* Use @ref sl_si91x_icm40627_enable_interrupt to configure interrupt sources.
*   - *Read sensor data:* Use @ref sl_si91x_icm40627_get_accel_data, @ref sl_si91x_icm40627_get_gyro_data, and @ref sl_si91x_icm40627_get_temperature_data to acquire measurements.
*   - *Calibrate sensors if necessary:* Use @ref sl_si91x_icm40627_calibrate_accel_and_gyro for bias correction.
*
* @li See the Function Documentation for detailed usage information of all APIs.
*
*
* @} (end addtogroup ICM40627 ICM40627 Motion Sensor) */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_ICM40627_H */