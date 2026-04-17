/***************************************************************************/
/**
 * @file led_test_main.c
 * @brief LED Unity Test Functions
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
#include "sl_si91x_led.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define TEST_LED0_PORT   4  // LED0 port number
#define TEST_LED0_PIN    2  // LED0 pin number
#define TEST_LED0_NUMBER 0  //LED instance number
#define TEST_LED1_PORT   0  // LED1 port number
#define TEST_LED1_PIN    10 // LED1 pin number
#define TEST_LED1_NUMBER 1  //LED instance number

/*******************************************************************************
 ***************************  Local Variables   *******************************
 ******************************************************************************/
sl_led_t const led0_instance = {
  .port       = TEST_LED0_PORT,
  .pin        = TEST_LED0_PIN,
  .led_number = TEST_LED0_NUMBER,
};

sl_led_t const led1_instance = {
  .port       = TEST_LED1_PORT,
  .pin        = TEST_LED1_PIN,
  .led_number = TEST_LED1_NUMBER,
};

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_sl_si91x_led_toggle(void);
void test_sl_si91x_led_set(void);
void test_sl_si91x_led_clear(void);
void test_sl_si91x_led_init(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("LED");

  RUN_TEST(test_sl_si91x_led_init, __LINE__);
  RUN_TEST(test_sl_si91x_led_set, __LINE__);
  RUN_TEST(test_sl_si91x_led_clear, __LINE__);
  RUN_TEST(test_sl_si91x_led_toggle, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test toggling the LED
 ******************************************************************************/
void test_sl_si91x_led_toggle(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing LED Toggle \n");

  sl_si91x_led_toggle(led1_instance.pin);
  UnityPrintf("LED toggled successfully \n");

  sl_si91x_led_toggle(led0_instance.pin);
  UnityPrintf("LED toggled successfully \n");

  UnityPrintf("LED toggle completed \n");
}

/*******************************************************************************
 * Function to test setting the LED state to ON
 ******************************************************************************/
void test_sl_si91x_led_set(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing LED Set \n");

  sl_si91x_led_set(led0_instance.pin);
  UnityPrintf("LED set to ON successfully \n");

  sl_si91x_led_set(led1_instance.pin);
  UnityPrintf("LED set to ON successfully \n");

  UnityPrintf("LED set completed \n");
}

/*******************************************************************************
 * Function to test clearing the LED state to OFF
 ******************************************************************************/
void test_sl_si91x_led_clear(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing LED Clear \n");

  sl_si91x_led_clear(led0_instance.pin);
  UnityPrintf("LED set to OFF successfully \n");

  sl_si91x_led_clear(led1_instance.pin);
  UnityPrintf("LED set to OFF successfully \n");

  UnityPrintf("LED clear completed \n");
}

/*******************************************************************************
 * Function to test initializing the LED
 ******************************************************************************/
void test_sl_si91x_led_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing LED Init \n");

  sl_si91x_led_init(&led0_instance);
  UnityPrintf("LED initialized successfully \n");

  sl_si91x_led_init(&led1_instance);
  UnityPrintf("LED initialized successfully \n");

  UnityPrintf("LED init completed \n");
}
