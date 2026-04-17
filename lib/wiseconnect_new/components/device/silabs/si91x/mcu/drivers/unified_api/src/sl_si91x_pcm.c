/***************************************************************************/ /**
 * @file sl_si91x_pcm.c
 * @brief PCM driver APIs
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
#include "sl_si91x_pcm_config.h"
#include "sl_si91x_pcm.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_i2s_xfer_config_t pcm_xfer_config = { 0 };
static sl_i2s_signal_event_t user_callback  = NULL;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void callback_event_handler(uint32_t event);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * PCM initialization function
 ******************************************************************************/
sl_status_t sl_si91x_pcm_init(uint32_t pcm_instance, sl_i2s_handle_t *pcm_handle)
{
  sl_status_t status = SL_STATUS_OK; /* Initialize status to OK */

  do {
    /* Initialize I2S handle and store driver handle in pcm_driver_handle */
    status = sl_si91x_i2s_init(pcm_instance, pcm_handle);
    if (status != SL_STATUS_OK) {
      break;
    }
  } while (false);

  return status;
}

/*******************************************************************************
 * PCM de-initialization function
 ******************************************************************************/
sl_status_t sl_si91x_pcm_deinit(sl_i2s_handle_t *pcm_handle)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    /* Deinitialize I2S handle and clear the driver handle */
    status = sl_si91x_i2s_deinit(pcm_handle);
    if (status != SL_STATUS_OK) {
      break;
    }

  } while (false);

  return status;
}

/*******************************************************************************
 * PCM de-initialization function
 ******************************************************************************/
sl_status_t sl_si91x_pcm_deinit_v2(sl_i2s_handle_t pcm_handle)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    /* Deinitialize I2S handle and clear the driver handle */
    status = sl_si91x_i2s_deinit_v2(pcm_handle);
    if (status != SL_STATUS_OK) {
      break;
    }

  } while (false);

  return status;
}
/*******************************************************************************
 * PCM set configuration function
 ******************************************************************************/
sl_status_t sl_si91x_pcm_set_configuration(sl_i2s_handle_t pcm_handle,
                                           uint16_t pcm_sampling_frequency,
                                           uint16_t pcm_resolution,
                                           uint16_t mode)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    pcm_xfer_config.mode          = mode;
    pcm_xfer_config.protocol      = SL_PCM_PROTOCOL; /* Set I2S protocol type */
    pcm_xfer_config.sync          = SL_I2S_ASYNC;
    pcm_xfer_config.transfer_type = SL_I2S_TRANSMIT;

    /* Configure ARM full power mode for I2S */
    status = sl_si91x_i2s_configure_power_mode(pcm_handle, SL_I2S_FULL_POWER);
    if (status != SL_STATUS_OK) {
      break;
    }

    // Validate the provided sampling frequency
    if (pcm_sampling_frequency != SL_I2S_SAMPLING_RATE_8000 && pcm_sampling_frequency != SL_PCM_SAMPLING_RATE_11025
        && pcm_sampling_frequency != SL_PCM_SAMPLING_RATE_16000 && pcm_sampling_frequency != SL_I2S_SAMPLING_RATE_22050
        && pcm_sampling_frequency != SL_I2S_SAMPLING_RATE_24000) {
      status = SL_STATUS_INVALID_PARAMETER; // Set error status for invalid frequency
      break;                                // Exit the loop
    }

    // Validate the provided resolution
    if (pcm_resolution != SL_I2S_RESOLUTION_16 && pcm_resolution != SL_I2S_RESOLUTION_24
        && pcm_resolution != SL_I2S_RESOLUTION_32) {
      status = SL_STATUS_INVALID_PARAMETER; // Set error status for invalid frequency
      break;                                // Exit the loop
    }

    /* Initialize PCM transfer structure with the required configurations */
    pcm_xfer_config.resolution    = pcm_resolution;
    pcm_xfer_config.sampling_rate = pcm_sampling_frequency;

    if (pcm_resolution == SL_I2S_RESOLUTION_16) {
      pcm_xfer_config.data_size = SL_I2S_DATA_SIZE16; /* Set data size to 16 bits */
    } else if (pcm_resolution == SL_I2S_RESOLUTION_24 || pcm_resolution == SL_I2S_RESOLUTION_32) {
      pcm_xfer_config.data_size = SL_I2S_DATA_SIZE32; /* Set data size to 32 bits */
    }

    status = sl_si91x_i2s_config_transmit_receive(pcm_handle, &pcm_xfer_config);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }

  } while (false);

  return status;
}

/*******************************************************************************
 * Configure PCM transmit and send data on the Tx line.
 ******************************************************************************/
sl_status_t sl_si91x_pcm_transmit_data(sl_i2s_handle_t pcm_handle, const void *data, uint32_t size)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    /* Validate the I2S transfer configuration */
    pcm_xfer_config.transfer_type = SL_I2S_TRANSMIT; /* Configure I2S for transmit operation */
    status                        = sl_si91x_i2s_config_transmit_receive(pcm_handle, &pcm_xfer_config);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_INVALID_CONFIGURATION;
    }

    //Configure I2S transmit DMA channel
    status = sl_si91x_i2s_transmit_data(pcm_handle, data, size);
  } while (false);

  return status;
}

/*******************************************************************************
 * Configure PCM receive and receive data on the Rx line.
 ******************************************************************************/
sl_status_t sl_si91x_pcm_receive_data(sl_i2s_handle_t pcm_handle, const void *data, uint32_t size)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    /* Validate the I2S transfer configuration */
    pcm_xfer_config.transfer_type = SL_I2S_RECEIVE; /* Configure I2S for receive operation */
    status                        = sl_si91x_i2s_config_transmit_receive(pcm_handle, &pcm_xfer_config);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }
    //Configure I2S receive DMA channel
    status = sl_si91x_i2s_receive_data(pcm_handle, data, size);
  } while (false);

  return status;
}

/*******************************************************************************
 * To register the user event callback
 ******************************************************************************/
sl_status_t sl_si91x_pcm_register_event_callback(sl_i2s_handle_t pcm_handle, sl_i2s_signal_event_t callback_event)
{
  sl_status_t status = SL_STATUS_OK;

  // Validates the null pointer, if true returns error code
  if (callback_event == NULL) {
    status = SL_STATUS_NULL_POINTER;
    return status;
  }

  user_callback = callback_event;
  status        = sl_si91x_i2s_register_event_callback(pcm_handle, callback_event_handler);

  return status;
}

/*******************************************************************************
 * To un-register the user event callback
 ******************************************************************************/
sl_status_t sl_si91x_pcm_unregister_event_callback(sl_i2s_handle_t pcm_handle)
{
  sl_status_t status = SL_STATUS_OK;

  status = sl_si91x_i2s_unregister_event_callback(pcm_handle);
  return status;
}

/*******************************************************************************
 * To abort Tx/Rx transfers
 ******************************************************************************/
sl_status_t sl_si91x_pcm_end_transfer(sl_i2s_handle_t pcm_handle, sl_i2s_xfer_type_t abort_type)
{
  sl_status_t status = SL_STATUS_OK;

  status = sl_si91x_i2s_end_transfer(pcm_handle, abort_type);
  return status;
}

/*******************************************************************************
 * To get the transmit data count of PCM
 ******************************************************************************/
uint32_t sl_si91x_pcm_get_transmit_data_count(sl_i2s_handle_t pcm_handle)
{
  return sl_si91x_i2s_get_transmit_data_count(pcm_handle);
}

/*******************************************************************************
 * To get the receive data count of PCM
 ******************************************************************************/
uint32_t sl_si91x_pcm_get_receive_data_count(sl_i2s_handle_t pcm_handle)
{
  return sl_si91x_i2s_get_receive_data_count(pcm_handle);
}

/*******************************************************************************
 * To get the transfer status PCM
 ******************************************************************************/
sl_i2s_status_t sl_si91x_pcm_get_status(sl_i2s_handle_t pcm_handle)
{
  return sl_si91x_i2s_get_status(pcm_handle);
}

/*******************************************************************************
 * Callback event function
 * It is responsible for the event which are triggered by GSPI interface
 * It updates the respective member of the structure as the event is triggered.
 * @param event - interrupt trigger event
 * @return none
 ******************************************************************************/
static void callback_event_handler(uint32_t event)
{
  switch (event) {
    case SL_I2S_SEND_COMPLETE:
      break;
    case SL_I2S_RECEIVE_COMPLETE:
      break;
    case SL_I2S_TX_UNDERFLOW:
      break;
    case SL_I2S_RX_OVERFLOW:
      break;
    case SL_I2S_FRAME_ERROR:
      break;
  }
  user_callback(event);
}
