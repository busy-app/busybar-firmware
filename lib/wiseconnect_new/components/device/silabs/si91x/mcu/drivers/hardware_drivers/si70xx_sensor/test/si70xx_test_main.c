/***************************************************************************/
/**
 * @file si70xx_test_main.c
 * @brief Si70xx Unity Test Functions
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
#include "unity.h"
#include "sl_si91x_si70xx.h"
#include "sl_si91x_driver_gpio.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define MODE_0            0    // Initializing GPIO MODE_0 value
#define OUTPUT_VALUE      1    // GPIO output value
#define TEST_TX_THRESHOLD 0    // tx threshold value
#define TEST_RX_THRESHOLD 0    // rx threshold value
#define TEST_USER_REG     0xBA // writing data into user register

/*******************************************************************************
 ***************************  Local Variables   *******************************
 ******************************************************************************/
sl_si70xx_registers_t mock_reg = SL_RH_T_USER_REG;
uint8_t mock_value             = 0;
uint32_t mock_humid_data       = 0;
int32_t mock_temp_data         = 0;
typedef sl_i2c_config_t test_i2c_configuration_t;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void test_i2c_leader_callback(sl_i2c_instance_t i2c_instance, uint32_t status);
void test_delay(uint32_t idelay);

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_si70xx_init(void);
void test_si70xx_is_present(void);
void test_si70xx_measure_rh_and_temp(void);
void test_si70xx_get_firmware_revision(void);
void test_si70xx_read_temp_from_rh(void);
void test_si70xx_start_no_hold_measure_rh_or_temp(void);
void test_si70xx_measure_humidity(void);
void test_si70xx_measure_temperature(void);
void test_si70xx_reset(void);
void test_si70xx_read_control_register(void);
void test_si70xx_write_control_register(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  test_i2c_configuration_t test_i2c_config;
  test_i2c_config.mode           = SL_I2C_LEADER_MODE;
  test_i2c_config.transfer_type  = SL_I2C_USING_NON_DMA;
  test_i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
  test_i2c_config.i2c_callback   = test_i2c_leader_callback;

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
    status = sl_gpio_driver_set_pin(&sensor_enable_port_pin); // Set ULP GPIO pin
  }
#endif

  /* Wait for sensor to become ready */
  test_delay(80);

  sl_i2c_driver_init(SI70XX_I2C_INSTANCE, &test_i2c_config);
  sl_i2c_driver_configure_fifo_threshold(SI70XX_I2C_INSTANCE, TEST_TX_THRESHOLD, TEST_RX_THRESHOLD);

  UnityBeginGroup("SI70XX");

  RUN_TEST(test_si70xx_reset, __LINE__);
  RUN_TEST(test_si70xx_init, __LINE__);
  RUN_TEST(test_si70xx_get_firmware_revision, __LINE__);
  RUN_TEST(test_si70xx_write_control_register, __LINE__);
  RUN_TEST(test_si70xx_read_control_register, __LINE__);
  RUN_TEST(test_si70xx_read_temp_from_rh, __LINE__);
  RUN_TEST(test_si70xx_measure_humidity, __LINE__);
  RUN_TEST(test_si70xx_measure_temperature, __LINE__);
  RUN_TEST(test_si70xx_measure_rh_and_temp, __LINE__);
  RUN_TEST(test_si70xx_is_present, __LINE__);
  RUN_TEST(test_si70xx_start_no_hold_measure_rh_or_temp, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test initializing the Si70xx sensor
 ******************************************************************************/
void test_si70xx_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Init \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_init(3, SI70XX_SLAVE_ADDR, SL_LAST_EID);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_init(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_LAST_EID);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_init(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_FIRST_BYTE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx initialized successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_init(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_SECOND_BYTE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx initialized successfully \n");

  UnityPrintf("Si70xx Init completed \n");
}

/*******************************************************************************
 * Function to test checking if the Si70xx sensor is present
 ******************************************************************************/
void test_si70xx_is_present(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Is Present \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_is_present(3, SI70XX_SLAVE_ADDR, SL_EID_SECOND_BYTE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_is_present(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_LAST_EID);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_is_present(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_FIRST_BYTE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx presence checked successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_is_present(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_SECOND_BYTE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx presence checked successfully \n");

  UnityPrintf("Si70xx Is Present completed \n");
}

/*******************************************************************************
 * Function to test measuring relative humidity and temperature
 ******************************************************************************/
void test_si70xx_measure_rh_and_temp(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Measure RH and Temp \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_measure_rh_and_temp(3, SI70XX_SLAVE_ADDR, &mock_humid_data, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, NULL, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &mock_humid_data, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status =
    sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &mock_humid_data, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx RH and Temp measured successfully \n");
  UnityPrintf("sensor humidity :%ld\n", mock_humid_data);
  UnityPrintf("sensor temperature :%ld\n", mock_temp_data);

  UnityPrintf("Si70xx Measure RH and Temp completed \n");
}

/*******************************************************************************
 * Function to test getting the firmware revision
 ******************************************************************************/
void test_si70xx_get_firmware_revision(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Get Firmware Revision \n");
  sl_status_t status;
  uint8_t mock_firmware_revision;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_get_firmware_revision(3, SI70XX_SLAVE_ADDR, &mock_firmware_revision);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_si70xx_get_firmware_revision(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_get_firmware_revision(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &mock_firmware_revision);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx firmware revision retrieved successfully \n");

  UnityPrintf("Si70xx Get Firmware Revision completed \n");
}

/*******************************************************************************
 * Function to test reading temperature from previous RH measurement
 ******************************************************************************/
void test_si70xx_read_temp_from_rh(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Read Temp from RH \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_read_temp_from_rh(3, SI70XX_SLAVE_ADDR, &mock_humid_data, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_si70xx_read_temp_from_rh(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, NULL, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_si70xx_read_temp_from_rh(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &mock_humid_data, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_read_temp_from_rh(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &mock_humid_data, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx Read temp from RH measured successfully \n");

  UnityPrintf("Si70xx Read Temp from RH completed \n");
}

/*******************************************************************************
 * Function to test starting no-hold measurement of RH or Temp
 ******************************************************************************/
void test_si70xx_start_no_hold_measure_rh_or_temp(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Start No-Hold Measure RH or Temp \n");
  sl_status_t status;
  uint32_t mock_data;
  sl_si70xx_measurement_type_t mock_type = SL_LAST_MEASUREMENT;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_start_no_hold_measure_rh_or_temp(3, SI70XX_SLAVE_ADDR, mock_type, &mock_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status =
    sl_si91x_si70xx_start_no_hold_measure_rh_or_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, mock_type, &mock_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  mock_type = SL_HUMIDITY;
  status =
    sl_si91x_si70xx_start_no_hold_measure_rh_or_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, mock_type, &mock_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx no-hold measure RH or Temp started successfully \n");

  UnityPrintf("Si70xx Start No-Hold Measure RH or Temp completed \n");
}

/*******************************************************************************
 * Function to test measuring humidity
 ******************************************************************************/
void test_si70xx_measure_humidity(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Measure Humidity \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_measure_humidity(3, SI70XX_SLAVE_ADDR, &mock_humid_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with NULL parameter \n");
  status = sl_si91x_si70xx_measure_humidity(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_measure_humidity(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &mock_humid_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx humidity measured successfully \n");

  UnityPrintf("Si70xx Measure Humidity completed \n");
}

/*******************************************************************************
 * Function to test measuring temperature
 ******************************************************************************/
void test_si70xx_measure_temperature(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Measure Temperature \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_measure_temperature(3, SI70XX_SLAVE_ADDR, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with NULL parameter \n");
  status = sl_si91x_si70xx_measure_temperature(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_measure_temperature(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &mock_temp_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx temperature measured successfully \n");

  UnityPrintf("Si70xx Measure Temperature completed \n");
}

/*******************************************************************************
 * Function to test resetting the Si70xx sensor
 ******************************************************************************/
void test_si70xx_reset(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Reset \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_reset(3, SI70XX_SLAVE_ADDR);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_reset(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx reset successfully \n");

  UnityPrintf("Si70xx Reset completed \n");
}

/*******************************************************************************
 * Function to test reading control register
 ******************************************************************************/
void test_si70xx_read_control_register(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Read Control Register \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_read_control_register(3, SI70XX_SLAVE_ADDR, mock_reg, &mock_value);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status =
    sl_si91x_si70xx_read_control_register(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_LAST_CONTROL_REG, &mock_value);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with NULL parameter \n");
  status = sl_si91x_si70xx_read_control_register(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, mock_reg, NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_read_control_register(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, mock_reg, &mock_value);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx control register read successfully \n");

  UnityPrintf("Si70xx Read Control Register completed \n");
}

/*******************************************************************************
 * Function to test writing control register
 ******************************************************************************/
void test_si70xx_write_control_register(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Si70xx Write Control Register \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_write_control_register(3, SI70XX_SLAVE_ADDR, mock_reg, TEST_USER_REG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_si70xx_write_control_register(3, SI70XX_SLAVE_ADDR, SL_LAST_CONTROL_REG, TEST_USER_REG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_si70xx_write_control_register(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, mock_reg, TEST_USER_REG);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Si70xx control register written successfully \n");

  UnityPrintf("Si70xx Write Control Register completed \n");
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

/*******************************************************************************
* Function to provide 1 ms Delay
*******************************************************************************/
void test_delay(uint32_t idelay)
{
  for (uint32_t x = 0; x < 4600 * idelay; x++) //1.002ms delay
  {
    __NOP();
  }
}
