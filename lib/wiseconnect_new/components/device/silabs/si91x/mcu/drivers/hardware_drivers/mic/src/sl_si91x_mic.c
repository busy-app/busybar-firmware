/***************************************************************************/ /**
 * @file sl_si91x_mic.c
 * @brief Mic driver APIs
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
#include "sl_si91x_i2s.h"
#include "sl_si91x_i2s_config.h"
#include "sl_si91x_mic.h"
#include "sl_si91x_mic_config.h"
#include "sl_driver_gpio.h"
#include "sl_si91x_driver_gpio.h"
#include <math.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define I2S_INSTANCE               0     // I2S instance
#define DEFAULT_MIC_BUFFER_SIZE    2048  // Default receive buffer size
#define POWER_CALCULATION_CONSTANT 10.0f // Constant for dB conversion
#if SL_MIC_DATA_SIZE == 16
#define MAX_AMPLITUDE                  32767.0f // Maximum value for a 16-bit signed integer
#define REFERENCE_SOUND_PRESSURE_LEVEL 140.0f   // Reference dBSPL level
#elif SL_MIC_DATA_SIZE == 32
#define MAX_AMPLITUDE                  2147483647.0f // Maximum value for a 32-bit signed integer
#define REFERENCE_SOUND_PRESSURE_LEVEL 120.0f        // Reference dBSPL level
#endif
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_i2s_handle_t i2s_mic_driver_handle = NULL;
static uint8_t mic_sample_receive_complete   = false;
static sl_i2s_xfer_config_t mic_xfer_config  = { 0 };
typedef enum { RECEIVE_DATA, WAIT_STATE, INVALID_STATE } transfer_state_t;

/* Flags to indicate the microphone and I2S states */
static volatile bool reading_samples_to_buffer; /* Flag indicating if the buffer is being filled with samples */
static volatile bool streaming_in_progress;     /* Flag indicating if the microphone is streaming audio data */
static bool mic_running;                        /* Flag indicating if the microphone is currently running */
static bool mic_initialized;                    /* Flag indicating if the microphone driver has been initialized */

static uint32_t mic_sample_buffer_size = DEFAULT_MIC_BUFFER_SIZE; // Receive buffer size
static uint8_t num_channels;                                      // Number of channels for mic

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void mic_callback_event(uint32_t event);
static void mic_enable(bool enable);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * I2S example initialization function
 ******************************************************************************/
sl_status_t sl_si91x_mic_init(uint16_t mic_sampling_frequency, uint8_t n_channels)
{
  sl_status_t status = SL_STATUS_OK; /* Initialize status to OK */

  do {

    // Validate the provided sampling frequency
    if (mic_sampling_frequency != SL_I2S_SAMPLING_RATE_16000 && mic_sampling_frequency != SL_I2S_SAMPLING_RATE_22050
        && mic_sampling_frequency != SL_I2S_SAMPLING_RATE_24000 && mic_sampling_frequency != SL_I2S_SAMPLING_RATE_32000
        && mic_sampling_frequency != SL_I2S_SAMPLING_RATE_44100) {
      status = SL_STATUS_INVALID_PARAMETER; // Set error status for invalid frequency
      break;                                // Exit the loop
    }

    /* Check if the microphone is already initialized */
    if (mic_initialized) {
      status = SL_STATUS_ALREADY_INITIALIZED;
      break;
    }

    /* Initialize I2S microphone and store driver handle in i2s_mic_driver_handle */
    status = sl_si91x_i2s_init(I2S_INSTANCE, &i2s_mic_driver_handle);
    if (status != SL_STATUS_OK) {
      break;
    }

    /* Configure ARM full power mode for I2S */
    status = sl_si91x_i2s_configure_power_mode(i2s_mic_driver_handle, SL_I2S_FULL_POWER);
    if (status != SL_STATUS_OK) {
      break;
    }

    /* Register the user callback handler for I2S events */
    status = sl_si91x_i2s_register_event_callback(i2s_mic_driver_handle, mic_callback_event);
    if (status != SL_STATUS_OK) {
      break;
    }

    /* Enable the MIC */
    mic_enable(true);

    /* Initialize I2S transfer structure with the required configurations */
    mic_xfer_config.mode          = SL_I2S_MASTER;           /* Set I2S to master mode */
    mic_xfer_config.protocol      = SL_I2S_PROTOCOL;         /* Set I2S protocol type */
    mic_xfer_config.sampling_rate = mic_sampling_frequency;  /* Set sampling rate */
    mic_xfer_config.sync          = SL_I2S_ASYNC;            /* Set I2S mode to asynchronous */
    mic_xfer_config.transfer_type = SL_MIC_ICS43434_RECEIVE; /* Configure I2S for receive operation */
#if SL_MIC_DATA_SIZE == 16
    mic_xfer_config.resolution = SL_I2S_RESOLUTION_16; /* Set 16-bit resolution for audio */
    mic_xfer_config.data_size  = SL_I2S_DATA_SIZE16;   /* Set data size to 16 bits */
#elif SL_MIC_DATA_SIZE == 32
    mic_xfer_config.resolution = SL_I2S_RESOLUTION_32; /* Set 32-bit resolution for audio */
    mic_xfer_config.data_size  = SL_I2S_DATA_SIZE32;   /* Set data size to 32 bits */
#endif

    /* Validate the I2S transfer configuration */
    status = sl_si91x_i2s_config_transmit_receive(i2s_mic_driver_handle, &mic_xfer_config);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }

    /* Update driver parameters to indicate initialization */
    num_channels              = n_channels;
    reading_samples_to_buffer = false;
    streaming_in_progress     = false;
    mic_running               = false;
    mic_initialized           = true;

  } while (false);

  return status;
}

/*******************************************************************************
 * I2S example de-initialization function
 ******************************************************************************/
sl_status_t sl_si91x_mic_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    /* Deinitialize I2S microphone and clear the driver handle */
    status = sl_si91x_i2s_deinit((sl_i2s_handle_t *)i2s_mic_driver_handle);
    if (status != SL_STATUS_OK) {
      break;
    }

    /* Disable the MIC using the new helper function */
    mic_enable(false);

    /* Update driver parameters to indicate deinitialization */
    reading_samples_to_buffer = false;
    streaming_in_progress     = false;
    mic_running               = false;
    mic_initialized           = false;

  } while (false);

  return status;
}

/***************************************************************************/
/**
 *    Read samples from the microphone sample buffer
 ******************************************************************************/
sl_status_t sl_si91x_mic_get_n_samples(void *buffer, uint32_t buffer_size)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    if (!mic_initialized) {
      /* Return an error if the microphone is not initialized */
      status = SL_STATUS_NOT_INITIALIZED;
      break;
    }

    if (reading_samples_to_buffer || streaming_in_progress) {
      /* Return an error if the microphone is already reading or streaming */
      status = SL_STATUS_INVALID_STATE;
      break;
    }

    /* Set the microphone sample buffer size */
    mic_sample_buffer_size = buffer_size * num_channels;

    if (!mic_running) {
      /* Start the microphone if it's not already running */
      sl_si91x_mic_start(buffer);
    }

    /* Update driver parameters to indicate that samples are being read into the buffer */
    reading_samples_to_buffer = true;

  } while (false);

  return status;
}

/***************************************************************************/
/**
 *    Start streaming
 ******************************************************************************/
sl_status_t sl_si91x_mic_start_streaming(void *buffer)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    if (!mic_initialized) {
      /* Return error if the microphone is not initialized */
      status = SL_STATUS_NOT_INITIALIZED;
      break;
    }

    if (reading_samples_to_buffer || streaming_in_progress) {
      /* Return error if the microphone is currently reading samples or already streaming */
      status = SL_STATUS_INVALID_STATE;
      break;
    }

    if (!mic_running) {
      /* Start the microphone if it's not already running */
      sl_si91x_mic_start(buffer);
    }

    /* Update driver parameters to indicate that streaming is in progress */
    streaming_in_progress = true;

  } while (false);

  return status;
}

/***************************************************************************/
/**
 *    Starts the microphone
 ******************************************************************************/
sl_status_t sl_si91x_mic_start(void *buffer)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    if (!mic_initialized) {
      /* Return error if the microphone is not initialized */
      status = SL_STATUS_NOT_INITIALIZED;
      break;
    }

    if (mic_running) {
      /* Return error if the microphone is already running */
      status = SL_STATUS_INVALID_STATE;
      break;
    }

    /* Receive data from the I2S interface and store it in the provided buffer */
    status = sl_si91x_i2s_receive_data(i2s_mic_driver_handle, (int32_t *)buffer, mic_sample_buffer_size);
    if (status != SL_STATUS_OK) {
      /* Return the error status if data reception fails */
      break;
    }

    /* Update driver parameters to indicate that the microphone is running */
    mic_running = true;

  } while (false);

  return status;
}

/***************************************************************************/ /**
*    Stops the microphone
******************************************************************************/
sl_status_t sl_si91x_mic_stop(void)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    if (!mic_running) {
      /* Set error status if the microphone is not currently running */
      status = SL_STATUS_INVALID_STATE;
      break;
    }

    /* End the I2S data transfer and abort reception */
    status = sl_si91x_i2s_end_transfer(i2s_mic_driver_handle, SL_I2S_RECEIVE_ABORT);
    if (status != SL_STATUS_OK) {
      /* Break the loop if ending the transfer fails */
      break;
    }

    /* Update driver parameters to indicate that the microphone has stopped running */
    mic_running               = false; /* Set microphone state to not running */
    reading_samples_to_buffer = false; /* Indicate that samples are no longer being read to buffer */
    streaming_in_progress     = false; /* Indicate that streaming is no longer in progress */

  } while (false);

  return status;
}

/***************************************************************************/ /**
*    Checks if the sample buffer is ready
******************************************************************************/
bool sl_si91x_mic_sample_buffer_ready(void)
{
  if (mic_sample_receive_complete) {
    mic_sample_receive_complete = false; /* Reset the flag */
    return true;                         /* Buffer is ready if I2S receive operation is complete */
  }
  return false; /* Buffer is not ready */
}

/***************************************************************************/ /**
*    Calculates the dBSPL value for a channel from a sample buffer
******************************************************************************/
sl_status_t sl_si91x_mic_calculate_sound_level(float *sound_level_dB,
                                               const int32_t *audio_samples,
                                               uint32_t n_frames,
                                               uint8_t channel)
{
  float sample;
  float power_variance;
  float mean_amplitude;
  uint32_t sample_index;

  if (channel >= n_frames) {
    return SL_STATUS_INVALID_PARAMETER;
  }

#if SL_MIC_DATA_SIZE == 16
  const int16_t *samples = (const int16_t *)audio_samples;
#elif SL_MIC_DATA_SIZE == 32
  const int32_t *samples = (const int32_t *)audio_samples;
#endif

  /* Calculate mean of the audio samples in the buffer */
  mean_amplitude = 0.0f;
  for (sample_index = channel; sample_index < (n_frames * num_channels); sample_index++) {
    mean_amplitude += (float)samples[sample_index];
  }
  mean_amplitude /= (float)n_frames;

  /* Estimate variance (power) */
  power_variance = 0.0f;
  for (sample_index = channel; sample_index < (n_frames * num_channels); sample_index++) {
    sample = ((float)samples[sample_index] - mean_amplitude) / MAX_AMPLITUDE;
    power_variance += sample * sample;
  }
  power_variance /= (float)n_frames;

  /* Convert variance to dBSPL (decibels Sound Pressure Level) */
  *sound_level_dB = POWER_CALCULATION_CONSTANT * log10f(power_variance) + REFERENCE_SOUND_PRESSURE_LEVEL;

  return SL_STATUS_OK;
}

/*******************************************************************************
 * Callback event function
 * It is responsible for the event which are triggered by GSPI interface
 * It updates the respective member of the structure as the event is triggered.
 * @param event - interrupt trigger event
 * @return none
 ******************************************************************************/
static void mic_callback_event(uint32_t event)
{
  switch (event) {
    case SL_I2S_SEND_COMPLETE:
      break;
    case SL_I2S_RECEIVE_COMPLETE:
      mic_sample_receive_complete = true;
      break;
    case SL_I2S_TX_UNDERFLOW:
      break;
    case SL_I2S_RX_OVERFLOW:
      break;
    case SL_I2S_FRAME_ERROR:
      break;
  }
}

/*******************************************************************************
 * Function to enable or disable the microphone via GPIO
 ******************************************************************************/
static void mic_enable(bool enable)
{
  /* Set the MIC_ENABLE GPIO pin configuration */
  sl_si91x_gpio_pin_config_t sl_gpio_pin_config = { { SL_SI91X_MIC_ENABLE_PORT, SL_SI91X_MIC_ENABLE_PIN },
                                                    GPIO_OUTPUT };
  sl_gpio_set_configuration(sl_gpio_pin_config);

  /* Enable or disable the MIC by setting or clearing the GPIO pin */
  sl_gpio_t gpio_port_pin = { SL_SI91X_MIC_ENABLE_PORT, SL_SI91X_MIC_ENABLE_PIN };
  if (enable) {
    sl_gpio_driver_set_pin(&gpio_port_pin); // Enable MIC
  } else {
    sl_gpio_driver_clear_pin(&gpio_port_pin); // Disable MIC
  }
}
