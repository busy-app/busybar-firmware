/***************************************************************************/ /**
 * @file icm40627_test_main.c
 * @brief ICM40627 Unity Test Functions
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
#include "sl_si91x_icm40627.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_ssi.h"
#include "rsi_rom_clks.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define SSI_MASTER_BIT_WIDTH            8        // SSI bit width
#define SSI_MASTER_BAUDRATE             10000000 // SSI baudrate
#define SSI_MASTER_RECEIVE_SAMPLE_DELAY 0        // By default sample delay is 0
#define DELAY_PERIODIC_MS1              2000     //sleeptimer1 periodic timeout in ms

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
// Test callback function

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_ssi_handle_t test_ssi_driver_handle = NULL;

static uint32_t test_ssi_slave_number = SSI_SLAVE_0;

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_icm40627_ssi_interface_init(void);
void test_icm40627_init(void);
void test_icm40627_enable_interrupt(void);
void test_icm40627_get_device_id(void);
void test_icm40627_get_temperature_data(void);
void test_icm40627_calibrate_accel_and_gyro(void);
void test_icm40627_deinit(void);
void test_icm40627_enable_sensor(void);
void test_icm40627_is_data_ready(void);
void test_icm40627_read_interrupt_status(void);
void test_icm40627_set_gyro_full_scale(void);
void test_icm40627_set_accel_full_scale(void);
void test_icm40627_get_gyro_resolution(void);
void test_icm40627_get_accel_resolution(void);
void test_icm40627_get_gyro_data(void);
void test_icm40627_get_accel_data(void);
void test_icm40627_gyro_set_sample_rate(void);
void test_icm40627_accel_set_sample_rate(void);
void test_icm40627_set_sample_rate(void);
void test_icm40627_enable_sleep_mode(void);
void test_icm40627_set_accel_bandwidth(void);
void test_icm40627_set_gyro_bandwidth(void);
void test_icm40627_software_reset(void);
void test_icm40627_select_register_bank(void);

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
#endif

  UnityBeginGroup("ICM40627");
  RUN_TEST(test_icm40627_ssi_interface_init, __LINE__);

  RUN_TEST(test_icm40627_software_reset, __LINE__);
  RUN_TEST(test_icm40627_get_device_id, __LINE__);
  RUN_TEST(test_icm40627_calibrate_accel_and_gyro, __LINE__);
  RUN_TEST(test_icm40627_enable_sensor, __LINE__);
  RUN_TEST(test_icm40627_init, __LINE__);
  RUN_TEST(test_icm40627_get_temperature_data, __LINE__);
  RUN_TEST(test_icm40627_get_accel_data, __LINE__);
  RUN_TEST(test_icm40627_get_gyro_data, __LINE__);
  RUN_TEST(test_icm40627_enable_interrupt, __LINE__);
  RUN_TEST(test_icm40627_is_data_ready, __LINE__);
  RUN_TEST(test_icm40627_read_interrupt_status, __LINE__);
  RUN_TEST(test_icm40627_set_gyro_full_scale, __LINE__);
  RUN_TEST(test_icm40627_set_accel_full_scale, __LINE__);
  RUN_TEST(test_icm40627_get_gyro_resolution, __LINE__);
  RUN_TEST(test_icm40627_get_accel_resolution, __LINE__);
  RUN_TEST(test_icm40627_set_sample_rate, __LINE__);
  RUN_TEST(test_icm40627_gyro_set_sample_rate, __LINE__);
  RUN_TEST(test_icm40627_accel_set_sample_rate, __LINE__);
  RUN_TEST(test_icm40627_enable_sleep_mode, __LINE__);
  RUN_TEST(test_icm40627_set_accel_bandwidth, __LINE__);
  RUN_TEST(test_icm40627_set_gyro_bandwidth, __LINE__);
  RUN_TEST(test_icm40627_select_register_bank, __LINE__);
  RUN_TEST(test_icm40627_deinit, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}
/*******************************************************************************
 * Function to test the SSI interface initialization for ICM40627
 ******************************************************************************/
void test_icm40627_ssi_interface_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 SSI Interface Initialization \n");
  sl_status_t status;

  UnityPrintf("Testing with NULL handle parameter \n");
  status = sl_si91x_icm40627_ssi_interface_init(NULL, 0);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Test passed: NULL handle correctly returned SL_STATUS_NULL_POINTER \n");

  UnityPrintf("Testing with valid parameters \n");
  status = sl_si91x_icm40627_ssi_interface_init(&test_ssi_driver_handle, test_ssi_slave_number);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Testing with valid parameters for status busy \n");
  status = sl_si91x_icm40627_ssi_interface_init(&test_ssi_driver_handle, test_ssi_slave_number);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_BUSY, status);
  UnityPrintf("Test passed: Valid parameters initialized SSI interface successfully \n");

  UnityPrintf("ICM40627 SSI Interface Initialization tests completed \n");
}

/*******************************************************************************
 * Function to test the initialization of ICM40627
 ******************************************************************************/
void test_icm40627_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 Initialization \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_init(NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_init(test_ssi_driver_handle);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 initialized successfully \n");

  UnityPrintf("ICM40627 Initialization completed \n");
}

/*******************************************************************************
 * Function to test the software reset of ICM40627
 ******************************************************************************/
void test_icm40627_software_reset(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 software reset \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_software_reset(NULL);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_software_reset(test_ssi_driver_handle);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 software reset successfully \n");

  UnityPrintf("ICM40627 software reset completed \n");
}

/*******************************************************************************
 * Function to test get the device id of ICM40627
 ******************************************************************************/
void test_icm40627_get_device_id(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 get the device id \n");
  sl_status_t status;
  uint8_t test_dev_id;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_get_device_id(NULL, &test_dev_id);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_get_device_id(test_ssi_driver_handle, &test_dev_id);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_UINT8(ICM40627_DEVICE_ID, test_dev_id);
  UnityPrintf("Status of API is correct, ICM40627 get the device id successfully \n");

  UnityPrintf("ICM40627 get the device id completed \n");
}

/*******************************************************************************
 * Function to test calibrate acceleration and gyroscope sensor of ICM40627
 ******************************************************************************/
void test_icm40627_calibrate_accel_and_gyro(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 calibrate acceleration and gyroscope \n");
  sl_status_t status;
  /* Calibration data */
  float test_accel[3] = { 0.01, 0.01, 0.01 };
  float test_gyro[3]  = { 0.01, 0.01, 0.01 };

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_calibrate_accel_and_gyro(NULL, test_accel, test_gyro);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_calibrate_accel_and_gyro(test_ssi_driver_handle, test_accel, test_gyro);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 calibrate acceleration and gyroscope successfully \n");

  UnityPrintf("ICM40627 calibrate acceleration and gyroscope completed \n");
}

/*******************************************************************************
 * Function to test enable sensor for ICM40627
 ******************************************************************************/
void test_icm40627_enable_sensor(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 enable sensor \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_enable_sensor(NULL, true, true, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_enable_sensor(test_ssi_driver_handle, true, true, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 enable sensor successfully \n");

  UnityPrintf("ICM40627 enable sensor completed \n");
}

/*******************************************************************************
 * Function to test get temperature data of ICM40627
 ******************************************************************************/
void test_icm40627_get_temperature_data(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 get temperature data \n");
  sl_status_t status;
  float test_temp = 0;

  status = sl_si91x_icm40627_get_temperature_data(test_ssi_driver_handle, &test_temp);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 get temperature data successfully \n");
  UnityPrintf("Temperature = %0.2f \n", test_temp);

  UnityPrintf("ICM40627 get temperature data completed \n");
}

/*******************************************************************************
 * Function to test get acceleration data of ICM40627
 ******************************************************************************/
void test_icm40627_get_accel_data(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 get acceleration data \n");
  sl_status_t status;
  float sensor_data[3];

  status = sl_si91x_icm40627_get_accel_data(test_ssi_driver_handle, sensor_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 get acceleration data successfully \n");
  UnityPrintf("Acceleration: {  ");
  for (int i = 0; i < 3; i++) {
    UnityPrintf("%0.2f  ", sensor_data[i]);
  }
  UnityPrintf("}\n");

  UnityPrintf("ICM40627 get acceleration data completed \n");
}

/*******************************************************************************
 * Function to test get gyroscope data of ICM40627
 ******************************************************************************/
void test_icm40627_get_gyro_data(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 get gyroscope data \n");
  sl_status_t status;
  float sensor_data[3];

  status = sl_si91x_icm40627_get_gyro_data(test_ssi_driver_handle, sensor_data);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 get gyroscope data successfully \n");
  UnityPrintf("Gyro: {  ");
  for (int i = 0; i < 3; i++) {
    UnityPrintf("%0.2f  ", sensor_data[i]);
  }
  UnityPrintf("}\n");

  UnityPrintf("ICM40627 get gyroscope data completed \n");
}

/*******************************************************************************
 * Function to test enable or disable interrupt for ICM40627
 ******************************************************************************/
void test_icm40627_enable_interrupt(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 enable or disable interrupt \n");
  sl_status_t status;

  status = sl_si91x_icm40627_enable_interrupt(test_ssi_driver_handle, false, false);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 disable interrupt successfully \n");

  status = sl_si91x_icm40627_enable_interrupt(test_ssi_driver_handle, true, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 enable interrupt successfully \n");

  UnityPrintf("ICM40627 enable or disable interrupt completed \n");
}

/*******************************************************************************
 * Function to test data ready for ICM40627
 ******************************************************************************/
void test_icm40627_is_data_ready(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 data ready \n");
  bool status;

  status = sl_si91x_icm40627_is_data_ready(test_ssi_driver_handle);
  TEST_ASSERT_TRUE(status);
  UnityPrintf("Status of API is correct, ICM40627 data ready successfully \n");

  UnityPrintf("ICM40627 data ready completed \n");
}

/*******************************************************************************
 * Function to test read interrupt status ICM40627
 ******************************************************************************/
void test_icm40627_read_interrupt_status(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 read interrupt status \n");
  sl_status_t status;
  uint32_t int_status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_read_interrupt_status(NULL, &int_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_read_interrupt_status(test_ssi_driver_handle, &int_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 read interrupt status successfully \n");
  UnityPrintf("Interrupt status = %ld \n", int_status);

  UnityPrintf("ICM40627 read interrupt status completed \n");
}

/*******************************************************************************
 * Function to test set gyroscope full scale for ICM40627
 ******************************************************************************/
void test_icm40627_set_gyro_full_scale(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 set gyroscope full scale \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_set_gyro_full_scale(NULL, SL_ICM40627_GYRO_FULLSCALE_2000DPS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_set_gyro_full_scale(test_ssi_driver_handle, SL_ICM40627_GYRO_FULLSCALE_2000DPS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 set gyroscope full scale successfully \n");

  UnityPrintf("ICM40627 set gyroscope full scale completed \n");
}

/*******************************************************************************
 * Function to test set acceleration full scale for ICM40627
 ******************************************************************************/
void test_icm40627_set_accel_full_scale(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 set acceleration full scale \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_set_accel_full_scale(NULL, SL_ICM40627_ACCEL_FULLSCALE_16G);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_set_accel_full_scale(test_ssi_driver_handle, SL_ICM40627_ACCEL_FULLSCALE_16G);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 set acceleration full scale successfully \n");

  UnityPrintf("ICM40627 set acceleration full scale completed \n");
}

/*******************************************************************************
 * Function to test get gyroscope resolution for ICM40627
 ******************************************************************************/
void test_icm40627_get_gyro_resolution(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 get gyroscope resolution \n");
  sl_status_t status;
  float gyro_res;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_get_gyro_resolution(NULL, &gyro_res);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_get_gyro_resolution(test_ssi_driver_handle, &gyro_res);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 get gyroscope resolution successfully \n");
  UnityPrintf("Actual gyroscope resolution = %0.2f \n", gyro_res);

  UnityPrintf("ICM40627 get gyroscope resolution completed \n");
}

/*******************************************************************************
 * Function to test get acceleration resolution for ICM40627
 ******************************************************************************/
void test_icm40627_get_accel_resolution(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 get acceleration resolution \n");
  sl_status_t status;
  float accel_res;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_get_accel_resolution(NULL, &accel_res);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_get_accel_resolution(test_ssi_driver_handle, &accel_res);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 get acceleration resolution successfully \n");
  UnityPrintf("Actual gyroscope resolution = %0.2f \n", accel_res);

  UnityPrintf("ICM40627 get acceleration resolution completed \n");
}

/*******************************************************************************
 * Function to test set gyroscope sample rate for ICM40627
 ******************************************************************************/
void test_icm40627_gyro_set_sample_rate(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 set gyroscope sample rate \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_gyro_set_sample_rate(NULL, SL_ICM40627_GYRO_ODR_VALUE_100);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_icm40627_gyro_set_sample_rate(test_ssi_driver_handle, SL_ICM40627_GYRO_ODR_VALUE_1101);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  status = sl_si91x_icm40627_gyro_set_sample_rate(test_ssi_driver_handle, SL_ICM40627_GYRO_ODR_VALUE_4000);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 set gyroscope sample rate successfully \n");

  UnityPrintf("ICM40627 set gyroscope sample rate completed \n");
}

/*******************************************************************************
 * Function to test set acceleration sample rate for ICM40627
 ******************************************************************************/
void test_icm40627_accel_set_sample_rate(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 set acceleration sample rate \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_accel_set_sample_rate(NULL, SL_ICM40627_ACCEL_ODR_VALUE_50);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_icm40627_accel_set_sample_rate(test_ssi_driver_handle, SL_ICM40627_ACCEL_ODR_VALUE_10);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  status = sl_si91x_icm40627_accel_set_sample_rate(test_ssi_driver_handle, SL_ICM40627_ACCEL_ODR_VALUE_4000);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 set acceleration sample rate successfully \n");

  UnityPrintf("ICM40627 set acceleration sample rate completed \n");
}

/*******************************************************************************
 * Function to test set sample rate for ICM40627
 ******************************************************************************/
void test_icm40627_set_sample_rate(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 set sample rate \n");
  float sample_rate;

  sample_rate = sl_si91x_icm40627_set_sample_rate(test_ssi_driver_handle, 4000.0);
  TEST_ASSERT_EQUAL(4000.0, sample_rate);
  UnityPrintf("Status of API is correct, ICM40627 set sample rate successfully \n");

  UnityPrintf("ICM40627 set sample rate completed \n");
}

/*******************************************************************************
 * Function to test enable sleep mode for ICM40627
 ******************************************************************************/
void test_icm40627_enable_sleep_mode(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 enable sleep mode \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_enable_sleep_mode(NULL, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_enable_sleep_mode(test_ssi_driver_handle, true);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 enable sleep mode successfully \n");

  status = sl_si91x_icm40627_enable_sleep_mode(test_ssi_driver_handle, false);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 enable sleep mode successfully \n");

  UnityPrintf("ICM40627 enable sleep mode completed \n");
}

/*******************************************************************************
 * Function to test set acceleration bandwidth for ICM40627
 ******************************************************************************/
void test_icm40627_set_accel_bandwidth(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 set acceleration bandwidth \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_set_accel_bandwidth(NULL, SL_ICM40627_ACCEL_ODR_VALUE_4000);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_set_accel_bandwidth(test_ssi_driver_handle, SL_ICM40627_ACCEL_ODR_VALUE_4000);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 set acceleration bandwidth successfully \n");

  UnityPrintf("ICM40627 set acceleration bandwidth completed \n");
}

/*******************************************************************************
 * Function to test set gyroscope bandwidth for ICM40627
 ******************************************************************************/
void test_icm40627_set_gyro_bandwidth(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 set gyroscope bandwidth \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = sl_si91x_icm40627_set_gyro_bandwidth(NULL, SL_ICM40627_GYRO_ODR_VALUE_4000);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = sl_si91x_icm40627_set_gyro_bandwidth(test_ssi_driver_handle, SL_ICM40627_GYRO_ODR_VALUE_4000);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 set gyroscope bandwidth successfully \n");

  UnityPrintf("ICM40627 set gyroscope bandwidth completed \n");
}

/*******************************************************************************
 * Function to test select register bank of ICM40627
 ******************************************************************************/
void test_icm40627_select_register_bank(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 select register bank \n");
  sl_status_t status;

  UnityPrintf("Testing with null parameter \n");
  status = icm40627_select_register_bank(NULL, SL_ICM40627_REG_GYRO_ACCEL_CONFIG0);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
  UnityPrintf("Testing with null parameter successfully \n");

  status = icm40627_select_register_bank(test_ssi_driver_handle, SL_ICM40627_REG_GYRO_ACCEL_CONFIG0);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 select register bank successfully \n");

  UnityPrintf("ICM40627 select register bank completed \n");
}

/*******************************************************************************
 * Function to test deinit ICM40627
 ******************************************************************************/
void test_icm40627_deinit(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing ICM40627 deinit \n");
  sl_status_t status;

  status = sl_si91x_icm40627_deinit();
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, ICM40627 deinit successfully \n");

  UnityPrintf("ICM40627 deinit completed \n");
}
