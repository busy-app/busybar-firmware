/***************************************************************************/ /**
 * @file veml6035_example.c
 * @brief veml6035 example APIs
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
#include "veml6035_example.h"
#include "sl_sleeptimer.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_driver_gpio.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define TX_THRESHOLD       0                     // tx threshold value
#define RX_THRESHOLD       0                     // rx threshold value
#define I2C                VEML6035_I2C_INSTANCE // I2C instance
#define DELAY_PERIODIC_MS1 2000                  // sleeptimer1 periodic timeout in ms
#define MODE_0             0                     // Initializing GPIO MODE_0 value
#define OUTPUT_VALUE       1                     // GPIO output value
#define SYNC_TIME          10                    // Sync time for sensor

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
typedef sl_i2c_config_t sl_i2c_configuration_t;
sl_sleeptimer_timer_handle_t timer1;      // sleeptimer1 handle
volatile boolean_t delay_timeout = false; // indicates sleeptimer1 timeout
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void i2c_leader_callback(sl_i2c_instance_t i2c_instance, uint32_t status);
static sl_status_t veml6035_measure_als_lux(void);
static sl_status_t veml6035_measure_white_ch_lux(void);
static void on_timeout_timer1(sl_sleeptimer_timer_handle_t *handle, void *data);
static void delay(uint32_t idelay);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * RHT example initialization function
 ******************************************************************************/
void veml6035_example_init(void)
{
  sl_status_t status;
  sl_i2c_configuration_t i2c_config;

  i2c_config.mode           = SL_I2C_LEADER_MODE;
  i2c_config.transfer_type  = SL_I2C_USING_NON_DMA;
  i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
  i2c_config.i2c_callback   = i2c_leader_callback;
  do {
#if defined(SENSOR_ENABLE_GPIO_MAPPED_TO_UULP)
    if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1) {
      // Enable GPIO ULP_CLK
      status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_enable_clock, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver clock enable is successful \n");
      // Set NPSS GPIO pin MUX
      status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SENSOR_ENABLE_GPIO_PIN, NPSS_GPIO_PIN_MUX_MODE0);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_pin_mux, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver uulp pin mux selection is successful \n");
      // Set NPSS GPIO pin direction
      status =
        sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_direction, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver uulp pin direction selection is successful \n");
      // Set UULP GPIO pin
      status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, SET);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_pin_value, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver set uulp pin value is successful \n");
    }
#else
    sl_gpio_t sensor_enable_port_pin = { SENSOR_ENABLE_GPIO_PORT, SENSOR_ENABLE_GPIO_PIN };
    uint8_t pin_value;

    status = sl_gpio_driver_get_pin(&sensor_enable_port_pin, &pin_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_driver_get_pin, Error code: %lu", status);
      break;
    }
    if (pin_value != 1) {
      // Enable GPIO CLK
#ifdef SENSOR_ENABLE_GPIO_MAPPED_TO_ULP
      status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
#else
      status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);
#endif
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_enable_clock, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver clock enable is successful \n");

      // Set the pin mode for GPIO pins.
      status = sl_gpio_driver_set_pin_mode(&sensor_enable_port_pin, MODE_0, OUTPUT_VALUE);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_gpio_driver_set_pin_mode, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver pin mode select is successful \n");
      // Select the direction of GPIO pin whether Input/ Output
      status = sl_si91x_gpio_driver_set_pin_direction(SENSOR_ENABLE_GPIO_PORT,
                                                      SENSOR_ENABLE_GPIO_PIN,
                                                      (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_set_pin_direction, Error code: %lu", status);
        break;
      }
      // Set GPIO pin
      status = sl_gpio_driver_set_pin(&sensor_enable_port_pin); // Set ULP GPIO pin
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_gpio_driver_set_pin, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver set pin value is successful \n");
    }
#endif
    //Start 2000 ms periodic timer
    sl_sleeptimer_start_periodic_timer_ms(&timer1,
                                          DELAY_PERIODIC_MS1,
                                          on_timeout_timer1,
                                          NULL,
                                          0,
                                          SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);

    // Initialize I2C bus
    status = sl_i2c_driver_init(I2C, &i2c_config);
    if (status != SL_I2C_SUCCESS) {
      DEBUGOUT("sl_i2c_driver_init : Invalid Parameters, Error Code: 0x%ld \n", status);
      break;
    } else {
      DEBUGOUT("Successfully initialized and configured i2c leader\n");
    }
    status = sl_i2c_driver_configure_fifo_threshold(I2C, TX_THRESHOLD, RX_THRESHOLD);
    if (status != SL_I2C_SUCCESS) {
      DEBUGOUT("sl_i2c_driver_configure_fifo_threshold : Invalid Parameters, Error Code: 0x%ld \n", status);
      break;
    } else {
      DEBUGOUT("Successfully configured i2c TX & RX FIFO thresholds\n");
    }

    // Initializes sensor and reads electronic ID 1st byte
    status = sl_si91x_veml6035_init(I2C, VEML6035_ADDR, true);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Sensor initialization un-successful, Error Code: 0x%ld \n", status);
      break;
    } else {
      DEBUGOUT("Successfully initialized sensor\n");
    }
  } while (false);
}

/*******************************************************************************
 * Function will run continuously in while loop and reads relative humidity and
 * temperature from sensor
 ******************************************************************************/
void veml6035_example_process_action(void)
{
  sl_status_t status;

  if (delay_timeout == true) {
    delay_timeout = false;

    status = veml6035_measure_als_lux();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("lux measurement failed, error Code: 0x%ld \n\n", status);
    } else {
      DEBUGOUT("lux measurement successful\n");
    }

    delay(SYNC_TIME);

    status = veml6035_measure_white_ch_lux();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("lux measurement failed, error Code: 0x%ld \n\n", status);
    } else {
      DEBUGOUT("lux measurement successful\n\n");
    }
  }
}
/*******************************************************************************
 * Function to measure White Channel Lux value of VEML6035
 ******************************************************************************/
static sl_status_t veml6035_measure_white_ch_lux(void)
{
  sl_status_t status = SL_STATUS_OK;
  float lux          = 0.0f;

  status = sl_si91x_veml6035_get_white_lux(I2C, VEML6035_ADDR, &lux);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("white ch lux measurement failed, error Code: 0x%ld \n", status);
    return status;
  } else {
    DEBUGOUT("measured white ch lux value: %.2f \r\n", (double)lux);
  }
  return status;
}
/*******************************************************************************
 * Function to measure ALS Lux value of VEML6035
 ******************************************************************************/
static sl_status_t veml6035_measure_als_lux(void)
{
  sl_status_t status = SL_STATUS_OK;
  float lux          = 0.0f;

  status = sl_si91x_veml6035_get_als_lux(I2C, VEML6035_ADDR, &lux);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("als lux measurement failed, error Code: 0x%ld \n", status);
    return status;
  } else {
    DEBUGOUT("measured als lux value: %.2lf\r\n", (double)lux);
  }
  return status;
}
/*******************************************************************************
 * Callback Function
 ******************************************************************************/
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
* Function to provide 1 ms Delay
*******************************************************************************/
static void delay(uint32_t idelay)
{
  for (uint32_t x = 0; x < 4600 * idelay; x++) //1.002ms delay
  {
    __NOP();
  }
}
