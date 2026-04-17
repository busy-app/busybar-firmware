/*******************************************************************************
 * @file  mic_example.c
 * @brief
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
/**============================================================================
 * @section Description
 * This file contains example code to demonstrate the mic functionality
 ============================================================================**/

// Include Files
#include "mic_example.h"
#include "rsi_ccp_user_config.h"
#include "rsi_debug.h"

#include "sl_si91x_mic.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#define MIC_N_CHANNELS         2   // Number of Mic channels
#define MIC_SAMPLE_BUFFER_SIZE 512 // Mic buffer size to collect mic samples

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
static float spl_0 = 0;
static float spl_1 = 0;
static int16_t buffer[MIC_SAMPLE_BUFFER_SIZE * MIC_N_CHANNELS];
static uint16_t sampling_frequency = 44100;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
* Initialize mic example.
******************************************************************************/
void mic_init(void)
{
  sl_status_t status;
  // Initialize the microphone module
  status = sl_si91x_mic_init(sampling_frequency, MIC_N_CHANNELS);
  if (status != SL_STATUS_OK) {
    // Print error message if mic initialization fails
    DEBUGOUT("Microphone initialization failed");
    return;
  } else {
    DEBUGOUT("Microphone initialized successfully.\r\n");
  }
}

/***************************************************************************/ /**
* Mic ticking function.
******************************************************************************/
void mic_process_action(void)
{
  sl_status_t status;
  uint8_t n_channels = MIC_N_CHANNELS;

  // Get the required number of samples from the microphone
  status = sl_si91x_mic_get_n_samples(buffer, MIC_SAMPLE_BUFFER_SIZE);
  if (status != SL_STATUS_OK) {
    // Print error message if sample retrieval fails
    DEBUGOUT("Failed to retrieve microphone samples");
  }

  while (!sl_si91x_mic_sample_buffer_ready()) {
    // Wait until the sample buffer is ready
  }

  // Calculate the sound level from the retrieved samples
  sl_si91x_mic_calculate_sound_level(&spl_0, (int32_t *)buffer, MIC_SAMPLE_BUFFER_SIZE, 0);
  if (n_channels == 2) {
    sl_si91x_mic_calculate_sound_level(&spl_1, (int32_t *)buffer, MIC_SAMPLE_BUFFER_SIZE, 1);
    DEBUGOUT("Sound level [dB]: %.2f %.2f \r\n", spl_0, spl_1);
  } else {
    DEBUGOUT("Sound level [dB]: %.2f \r\n", spl_0);
  }

  // Stop the microphone from capturing further audio
  sl_si91x_mic_stop();
}
