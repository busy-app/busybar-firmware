#include "unity.h"
#include "sl_si91x_rgb_led.h"
#include "sl_sleeptimer.h"
#include "sl_si91x_rgb_led_instances.h"
#include "sl_si91x_clock_manager.h"

// Mock RGB LED structure
#ifndef RGB_LED
#define RGB_LED led_led0
#endif

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_init(void);
void test_sl_si91x_simple_rgb_led_on(void);
void test_sl_si91x_simple_rgb_led_off(void);
void test_sl_si91x_simple_rgb_led_toggle(void);
void test_sl_si91x_simple_rgb_led_set_colour(void);
void test_sl_si91x_simple_rgb_led_get_current_state(void);
void test_sl_si91x_simple_rgb_led_get_colour(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("RGB_LED");

  RUN_TEST(test_sl_si91x_simple_rgb_led_init, __LINE__);
  RUN_TEST(test_sl_si91x_simple_rgb_led_on, __LINE__);
  RUN_TEST(test_sl_si91x_simple_rgb_led_off, __LINE__);
  RUN_TEST(test_sl_si91x_simple_rgb_led_toggle, __LINE__);
  RUN_TEST(test_sl_si91x_simple_rgb_led_set_colour, __LINE__);
  RUN_TEST(test_sl_si91x_simple_rgb_led_get_current_state, __LINE__);
  RUN_TEST(test_sl_si91x_simple_rgb_led_get_colour, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test initializing the RGB LED
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing RGB LED Init \n");

  sl_si91x_simple_rgb_led_init(&RGB_LED);
  // Assuming there's a way to verify the initialization, add assertions here
  UnityPrintf("RGB LED initialized successfully \n");
}

/*******************************************************************************
 * Function to test turning on the RGB LED
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_on(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing RGB LED On \n");

  sl_si91x_simple_rgb_led_on(&RGB_LED);
  sl_si91x_delay_ms(100);
  // Assuming there's a way to verify the state change, add assertions here
  UnityPrintf("RGB LED turned on successfully \n");
}

/*******************************************************************************
 * Function to test turning off the RGB LED
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_off(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing RGB LED Off \n");

  sl_si91x_simple_rgb_led_off(&RGB_LED);
  sl_si91x_delay_ms(100);
  // Assuming there's a way to verify the state change, add assertions here
  UnityPrintf("RGB LED turned off successfully \n");
}

/*******************************************************************************
 * Function to test toggling the RGB LED
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_toggle(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing RGB LED Toggle \n");

  sl_si91x_simple_rgb_led_toggle(&RGB_LED);
  sl_si91x_delay_ms(100);
  // Assuming there's a way to verify the state change, add assertions here
  UnityPrintf("RGB LED toggled successfully \n");
}

/*******************************************************************************
 * Function to test setting the color of the RGB LED
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_set_colour(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing RGB LED Set Colour \n");
  int rgb_colour = 0xFF0000; // red color
  uint16_t red, green, blue;

  sl_si91x_simple_rgb_led_set_colour(&RGB_LED, rgb_colour);
  sl_si91x_delay_ms(100);
  sl_si91x_simple_rgb_led_get_colour(&RGB_LED, &red, &green, &blue);
  TEST_ASSERT_GREATER_THAN(500, red);
  UnityPrintf("red colour value %d\n", red);

  UnityPrintf("RGB LED color set successfully \n");
}

/*******************************************************************************
 * Function to test getting the current state of the RGB LED
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_get_current_state(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing RGB LED Get Current State \n");

  uint8_t state = sl_si91x_simple_rgb_led_get_current_state(&RGB_LED);
  TEST_ASSERT(state == 0 || state == 1);
  UnityPrintf("RGB LED current state retrieved successfully \n");
}

/*******************************************************************************
 * Function to test getting the current color of the RGB LED
 ******************************************************************************/
void test_sl_si91x_simple_rgb_led_get_colour(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing RGB LED Get Colour \n");
  uint16_t red, green, blue;
  int rgb_colour = 0xFF0000; // Red
  sl_si91x_simple_rgb_led_set_colour(&RGB_LED, rgb_colour);

  sl_si91x_simple_rgb_led_get_colour(&RGB_LED, &red, &green, &blue);
  TEST_ASSERT_GREATER_THAN(500, red);
  UnityPrintf("red colour value %d\n", red);
  rgb_colour = 0x00FF00; // Green
  sl_si91x_simple_rgb_led_set_colour(&RGB_LED, rgb_colour);

  sl_si91x_simple_rgb_led_get_colour(&RGB_LED, &red, &green, &blue);
  TEST_ASSERT_GREATER_THAN(500, green);

  UnityPrintf("green colour value %d\n", green);
  rgb_colour = 0x0000FF; // Blue
  sl_si91x_simple_rgb_led_set_colour(&RGB_LED, rgb_colour);

  sl_si91x_simple_rgb_led_get_colour(&RGB_LED, &red, &green, &blue);
  TEST_ASSERT_GREATER_THAN(500, blue);

  UnityPrintf("blue colour value %d\n", blue);

  UnityPrintf("RGB LED color values retrieved successfully \n");
}