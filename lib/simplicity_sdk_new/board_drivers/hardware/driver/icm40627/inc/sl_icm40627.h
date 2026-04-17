/***************************************************************************//**
 * @file
 * @brief Driver for the Invensense ICM40627 6-axis motion sensor
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

#ifndef SL_ICM40627_H
#define SL_ICM40627_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup icm40627 ICM40627 - Motion Sensor
 * @brief Driver for the Invensense ICM40627 6-axis motion sensor.
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup icm40627_details Register definitions
 * @brief Register definitions.
 * @{
 ******************************************************************************/

/**************************************************************************//**
* @name Register and associated bit definitions
* @{
******************************************************************************/
#include "sl_icm40627_defs.h"

#define ICM40627_WHO_AM_I_ID            (0x4E)                                         /**< Device ID register         */
#define ICM40627_DEVICE_ID              ICM40627_WHO_AM_I_ID                           /**< ICM40627 Device ID value    */

#define FIFO_SAMPLE_COUNT               (12 * 42)    /**< Total number of samples in the FIFO */
#define FIFO_HEADER_SIZE                1            /**< Size of the FIFO header in bytes */
#define FIFO_ACCEL_DATA_SIZE            6            /**< Size of the accelerometer data in the FIFO in bytes */
#define FIFO_GYRO_DATA_SIZE             6            /**< Size of the gyroscope data in the FIFO in bytes */
#define FIFO_TEMP_DATA_SIZE             1            /**< Size of the temperature data in the FIFO in bytes */
#define FIFO_TS_FSYNC_SIZE              2            /**< Size of the timestamp/fSync data in the FIFO in bytes */

typedef enum {
  sl_accelODR_8000Hz   = ICM40627_ACCEL_ODR_VALUE_8000,     /**< 8 kHz (LN mode) >**/
  sl_accelODR_4000Hz   = ICM40627_ACCEL_ODR_VALUE_4000,     /**< 4 kHz (LN mode) >**/
  sl_accelODR_2000Hz   = ICM40627_ACCEL_ODR_VALUE_2000,     /**< 2 kHz (LN mode) >**/
  sl_accelODR_1000Hz   = ICM40627_ACCEL_ODR_VALUE_1000,     /**< 1 kHz (LN mode) >**/
  sl_accelODR_500Hz    = ICM40627_ACCEL_ODR_VALUE_500,      /**< 500 Hz (LP or LN mode) >**/
  sl_accelODR_200Hz    = ICM40627_ACCEL_ODR_VALUE_200,      /**< 200 Hz (LP or LN mode) >**/
  sl_accelODR_100Hz    = ICM40627_ACCEL_ODR_VALUE_100,      /**< 100 Hz (LP or LN mode) >**/
  sl_accelODR_50Hz     = ICM40627_ACCEL_ODR_VALUE_50,       /**< 50 Hz (LP or LN mode) >**/
  sl_accelODR_25Hz     = ICM40627_ACCEL_ODR_VALUE_25,       /**< 25 Hz (LP or LN mode) >**/
  sl_accelODR_12_5Hz   = ICM40627_ACCEL_ODR_VALUE_12_5,     /**< 12.5 Hz (LP or LN mode) >**/
  sl_accelODR_6_25Hz   = ICM40627_ACCEL_ODR_VALUE_6_25,     /**< 6.25 Hz (LP mode) >**/
  sl_accelODR_3_125Hz  = ICM40627_ACCEL_ODR_VALUE_3_125,    /**< 3.125 Hz (LP mode) >**/
  sl_accelODR_1_5625Hz = ICM40627_ACCEL_ODR_VALUE_1_5625,   /**< 1.5625 Hz (LP mode) >**/
} sl_accel_ODR_t;

typedef enum {
  sl_gyroODR_8000Hz   = ICM40627_GYRO_ODR_VALUE_8000,     /**< 8 kHz >**/
  sl_gyroODR_4000Hz   = ICM40627_GYRO_ODR_VALUE_4000,     /**< 4 kHz >**/
  sl_gyroODR_2000Hz   = ICM40627_GYRO_ODR_VALUE_2000,     /**< 2 kHz >**/
  sl_gyroODR_1000Hz   = ICM40627_GYRO_ODR_VALUE_1000,     /**< 1 kHz >**/
  sl_gyroODR_500Hz    = ICM40627_GYRO_ODR_VALUE_500,      /**< 500 Hz >**/
  sl_gyroODR_200Hz    = ICM40627_GYRO_ODR_VALUE_200,      /**< 200 Hz >**/
  sl_gyroODR_100Hz    = ICM40627_GYRO_ODR_VALUE_100,      /**< 100 Hz >**/
  sl_gyroODR_50Hz     = ICM40627_GYRO_ODR_VALUE_50,       /**< 50 Hz >**/
  sl_gyroODR_25Hz     = ICM40627_GYRO_ODR_VALUE_25,       /**< 25 Hz >**/
  sl_gyroODR_12_5Hz   = ICM40627_GYRO_ODR_VALUE_12_5,     /**< 12.5 Hz >**/
} sl_gyro_ODR_t;

typedef enum {
  sl_gyroFS_2000dps   = ICM40627_GYRO_FS_SEL_VALUE_2000,     /**< ±2000 dps (default) >**/
  sl_gyroFS_1000dps   = ICM40627_GYRO_FS_SEL_VALUE_1000,     /**< ±1000 dps >**/
  sl_gyroFS_500dps    = ICM40627_GYRO_FS_SEL_VALUE_500,      /**< ±500 dps >**/
  sl_gyroFS_250dps    = ICM40627_GYRO_FS_SEL_VALUE_250,      /**< ±250 dps >**/
  sl_gyroFS_125dps    = ICM40627_GYRO_FS_SEL_VALUE_125,      /**< ±125 dps >**/
  sl_gyroFS_62_5dps   = ICM40627_GYRO_FS_SEL_VALUE_62_5,     /**< ±62.5 dps >**/
  sl_gyroFS_31_25dps  = ICM40627_GYRO_FS_SEL_VALUE_31_25,    /**< ±31.25 dps >**/
  sl_gyroFS_15_625dps = ICM40627_GYRO_FS_SEL_VALUE_15_625    /**< ±15.625 dps >**/
} sl_gyro_FS_t;

typedef enum {
  sl_accelFS_16g = ICM40627_ACCEL_FS_SEL_VALUE_16G,    /**< ±16g (default) >**/
  sl_accelFS_8g  = ICM40627_ACCEL_FS_SEL_VALUE_8G,     /**< ±8g >**/
  sl_accelFS_4g  = ICM40627_ACCEL_FS_SEL_VALUE_4G,     /**< ±4g >**/
  sl_accelFS_2g  = ICM40627_ACCEL_FS_SEL_VALUE_2G      /**< ±2g >**/
} sl_Accel_FS_t;

typedef enum {
  sl_gyroBW_ODR_DIV_2   = ICM40627_GYRO_UI_FILT_BW_VALUE_0,   /**< BW = ODR / 2 */
  sl_gyroBW_ODR_DIV_4   = ICM40627_GYRO_UI_FILT_BW_VALUE_1,   /**< BW = max(400 Hz, ODR) / 4 (default) */
  sl_gyroBW_ODR_DIV_5   = ICM40627_GYRO_UI_FILT_BW_VALUE_2,   /**< BW = max(400 Hz, ODR) / 5 */
  sl_gyroBW_ODR_DIV_8   = ICM40627_GYRO_UI_FILT_BW_VALUE_3,   /**< BW = max(400 Hz, ODR) / 8 */
  sl_gyroBW_ODR_DIV_10  = ICM40627_GYRO_UI_FILT_BW_VALUE_4,   /**< BW = max(400 Hz, ODR) / 10 */
  sl_gyroBW_ODR_DIV_16  = ICM40627_GYRO_UI_FILT_BW_VALUE_5,   /**< BW = max(400 Hz, ODR) / 16 */
  sl_gyroBW_ODR_DIV_20  = ICM40627_GYRO_UI_FILT_BW_VALUE_6,   /**< BW = max(400 Hz, ODR) / 20 */
  sl_gyroBW_ODR_DIV_40  = ICM40627_GYRO_UI_FILT_BW_VALUE_7,    /**< BW = max(400 Hz, ODR) / 40 */
  sl_gyroBW_ODR_DIV_1   = ICM40627_GYRO_UI_FILT_BW_VALUE_8,   /**< BW =  max(400 Hz, ODR) */
  sl_gyroBW_ODR_MUL_8   = ICM40627_GYRO_UI_FILT_BW_VALUE_9   /**< BW = max(200 Hz, 8*ODR) */
} sl_gyro_BW_t;

typedef enum {
  sl_accelBW_ODR_DIV_2   = ICM40627_ACCEL_UI_FILT_BW_VALUE_0,   /**< BW = ODR / 2 */
  sl_accelBW_ODR_DIV_4   = ICM40627_ACCEL_UI_FILT_BW_VALUE_1,   /**< BW = max(400 Hz, ODR) / 4 (default) */
  sl_accelBW_ODR_DIV_5   = ICM40627_ACCEL_UI_FILT_BW_VALUE_2,   /**< BW = max(400 Hz, ODR) / 5 */
  sl_accelBW_ODR_DIV_8   = ICM40627_ACCEL_UI_FILT_BW_VALUE_3,   /**< BW = max(400 Hz, ODR) / 8 */
  sl_accelBW_ODR_DIV_10  = ICM40627_ACCEL_UI_FILT_BW_VALUE_4,   /**< BW = max(400 Hz, ODR) / 10 */
  sl_accelBW_ODR_DIV_16  = ICM40627_ACCEL_UI_FILT_BW_VALUE_5,   /**< BW = max(400 Hz, ODR) / 16 */
  sl_accelBW_ODR_DIV_20  = ICM40627_ACCEL_UI_FILT_BW_VALUE_6,   /**< BW = max(400 Hz, ODR) / 20 */
  sl_accelBW_ODR_DIV_40  = ICM40627_ACCEL_UI_FILT_BW_VALUE_7,    /**< BW = max(400 Hz, ODR) / 40 */
  sl_accelBW_ODR_DIV_1   = ICM40627_ACCEL_UI_FILT_BW_VALUE_8,   /**< BW =  max(400 Hz, ODR) */
  sl_accelBW_ODR_MUL_8   = ICM40627_ACCEL_UI_FILT_BW_VALUE_9   /**< BW = max(200 Hz, 8*ODR) */
} sl_accel_BW_t;

/**@}*/
/**@}*/ //icm40627_details

/***************************************************************************//**
 * @brief
 *    Initialize the SPI bus to communicate with the ICM40627.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_spi_init(void);

/***************************************************************************//**
 * @brief
 *    De-initialize the ICM40627 sensor.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_deinit(void);

/***************************************************************************//**
 * @brief
 *    Read register from the ICM40627 device.
 *
 * @param[in] addr
 *    The register address to read from in the sensor
 *    Bit[7:0] - register address
 *
 * @param[in] num_bytes
 *    The number of bytes to read
 *
 * @param[out] data
 *    The data read from the register
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_read_register(uint16_t addr, uint16_t num_bytes, uint8_t *data);

/***************************************************************************//**
 * @brief
 *    Write a register in the ICM40627 device.
 *
 * @param[in] addr
 *    The register address to write
 *    Bit[7:0] - register address
 *
 * @param[in] data
 *    The data to write to the register
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_write_register(uint16_t addr, uint8_t data);

/***************************************************************************//**
 * @brief
 *    Perform soft reset on the ICM40627 chip.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_reset(void);

/***************************************************************************//**
 * @brief
 *    Initialize the ICM40627 sensor. Enable the power supply and SPI lines,
 *    set up the host SPI controller, configure the chip control interface,
 *    clock generator, and interrupt line.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_init(void);

/***************************************************************************//**
 * @brief
 *    Set the bandwidth of the gyroscope.
 *
 * @param[in] gyro_Bw
 *    The desired bandwidth value by choosing a UI filter Bandwidth mode.
 *    Use the sl_gyro_BW_t enum, which are defined in the icm40627.h file.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_set_bandwidth(sl_gyro_BW_t gyro_Bw);

/***************************************************************************//**
 * @brief
 *    Set the bandwidth of the accelerometer.
 *
 * @param[in] accel_Bw
 *    The desired bandwidth value by choosing a UI filter Bandwidth mode.
 *    Use the sl_accel_BW_t enum, which are defined in the icm40627.h file.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_accel_set_bandwidth(sl_accel_BW_t accel_Bw);

/***************************************************************************//**
 * @brief
 *    Sets the sample rate for both the accelerometer and the gyroscope.
 *
 * @param[in] sample_rate
 *    The desired sample rate in Hz.
 *
 * @return
 *    The actual sample rate, which may be different than the desired value because
 *    of the finite and discrete number of divider settings.
 *
 ******************************************************************************/
float sl_icm40627_set_sample_rate(float sample_rate);

/***************************************************************************//**
 * @brief
 *    Set the sample rate(Output Data Rate) of the accelerometer.
 *
 * @param[in] accel_ODR
 *    The desired sample rate value. Use the sl_accel_ODR_t enum,
 *    which are defined in the icm40627.h file. The value can be
 *    8 kHz, 4 kHz, 2 kHz, 1 kHz, 500 Hz, 200 Hz, 100 Hz, 50 Hz
 *    25 Hz, 12.5 Hz, 6.25 Hz, 3.125 Hz, 1.5625 Hz.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_accel_set_sample_rate(sl_accel_ODR_t accel_ODR);

/***************************************************************************//**
 * @brief
 *    Set the sample rate(Output Data Rate) of the gyroscope.
 *
 * @param[in] gyro_ODR
 *    The desired sample rate value. Use the sl_gyro_ODR_t enum,
 *    which are defined in the icm40627.h file. The value can be
 *    8 kHz, 4 kHz, 2 kHz, 1 kHz, 500 Hz, 200 Hz, 100 Hz, 50 Hz
 *    25 Hz, 12.5 Hz.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_set_sample_rate(sl_gyro_ODR_t gyro_ODR);

/***************************************************************************//**
 * @brief
 *    Read the raw acceleration value and convert to g value based on
 *    the actual resolution.
 *
 * @param[out] accel
 *    A 3-element array of float numbers containing the acceleration values
 *    for the x, y and z axes in g units.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_accel_read_data(float accel[3]);

/***************************************************************************//**
 * @brief
 *    Read the raw gyroscope value and convert to deg/sec value based on
 *    the actual resolution.
 *
 * @param[out] gyro
 *    A 3-element array of float numbers containing the gyroscope values
 *    for the x, y and z axes in deg/sec units.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_read_data(float gyro[3]);

/***************************************************************************//**
 * @brief
 *    Get the actual resolution of the accelerometer.
 *
 * @param[out] accel_res
 *    The resolution in g/bit units
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_accel_get_resolution(float *accel_res);

/***************************************************************************//**
 * @brief
 *    Get the actual resolution of the gyroscope.
 *
 * @param[out] gyro_res
 *    The actual resolution in (deg/sec)/bit units
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_get_resolution(float *gyro_res);

/***************************************************************************//**
 * @brief
 *    Set the Full scale value of the accelerometer.
 *
 * @param[in] accelFs
 *    The desired Full scale value. Use the sl_Accel_FS_t enum,
 *    which are defined in the icm40627.h file. The value can be
 *    16g, 8g, 4g, 2g.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_accel_set_full_scale(sl_Accel_FS_t accelFs);

/***************************************************************************//**
 * @brief
 *    Set the full scale value of the gyroscope.
 *
 * @param[in] gyroFs
 *    The desired full scale value. Use the sl_gyro_FS_t enum,
 *    which are defined in the icm40627.h file. The value can be
 *    2000, 1000, 500, 250, 125, 62.5, 31.25, 15.625 (in dps).
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_gyro_set_full_scale(sl_gyro_FS_t gyroFs);

/***************************************************************************//**
 * @brief
 *    Enable or disable the sleep mode of the device.
 *
 * @param[in] enable
 *    If true, sleep mode is enabled. Set to false to disable sleep mode.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_enable_sleep_mode(bool enable);

/***************************************************************************//**
 * @brief
 *    Enable or disable the sensors in the ICM40627 chip.
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
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_enable_sensor(bool accel, bool gyro, bool temp);

/***************************************************************************//**
 * @brief
 *    Enable or disable the interrupts in the ICM40627 chip.
 *
 * @param[in] data_ready_enable
 *    If true, enables the Raw Data Ready interrupt, otherwise disables.
 *
 * @param[in] wom_enable
 *    If true, enables the Wake-up On Motion interrupt, otherwise disables.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_enable_interrupt(bool data_ready_enable, bool wom_enable);

/***************************************************************************//**
 * @brief
 *    Read the interrupt status registers of the ICM40627 chip.
 *
 * @param[out] int_status
 *    The content of the three interrupt registers which are INT_STATUS,
 *    INT_STATUS2, INT_STATUS3.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_read_interrupt_status(uint32_t *int_status);

/***************************************************************************//**
 * @brief
 *    Check if new data is available to read.
 *
 * @return
 *    Returns true if the Raw Data Ready interrupt bit set, false otherwise
 ******************************************************************************/
bool sl_icm40627_is_data_ready(void);

/***************************************************************************//**
 * @brief
 *    Accelerometer and gyroscope calibration function. Reads the gyroscope
 *    and accelerometer values while the device is at rest and in level. The
 *    resulting values are loaded to the accel and gyro bias registers to cancel
 *    the static offset error.
 *
 * @param[out] accel_bias_scaled
 *    The measured acceleration sensor bias in mg
 *
 * @param[out] gyro_bias_scaled
 *    The measured gyro sensor bias in deg/sec
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_calibrate_accel_and_gyro(float *accel_bias_scaled, float *gyro_bias_scaled);

/***************************************************************************//**
 * @brief
 *    Read the temperature sensor raw value and convert to Celsius.
 *
 * @param[out] temperature
 *    The measured temperature in Celsius
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_read_temperature_data(float *temperature);

/***************************************************************************//**
 * @brief
 *    Read the device ID of the ICM40627.
 *
 * @param[out] dev_id
 *    The ID of the device read from the WHO_AM_I register. Expected value 0x4E.
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_get_device_id(uint8_t *dev_id);

/***************************************************************************//**
 * @brief
 *    Gyroscope calibration function. Read the gyroscope
 *    values while the device is at rest and in level. The
 *    resulting values are loaded to the gyro bias registers to cancel
 *    the static offset error.
 *
 * @param[out] gyroBiasScaled
 *    The measured gyro sensor bias in deg/sec
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_calibrate_gyro(float *gyroBiasScaled);

/***************************************************************************//**
 * @brief
 *    Enable or disable the accelerometer sensor in low-power mode in the ICM40627 chip.
 *
 * @param[in] enAccel
 *    If true, enables the acceleration sensor in low power mode
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_accel_enter_low_power_mode(bool enAccel);

/***************************************************************************//**
 * @brief
 *    Select the desired register bank.
 *
 * @param[in] bank
 *    The address of the register bank (0,1,2,4)
 *
 * @return
 *    Returns zero on OK, non-zero otherwise
 ******************************************************************************/
sl_status_t sl_icm40627_select_register_bank(uint8_t bank);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // SL_ICM40627_H
