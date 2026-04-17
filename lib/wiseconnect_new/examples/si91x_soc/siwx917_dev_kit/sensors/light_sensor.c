/***************************************************************************/ /**
 * @file light_sensor.c
 * @brief Light Sensor
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
#include "sl_si91x_veml6035.h"
#include "light_sensor.h"
#include "sl_sleeptimer.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_driver_gpio.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

#define TX_THRESHOLD       0                     // tx threshold value
#define RX_THRESHOLD       0                     // rx threshold value
#define I2C                VEML6035_I2C_INSTANCE // I2C 2 instance
#define DELAY_PERIODIC_MS1 2000                  // sleeptimer1 periodic timeout in ms
#define MODE_0             0                     // Initializing GPIO MODE_0 value
#define OUTPUT_VALUE       1                     // GPIO output value

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void i2c_leader_callback(sl_i2c_instance_t i2c_instance, uint32_t status);

void light_sensor_init(void)
{
  sl_status_t status;
  sl_i2c_config_t i2c_config;

  i2c_config.mode           = SL_I2C_LEADER_MODE;
  i2c_config.transfer_type  = SL_I2C_USING_NON_DMA;
  i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
  i2c_config.i2c_callback   = i2c_leader_callback;

#if defined(SENSOR_ENABLE_GPIO_MAPPED_TO_UULP)
  if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1) {
    // Enable GPIO ULP_CLK
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_enable_clock, Error code: %lu\r\n", status);
      return;
    }
    // Set NPSS GPIO pin MUX
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SENSOR_ENABLE_GPIO_PIN, NPSS_GPIO_PIN_MUX_MODE0);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_pin_mux, Error code: %lu\r\n", status);
      return;
    }
    // Set NPSS GPIO pin direction
    status =
      sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_direction, Error code: %lu\r\n", status);
      return;
    }
    // Set UULP GPIO pin
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, SET);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_pin_value, Error code: %lu\r\n", status);
      return;
    }
  }
#else
  sl_gpio_t sensor_enable_port_pin = { SENSOR_ENABLE_GPIO_PORT, SENSOR_ENABLE_GPIO_PIN };
  uint8_t pin_value;

  status = sl_gpio_driver_get_pin(&sensor_enable_port_pin, &pin_value);
  if (status != SL_STATUS_OK) {
    printf("sl_gpio_driver_get_pin, Error code: %lu\r\n", status);
    return;
  }
  if (pin_value != 1) {
    // Enable GPIO CLK
#ifdef SENSOR_ENABLE_GPIO_MAPPED_TO_ULP
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
#else
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);
#endif
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_enable_clock, Error code: %lu\r\n", status);
      return;
    }
    // Set the pin mode for GPIO pins.
    status = sl_gpio_driver_set_pin_mode(&sensor_enable_port_pin, MODE_0, OUTPUT_VALUE);
    if (status != SL_STATUS_OK) {
      printf("sl_gpio_driver_set_pin_mode, Error code: %lu\r\n", status);
      return;
    }
    // Select the direction of GPIO pin whether Input/ Output
    status = sl_si91x_gpio_driver_set_pin_direction(SENSOR_ENABLE_GPIO_PORT,
                                                    SENSOR_ENABLE_GPIO_PIN,
                                                    (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_pin_direction, Error code: %lu\r\n", status);
      return;
    }
    // Set GPIO pin
    status = sl_gpio_driver_set_pin(&sensor_enable_port_pin); // Set ULP GPIO pin
    if (status != SL_STATUS_OK) {
      printf("sl_gpio_driver_set_pin, Error code: %lu\r\n", status);
      return;
    }
  }
#endif

  // Initialize I2C bus
  status = sl_i2c_driver_init(I2C, &i2c_config);
  if (status != SL_I2C_SUCCESS) {
    printf("sl_i2c_driver_init : Invalid Parameters, Error Code: 0x%ld \r\n", status);
    return;
  }
  status = sl_i2c_driver_configure_fifo_threshold(I2C, TX_THRESHOLD, RX_THRESHOLD);
  if (status != SL_I2C_SUCCESS) {
    printf("sl_i2c_driver_configure_fifo_threshold : Invalid Parameters, Error Code: 0x%ld \r\n", status);
    return;
  }
  // Initializes sensor and reads electronic ID 1st byte
  status = sl_si91x_veml6035_init(I2C, VEML6035_ADDR, true);
  if (status != SL_STATUS_OK) {
    printf("Sensor initialization un-successful, Error Code: 0x%ld \r\n", status);
    return;
  }
  printf("Light sensor initialized\r\n");
}

void i2c_leader_callback(sl_i2c_instance_t i2c_instance, uint32_t status)
{
  (void)i2c_instance;
  switch (status) {
    case SL_I2C_DATA_TRANSFER_COMPLETE:
      break;
    default:
      break;
  }
}

sl_status_t white_light_read(float *white_light_lux)
{
  sl_status_t status = sl_si91x_veml6035_get_white_lux(I2C, VEML6035_ADDR, white_light_lux);
  if (status != SL_STATUS_OK) {
    printf("white ch lux measurement failed, error Code: 0x%ld \r\n", status);
  }
  return status;
}

sl_status_t ambient_light_read(float *ambient_light_lux)
{
  sl_status_t status = sl_si91x_veml6035_get_als_lux(I2C, VEML6035_ADDR, ambient_light_lux);
  if (status != SL_STATUS_OK) {
    printf("ambient ch lux measurement failed, error Code: 0x%ld \r\n", status);
  }
  return status;
}
