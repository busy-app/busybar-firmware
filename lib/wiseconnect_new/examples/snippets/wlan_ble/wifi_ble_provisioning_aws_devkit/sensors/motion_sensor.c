/***************************************************************************/ /**
 * @file motion_sensor.c
 * @brief Motion Sensor
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#define SSI_MASTER_BIT_WIDTH            8        // SSI bit width
#define SSI_MASTER_BAUDRATE             10000000 // SSI baudrate
#define SSI_MASTER_RECEIVE_SAMPLE_DELAY 0        // By default sample delay is 0

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_ssi_handle_t ssi_driver_handle = NULL;
static boolean_t ssi_master_transfer_complete;
static uint32_t ssi_slave_number = SSI_SLAVE_0;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/

static void ssi_master_callback_event_handler(uint32_t event);

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
  // Configuring the user configuration structure
  sl_ssi_control_config_t ssi_master_config;
  ssi_master_config.bit_width            = SSI_MASTER_BIT_WIDTH;
  ssi_master_config.device_mode          = SL_SSI_ULP_MASTER_ACTIVE;
  ssi_master_config.clock_mode           = SL_SSI_PERIPHERAL_CPOL0_CPHA0;
  ssi_master_config.baud_rate            = SSI_MASTER_BAUDRATE;
  ssi_master_config.receive_sample_delay = SSI_MASTER_RECEIVE_SAMPLE_DELAY;

#if defined(SENSOR_ENABLE_GPIO_MAPPED_TO_UULP)
  if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1) {
    // Enable GPIO ULP_CLK
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_enable_clock, Error code: %lu", status);
      return;
    }
    // Set NPSS GPIO pin MUX
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SENSOR_ENABLE_GPIO_PIN, NPSS_GPIO_PIN_MUX_MODE0);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_pin_mux, Error code: %lu", status);
      return;
    }
    // Set NPSS GPIO pin direction
    status =
      sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_direction, Error code: %lu", status);
      return;
    }
    // Set UULP GPIO pin
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, SET);
    if (status != SL_STATUS_OK) {
      printf("sl_si91x_gpio_driver_set_uulp_npss_pin_value, Error code: %lu", status);
      return;
    }
  }
#endif
  // Initialize the SSI driver
  sl_status = sl_si91x_ssi_init(ssi_master_config.device_mode, &ssi_driver_handle);
  if (sl_status != SL_STATUS_OK) {
    printf("SSI Initialization Failed, Error Code : %lu \r\n", sl_status);
    return;
  }
  // Configure the SSI to Master, 16-bit mode @10000 kBits/sec
  sl_status = sl_si91x_ssi_set_configuration(ssi_driver_handle, &ssi_master_config, ssi_slave_number);
  if (sl_status != SL_STATUS_OK) {
    printf("Failed to Set Configuration Parameters to SSI, Error Code : %lu \r\n", sl_status);
    return;
  }
  // Register the user callback
  sl_status = sl_si91x_ssi_register_event_callback(ssi_driver_handle, ssi_master_callback_event_handler);
  if (sl_status != SL_STATUS_OK) {
    printf("SSI register event callback Failed, Error Code : %lu \r\n", sl_status);
    return;
  }
  // Set the slave number
  sl_si91x_ssi_set_slave_number((uint8_t)ssi_slave_number);

  // reset the sensor
  sl_status = sl_si91x_icm40627_software_reset(ssi_driver_handle);
  if (sl_status != SL_STATUS_OK) {
    printf("Sensor Software reset un-successful, Error Code: 0x%ld \r\n", sl_status);
    return;
  }
  /* Read Who am I register, should get ICM40627_DEVICE_ID */
  sl_status = sl_si91x_icm40627_get_device_id(ssi_driver_handle, &dev_id);
  if ((sl_status == SL_STATUS_OK) && (dev_id == ICM40627_DEVICE_ID)) {
    // Initializes sensor and reads electronic ID 1st byte
    sl_status = sl_si91x_icm40627_init(ssi_driver_handle);
    if (sl_status != SL_STATUS_OK) {
      printf("Sensor initialization un-successful, Error Code: 0x%ld \r\n", sl_status);
      return;
    }
  }
  printf("Motion sensor initialized\r\n");
}

sl_status_t accelerometer_read(float *x, float *y, float *z)
{
  float sensor_data[3];
  sl_status_t status = sl_si91x_icm40627_get_accel_data(ssi_driver_handle, sensor_data);
  if (status != SL_STATUS_OK) {
    printf("Acceleration read failed, Error Code: 0x%ld \r\n", status);
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
    printf("Gyro read failed, Error Code: 0x%ld \r\n", status);
  } else {
    *x = sensor_data[0];
    *y = sensor_data[1];
    *z = sensor_data[2];
  }
  return status;
}

/*******************************************************************************
 * @brief  SSI Master callback handler
 * @param[in]event SSI Master transmit and receive events
 * @return   None
*******************************************************************************/
static void ssi_master_callback_event_handler(uint32_t event)
{
  switch (event) {
    case SSI_EVENT_TRANSFER_COMPLETE:
      ssi_master_transfer_complete = true;
      break;

    case SSI_EVENT_DATA_LOST:
      // Occurs in slave mode when data is requested/sent by master
      // but send/receive/transfer operation has not been started
      // and indicates that data is lost. Occurs also in master mode
      // when driver cannot transfer data fast enough.
      break;

    case SSI_EVENT_MODE_FAULT:
      // Occurs in master mode when Slave Select is deactivated and
      // indicates Master Mode Fault.
      break;
  }
}

//deinit accel,gyro,temp
void motion_sensor_deinit(void)
{
  sl_status_t status;
  status = sl_si91x_ssi_deinit(ssi_driver_handle);
  if (status != SL_STATUS_OK) {
    printf("Sensor initialization un-successful, Error Code: 0x%ld \r\n", status);
    return;
  }
  printf("Motion sensor de-initialized\r\n");
}