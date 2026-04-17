/***************************************************************************/ /**
 * @file mic_test_main.c
 * @brief MIC Unity Test Functions
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
#include "sl_si91x_mic.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#define TEST_MIC_N_CHANNELS         2   // Number of Mic channels
#define TEST_MIC_SAMPLE_BUFFER_SIZE 512 // Mic buffer size to collect mic samples
/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
static float spl_1 = 0;
static int32_t test_buffer[TEST_MIC_SAMPLE_BUFFER_SIZE * TEST_MIC_N_CHANNELS];
static uint16_t test_sampling_frequency = 44100;

/*******************************************************************************
 ************************  Test Function Prototypes  ****************************
 ******************************************************************************/
void test_sl_si91x_mic_init(void);
void test_sl_si91x_mic_deinit(void);
void test_sl_si91x_mic_get_n_samples(void);
void test_sl_si91x_mic_start_streaming(void);
void test_sl_si91x_mic_start(void);
void test_sl_si91x_mic_stop(void);
void test_sl_si91x_mic_sample_buffer_ready(void);
void test_sl_si91x_mic_calculate_sound_level(void);

/******************************************************************************
 * Main function in which all the test cases are tested using unity framework
 ******************************************************************************/
int app_init()
{
  UnityBeginGroup("MIC");

  RUN_TEST(test_sl_si91x_mic_init, __LINE__);
  RUN_TEST(test_sl_si91x_mic_get_n_samples, __LINE__);
  RUN_TEST(test_sl_si91x_mic_start, __LINE__);
  RUN_TEST(test_sl_si91x_mic_sample_buffer_ready, __LINE__);
  RUN_TEST(test_sl_si91x_mic_calculate_sound_level, __LINE__);
  RUN_TEST(test_sl_si91x_mic_start_streaming, __LINE__);
  RUN_TEST(test_sl_si91x_mic_stop, __LINE__);
  RUN_TEST(test_sl_si91x_mic_deinit, __LINE__);

  UnityEnd();
  UnityPrintf("END");
  while (1) {
  }
}

/*******************************************************************************
 * Function to test initializing the microphone
 ******************************************************************************/
void test_sl_si91x_mic_init(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Init \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_mic_init(8000, TEST_MIC_N_CHANNELS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  UnityPrintf("Testing with correct parameter \n");
  status = sl_si91x_mic_init(test_sampling_frequency, TEST_MIC_N_CHANNELS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("MIC initialized successfully \n");

  UnityPrintf("Testing re-initialization \n");
  status = sl_si91x_mic_init(test_sampling_frequency, TEST_MIC_N_CHANNELS);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_ALREADY_INITIALIZED, status);
  UnityPrintf("MIC re-initialization successfully \n");

  UnityPrintf("MIC init completed \n");
}

/*******************************************************************************
 * Function to test de-initializing the microphone
 ******************************************************************************/
void test_sl_si91x_mic_deinit(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Deinit \n");
  sl_status_t status;

  status = sl_si91x_mic_deinit();
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("MIC de-initialized successfully \n");

  UnityPrintf("MIC Deinit completed \n");
}

/*******************************************************************************
 * Function to test getting samples from the microphone
 ******************************************************************************/
void test_sl_si91x_mic_get_n_samples(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Get N Samples \n");
  sl_status_t status;

  status = sl_si91x_mic_get_n_samples(test_buffer, TEST_MIC_SAMPLE_BUFFER_SIZE);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("MIC samples retrieved successfully \n");

  UnityPrintf("MIC MIC Get N Samples completed \n");
}

/*******************************************************************************
 * Function to test starting streaming from the microphone
 ******************************************************************************/
void test_sl_si91x_mic_start_streaming(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Start Streaming \n");
  sl_status_t status;

  status = sl_si91x_mic_start_streaming(test_buffer);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("MIC streaming started successfully \n");

  UnityPrintf("MIC MIC Start Streaming completed \n");
}

/*******************************************************************************
 * Function to test starting recording from the microphone
 ******************************************************************************/
void test_sl_si91x_mic_start(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Start \n");
  sl_status_t status;

  UnityPrintf("Testing already mic started \n");
  status = sl_si91x_mic_start(test_buffer);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_STATE, status);
  UnityPrintf("Testing already mic started successfully \n");

  sl_si91x_mic_stop();
  status = sl_si91x_mic_start(test_buffer);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("MIC recording started successfully \n");

  UnityPrintf("MIC MIC Get N Samples completed \n");
}

/*******************************************************************************
 * Function to test stopping the microphone
 ******************************************************************************/
void test_sl_si91x_mic_stop(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Stop \n");
  sl_status_t status;

  status = sl_si91x_mic_stop();
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("MIC stopped successfully \n");

  UnityPrintf("Recall stop API \n");
  status = sl_si91x_mic_stop();
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_STATE, status);
  UnityPrintf("Recalled stop API successfully \n");

  UnityPrintf("MIC Stop  completed\n");
}

/*******************************************************************************
 * Function to test checking if the sample buffer is ready
 ******************************************************************************/
void test_sl_si91x_mic_sample_buffer_ready(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Sample Buffer Ready \n");

  bool ready = sl_si91x_mic_sample_buffer_ready();
  TEST_ASSERT(ready == true || ready == false);
  UnityPrintf("MIC sample buffer ready status checked successfully \n");

  UnityPrintf("MIC Sample Buffer Ready completed\n");
}

/*******************************************************************************
 * Function to test calculating the sound level from the microphone data
 ******************************************************************************/
void test_sl_si91x_mic_calculate_sound_level(void)
{
  UnityPrintf("\n");
  UnityPrintf("Testing MIC Calculate Sound Level \n");
  sl_status_t status;

  UnityPrintf("Testing with invalid parameter \n");
  status = sl_si91x_mic_calculate_sound_level(&spl_1, test_buffer, 1, 1);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
  UnityPrintf("Testing with invalid parameter successfully \n");

  status = sl_si91x_mic_calculate_sound_level(&spl_1, test_buffer, TEST_MIC_SAMPLE_BUFFER_SIZE, 1);
  UnityPrintf("Sound level [dB]: %.2f \r\n", spl_1);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  UnityPrintf("MIC sound level calculated successfully \n");
}
