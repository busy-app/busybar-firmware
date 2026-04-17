/***************************************************************************/ /**
 * @file button_test_main.c
 * @brief Button Unity Test Functions
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
#include "sl_si91x_button.h"

/*******************************************************************************
 ***************************  Local Variables   *******************************
 ******************************************************************************/
// Mock button configuration
sl_button_t mock_button = { .pin           = 7,
                            .port          = 0,
                            .button_number = 0,
#ifdef SL_BUTTON_BTN0_PAD
                            .pad = 0,
#endif
                            .interrupt_config = RISE_EDGE_AND_FALL_EDGE_INTERRUPT };
uint8_t mock_pin   = 1;
uint8_t mock_state = BUTTON_PRESSED;
#define ULP       4
#define UULP_VBAT 5

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_button_init(void);
void test_button_set_state(void);
void test_button_isr(void);
void test_button_toggle_state(void);
void test_button_state(void);
void test_button_get_state(void);
void test_button_deinit(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("BUTTON");

  RUN_TEST(test_button_init, __LINE__);
  RUN_TEST(test_button_set_state, __LINE__);
  RUN_TEST(test_button_state, __LINE__);
  RUN_TEST(test_button_isr, __LINE__);
  RUN_TEST(test_button_toggle_state, __LINE__);
  RUN_TEST(test_button_get_state, __LINE__);
  RUN_TEST(test_button_deinit, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test initializing the button
 ******************************************************************************/
void test_button_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Button Init \n");

  UnityPrintf("Testing with correct parameter \n");
  sl_si91x_button_init(&mock_button);
  UnityPrintf("Button initialized successfully \n");

  UnityPrintf("Button init completed \n");
}

/*******************************************************************************
 * Function to test the button set state.
 ******************************************************************************/
void test_button_set_state(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Button set State \n");

  sl_si91x_button_set_state(mock_button.button_number, BUTTON_RELEASED);
  UnityPrintf("Button set released state successfully \n");

  sl_si91x_button_set_state(mock_button.button_number, mock_state);
  UnityPrintf("Button set pressed state successfully \n");

  UnityPrintf("Button set state completed \n");
}

/*******************************************************************************
 * Function to test the button ISR
 ******************************************************************************/
void test_button_isr(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Button ISR \n");

  sl_si91x_button_isr(mock_pin, mock_state);
  UnityPrintf("Button ISR handled successfully \n");

  sl_si91x_button_isr(mock_pin, BUTTON_RELEASED);
  UnityPrintf("Button ISR handled successfully \n");

  sl_si91x_button_isr(2, mock_state);
  UnityPrintf("Button ISR handled successfully \n");

  sl_si91x_button_isr(11, mock_state);
  UnityPrintf("Button ISR handled successfully \n");

  UnityPrintf("Button ISR completed \n");
}

/*******************************************************************************
 * Function to test the button state
 ******************************************************************************/
void test_button_state(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Button state \n");
  uint8_t state;

  state = sl_si91x_button_state(mock_button.pin, 0);
  UnityPrintf("Button state get successfully \n");
  UnityPrintf("Button state = %d \n", state);

  state = sl_si91x_button_state(mock_button.pin, 4);
  UnityPrintf("Button state get successfully \n");
  UnityPrintf("Button state = %d \n", state);

  state = sl_si91x_button_state(mock_button.pin, mock_button.port);
  UnityPrintf("Button state get successfully \n");
  UnityPrintf("Button state = %d \n", state);

  UnityPrintf("Button state completed \n");
}

/*******************************************************************************
 * Function to test the button toggling state
 ******************************************************************************/
void test_button_toggle_state(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Button Toggle State \n");

  sl_si91x_button_toggle_state(mock_button.button_number);
  UnityPrintf("Button toggle state successfully \n");

  sl_si91x_button_toggle_state(1);
  UnityPrintf("Button toggle state successfully \n");

  UnityPrintf("Button toggle state completed \n");
}

/*******************************************************************************
 * Function to test the button get state
 ******************************************************************************/
void test_button_get_state(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing Button get State \n");
  uint8_t status;
  status = sl_si91x_button_get_state(mock_button.button_number);
  UnityPrintf("Button get state successfully \n");
  UnityPrintf("Button status = %d \n", status);
  status = sl_si91x_button_get_state(1);
  UnityPrintf("Button get state successfully \n");
  UnityPrintf("Button status = %d \n", status);

  UnityPrintf("Button get state completed successfully \n");
}
/*******************************************************************************
 * Function to test the deinitilaize the button
 ******************************************************************************/
void test_button_deinit(void)
{
  sl_status_t status = sl_si91x_button_deinit(&mock_button);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("\n");
  UnityPrintf("Testing Button deinit\n");
  UnityPrintf("HP button deinit passed\n");
  mock_button.port = ULP;
  mock_button.pin  = 1;
  sl_si91x_button_init(&mock_button);
  status = sl_si91x_button_deinit(&mock_button);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("ULP button deinit passed\n");
  mock_button.port = UULP_VBAT;
  mock_button.pin  = 2;
  sl_si91x_button_init(&mock_button);
  status = sl_si91x_button_deinit(&mock_button);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("UULP button deinit passed\n");
  mock_button.port          = ULP;
  mock_button.button_number = 2;
  status = sl_si91x_button_deinit(&mock_button); //invalid handle since button number not initialized or not registerd.
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_HANDLE, status);
  UnityPrintf("Invalid ULP button deinit handled correctly\n");
}
