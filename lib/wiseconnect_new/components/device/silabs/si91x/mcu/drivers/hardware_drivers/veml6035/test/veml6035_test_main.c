/***************************************************************************/ /**
 * @file veml6035_test_main.c
 * @brief VEML6035 Unity Test Functions
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
#include "unity.h"
#include "sl_si91x_veml6035.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_driver_gpio.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define TEST_TX_THRESHOLD 0                     // tx threshold value
#define TEST_RX_THRESHOLD 0                     // rx threshold value
#define TEST_I2C          VEML6035_I2C_INSTANCE // I2C instance
#define SENSITIVITY_LOW   1
#define SENSITIVITY_HIGH  0
#define GAIN_NORMAL       1
#define GAIN_DOUBLE       2
#define GAIN_QUADRULPLE   4

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
typedef sl_i2c_config_t sl_i2c_configuration_t;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void test_i2c_leader_callback(sl_i2c_instance_t i2c_instance, uint32_t status);

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_sl_si91x_veml6035_init(void);
void test_sl_si91x_veml6035_get_als_lux(void);
void test_sl_si91x_veml6035_get_white_lux(void);
void test_sl_si91x_veml6035_enable_sensor(void);
void test_sl_si91x_veml6035_reset(void);
void test_sl_si91x_veml6035_configure_sensitivity(void);
void test_sl_si91x_veml6035_configure_integration_time(void);
void test_sl_si91x_veml6035_configure_psm(void);
void test_sl_si91x_veml6035_configure_interrupt_mode(void);
void test_sl_si91x_veml6035_enable_interrupt_mode(void);
void test_sl_si91x_veml6035_read_interrupt_status(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
#if defined(SENSOR_ENABLE_GPIO_MAPPED_TO_UULP)
  if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1) {
    // Enable GPIO ULP_CLK
    sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
    // Set NPSS GPIO pin MUX
    sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SENSOR_ENABLE_GPIO_PIN, NPSS_GPIO_PIN_MUX_MODE0);
    // Set NPSS GPIO pin direction
    sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    // Set UULP GPIO pin
    sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, SET);
  }
#else
  sl_gpio_t sensor_enable_port_pin = { SENSOR_ENABLE_GPIO_PORT, SENSOR_ENABLE_GPIO_PIN };
  uint8_t pin_value;

  sl_gpio_driver_get_pin(&sensor_enable_port_pin, &pin_value);
  if (pin_value != 1) {
    // Enable GPIO CLK
#ifdef SENSOR_ENABLE_GPIO_MAPPED_TO_ULP
    sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
#else
    sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);
#endif
    // Set the pin mode for GPIO pins.
    sl_gpio_driver_set_pin_mode(&sensor_enable_port_pin, MODE_0, OUTPUT_VALUE);
    // Select the direction of GPIO pin whether Input/ Output
    sl_si91x_gpio_driver_set_pin_direction(SENSOR_ENABLE_GPIO_PORT,
                                           SENSOR_ENABLE_GPIO_PIN,
                                           (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    // Set GPIO pin
    sl_gpio_driver_set_pin(&sensor_enable_port_pin); // Set ULP GPIO pin
  }
#endif

  sl_i2c_configuration_t test_i2c_config;

  test_i2c_config.mode           = SL_I2C_LEADER_MODE;
  test_i2c_config.transfer_type  = SL_I2C_USING_NON_DMA;
  test_i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
  test_i2c_config.i2c_callback   = test_i2c_leader_callback;

  // Initialize I2C bus
  sl_i2c_driver_init(TEST_I2C, &test_i2c_config);
  sl_i2c_driver_configure_fifo_threshold(TEST_I2C, TEST_TX_THRESHOLD, TEST_RX_THRESHOLD);

  UnityBeginGroup("VEML6035");

  RUN_TEST(test_sl_si91x_veml6035_init, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_get_als_lux, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_get_white_lux, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_reset, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_configure_sensitivity, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_configure_integration_time, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_enable_sensor, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_configure_psm, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_configure_interrupt_mode, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_enable_interrupt_mode, __LINE__);
  RUN_TEST(test_sl_si91x_veml6035_read_interrupt_status, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test initializing the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Init \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_init(SL_I2C_LAST, VEML6035_ADDR, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_init(TEST_I2C, (uint8_t)0x3FF, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_init(TEST_I2C, VEML6035_ADDR, false);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 initialized successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_init(TEST_I2C, VEML6035_ADDR, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 initialized successfully \n");

  UnityPrintf("VEML6035 init completed \n");
}

/*******************************************************************************
 * Function to test getting ALS lux value
 ******************************************************************************/
void test_sl_si91x_veml6035_get_als_lux(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Get ALS Lux \n");
  sl_status_t status;
  float test_lux = 0.0f;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_get_als_lux(SL_I2C_LAST, VEML6035_ADDR, &test_lux);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_get_als_lux(TEST_I2C, (uint8_t)0x3FF, &test_lux);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_get_als_lux(TEST_I2C, VEML6035_ADDR, &test_lux);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 ALS lux value retrieved successfully \n");

  UnityPrintf("VEML6035 Get ALS Lux completed \n");
}

/*******************************************************************************
 * Function to test getting white lux value
 ******************************************************************************/
void test_sl_si91x_veml6035_get_white_lux(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Get White Lux \n");
  sl_status_t status;
  float test_lux = 0.0f;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_get_white_lux(SL_I2C_LAST, VEML6035_ADDR, &test_lux);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_get_white_lux(TEST_I2C, (uint8_t)0x3FF, &test_lux);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_get_white_lux(TEST_I2C, VEML6035_ADDR, &test_lux);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 white lux value retrieved successfully \n");

  UnityPrintf("VEML6035 Get White Lux completed \n");
}

/*******************************************************************************
 * Function to test enabling the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_enable_sensor(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Enable Sensor \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_enable_sensor(SL_I2C_LAST, VEML6035_ADDR, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_enable_sensor(TEST_I2C, (uint8_t)0x3FF, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_enable_sensor(TEST_I2C, VEML6035_ADDR, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 sensor enabled successfully \n");

  UnityPrintf("VEML6035 Enable Sensor completed \n");
}

/*******************************************************************************
 * Function to test resetting the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_reset(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Reset \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_reset(SL_I2C_LAST, VEML6035_ADDR);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_reset(TEST_I2C, (uint8_t)0x3FF);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_reset(TEST_I2C, VEML6035_ADDR);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 sensor reset successfully \n");

  UnityPrintf("VEML6035 Reset completed \n");
}

/*******************************************************************************
 * Function to test configuring the sensitivity of the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_configure_sensitivity(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Configure Sensitivity \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_sensitivity(SL_I2C_LAST, VEML6035_ADDR, SENSITIVITY_LOW, GAIN_NORMAL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_sensitivity(TEST_I2C, (uint8_t)0x3FF, SENSITIVITY_LOW, GAIN_NORMAL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_sensitivity(TEST_I2C, VEML6035_ADDR, SENSITIVITY_LOW, 0);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_configure_sensitivity(TEST_I2C, VEML6035_ADDR, SENSITIVITY_HIGH, GAIN_NORMAL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 Configure Sensitivity successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_configure_sensitivity(TEST_I2C, VEML6035_ADDR, SENSITIVITY_LOW, GAIN_DOUBLE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 Configure Sensitivity successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_configure_sensitivity(TEST_I2C, VEML6035_ADDR, SENSITIVITY_LOW, GAIN_QUADRULPLE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 Configure Sensitivity successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_configure_sensitivity(TEST_I2C, VEML6035_ADDR, SENSITIVITY_LOW, GAIN_NORMAL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 Configure Sensitivity successfully \n");

  UnityPrintf("VEML6035 Configure Sensitivity completed \n");
}

/*******************************************************************************
 * Function to test configuring the integration time of the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_configure_integration_time(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Configure Integration Time \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_integration_time(SL_I2C_LAST, VEML6035_ADDR, SL_VEML6035_ALS_IT_100MS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_integration_time(TEST_I2C, (uint8_t)0x3FF, SL_VEML6035_ALS_IT_100MS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_configure_integration_time(TEST_I2C, VEML6035_ADDR, SL_VEML6035_ALS_IT_100MS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 integration time configured successfully \n");

  UnityPrintf("VEML6035 Configure Integration Time completed \n");
}

/*******************************************************************************
 * Function to test configuring the power save mode of the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_configure_psm(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Configure PSM \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_psm(SL_I2C_LAST, VEML6035_ADDR, veml6035_psm_wait_1600_ms, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_psm(TEST_I2C, (uint8_t)0x3FF, veml6035_psm_wait_1600_ms, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_configure_psm(TEST_I2C, VEML6035_ADDR, veml6035_psm_wait_1600_ms, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 power save mode configured successfully \n");

  UnityPrintf("VEML6035 Configure PSM completed \n");
}

/*******************************************************************************
 * Function to test configuring the interrupt mode of the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_configure_interrupt_mode(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Configure Interrupt Mode \n");
  sl_status_t status;
  uint16_t mock_high_threshold = 1000;
  uint16_t mock_low_threshold  = 100;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_interrupt_mode(SL_I2C_LAST,
                                                      VEML6035_ADDR,
                                                      mock_high_threshold,
                                                      mock_low_threshold,
                                                      veml6035_als_pers_2,
                                                      false,
                                                      true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_configure_interrupt_mode(TEST_I2C,
                                                      (uint8_t)0x3FF,
                                                      mock_high_threshold,
                                                      mock_low_threshold,
                                                      veml6035_als_pers_2,
                                                      false,
                                                      true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_configure_interrupt_mode(TEST_I2C,
                                                      VEML6035_ADDR,
                                                      mock_high_threshold,
                                                      mock_low_threshold,
                                                      veml6035_als_pers_2,
                                                      false,
                                                      true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 interrupt mode configured successfully \n");

  UnityPrintf("VEML6035 Configure Interrupt Mode completed \n");
}

/*******************************************************************************
 * Function to test enabling the interrupt mode of the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_enable_interrupt_mode(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Enable Interrupt Mode \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_enable_interrupt_mode(SL_I2C_LAST, VEML6035_ADDR, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_enable_interrupt_mode(TEST_I2C, (uint8_t)0x3FF, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_enable_interrupt_mode(TEST_I2C, VEML6035_ADDR, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 interrupt mode enabled successfully \n");

  UnityPrintf("VEML6035 Enable Interrupt Mode completed \n");
}

/*******************************************************************************
 * Function to test reading the interrupt status of the VEML6035 sensor
 ******************************************************************************/
void test_sl_si91x_veml6035_read_interrupt_status(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing VEML6035 Read Interrupt Status \n");
  sl_status_t status;
  bool threshold_low;
  bool threshold_high;

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_read_interrupt_status(SL_I2C_LAST, VEML6035_ADDR, &threshold_low, &threshold_high);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter\n");
  status = sl_si91x_veml6035_read_interrupt_status(TEST_I2C, (uint8_t)0x3FF, &threshold_low, &threshold_high);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter\n");
  status = sl_si91x_veml6035_read_interrupt_status(TEST_I2C, VEML6035_ADDR, &threshold_low, &threshold_high);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("VEML6035 interrupt status read successfully \n");

  UnityPrintf("VEML6035 Read Interrupt Status completed \n");
}

/*******************************************************************************
 * Callback Function
 ******************************************************************************/
void test_i2c_leader_callback(sl_i2c_instance_t i2c_instance, uint32_t status)
{
  (void)i2c_instance;
  switch (status) {
    case SL_I2C_DATA_TRANSFER_COMPLETE:
      break;
    default:
      break;
  }
}
