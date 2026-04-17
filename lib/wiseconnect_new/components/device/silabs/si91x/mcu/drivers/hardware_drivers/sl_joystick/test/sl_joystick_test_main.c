/***************************************************************************/ /**
 * @file sl_joystick_test_main.c
 * @brief SL JOYSTICK Unity Test Functions
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
 ******************************************************************************
 ******************************************************************************/
#include "sl_si91x_joystick.h"
#include "rsi_debug.h"
#include "unity.h"

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_joystick_init(void);
void test_joystick_start(void);
void test_joystick_get_position(void);
void test_joystick_stop(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("JOYSTICK");
  RUN_TEST(test_joystick_init, __LINE__);
  RUN_TEST(test_joystick_start, __LINE__);
  RUN_TEST(test_joystick_get_position, __LINE__);
  RUN_TEST(test_joystick_stop, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test the initialization of Joystick
 ******************************************************************************/
void test_joystick_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Joystick Initialization \n");
  sl_status_t status;

  status = sl_si91x_joystick_init();
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Joystick Init successfully \n");

  UnityPrintf("Joystick Initialization test completed \n ");
}

/*******************************************************************************
 * Function to test start the joystick.
 ******************************************************************************/
void test_joystick_start(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Joystick start \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_joystick_start(SL_JOYSTICK_DISABLED);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ABORT, status);
  UnityPrintf("Status of API is correct, invalid parameter test successfully \n");

  UnityPrintf("Testing with valid parameter \n");
  status = sl_si91x_joystick_start(SL_JOYSTICK_ENABLED);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Joystick start successfully \n");

  UnityPrintf("Joystick start test completed \n ");
}

/*******************************************************************************
 * Function to test get the Joystick position/direction
 ******************************************************************************/
void test_joystick_get_position(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing to get the Joystick position/direction \n");
  sl_status_t status;
  sl_joystick_position_t pos;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_joystick_get_position(SL_JOYSTICK_DISABLED, &pos);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NOT_READY, status);
  UnityPrintf("Status of API is correct, invalid parameter test successfully \n");

  UnityPrintf("Testing with valid parameter \n");
  status = sl_si91x_joystick_get_position(SL_JOYSTICK_ENABLED, &pos);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Joystick start successfully \n");

  switch (pos) {
    case SL_JOYSTICK_NONE:
      UnityPrintf("Not Pressed\n");
      break;
    case SL_JOYSTICK_C:
      UnityPrintf("Center\n");
      break;
    case SL_JOYSTICK_N:
      UnityPrintf("North\n");
      break;
    case SL_JOYSTICK_S:
      UnityPrintf("South\n");
      break;
    case SL_JOYSTICK_E:
      UnityPrintf("East\n");
      break;
    case SL_JOYSTICK_W:
      UnityPrintf("West\n");
      break;
  }

  UnityPrintf("Get the Joystick position test completed \n ");
}

/*******************************************************************************
 * Function to test stop the joystick.
 ******************************************************************************/
void test_joystick_stop(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Joystick stop \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_joystick_stop(SL_JOYSTICK_ENABLED);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_BUSY, status);
  UnityPrintf("Status of API is correct, invalid parameter test successfully \n");

  UnityPrintf("Testing with valid parameter \n");
  status = sl_si91x_joystick_stop(SL_JOYSTICK_DISABLED);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("Status of API is correct, Joystick stop successfully \n");

  UnityPrintf("Joystick stop test completed \n ");
}
