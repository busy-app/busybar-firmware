/***************************************************************************/ /**
 * @file humidity_sensor.c
 * @brief Humidity sensor
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
#include "sl_si91x_si70xx.h"
#include "humidity_sensor.h"
#include "sl_sleeptimer.h"
#include "sl_si91x_driver_gpio.h"
#include "cmsis_os2.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define TX_THRESHOLD 0                   // tx threshold value
#define RX_THRESHOLD 0                   // rx threshold value
#define I2C          SI70XX_I2C_INSTANCE // I2C 2 instance
#define USER_REG_1   0xBA                // writing data into user register
#define MODE_0       0                   // Initializing GPIO MODE_0 value
#define OUTPUT_VALUE 1                   // GPIO output value

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/

void humidity_sensor_init(void)
{
  sl_status_t status;
  uint8_t firm_rev;
  uint32_t humidity;
  int32_t temperature;
  uint8_t value;

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
  //    status = sl_i2c_driver_init(I2C, &i2c_config);
  //    if (status != SL_I2C_SUCCESS) {
  //      printf("sl_i2c_driver_init : Invalid Parameters, Error Code: 0x%ld \r\n", status);
  //      return;
  //    } else {
  //      printf("Successfully initialized and configured i2c leader\r\n");
  //    }
  //    status = sl_i2c_driver_configure_fifo_threshold(I2C, TX_THRESHOLD, RX_THRESHOLD);
  //    if (status != SL_I2C_SUCCESS) {
  //      printf("sl_i2c_driver_configure_fifo_threshold : Invalid Parameters, Error Code: 0x%ld \r\n", status);
  //      return;
  //    } else {
  //      printf("Successfully configured i2c TX & RX FIFO thresholds\r\n");
  //    }
  // reset the sensor
  status = sl_si91x_si70xx_reset(I2C, SI70XX_SLAVE_ADDR);
  if (status != SL_STATUS_OK) {
    printf("Sensor reset un-successful, Error Code: 0x%ld \r\n", status);
    return;
  }
  osDelay(10);
  // Initializes sensor and reads electronic ID 1st byte
  status = sl_si91x_si70xx_init(I2C, SI70XX_SLAVE_ADDR, SL_EID_FIRST_BYTE);
  if (status != SL_STATUS_OK) {
    printf("Sensor initialization un-successful, Error Code: 0x%ld \r\n", status);
    return;
  }
  osDelay(10);
  // Initializes sensor and reads electronic ID 2nd byte
  status = sl_si91x_si70xx_init(I2C, SI70XX_SLAVE_ADDR, SL_EID_SECOND_BYTE);
  if (status != SL_STATUS_OK) {
    printf("Sensor initialization un-successful, Error Code: 0x%ld \r\n", status);
    return;
  }
  osDelay(10);
  // Get sensor internal firmware version of sensor
  status = sl_si91x_si70xx_get_firmware_revision(I2C, SI70XX_SLAVE_ADDR, &firm_rev);
  if (status != SL_STATUS_OK) {
    printf("Sensor firmware version un-successful, Error Code: 0x%ld \r\n", status);
    return;
  }
  osDelay(10);
  // write register data into sensor
  status = sl_si91x_si70xx_write_control_register(I2C, SI70XX_SLAVE_ADDR, SL_RH_T_USER_REG, USER_REG_1);
  if (status != SL_STATUS_OK) {
    printf("Sensor user register 1 write data failed, Error Code: 0x%ld \r\n", status);
    return;
  }
  // Reads register data from sensor
  status = sl_si91x_si70xx_read_control_register(I2C, SI70XX_SLAVE_ADDR, SL_RH_T_USER_REG, &value);
  if (status != SL_STATUS_OK) {
    printf("Sensor user register 1 read failed, Error Code: 0x%ld \r\n", status);
    return;
  }
  osDelay(10);
  // Reads temperature from humidity from sensor
  status = sl_si91x_si70xx_read_temp_from_rh(I2C, SI70XX_SLAVE_ADDR, &humidity, &temperature);
  if (status != SL_STATUS_OK) {
    printf("Sensor temperature read failed, Error Code: 0x%ld \r\n", status);
    return;
  }
  // measure humidity data from sensor
  status = sl_si91x_si70xx_measure_humidity(I2C, SI70XX_SLAVE_ADDR, &humidity);
  if (status != SL_STATUS_OK) {
    printf("Sensor humidity read failed, Error Code: 0x%ld \r\n", status);
    return;
  }
  osDelay(10);
  // measure temperature data from sensor
  status = sl_si91x_si70xx_measure_temperature(I2C, SI70XX_SLAVE_ADDR, &temperature);
  if (status != SL_STATUS_OK) {
    printf("Sensor temperature read failed, Error Code: 0x%ld \r\n", status);
    return;
  }
  osDelay(10);
  printf("Humidity sensor initialized\r\n");
}

sl_status_t humidity_read(uint32_t *humidity)
{
  int32_t temperature = 0;
  sl_status_t status  = sl_si91x_si70xx_measure_rh_and_temp(I2C, SI70XX_SLAVE_ADDR, humidity, &temperature);
  if (status != SL_STATUS_OK) {
    printf("Sensor temperature read failed, Error Code: 0x%ld \r\n", status);
  }
  return status;
}
