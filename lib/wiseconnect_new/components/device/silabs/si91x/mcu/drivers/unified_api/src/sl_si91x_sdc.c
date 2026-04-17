/******************************************************************************
* @file  sl_si91x_sdc.c
* @brief SDC API implementation
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
******************************************************************************/

#include "sl_si91x_sdc.h"
#include "rsi_bod.h"
#include "aux_reference_volt_config.h"
#include "rsi_rtc.h"
#include "rsi_rom_ulpss_clk.h"
#include "sl_si91x_adc.h"

/********************************************************************************************************************************
**************************************************** Structures **********************************************************
******************************************************************************************************************************* */

typedef struct {
  uint16_t sdc_single_mode_offset;       //single ended mode offset
  uint16_t sdc_differential_mode_offset; //differential mode offset
  float sdc_single_mode_gain;            //single ended mode gain
  float sdc_differential_mode_gain;      //differential mode gain
} sl_si91x_sdc_calibration_data;

/********************************************************************************************************************************
**************************************************** Global variable **********************************************************
******************************************************************************************************************************* */

sl_si91x_sdc_channel_info_t sdc_channel_info = { 0 }; //Structure for storing channel info data

sl_si91x_sdc_calibration_data sdc_calibration_data = { 0 }; //Structure for storing calibration data

/********************************************************************************************************************************
********************************************************* Macros **************************************************************
******************************************************************************************************************************** */

#define sl_si91x_sdc_irqhandler IRQ025_Handler

/*****************************************************************************************************************************************
****************************************************Local Functions Prototypes *******************************************************
******************************************************************************************************************************************/

/**
 * @brief Set up adc neccessary configurations for sdc
 *
 * This function performs the necessary configurations of ADC hardware,
 * needed for SDC working
 *
 * @return
 *   - SL_STATUS_OK if the calibration was successful.
 *   - Appropriate error code otherwise.
 */
static sl_status_t sli_si91x_setup_adc(void);

/**
 * @brief Caliculates and stores calibration data for single ended and differential mode
 * 
 */
static void sli_si91x_sdc_calculate_offset_gain_data(void);

/**
 * @brief Processes the data with offset and gain calibration data.
 * 
 * @param data data that is to be calibrated
 * @param diff_en differential mode or not
 * @return int16_t calibrated data
 */
static int16_t sli_si91x_sdc_process_data(int16_t data, uint8_t diff_en);

/********************************************************************************************************************************
************************************************Function Prototypes************************************************************
********************************************************************************************************************************* */
/**
 * @brief Interrupt handler for the SDC
 *
 * This function handles interrupts generated by the Sensor Data Collector (SDC).
 * It should be called by the interrupt vector when an SDC-related interrupt occurs.
 *
 */
void sl_si91x_sdc_irqhandler(void);

/********************************************************************************************************************************
************************************************ Local Functions **************************************************************
********************************************************************************************************************************* */
/**
 * @brief  Configures ADC clock and Calibrates the ADC (Analog-to-Digital Converter) on the device.

  * @details
      1. Configures the ADC power mode to ON.
      2. Configures the ULPSS clock for ADC operation.
      3. Sets the clock division factor for calibration to 4 MHz.
      4. Sets the analog reference voltage.
      5. Performs ADC calibration using the RSI_ADC_Calibration function.
 * 
 */
static sl_status_t sli_si91x_setup_adc(void)
{

  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_adc_set_power_mode(ADC_POWER_ON);
  if (status != SL_STATUS_OK) {
    return status;
  }

  RSI_ULPSS_AuxClkConfig(ULPCLK, ENABLE_STATIC_CLK, 4); //configure the ULPSS clock for ADC operation

  //Clock division factor for calibration,Calibrate ADC on 4MHZ clock
  RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP, 0, 4);

  //Set analog reference voltage/
  RSI_AUX_RefVoltageConfig((float)2.8, (float)3.2);
  // ADC Calibration
  RSI_ADC_Calibration();
  return status;
}

/**
 * @brief  Calculates and stores the offset and gain values for the SDC (Single Data Channel).
 * 
  * @details
  * This function performs the following steps:
  * 1. Reads the single-ended mode offset value from the eFuse and stores it
  *    in the `sdc_calibration_data.sdc_single_mode_offset`.
  * 2. Reads the differential mode offset value from the eFuse and stores it
  *    in the `sdc_calibration_data.sdc_differential_mode_offset`.
  * 3. Reads the single-ended mode gain value from the eFuse, extracts the
  *    integer and fractional parts, and stores the combined value in
  *    `sdc_calibration_data.sdc_single_mode_gain`.
  * 4. Reads the differential mode gain value from the eFuse, extracts the
  *    integer and fractional parts, and stores the combined value in
  *    `sdc_calibration_data.sdc_differential_mode_gain`.
 */
static void sli_si91x_sdc_calculate_offset_gain_data(void)
{
  uint32_t integer_val = 0;
  float frac           = 0;

  // Offset value and gain value read from efuse
  sdc_calibration_data.sdc_single_mode_offset = (uint16_t)RSI_IPMU_Auxadcoff_SeEfuse();

  sdc_calibration_data.sdc_differential_mode_offset = (uint16_t)RSI_IPMU_Auxadcoff_DiffEfuse();

  // Single ended gain
  integer_val = RSI_IPMU_Auxadcgain_SeEfuse();
  frac        = (float)((integer_val) & (0x3FFF)); // Extracting fractional part
  frac /= 1000;                                    // Convert to float
  sdc_calibration_data.sdc_single_mode_gain =
    ((float)(integer_val >> 14) + frac); // Extracting integer part and adding fractional part

  //  // Differential ended gain
  integer_val = RSI_IPMU_Auxadcgain_DiffEfuse();
  frac        = (float)((integer_val) & (0x3FFF)); // Extracting fractional part
  frac /= 1000;                                    // Convert to float
  sdc_calibration_data.sdc_differential_mode_gain =
    (((float)(integer_val >> 14)) + frac); // Extracting integer part and adding fractional part
}

/**
 * @brief  Processes the SDC data with offset and gain calibration.
 * @details
 *    1. Checks if the data is in the correct format by checking the 11th bit.
  *    2. If the data is in differential mode, it applies the differential mode offset and gain.
  *    3. If the data is in single-ended mode, it applies the single-ended mode offset and gain.
  *    4. Clamps the processed data to the maximum and minimum values defined by SDC_MAX_VALUE and SDC_MIN_VALUE.
  *    5. Adjusts the processed data to be centered around SDC_MID_VALUE.
  *    6. Ensures the processed data is in the correct format by checking the 11th bit again. 
 * 
 */
static int16_t sli_si91x_sdc_process_data(int16_t data, uint8_t diff_en)
{
  float sdc_sing_gain      = sdc_calibration_data.sdc_single_mode_gain;
  uint16_t sdc_sing_offset = sdc_calibration_data.sdc_single_mode_offset;
  float sdc_diff_gain      = sdc_calibration_data.sdc_differential_mode_gain;
  uint16_t sdc_diff_offset = sdc_calibration_data.sdc_differential_mode_offset;

  if (data & BIT(11)) {
    data = data & (int16_t)(ADC_MASK_VALUE);
  } else {
    data = data | BIT(11);
  }

  if (diff_en) {
    data = (int16_t)((data - sdc_diff_offset) * sdc_diff_gain);
  } else {
    data = (int16_t)((data - sdc_sing_offset) * sdc_sing_gain);
  }
  if (data > SDC_MAX_VALUE) {
    data = SDC_MAX_VALUE;
  }
  if (data <= SDC_MIN_VALUE) {
    data = SDC_MIN_VALUE;
  }
  if (data >= SDC_MID_VALUE) {
    data = data - SDC_MID_VALUE;
  } else {
    data = data + SDC_MID_VALUE;
  }

  if (data & BIT(11)) {
    data = data & (int16_t)(ADC_MASK_VALUE);
  } else {
    data = data | BIT(11);
  }

  return data;
}

/**************************************************************************************************************************************
******************************************************** Functions *******************************************************************
************************************************************************************************************************************** */

/**
 * @brief  IRQ handler for SDC peripheral
 * 
 * @details
 *     This function is called when an interrupt occurs in the SDC peripheral.
 *     calls the callback function registered by the user with the SDC driver.
 *     It clears the write pointer reset and interrupt status clear bits in the SDC registers.
 *     It also clears the SDC interrupt in the NPSS interrupt clear register.

 */

void sl_si91x_sdc_irqhandler(void)
{

  if (sdc_channel_info.callback_event != NULL) {
    sdc_channel_info.callback_event(0, SDC_EVENT_DATA_READY);
  }

  /*Reset the SDC FIFO write pointer */
  sl_si91x_sdc_fifo_reset();

  SDC->SDC_GEN_CONFIG_0_b.INTR_STATUS_CLEAR = 1;
  /*Clear SDC interrupt */
  NPSS_INTR_CLEAR_REG = NPSS_TO_MCU_SDC_INTR;
}

/**
 @brief Initializes the SDC driver.
 
 @details
  * This function initializes the SDC driver by performing the following steps:
  * 1. Initializes the RTC block.
  * 2. Starts the RTC.   
  * 3. Initializes the SDC peripheral.
  * 4. Gets the chip voltage.
  * 5. Calibrates the ADC.
  * 6. Calculates the offset and gain values.
  * 7. Configures the SDC LDO voltage based on the reference voltage and
*/

sl_status_t sl_si91x_sdc_driver_init(float vref)
{
  sl_status_t status = SL_STATUS_OK;
  float chip_volt    = 0;

  /*Init the rtc block */
  RSI_RTC_Init(RTC);

  /*Start RTC */
  RSI_RTC_Start(RTC);

  // Initialize the SDC peripheral
  status = sl_si91x_sdc_init();
  if (status != SL_STATUS_OK) {
    return status; // Return error if initialization fails
  }

  NPSS_INTR_MASK_CLR_REG = NPSS_TO_MCU_SDC_INTR; // Unmask the SDC interrupt

  chip_volt = sl_si91x_adc_get_chip_voltage();

  sli_si91x_setup_adc(); // Setup ADC necessary for SDC

  sli_si91x_sdc_calculate_offset_gain_data();

  sl_si91x_sdc_ldo_enable(); // Enable the SDC LDO before configuring it
  sl_si91x_sdc_ldo_config(vref,
                          chip_volt); // Configure the SDC LDO voltage based on the reference voltage and chip voltage

  return status;
}

/**
 * @brief   Deinitializes the SDC driver.
 * 
 * @details
 * This function performs the following steps:
 * 1. Stops the RTC to ensure no further operations are performed on it.
 * 2. Disables the SDC NVIC interrupt to prevent further interrupts.
 * 3. Deinitializes the SDC peripheral.
 * 4. Disables the SDC LDO to save power.
 * 5. Resets the global variable sdc_channel_info to zero.
 * 6. Resets the global variable sdc_calibration_data to zero.

 */
sl_status_t sl_si91x_sdc_driver_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;

  RSI_RTC_Stop(RTC); // Stop the RTC

  NVIC_DisableIRQ(NPSS_TO_MCU_SDC_INTR_IRQn); // Disable the SDC NVIC interrupt

  sl_si91x_sdc_ldo_disable(); // Disable the SDC LDO

  status = sl_si91x_sdc_deinit(); // Deinitialize the SDC peripheral
  if (status != SL_STATUS_OK) {
    return status; // Return error if deinitialization fails
  }

  memset(&sdc_channel_info, 0, sizeof(sdc_channel_info)); //Make global variable sdc_channel_info zero

  memset(&sdc_calibration_data, 0, sizeof(sdc_calibration_data)); //Make global variable sdc_channel_info zero

  return status; // Return the status of the deinitialization
}

/**
 * @brief  Initializes the SDC driver with the provided configuration.
 * 
 * @details
 * This function performs the following steps:
  * 1. Checks if the provided configuration pointer is NULL and returns an error if it is.
  * 2. Decrements the sample threshold and trigger event select values in the configuration to match the expected values in the SDC register.
  * 3. Configures the SDC with the provided configuration using the `sl_si91x_sdc_config` function.
  * 4. Returns the status of the configuration operation.
*/

sl_status_t sl_si91x_sdc_driver_config(sl_si91x_sdc_config_t *sl_sdc_config)
{

  sl_status_t status = SL_STATUS_OK;

  if (sl_sdc_config == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (sl_sdc_config->sample_threshold == 0 || sl_sdc_config->sampling_interval == 0) {
    return SL_STATUS_INVALID_PARAMETER; // Return error if sample threshold or sampling interval is zero
  }
  sl_sdc_config
    ->sample_threshold--; // Decrement the sample threshold by 1 to match the expected value in the SDC register

  sl_sdc_config
    ->sampling_interval--; // Decrement the trigger event select by 1 to match the expected value in the SDC register

  // Configure the SDC channel with the provided configuratio
  status = sl_si91x_sdc_config(sl_sdc_config);
  if (status != SL_STATUS_OK) {
    return status; // Return error if configuration fails
  }

  return status;
}

/** 
 * @brief Configures the SDC ADC pins and initializes the channel information.
  *
  * @details
  * This function performs the following steps:
  * 1. Checks if the provided pin configuration and channel configuration pointers are NULL and returns an error if they are.
  * 2. Configures the SDC ADC pins using the `sl_si91x_sdc_adc_pin_config` function with the provided pin configuration and the number of channels.
  * 3. Iterates through each channel and initializes the channel information structure with the provided settings:
  *    - Sets the sample length for each channel.
  *    - Sets the receive buffer for each channel.
  *    - Sets the differential mode and auto buffer reset bits in the channel info bit field.
  * 4. Returns the status of the configuration operation.
 */
sl_status_t sl_si91x_sdc_driver_channel_config(sl_si91x_sdc_adc_config_t *pin_config,
                                               sl_si91x_sdc_channel_info_t *sdc_channel_config,
                                               sl_si91x_sdc_channel_misc_config_t *misc_config)
{
  sl_status_t status = SL_STATUS_OK;

  if (pin_config == NULL || sdc_channel_config == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sdc_channel_info.channel_info_bit_field = 0; //Reset the info bit field variable

  // Configure the SDC ADC pins
  sl_si91x_sdc_adc_pin_config(pin_config, SDC_NUMBER_OF_CHANNELS);

  for (int i = 0; i < SDC_NUMBER_OF_CHANNELS; i++) {
    sdc_channel_info.sample_length[i] = sdc_channel_config->sample_length[i]; // Set the sample length for each channel
    sdc_channel_info.recieve_buffer[i] =
      sdc_channel_config->recieve_buffer[i]; // Set the receive buffer for each channel
    sdc_channel_info.channel_info_bit_field |=
      (misc_config[i].buffer_auto_reset << i)
      << SDC_AUTO_BUFFER_RESET_POS; // Set the auto buffer reset bit in the channel info bit field
    sdc_channel_info.channel_info_bit_field |=
      (pin_config[i].sdc_adc_mode << i)
      << SDC_DIFFERENTIAL_MODE_POS; // Set the differential mode bit in the channel info bit field
  }

  return status;
}

/**
 * @brief   Registers a callback function for SDC events.
  * @details
  * This function registers a user-provided callback function that will be called when an SDC event occurs.
  * These events can include data ready, sample complete, or sdc error
 */
sl_status_t sl_si91x_sdc_driver_register_callback(sl_sdc_callback_t callback_event)
{

  if (callback_event == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sdc_channel_info.callback_event = callback_event;
  return SL_STATUS_OK;
}

/**
 * @brief   Unregisters the SDC callback function.
 * 
 */
sl_status_t sl_si91x_sdc_driver_unregister_callback(void)
{
  sdc_channel_info.callback_event = NULL;
  return SL_STATUS_OK;
}

/**
 * @brief   Starts the SDC data reading process.
 * 
 */
sl_status_t sl_si91x_sdc_driver_read_data_start(void)
{

  sl_si91x_sdc_sample_enable(); // Enable SDC sampling

  return SL_STATUS_OK;
}

/**
 * @brief   Stops the SDC data sampling process.
 * 
 */

sl_status_t sl_si91x_sdc_driver_stop(void)
{

  sl_si91x_sdc_stop(); // Disable SDC sampling

  return SL_STATUS_OK;
}

/**
 * @brief   Stores the data from the SDC FIFO registers into the buffers user provided during channel config.
 * @details Uses the following steps:
  * 1. Reads the data from the SDC FIFO.
  * channel_id is decremented by 1 to match the zero-based index.
  * 2. Checks if the channel is in differential mode and processes the data accordingly.
  * 3. Stores the processed data in the channel's receive buffer.
  * 4. Increments the transfer count for the channel.
  * 5. Checks if the transfer count has reached the sample length for the channel.
  *    If it has, it calls the callback function with the SDC_EVENT_SAMPLE_COMPLETE event for that channel.
  * 6. If the channel is set to auto buffer reset, it resets the transfer count for the channel and clears the transfer complete bit in the channel info bit field.

 * 
 * @return sl_status_t 
 */
sl_status_t sl_si91x_sdc_driver_store_data(void)
{
  int16_t data            = 0;
  uint32_t channel_id     = 0;
  uint32_t transfer_count = 0;
  uint32_t diff_mode      = 0;

  for (uint32_t j = 0; j < SL_SDC_SAMPLING_THRESHOLD; j++) {
    sl_si91x_sdc_read_data(&data, j, &channel_id); //read data from SDC FIFO

    channel_id--; // Decrement channel_id by 1 to match the zero-based index

    diff_mode = (sdc_channel_info.channel_info_bit_field
                 & ((0x1 << channel_id) << SDC_DIFFERENTIAL_MODE_POS)); // Check if the channel is in differential mode
    data      = sli_si91x_sdc_process_data(data, diff_mode); // Process the data with offset and gain calibration
    if (!(sdc_channel_info.channel_info_bit_field & (0x1 << channel_id) << SDC_TRANSFER_COMPLETE_POS)) {
      transfer_count = sdc_channel_info.transfer_count[channel_id]; // Get the current transfer count for the channel

      sdc_channel_info.recieve_buffer[channel_id][transfer_count] =
        data; // Store the received data in the channel's receive buffer

      sdc_channel_info.transfer_count[channel_id]++; // Increment the transfer count for the channel

      // Check if the transfer count for the channel has reached the transfer length
      if (sdc_channel_info.transfer_count[channel_id] == sdc_channel_info.sample_length[channel_id]) {
        sdc_channel_info.callback_event(
          channel_id + 1,
          SDC_EVENT_SAMPLE_COMPLETE); // Call the callback with SDC_SAMPLE_COMPLETE event for the channel
        sdc_channel_info.channel_info_bit_field |= 0x1 << channel_id;
        if (sdc_channel_info.channel_info_bit_field & ((0x1 << channel_id) << SDC_AUTO_BUFFER_RESET_POS)) {
          sdc_channel_info.transfer_count[channel_id] = 0; // Reset transfer count for the channel
          sdc_channel_info.channel_info_bit_field &= ~((0x1 << channel_id) << SDC_TRANSFER_COMPLETE_POS);
          sdc_channel_info.callback_event(
            channel_id + 1,
            SDC_EVENT_BUFFER_RESET); // Call the callback with SDC_EVENT_BUFFER_RESET event for the channel
        }
      }
    } else {
      sdc_channel_info.callback_event(
        channel_id + 1,
        SDC_EVENT_BUFFER_FULL); // Call the callback with SDC_EVENT_BUFFER_FULL event for the channel
    }
  }

  return SL_STATUS_OK;
}
/**
 * @brief   Reads data from the SDC (Sensor Data Controller) driver for the specified channel.

 * @details This function reads data from the SDC FIFO registers for a specified channel and stores it in the provided buffer. 
 */

sl_status_t sl_si91x_sdc_driver_read_data(uint32_t channel, int16_t *buffer, uint8_t length)
{
  int16_t data        = 0;
  uint32_t diff_mode  = 0;
  uint8_t iterator    = 0;
  uint32_t channel_id = 0;

  if (buffer == NULL) {
    return SL_STATUS_NULL_POINTER; // Return error if the buffer is NULL
  }
  if (length == 0 || length > SDC_ONE_CH_MAX_FIFO_TH) {
    return SL_STATUS_INVALID_PARAMETER; // Return error if the length is zero
  }

  if (channel >= SDC_NUMBER_OF_CHANNELS) {
    return SL_STATUS_INVALID_PARAMETER; // Return error if the channel number is invalid
  }

  if (SDC_NUMBER_OF_CHANNELS * length > SL_SDC_SAMPLING_THRESHOLD) {
    return SL_STATUS_INVALID_PARAMETER; // Return error if the total number of samples exceeds the sampling threshold
  }

  for (uint8_t j = 0; j < SL_SDC_SAMPLING_THRESHOLD; j++) {
    sl_si91x_sdc_read_data(&data, j, &channel_id); //read data from SDC FIFO
    if (channel_id != channel) {
      continue; // Skip if the channel ID does not match the requested channel
    }
    channel_id--;
    diff_mode        = (sdc_channel_info.channel_info_bit_field & ((0x1 << channel_id) << SDC_DIFFERENTIAL_MODE_POS));
    data             = sli_si91x_sdc_process_data(data, diff_mode);
    buffer[iterator] = data; // Store the processed data in the provided buffer
    iterator++;              // Increment the iterator for the buffer
  }

  return SL_STATUS_OK; // Return success status
}
