/***************************************************************************/ /**
 * @file icm40627_example.c
 * @brief ICM40627 example APIs
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
#include "icm40627_example.h"
#include "sl_sleeptimer.h"
#include "sl_si91x_ssi.h"
#include "rsi_rom_clks.h"
#include "sl_status.h"
#include "sl_si91x_driver_gpio.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define DELAY_PERIODIC_MS1 2000 //sleeptimer1 periodic timeout in ms

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
sl_sleeptimer_timer_handle_t timer1; //sleeptimer1 handle
boolean_t delay_timeout;             //Indicates sleeptimer1 timeout
static sl_ssi_handle_t ssi_driver_handle = NULL;
static uint32_t ssi_slave_number         = SSI_SLAVE_0;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void on_timeout_timer1(sl_sleeptimer_timer_handle_t *handle, void *data);
static sl_status_t enable_icm40627(bool connect);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * RHT example initialization function
 ******************************************************************************/
void icm40627_example_init(void)
{
  sl_status_t sl_status;
  uint8_t dev_id;

  do {
    // Enable the sensor
    sl_status = enable_icm40627(true);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("ICM40627 enable failed, Error Code: 0x%ld \n", sl_status);
      break;
    } else {
      DEBUGOUT("ICM40627 enable successful\n");
    }

    // SSI interface init moved to driver API
    sl_status = sl_si91x_icm40627_ssi_interface_init(&ssi_driver_handle, ssi_slave_number);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("ICM40627 SSI interface init failed, Error Code: 0x%ld \n", sl_status);
      break;
    } else {
      DEBUGOUT("ICM40627 SSI interface init successful\n");
    }

    //Start 2000 ms periodic timer
    sl_sleeptimer_start_periodic_timer_ms(&timer1,
                                          DELAY_PERIODIC_MS1,
                                          on_timeout_timer1,
                                          NULL,
                                          0,
                                          SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
    // reset the sensor
    sl_status = sl_si91x_icm40627_software_reset(ssi_driver_handle);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("ICM40627 software reset un-successful, Error Code: 0x%ld \n", sl_status);
      break;
    } else {
      DEBUGOUT("ICM40627 software reset successful\n");
    }

    // Read Who am I register, should get ICM40627_DEVICE_ID
    sl_status = sl_si91x_icm40627_get_device_id(ssi_driver_handle, &dev_id);
    if ((sl_status == SL_STATUS_OK) && (dev_id == ICM40627_DEVICE_ID)) {
      DEBUGOUT("ICM40627 device ID verification successful \n");
    } else {
      DEBUGOUT("ICM40627 device ID verification failed\n");
      break;
    }

    // Initializes sensor and reads electronic ID 1st byte
    sl_status = sl_si91x_icm40627_init(ssi_driver_handle);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("ICM40627 initialization failed, Error Code: 0x%ld \n", sl_status);
      break;
    } else {
      DEBUGOUT("ICM40627 initialization successful\n");
    }
  } while (false);
}

/*******************************************************************************
 * Function will run continuously in while loop and reads relative humidity and
 * temperature from sensor
 ******************************************************************************/
void icm40627_example_process_action(void)
{
  sl_status_t status;
  float temperature = 0;
  float sensor_data[3];
  if (delay_timeout == true) {
    delay_timeout = false;

    //Reads temperature data from sensor
    status = sl_si91x_icm40627_get_temperature_data(ssi_driver_handle, &temperature);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Temperature read failed, Error Code: 0x%ld \n", status);
    } else {
      DEBUGOUT("Temperature: %0.2lf\n", temperature);
    }

    //Reads accelerometer data from sensor
    status = sl_si91x_icm40627_get_accel_data(ssi_driver_handle, sensor_data);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Acceleration read failed, Error Code: 0x%ld \n", status);
    } else {
      DEBUGOUT("Acceleration: {  ");
      for (int i = 0; i < 3; i++) {
        DEBUGOUT("%0.2f  ", sensor_data[i]);
      }
      DEBUGOUT("}\n");
    }

    //Reads gyro data from sensor
    status = sl_si91x_icm40627_get_gyro_data(ssi_driver_handle, sensor_data);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Gyro read failed, Error Code: 0x%ld \n", status);
    } else {
      DEBUGOUT("Gyro: {  ");
      for (int i = 0; i < 3; i++) {
        DEBUGOUT("%0.2f  ", sensor_data[i]);
      }
      DEBUGOUT("}\n\n");
    }
  }
}

/***************************************************************************/ /**
 * Sleeptimer timeout callback.
 ******************************************************************************/
static void on_timeout_timer1(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)&handle;
  (void)&data;
  delay_timeout = true;
}

/*******************************************************************************
 * Function to connect ICM40627 Sensor
 ******************************************************************************/
static sl_status_t enable_icm40627(bool connect)
{
  sl_status_t status;
  if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1) {
    // Enable GPIO ULP_CLK
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
    if (status != SL_STATUS_OK) {
      return status;
    }

    if (connect) {
      // Set NPSS GPIO pin MUX
      status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SENSOR_ENABLE_GPIO_PIN, NPSS_GPIO_PIN_MUX_MODE1);
      if (status != SL_STATUS_OK) {
        return status;
      }
      // Set NPSS GPIO pin direction
      status =
        sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
      if (status != SL_STATUS_OK) {
        return status;
      }
      // Set UULP GPIO pin
      status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, SET);
      if (status != SL_STATUS_OK) {
        return status;
      }
    } else {
      // Disable the sensor
      status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, CLR);
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
  }
  return SL_STATUS_OK;
}
