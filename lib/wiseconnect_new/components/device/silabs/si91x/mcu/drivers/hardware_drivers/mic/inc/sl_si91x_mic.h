/***************************************************************************/
/**
 * @file sl_si91x_mic.h
 * @brief See @ref Mic for documentation.
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
#ifndef __SL_SI91X_MIC_H__
#define __SL_SI91X_MIC_H__

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"

/** 
 * @addtogroup Mic Mic
 * @ingroup SI91X_HARDWARE_DRIVER_APIS
 * @{
 */

/***************************************************************************/
/**
 * @brief  Initializes the I2S module and enables the mic.
 * 
 * @details This API initializes the I2S module, sets the operation modes, and enables the microphone.
 * 
 * @param[in] mic_sampling_frequency The sampling frequency for the microphone to ensure accurate audio capture.
 * @param[in] n_channels The number of channels for I2S mic.
 * 
 * @pre The I2S module will be initialized.
 * @post The mic will be enabled along with the I2S module.
 *****************************************************************************/
sl_status_t sl_si91x_mic_init(uint16_t mic_sampling_frequency, uint8_t n_channels);

/***************************************************************************/
/**
 * @brief  Deinitializes the mic and disables the I2S module.
 * 
 * @details This API disables the microphone and deinitializes the I2S module, freeing up any resources used by it.
 * 
 * 
 * @pre The mic and I2S module must be initialized before calling this function.
 * @post The mic and I2S module will be deinitialized and disabled.
 *****************************************************************************/
sl_status_t sl_si91x_mic_deinit(void);

/***************************************************************************/
/**
 * @brief  Retrieves the specified number of samples from the mic.
 * 
 * @details This API retrieves 'n' samples from the microphone and stores them in the provided buffer.
 * 
 * @param[in] buffer       Pointer to the buffer to store the samples.
 * @param[in] buffer_size  Size of the buffer in bytes.
 * 
 * @pre The mic must be initialized and started before calling this API.
 * @post The buffer will be filled with the specified number of samples.
 *****************************************************************************/
sl_status_t sl_si91x_mic_get_n_samples(void *buffer, uint32_t buffer_size);

/***************************************************************************/
/**
 * @brief  Starts streaming audio data from the mic.
 * 
 * @details This API begins streaming audio data and writes the samples to the provided buffer continuously.
 * 
 * @param[in] buffer  Pointer to the buffer where audio data will be stored.
 * 
 * @pre The mic must be initialized before streaming.
 * @post Audio streaming from the mic will begin, and the buffer will be continuously updated.
 *****************************************************************************/
sl_status_t sl_si91x_mic_start_streaming(void *buffer);

/***************************************************************************/
/**
 * @brief  Starts recording from the mic.
 * 
 * @details This API begins the process of capturing audio from the microphone and writes the captured data into the provided buffer.
 * 
 * @param[in] buffer  Pointer to the buffer where captured data will be stored.
 * 
 * @pre The mic must be initialized and ready before starting.
 * @post Audio data will be recorded and stored in the buffer.
 *****************************************************************************/
sl_status_t sl_si91x_mic_start(void *buffer);

/***************************************************************************/
/**
 * @brief  Stops the mic from capturing or streaming audio data.
 * 
 * @details This API halts the current audio capture or streaming process and stops the mic.
 * 
 * @pre The mic must be started before calling this API.
 * @post The mic will stop capturing or streaming audio data.
 *****************************************************************************/
sl_status_t sl_si91x_mic_stop(void);

/***************************************************************************/
/**
 * @brief  Checks if the mic sample buffer is ready.
 * 
 * @details This API checks if the sample buffer contains enough data or if it is ready for processing.
 * 
 * @pre The mic must be streaming or capturing data.
 * @post Returns true if the sample buffer is ready, otherwise returns false.
 *****************************************************************************/
bool sl_si91x_mic_sample_buffer_ready(void);

/***************************************************************************/
/**
 * @brief  Calculates the sound level from the mic data.
 * 
 * @details This API computes the sound level based on the samples stored in the provided buffer.
 * 
 * @param[out] sound_level_dB  Pointer to a variable that will store the calculated sound level.
 * @param[in]  audio_samples  Pointer to the buffer containing audio samples.
 * @param[in]  n_frames  The number of samples in the buffer.
 * @param[in]  channel  The I2S channel
 *
 * @pre The buffer must contain valid audio samples captured from the mic.
 * @post The calculated sound level will be stored in the variable pointed by
 *`sound_level`.
 *****************************************************************************/
sl_status_t sl_si91x_mic_calculate_sound_level(float *sound_level_dB,
                                               const int32_t *audio_samples,
                                               uint32_t n_frames,
                                               uint8_t channel);
/// @} end group Mic ********************************************************/
/***************************************************************************/
/**
 * @addtogroup Mic
 * @{
 *
 *  @details
 *
 *   @section Mic_Intro Introduction
 *
 * @details
 *
 * The Mic module provides APIs to initialize, deinitialize, and interact with
 * the microphone using the I2S interface. It supports functionalities such as
 * starting and stopping audio capture, streaming audio data, and calculating
 * sound levels from the captured audio samples.
 *
 * @section Mic_Configuration Configuration
 *
 * To configure the Mic module, you need to initialize it with the desired
 * sampling frequency and number of channels. This can be done using the
 * `sl_si91x_mic_init` API. Ensure that the I2S module is properly configured
 * before initializing the mic.
 * */
/// @} end group Mic ********************************************************/
#ifdef __cplusplus
}
#endif

#endif //__SL_SI91X_MIC_H__