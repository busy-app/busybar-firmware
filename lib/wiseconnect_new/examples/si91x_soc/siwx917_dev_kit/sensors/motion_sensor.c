/***************************************************************************/ /**
 * @file motion_sensor.c
 * @brief Motion Sensor
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "rsi_debug.h"
#include "sl_si91x_icm40627.h"
#include "motion_sensor.h"
#include "sl_sleeptimer.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_ssi.h"
#include "rsi_rom_clks.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_ssi_handle_t ssi_driver_handle = NULL;
static uint32_t ssi_slave_number         = SSI_SLAVE_0;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * RHT example initialization function
 ******************************************************************************/

void motion_sensor_init(void)
{
  sl_status_t sl_status;
  sl_status_t status;
  uint8_t dev_id;

#if defined(SENSOR_ENABLE_GPIO_MAPPED_TO_UULP)
  if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1) {
    // Enable GPIO ULP_CLK
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_enable_clock failed, Error code: 0x%08lX\n", (unsigned long)status);
      return;
    }
    // Set NPSS GPIO pin MUX
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SENSOR_ENABLE_GPIO_PIN, NPSS_GPIO_PIN_MUX_MODE0);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_pin_mux failed, Error code: 0x%08lX\n", (unsigned long)status);
      return;
    }
    // Set NPSS GPIO pin direction
    status =
      sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_direction failed, Error code: 0x%08lX\n", (unsigned long)status);
      return;
    }
    // Set UULP GPIO pin
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, SET);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_pin_value failed, Error code: 0x%08lX\n", (unsigned long)status);
      return;
    }
  }
#endif
  // SSI interface init
  sl_status = sl_si91x_icm40627_ssi_interface_init(&ssi_driver_handle, ssi_slave_number);
  if (sl_status != SL_STATUS_OK) {
    printf("ICM40627 SSI interface init failed, Error code: 0x%08lX\n", (unsigned long)sl_status);
    return;
  } else {
    printf("ICM40627 SSI interface init successful\n");
  }

  // reset the sensor
  sl_status = sl_si91x_icm40627_software_reset(ssi_driver_handle);
  if (sl_status != SL_STATUS_OK) {
    printf("Sensor Software reset un-successful, Error code: 0x%08lX\n", (unsigned long)sl_status);
    return;
  }
  /* Read Who am I register, should get ICM40627_DEVICE_ID */
  sl_status = sl_si91x_icm40627_get_device_id(ssi_driver_handle, &dev_id);
  if ((sl_status == SL_STATUS_OK) && (dev_id == ICM40627_DEVICE_ID)) {
  } else {
    printf("ICM40627 Get Device ID failed, Error code: 0x%08lX\n", (unsigned long)sl_status);
    return;
  }
  // Initializes sensor
  sl_status = sl_si91x_icm40627_init(ssi_driver_handle);
  if (sl_status != SL_STATUS_OK) {
    printf("Sensor initialization un-successful, Error code: 0x%08lX\n", (unsigned long)sl_status);
    return;
  }
  printf("Motion sensor initialized\r\n");
}

sl_status_t accelerometer_read(float *x, float *y, float *z)
{
  float sensor_data[3];
  sl_status_t status = sl_si91x_icm40627_get_accel_data(ssi_driver_handle, sensor_data);
  if (status != SL_STATUS_OK) {
    printf("Acceleration read failed, Error code: 0x%08lX\n", (unsigned long)status);
  } else {
    *x = sensor_data[0];
    *y = sensor_data[1];
    *z = sensor_data[2];
  }
  return status;
}

sl_status_t gyro_read(float *x, float *y, float *z)
{
  float sensor_data[3];
  sl_status_t status = sl_si91x_icm40627_get_gyro_data(ssi_driver_handle, sensor_data);
  if (status != SL_STATUS_OK) {
    printf("Gyro read failed, Error code: 0x%08lX\n", (unsigned long)status);
  } else {
    *x = sensor_data[0];
    *y = sensor_data[1];
    *z = sensor_data[2];
  }
  return status;
}

sl_status_t temperature_read(float *temperature)
{
  sl_status_t status = sl_si91x_icm40627_get_temperature_data(ssi_driver_handle, temperature);
  if (status != SL_STATUS_OK) {
    printf("Temperature read failed, Error code: 0x%08lX\n", (unsigned long)status);
  }
  return status;
}
