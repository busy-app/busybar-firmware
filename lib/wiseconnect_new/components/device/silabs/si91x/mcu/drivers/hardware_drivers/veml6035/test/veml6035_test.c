/***************************************************************************/ /**
 * @file
 * @brief VEML6035 driver test
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "unity.h"
#include "sl_status.h"
#include "sl_veml6035.h"
#include "sl_i2cspm_instances.h"
#include "sl_board_control.h"
#include "sl_sleeptimer.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"

#ifndef TEST_VEML6035_I2CSPM_INSTANCE
#define TEST_VEML6035_I2CSPM_INSTANCE sl_i2cspm_sensor
#endif

#if defined(__GNUC__)
__asm(".global _printf_float");
#endif

static void test_sensor_init(void);
static void test_sensor_measure_als(void);
static void test_sensor_measure_white_channel(void);

void tearDown(void)
{
  sl_led_turn_off(&sl_led_led0);
}

int main(void)
{

  sl_unity_start_test("VEML6035");

  RUN_TEST(test_sensor_init, __LINE__);
  RUN_TEST(test_sensor_measure_als, __LINE__);
  RUN_TEST(test_sensor_measure_white_channel, __LINE__);

  sl_unity_stop_test(); // never returns
}

static void test_sensor_init(void)
{
  sl_board_enable_sensor(SL_BOARD_SENSOR_LIGHT);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, sl_veml6035_init(TEST_VEML6035_I2CSPM_INSTANCE, true));
}

static void test_sensor_measure_als(void)
{
  float luxOff = 0.0f;
  float luxOn  = 0.0f;

  // Mesure lux with the led off
  sl_led_turn_off(&sl_led_led0);

  // Wait for at least one measurement
  sl_sleeptimer_delay_millisecond(150);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, sl_veml6035_get_als_lux(TEST_VEML6035_I2CSPM_INSTANCE, &luxOff));

  UnityPrintf("Measured lux led off value: %.2f \r\n", (double)luxOff);

  // Mesure lux with the led on
  sl_led_turn_on(&sl_led_led0);

  // Wait for at least one measurement
  sl_sleeptimer_delay_millisecond(150);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, sl_veml6035_get_als_lux(TEST_VEML6035_I2CSPM_INSTANCE, &luxOn));

  UnityPrintf("Measured lux led on value: %.2f \r\n", (double)luxOn);

  // Test if measured value with led on is equal or greater than with led off
  TEST_ASSERT_TRUE(luxOff <= luxOn);
  TEST_ASSERT_TRUE(luxOn > 0.0f);
}

static void test_sensor_measure_white_channel(void)
{
  float luxOn  = 0.0f;
  float luxOff = 0.0f;

  // Mesure lux with the led off
  sl_led_turn_off(&sl_led_led0);

  // Wait for at least one measurement
  sl_sleeptimer_delay_millisecond(150);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, sl_veml6035_get_white_lux(TEST_VEML6035_I2CSPM_INSTANCE, &luxOff));

  UnityPrintf("Measured lux led off value: %.2f \r\n", (double)luxOff);

  // Mesure lux with the led on
  sl_led_turn_on(&sl_led_led0);

  // Wait for at least one measurement
  sl_sleeptimer_delay_millisecond(150);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, sl_veml6035_get_white_lux(TEST_VEML6035_I2CSPM_INSTANCE, &luxOn));

  UnityPrintf("Measured lux led on value: %.2f \r\n", (double)luxOn);

  // Test if measured value with led on is greater than value with led off
  TEST_ASSERT_TRUE(luxOff + 0.1f < luxOn);
}
