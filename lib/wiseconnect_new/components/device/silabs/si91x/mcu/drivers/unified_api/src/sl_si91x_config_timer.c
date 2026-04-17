/******************************************************************************
* @file sl_si91x_config-timer.c
* @brief CONFIG TIMER API implementation
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_config_timer.h"
#include "sl_si91x_config_timer_config.h"
#include "rsi_rom_ct.h"
#include "rsi_rom_clks.h"
#include "clock_update.h"
/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define RESET_VALUE                  0              // Macro for reset value
#define MAX_VALID_BITS               15             // Macro for and/or event maximum valid bits
#define MAX_ADC_PIN_NUMBER           15             // maximum pin number value for ADC output
#define MAX_COUNT_VALUE_16BIT        65535          // maximum count value for 16-bit counter
#define OCU_RESET_VALUE              0xFFFFFFFF     // reset value for ocu configuration
#define CT_IRQHandler                IRQ034_Handler // renaming IRQ-handler for config-timer
#define CONFIG_TIMER_RELEASE_VERSION 0              // Config-timer Release version
#define CONFIG_TIMER_MAJOR_VERSION   0              // Config-timer SQA version
#define CONFIG_TIMER_MINOR_VERSION   1              // Config-timer Developer version
#define COUNTER_0_UP_DOWN_MASK       0x00000030     // Mask to isolate bits 4 and 5
#define COUNTER_1_UP_DOWN_MASK       0x00300000     // Mask to extract bits 21-20 (2 bits)
#define BIT_0_MASK                   0x1            // Mask to extract bit 0
/*******************************************************************************
 ***************************  Local TYPES  ********************************
 ******************************************************************************/
static sl_dma_callback_t ct_dma_callback; // DMA callback structure
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_config_timer_callback_t callback_function_ptr = NULL;
static void *callback_flags                             = NULL;
sl_dma_xfer_t ct_dma_transfer_channel_0;               // DMA transfer configuration for Channel 0
sl_dma_xfer_t ct_dma_transfer_channel_1;               // DMA transfer configuration for Channel 1
volatile uint8_t ct_dma_transfer_flag_channel_0   = 0; // Transfer flag for Channel 0
volatile uint8_t ct_dma_transfer_flag_channel_1   = 0; // Transfer flag for Channel 1
volatile uint32_t ct_dma_transfer_index_channel_0 = 0; // DMA transfer index Channel 0
volatile uint32_t ct_dma_transfer_index_channel_1 = 0; // DMA transfer index Channel 1
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t evaluate_config_params(sl_config_timer_config_t *config_handle_ptr, uint32_t *config_value);
static sl_status_t evaluate_ocu_params(sl_config_timer_ocu_config_t *ocu_config_handle_ptr, uint32_t *ocu_config_value);
static sl_status_t validate_counter_configuration(sl_counter_number_t counter_number);
static void ct_dma_transfer_complete_callback(uint32_t channel, void *data);
static void ct_dma_error_callback(uint32_t channel, void *data);
/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
 ***********************  Global function Prototypes *************************
 ******************************************************************************/
void IRQ034_Handler(void);
/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/

/*******************************************************************************
* @brief: Initializes config-timer output pins and configures clock
*
* @details:
* Enables CT clock as 16 MHZ
* Enables CT output pin GPIOs and enables their pad receive
*******************************************************************************/
void sl_si91x_config_timer_init(void)
{
  // Configures config timer output pins and enables clock
  RSI_CT_Init();
}

/*****************************************************************************
 * Initialize and configures DMA transfer
 * Registers DMA callback & allocates channel
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_set_dma_configuration(uint32_t *compare_values, uint32_t channel)
{
  sl_status_t status        = SL_STATUS_FAIL;
  sl_dma_init_t ct_dma_init = { .dma_number = CT_DMA_NUMBER };
  sl_dma_xfer_t *p_transfer = NULL;
  // Initialize DMA
  status = sl_si91x_dma_init(&ct_dma_init);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Register DMA callbacks
  ct_dma_callback.transfer_complete_cb = ct_dma_transfer_complete_callback;
  ct_dma_callback.error_cb             = ct_dma_error_callback;

  // Select which transfer structure to use based on the channel
  if (channel == CT_DMA_CHANNEL_0) {
    p_transfer = &ct_dma_transfer_channel_0;

    // Configure DMA transfer for Channel 0
    p_transfer->src_addr  = (uint32_t *)((uint32_t)&compare_values[ct_dma_transfer_index_channel_0]);
    p_transfer->dest_addr = (uint32_t *)((uint32_t)&CT->CT_OCU_COMPARE_NXT_REG);
  } else if (channel == CT_DMA_CHANNEL_8) {
    p_transfer = &ct_dma_transfer_channel_1;

    // Configure DMA transfer for Channel 1
    p_transfer->src_addr  = (uint32_t *)((uint32_t)&compare_values[ct_dma_transfer_index_channel_1]);
    p_transfer->dest_addr = (uint32_t *)((uint32_t)&CT->CT_OCU_COMPARE2_NXT_REG);
  }

  // Configure common DMA transfer parameters
  p_transfer->src_inc        = SL_TRANSFER_SRC_INC_32;   // Increment source address
  p_transfer->dst_inc        = SL_TRANSFER_DST_INC_NONE; // Fixed destination address
  p_transfer->xfer_size      = SL_TRANSFER_SIZE_32;      // Transfer size: 32 bits
  p_transfer->transfer_count = CT_DMA_TRANSFER_SIZE;     // Number of transfers
  p_transfer->transfer_type  = SL_DMA_MEMORY_TO_MEMORY;  // Memory to Memory transfer
  p_transfer->dma_mode       = SL_DMA_BASIC_MODE;        // Basic DMA mode
  p_transfer->signal         = 0;                        // No signal required

  // Allocate DMA channel
  status = sl_si91x_dma_allocate_channel(CT_DMA_NUMBER, &channel, CT_CHANNEL_PRIORITY);
  if (status != SL_STATUS_OK && status != SL_STATUS_DMA_CHANNEL_ALLOCATED) {
    return status;
  }
  // Register callbacks
  status = sl_si91x_dma_register_callbacks(CT_DMA_NUMBER, channel, &ct_dma_callback);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Enable DMA controller
  status = sl_si91x_dma_enable(CT_DMA_NUMBER);
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}

/*****************************************************************************
 * This API configures and starts a DMA transfer to automatically update
 * the OCU compare values for a specified Config Timer channel. The API handles
 * the selection of the appropriate DMA channel based on input parameters,
 * initializes the transfer parameters, and enables the DMA channel to begin the transfer.
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_dma_transfer(uint32_t *compare_values, uint32_t channel, sl_dma_xfer_t *dma_transfer)
{
  sl_status_t status = SL_STATUS_FAIL;
  if (channel == CT_DMA_CHANNEL_0) {
    // Configure DMA to start with the next value
    channel                = channel + 1;
    dma_transfer->src_addr = (uint32_t *)((uint32_t)&compare_values[ct_dma_transfer_index_channel_0]);
    ct_dma_transfer_index_channel_0++;
  } else if (channel == CT_DMA_CHANNEL_8) {
    // Configure DMA to start with the next value
    dma_transfer->src_addr = (uint32_t *)((uint32_t)&compare_values[ct_dma_transfer_index_channel_1]);
    ct_dma_transfer_index_channel_1++;
  }
  // Start DMA transfer
  status = sl_si91x_dma_transfer(CT_DMA_NUMBER, channel, dma_transfer);
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}

/*******************************************************************************
* @brief:set configurations for config-timer
*
* @details:
* Sets 16 bit timer mode
* Sets timer Periodic or one shot mode
* Sets counter direction
* Enables/disables Counter Buffer
* Enables/disables soft reset trigger
* Enables/disables counter trigger
* Enables/disables counter sync trigger
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_configuration(sl_config_timer_config_t *timer_config_ptr)
{
  uint32_t config_value;
  sl_status_t status;

  /* CONFIG_TIMER_UC is defined by default. when this macro (CONFIG_TIMER_UC) is defined, peripheral
    * configuration is directly taken from the configuration set in the universal configuration (UC).
    * if the application requires the configuration to be changed in run-time, undefined this macro
    * and change the peripheral configuration.
    */
#if (CONFIG_TIMER_UC == 1)
  timer_config_ptr = &ct_configuration;
#endif

  do {
    // Validates the null pointer, if true returns error code
    if (timer_config_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating counter-0 direction value
    if (timer_config_ptr->counter0_direction >= SL_COUNTER0_DIRECTION_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating counter-1 direction value
    if (timer_config_ptr->counter1_direction >= SL_COUNTER1_DIRECTION_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    // Evaluating timer-config parameters ORed value
    status = evaluate_config_params(timer_config_ptr, &config_value);
    if (status != SL_STATUS_OK) {
      break;
    }
    RSI_CT_SetControl(CT, config_value);
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Resets or clears configurations of config-timer
*
* @details:
* Sets 16 bit timer mode
* Sets periodic mode for both counters
* Disables counter0 & counter1 buffer
* Sets counter direction up 
*******************************************************************************/
void sl_si91x_config_timer_reset_configuration(void)
{
  uint32_t reset_value;
  reset_value = RESET_VALUE;
  RSI_CT_ClearControl(CT, reset_value);
}

/*******************************************************************************
* @brief:set config-timer's ocu mode configurations
*
* @details:
* Sets counter-0 & counter-1 output in ocu mode
* Activate or deactivates OCU DMA mode for counter-0 & counter-1
* Activate or deactivates channel sync with counter-0 & counter-1
* Sets whether 16-bits or 8-bits used in OCU mode for counter-0 & counter-1 
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_ocu_configuration(sl_config_timer_ocu_config_t *ocu_config_ptr)
{
  uint32_t ocu_config_value;
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if (ocu_config_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Evaluating ocu-config parameters ORed value
    status = evaluate_ocu_params(ocu_config_ptr, &ocu_config_value);
    if (status != SL_STATUS_OK) {
      break;
    }
    RSI_CT_OCUConfigSet(CT, ocu_config_value);
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Sets config-timer's OCU mode threshold values
*
* @details:
* Sets first & next threshold values for counter-0 & counter-1
* Activate or deactivates OCU DMA mode for counter-0 & counter-1
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_ocu_control(sl_config_timer_ocu_control_t *ocu_params)
{
  rsi_error_t error_status;
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validates the null pointer, if true returns error code
    if (ocu_params == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validates the null pointer, if true returns error code
    if (ocu_params->params == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Configuring threshold values for counter0 and counter1 outputs
    error_status = RSI_CT_OCUControl(CT,
                                     ocu_params->is_counter_number_1,
                                     ocu_params->is_dma_state_enabled,
                                     ocu_params->params,
                                     ocu_params->callback);
    // Validating RSI API return status
    if (error_status != RSI_OK) {
      status = SL_STATUS_INVALID_PARAMETER;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief:Resets config-timer OCU configurations
*
* @details:
* Resets counter-0 & counter-1 output in ocu mode
* Deactivate OCU DMA mode for counter-0 & counter-1 
* Indicates channel sync with counter-0 & counter-1
* Resets whether 16-bits or 8-bits used in OCU mode for counter-0 & counter-1 
*******************************************************************************/
void sl_si91x_config_timer_reset_ocu_configuration(void)
{
  RSI_CT_OCUConfigReset(CT, OCU_RESET_VALUE);
}

/*******************************************************************************
* @brief: Sets WFG mode configurations for config-timer
*
* @details:
* Selects output-0 toggle high
* Selects output-0 toggle low
* Selects output-1 toggle high
* Selects output-1 toggle low
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_wfg_configuration(sl_config_timer_wfg_config_t *wfg_config_ptr)
{
  sl_status_t status;
  rsi_error_t error_status;
  status = SL_STATUS_OK;
  do {
    // Validates the null pointer, if true returns error code
    if (wfg_config_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    error_status = RSI_CT_WFGControlConfig(CT, *wfg_config_ptr);
    if (error_status == ERROR_CT_INVALID_ARG) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief:sets match value for counter-0 or counter-1 ,as per counter number and ct MODE
*
* @details:
* It sets match-count value for respective counter
* If mode is 16-bit as per passed counter number it updates the match value
* For 16-bit mode it takes 16-bit match-value only, else through error
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_match_count(sl_config_timer_mode_t mode,
                                                  sl_counter_number_t counter_number,
                                                  uint32_t match_value)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validating counter number
    if (counter_number >= SL_COUNTER_NUMBER_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating counter mode
    if (mode >= SL_COUNTER_MODE_LAST) {
      status = SL_STATUS_INVALID_MODE;
      break;
    }
    if ((mode == SL_COUNTER_16BIT) && (match_value > MAX_COUNT_VALUE_16BIT)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    RSI_CT_SetMatchCount(CT, match_value, (boolean_t)mode, (boolean_t)counter_number);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: API to calculate and return the match value of the timer for desired time period
*******************************************************************************/
sl_status_t sl_si91x_config_timer_get_match_value(uint32_t time_period_in_us, uint32_t *match_value)
{
  uint32_t ct_base_clock;
  // Get CT base clock
  ct_base_clock = RSI_CLK_GetBaseClock(M4_CT);
  ct_base_clock /= 1000000;
  // Validate the time period by comparing with maximum count of 16-bit counter and the CT base clock.
  // For example, for a CT base clock of 32MHz, the maximum time period achieved is 2047us.
  // For a CT base clock of 180MHz, the maximum time period achieved is 364us.
  // The counter can be loaded multiple times if requires a higher time period.
  if (time_period_in_us > (MAX_COUNT_VALUE_16BIT / ct_base_clock)) {
    return SL_STATUS_INVALID_COUNT;
  }
  // Calculate match value
  *match_value = ct_base_clock * time_period_in_us; // time period in microseconds
  return SL_STATUS_OK;
}

/*******************************************************************************
* @brief:sets initial value for counter-0 or counter-1 ,as per ct MODE
*
* @details:
* For 16--bit mode counters, ored value of both initial value is passed to the register 
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_initial_count(sl_config_timer_mode_t mode,
                                                    uint32_t counter0_initial_value,
                                                    uint32_t counter1_initial_value)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validating counter mode
    if (mode >= SL_COUNTER_MODE_LAST) {
      status = SL_STATUS_INVALID_MODE;
      break;
    }
    // Validating count value for 16-bit counter0
    if ((mode == SL_COUNTER_16BIT) && (counter0_initial_value > MAX_COUNT_VALUE_16BIT)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // for clearing previous counter values
    sl_si91x_config_timer_reset_counter(SL_COUNTER_0);
    sl_si91x_config_timer_reset_counter(SL_COUNTER_1);
    // Setting initial count value for 16-bit mode counter0 & counter1
    if (mode == SL_COUNTER_16BIT) {
      uint32_t _ORed_value = (counter0_initial_value) | (counter1_initial_value << 16);
      RSI_CT_SetCount(CT, _ORed_value);
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief:sets wfg mode compare value for counter-0 or counter-1, as per CT mode
*
* @details:
* For 16--bit mode counters, ored value of both initial value is passed to the register 
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_wfg_compare_values(sl_counter_number_t counter_number,
                                                         sl_config_timer_ocu_params_t *ocu_params)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validates the null pointer, if true returns error code
    if (ocu_params == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating counter number
    if (counter_number >= SL_COUNTER_NUMBER_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    RSI_CT_WFGComapreValueSet(CT, (boolean_t)counter_number, ocu_params);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief:set config timer mode 
*
* @details:
* Sets 16 bit timer mode
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_mode(sl_config_timer_mode_t mode)
{
  sl_status_t status;
  // Validating timer mode
  if (mode >= SL_COUNTER_MODE_LAST) {
    status = SL_STATUS_INVALID_MODE;
  } else {
    // Setting ct mode
    RSI_CT_Config(CT, (boolean_t)mode);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
* @brief: Starts the Counter form software trigger register
*
* @details:
* Starts the particular counter as per counter number
*******************************************************************************/
sl_status_t sl_si91x_config_timer_start_on_software_trigger(sl_counter_number_t counter_number)
{
  sl_status_t status;
  // Validating counter number
  if (counter_number >= SL_COUNTER_NUMBER_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    // validate the counter configuration
    status = validate_counter_configuration(counter_number);
    if (status != SL_STATUS_OK) {
      return status;
    }
    RSI_CT_StartSoftwareTrig(CT, counter_number);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
* @brief: validate the counter configuration
*******************************************************************************/
static sl_status_t validate_counter_configuration(sl_counter_number_t counter_number)
{
  sl_status_t status = SL_STATUS_OK;

  // Read the value of the register
  uint32_t ctrl_set_reg_value = CT->CT_GEN_CTRL_SET_REG;
  uint32_t counter_reg_value  = CT->CT_COUNTER_REG; // Read the value of the counter register
  uint8_t counter_type        = (ctrl_set_reg_value & BIT_0_MASK);

  if (counter_type == SL_COUNTER_16BIT) {
    if (counter_number == SL_COUNTER_0) {
      // Extract the COUNTER_0_UP_DOWN field (bits 4-5) directly from the register
      uint32_t counter0_direction     = (ctrl_set_reg_value & COUNTER_0_UP_DOWN_MASK);
      uint16_t counter0_initial_value = (uint16_t)(counter_reg_value & 0xFFFF); // Extract lower 16 bits for Counter 0
      uint32_t counter0_match_value   = CT->CT_MATCH_REG_b.COUNTER_0_MATCH;

      // Check the direction based on the extracted value
      if (counter0_direction == COUNTER0_UP) {
        if (counter0_match_value < counter0_initial_value) {
          status = SL_STATUS_INVALID_PARAMETER;
        }
      } else if (counter0_direction == COUNTER0_DOWN) {
        if (counter0_match_value > counter0_initial_value) {
          status = SL_STATUS_INVALID_PARAMETER;
        }
      } else if (counter0_direction == COUNTER0_UP_DOWN) {
        if (counter0_match_value == counter0_initial_value) {
          status = SL_STATUS_INVALID_PARAMETER;
        }
      }
    }

    else if (counter_number == SL_COUNTER_1) {
      // Extract the COUNTER_1_UP_DOWN field (bits 21-20) directly from the register
      uint32_t counter1_direction = (ctrl_set_reg_value & COUNTER_1_UP_DOWN_MASK);
      uint16_t counter1_initial_value =
        (uint16_t)((counter_reg_value >> 16) & 0xFFFF); // Extract upper 16 bits for Counter 1
      uint32_t counter1_match_value = CT->CT_MATCH_REG_b.COUNTER_1_MATCH;

      // Check the direction based on the extracted value
      if (counter1_direction == COUNTER1_UP) {
        if (counter1_match_value < counter1_initial_value) {
          status = SL_STATUS_INVALID_PARAMETER;
        }
      } else if (counter1_direction == COUNTER1_DOWN) {
        if (counter1_match_value > counter1_initial_value) {
          status = SL_STATUS_INVALID_PARAMETER;
        }
      } else if (counter1_direction == COUNTER1_UP_DOWN) {
        if (counter1_match_value == counter1_initial_value) {
          status = SL_STATUS_INVALID_PARAMETER;
        }
      }
    }
  }
  return status;
}

/*******************************************************************************
* @brief: Resets the Count values, as per counter number
*
*******************************************************************************/
sl_status_t sl_si91x_config_timer_reset_counter(sl_counter_number_t counter_number)
{
  sl_status_t status;
  // Validating counter number
  if (counter_number >= SL_COUNTER_NUMBER_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    RSI_CT_PeripheralReset(CT, (boolean_t)counter_number);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
* @brief: gets count value for counter-0 or counter-1, as per counter number and ct mode
*
* @details:
* It reads counter register of respective counter (not match register)
*******************************************************************************/
sl_status_t sl_si91x_config_timer_get_count(sl_config_timer_mode_t mode,
                                            sl_counter_number_t counter_number,
                                            uint32_t *count_value)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validates the null pointer, if true returns error code
    if (count_value == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating counter number
    if (counter_number >= SL_COUNTER_NUMBER_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating counter mode
    if (mode >= SL_COUNTER_MODE_LAST) {
      status = SL_STATUS_INVALID_MODE;
      break;
    }
    *count_value = RSI_CT_GetCounter(CT, (boolean_t)counter_number, (boolean_t)mode);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Configures AND-event & OR-event for various CT-actions 
*
* @details:
* It configures one AND-event & one OR-event for select event of selected CT-action
* for both counters.
* Also selects the valid bits for AND-event & OR-event of action
*******************************************************************************/
sl_status_t sl_si91x_config_timer_configure_action_event(sl_config_action_event_t *event_config_handle)
{
  sl_status_t status;
  status             = SL_STATUS_OK;
  uint32_t and_value = 0;
  uint32_t or_value  = 0;

  do {
    // Validating the null pointer
    if (event_config_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating event value
    if ((event_config_handle->and_event_counter0 >= SL_CT_EVENT_LAST)
        || (event_config_handle->or_event_counter0 >= SL_CT_EVENT_LAST)
        || (event_config_handle->and_event_counter1 >= SL_CT_EVENT_LAST)
        || (event_config_handle->or_event_counter1 >= SL_CT_EVENT_LAST)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating valid bits value
    if ((event_config_handle->and_event_valid_bits_counter0 > MAX_VALID_BITS)
        || (event_config_handle->and_event_valid_bits_counter1 > MAX_VALID_BITS)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating counter action
    if (event_config_handle->action >= ACTION_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Evaluating and_value & or_value
    and_value = event_config_handle->and_event_counter0;
    and_value |= ((event_config_handle->and_event_valid_bits_counter0) << 8);
    and_value |= ((event_config_handle->and_event_counter1) << 16);
    and_value |= ((event_config_handle->and_event_valid_bits_counter1) << 24);

    or_value = event_config_handle->or_event_counter0;
    or_value |= ((event_config_handle->or_event_valid_bits_counter0) << 8);
    or_value |= ((event_config_handle->or_event_counter1) << 16);
    or_value |= ((event_config_handle->or_event_valid_bits_counter1) << 24);
    if ((event_config_handle->action == START)) {
      RSI_CT_StartEventConfig(CT, and_value, or_value);
      break;
    }
    if ((event_config_handle->action == STOP)) {
      RSI_CT_StopEventConfig(CT, and_value, or_value);
      break;
    }
    if ((event_config_handle->action == CONTINUE)) {
      RSI_CT_ContinueEventConfig(CT, and_value, or_value);
      break;
    }
    if ((event_config_handle->action == HALT)) {
      RSI_CT_HaltEventConfig(CT, and_value, or_value);
      break;
    }
    if ((event_config_handle->action == INCREMENT)) {
      RSI_CT_IncrementEventConfig(CT, and_value, or_value);
      break;
    }
    if ((event_config_handle->action == CAPTURE)) {
      RSI_CT_CaptureEventConfig(CT, and_value, or_value);
      break;
    }
    if ((event_config_handle->action == INTERRUPT)) {
      RSI_CT_InterruptEventConfig(CT, and_value, or_value);
      break;
    }
    if ((event_config_handle->action == OUTPUT)) {
      RSI_CT_OutputEventConfig(CT, and_value, or_value);
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: To select external input event for triggering selected timer-action such as
* start, stop, continue, halt, increment, capture, interrupt and output
*
* @details:
* todo
*******************************************************************************/
sl_status_t sl_si91x_config_timer_select_action_event(sl_config_timer_action_t action,
                                                      sl_config_timer_event_t select_event_counter0,
                                                      sl_config_timer_event_t select_event_counter1)
{
  sl_status_t status;
  status              = SL_STATUS_OK;
  uint32_t ored_value = 0;
  uint32_t counter1_event_value;
  counter1_event_value = ((uint32_t)select_event_counter1 << 16);
  ored_value           = (((uint32_t)select_event_counter0) | (counter1_event_value));
  do {
    // Validating counter action
    if (action >= ACTION_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating select-event value
    if ((select_event_counter0 >= SL_CT_EVENT_LAST) || (select_event_counter1 >= SL_CT_EVENT_LAST)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (action == START) {
      RSI_CT_StartEventSelect(CT, ored_value);
      break;
    }
    if (action == STOP) {
      RSI_CT_StopEventSelect(CT, ored_value);
      break;
    }
    if (action == CONTINUE) {
      RSI_CT_ContinueEventSelect(CT, ored_value);
      break;
    }
    if (action == HALT) {

      RSI_CT_HaltEventSelect(CT, ored_value);
      break;
    }
    if (action == INCREMENT) {
      RSI_CT_IncrementEventSelect(CT, ored_value);
      break;
    }
    if (action == CAPTURE) {
      RSI_CT_CaptureEventSelect(CT, ored_value);
      break;
    }
    if (action == INTERRUPT) {
      RSI_CT_InterruptEventSelect(CT, ored_value);
      break;
    }
    if (action == OUTPUT) {
      RSI_CT_OutputEventSelect(CT, ored_value);
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Registers callback of timer interrupt and enabling respective interrupts as
*        per selected interrupt flag
*
* @pre \ref sl_si91x_config_timer_init();\n
*      \ref sl_si91x_config_timer_set_configuration(), keep software trigger disable here
*      \ref sl_si91x_config_timer_unregister_timeout_callback(), if already registered for any interrupt
*
*******************************************************************************/
sl_status_t sl_si91x_config_timer_register_callback(sl_config_timer_callback_t on_config_timer_callback,
                                                    void *callback_flag_value,
                                                    sl_config_timer_interrupt_flags_t *interrupt_flags)
{
  sl_status_t status;
  status                     = SL_STATUS_OK;
  uint32_t flag_unmask_value = 0;
  do {
    // Validating the 'callback function pointer' parameter and pointer to interrupt_flag,
    // if they are NULL will return an error code
    if ((on_config_timer_callback == NULL) || (interrupt_flags == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the function pointer, if the parameters is not NULL then it
    // will return an busy error code
    if (callback_function_ptr != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    if (interrupt_flags->is_counter0_event_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_INTR_0_l;
    }
    if (interrupt_flags->is_counter0_fifo_full_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_FIFO_0_FULL_l;
    }
    if (interrupt_flags->is_counter0_hit_zero_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_COUNTER_0_IS_ZERO_l;
    }
    if (interrupt_flags->is_counter0_hit_peak_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_COUNTER_0_IS_PEAK_l;
    }
    if (interrupt_flags->is_counter1_event_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_INTR_1_l;
    }
    if (interrupt_flags->is_counter1_fifo_full_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_FIFO_1_FULL_l;
    }
    if (interrupt_flags->is_counter1_hit_zero_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_COUNTER_1_IS_ZERO_l;
    }
    if (interrupt_flags->is_counter1_hit_peak_interrupt_enabled) {
      flag_unmask_value |= RSI_CT_EVENT_COUNTER_1_IS_PEAK_l;
    }
    // Enabling Interrupt as per flags
    RSI_CT_InterruptEnable(CT, flag_unmask_value);
    // The function pointer is fed to the static variable, which will be called
    // in the IRQ handler
    callback_function_ptr = on_config_timer_callback;
    callback_flags        = callback_flag_value;
    // active_flags = flag_unmask_value;
    // Enabling NVIC
    NVIC_EnableIRQ(CT_IRQn);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Resumes halt operation of Config-timer counter
*
* @details:
* Starts the particular counter as per counter number
*******************************************************************************/
sl_status_t sl_si91x_config_timer_resume_halt_event(sl_counter_number_t counter_number)
{
  sl_status_t status;
  // Validating counter number
  if (counter_number >= SL_COUNTER_NUMBER_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    RSI_CT_ResumeHaltEvent(CT, (boolean_t)counter_number);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
* @brief: Reads count value on capture of Config-timer counter
*
* @details:
* It reads respective counter count at the time of capture event occurrence
*******************************************************************************/
sl_status_t sl_si91x_config_timer_read_capture(sl_counter_number_t counter_number, uint16_t *capture_value)
{
  sl_status_t status;
  // Validating the 'callback function pointer' parameter and pointer to interrupt_flag,
  // if they are NULL will return an error code
  if (capture_value == NULL) {
    status = SL_STATUS_NULL_POINTER;
    return status;
  }
  // Validating counter number
  if (counter_number >= SL_COUNTER_NUMBER_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }
  *capture_value = RSI_CT_CaptureRead(CT, counter_number);
  status         = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
* @brief: Sets counter sync with other channels
*
* @details:
* It sets respective counter output sync with respective channel,
* possible channel values 0 to 7
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_counter_sync(sl_counter_number_t counter_number, uint8_t sync_counter_value)
{
  sl_status_t status;
  // Validating counter number
  if (counter_number >= SL_COUNTER_NUMBER_LAST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    RSI_CT_SetCounerSync(CT, sync_counter_value, (boolean_t)counter_number);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * @breif: To sync counter output with other channels
 *
 * @pre \ref sl_si91x_config_timer_init();\n
 *      \ref sl_si91x_config_timer_set_configuration();\n
 *      \ref
 *
 * @param[in]  counter_number \ref sl_counter_number_t for possible values
 * @param[in]  sync_counter (uint8_t)
 *
 * @return
 *         \ref SL_STATUS_INVALID_PARAMETER 0x0021) - 'counter_number' parameter value is invalid.\n
 *         \ref SL_STATUS_OK (0x0000) - Success, count-value is set properly\n
*******************************************************************************/
sl_status_t sl_si91x_config_timer_set_output_adc_pin(uint8_t pin1, uint8_t pin2)
{
  sl_status_t status;
  // Validating pin numbers
  if ((pin1 > MAX_ADC_PIN_NUMBER) || (pin2 > MAX_ADC_PIN_NUMBER)) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    RSI_CT_OutputEventADCTrigger(CT_MUX_REG, pin1, pin2);
    status = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
* @brief: Unregisters Callback on timer expiration and disables interrupts

* @details:
* It will unregister the callback, i.e., clear the callback function address
  and pass NULL value to the variable

* It will also mask and disable the timer interrupt.
*******************************************************************************/
sl_status_t sl_si91x_config_timer_unregister_callback(sl_config_timer_interrupt_flags_t *interrupt_flags)
{
  sl_status_t status;
  uint32_t flag_mask_value = 0;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating the 'callback function pointer' parameter and pointer to interrupt_flag,
    // if they are NULL will return an error code
    if (interrupt_flags == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (interrupt_flags->is_counter0_event_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_INTR_0_l;
    }
    if (interrupt_flags->is_counter0_fifo_full_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_FIFO_0_FULL_l;
    }
    if (interrupt_flags->is_counter0_hit_zero_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_COUNTER_0_IS_ZERO_l;
    }
    if (interrupt_flags->is_counter0_hit_peak_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_COUNTER_0_IS_PEAK_l;
    }
    if (interrupt_flags->is_counter1_event_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_INTR_1_l;
    }
    if (interrupt_flags->is_counter1_fifo_full_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_FIFO_1_FULL_l;
    }
    if (interrupt_flags->is_counter1_hit_zero_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_COUNTER_1_IS_ZERO_l;
    }
    if (interrupt_flags->is_counter1_hit_peak_interrupt_enabled) {
      flag_mask_value |= RSI_CT_EVENT_COUNTER_1_IS_PEAK_l;
    }
    // Disabling passed timer instance interrupt
    RSI_CT_InterruptDisable(CT, flag_mask_value);
    //The callback should be null in the unregister callback api because
    //one cannot register the callback if it is not null
    callback_function_ptr = NULL;
    callback_flags        = NULL;
  } while (false);
  return status;
}

/*******************************************************************************
 * To get the release, sqa and dev version of Config-timer
 * It returns the structure for Config-timer version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_config_timer_version_t sl_si91x_config_timer_get_version(void)
{
  sl_config_timer_version_t version;
  version.release = CONFIG_TIMER_RELEASE_VERSION;
  version.major   = CONFIG_TIMER_MAJOR_VERSION;
  version.minor   = CONFIG_TIMER_MINOR_VERSION;
  return version;
}

/*******************************************************************************
* De-initializes config-timer output pins and configures clock
* Unregisters callback and disable all CT interrupts.
*
* @details:
* Disables CT clock
*******************************************************************************/
void sl_si91x_config_timer_deinit(void)
{
  sl_config_timer_interrupt_flags_t interrupt_flag;
  interrupt_flag.is_counter0_event_interrupt_enabled     = true;
  interrupt_flag.is_counter0_fifo_full_interrupt_enabled = true;
  interrupt_flag.is_counter0_hit_peak_interrupt_enabled  = true;
  interrupt_flag.is_counter0_hit_zero_interrupt_enabled  = true;
  interrupt_flag.is_counter1_event_interrupt_enabled     = true;
  interrupt_flag.is_counter1_fifo_full_interrupt_enabled = true;
  interrupt_flag.is_counter1_hit_peak_interrupt_enabled  = true;
  interrupt_flag.is_counter1_hit_zero_interrupt_enabled  = true;
  // Unregistering callback and disabling all CT interrupts
  sl_si91x_config_timer_unregister_callback(&interrupt_flag);
  // Disabling peripheral clock
  RSI_CLK_PeripheralClkDisable(M4CLK, CT_CLK);
}

/*****************************************************************************
 * De-initializes CT DMA
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_dma_deinit(uint32_t channel)
{
  sl_status_t status = SL_STATUS_FAIL;
  if (channel == CT_DMA_CHANNEL_0) {
    channel = (CT_DMA_CHANNEL_0 + 1);
  } else if (channel == CT_DMA_CHANNEL_8) {
    channel = (CT_DMA_CHANNEL_8 + 1);
  }
  // Stop DMA transfer
  status = sl_si91x_dma_stop_transfer(CT_DMA_NUMBER, channel);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Disables DMA
  status = sl_si91x_dma_channel_disable(CT_DMA_NUMBER, channel);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // De-init DMA
  status = sl_si91x_dma_deinit(CT_DMA_NUMBER);
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}

/*******************************************************************************
* Interrupt handler for config-timer
*
* @param none
* @return none
*******************************************************************************/
void CT_IRQHandler(void)
{
  do {
    // Clears interrupt
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_COUNTER_0_IS_PEAK_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_COUNTER_0_IS_PEAK_FLAG);
      *(uint32_t *)callback_flags = SL_CT_COUNTER_0_IS_PEAK_FLAG;
      break;
    }
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_COUNTER_1_IS_PEAK_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_COUNTER_1_IS_PEAK_FLAG);
      *(uint32_t *)callback_flags = SL_CT_COUNTER_1_IS_PEAK_FLAG;
      break;
    }
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_EVENT_INTR_0_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_EVENT_INTR_0_FLAG);
      *(uint32_t *)callback_flags = SL_CT_EVENT_INTR_0_FLAG;
      break;
    }
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_FIFO_0_FULL_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_FIFO_0_FULL_FLAG);
      *(uint32_t *)callback_flags = SL_CT_FIFO_0_FULL_FLAG;
      break;
    }
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_COUNTER_0_IS_ZERO_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_COUNTER_0_IS_ZERO_FLAG);
      *(uint32_t *)callback_flags = SL_CT_COUNTER_0_IS_ZERO_FLAG;
      break;
    }
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_EVENT_INTR_1_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_EVENT_INTR_1_FLAG);
      *(uint32_t *)callback_flags = SL_CT_EVENT_INTR_1_FLAG;
      break;
    }
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_FIFO_1_FULL_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_FIFO_1_FULL_FLAG);
      *(uint32_t *)callback_flags = SL_CT_FIFO_0_FULL_FLAG;
      break;
    }
    if (RSI_CT_GetInterruptStatus(CT) & SL_CT_COUNTER_1_IS_ZERO_FLAG) {
      RSI_CT_InterruptClear(CT, SL_CT_COUNTER_1_IS_ZERO_FLAG);
      *(uint32_t *)callback_flags = SL_CT_COUNTER_1_IS_ZERO_FLAG;
      break;
    }
  } while (false);
  // calling the callback function
  callback_function_ptr(callback_flags);
}

//---------------------------------------------------------------------------------
//Internal functions
/*******************************************************************************
* @brief:Function to calculate ct config register value 
*
*******************************************************************************/
static sl_status_t evaluate_config_params(sl_config_timer_config_t *config_handle_ptr, uint32_t *config_value)
{
  sl_status_t status;
  status        = SL_STATUS_OK;
  *config_value = 0;
  do {
    // Validates the null pointer, if true returns error code
    if (config_handle_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validates the null pointer, if true returns error code
    if (config_value == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Evaluating ORed value of timer parameters
    if (config_handle_ptr->is_counter0_soft_reset_enabled) {
      *config_value |= SL_COUNTER0_SOFT_RESET_ENABLE;
    }
    if (config_handle_ptr->is_counter0_periodic_enabled) {
      *config_value |= SL_COUNTER0_PERIODIC_ENABLE;
    }
    if (config_handle_ptr->is_counter0_trigger_enabled) {
      *config_value |= SL_COUNTER0_TRIGGER_ENABLE;
    }
    if (config_handle_ptr->is_counter0_sync_trigger_enabled) {
      *config_value |= SL_COUNTER0_SYNC_TRIGGER_ENABLE;
    }
    if (config_handle_ptr->is_counter0_buffer_enabled) {
      *config_value |= SL_COUNTER0_BUFFER_ENABLE;
    }
    if ((config_handle_ptr->counter0_direction) == SL_COUNTER0_UP) {
      *config_value |= SL_COUNTER0_UP_DIRECTION;
    }
    if ((config_handle_ptr->counter0_direction) == SL_COUNTER0_DOWN) {
      *config_value |= SL_COUNTER0_DOWN_DIRECTION;
    }
    if ((config_handle_ptr->counter0_direction) == SL_COUNTER0_UP_DOWN) {
      *config_value |= SL_COUNTER0_UP_DOWN_DIRECTION;
    }
    if (config_handle_ptr->is_counter1_soft_reset_enabled) {
      *config_value |= SL_COUNTER1_SOFT_RESET_ENABLE;
    }
    if (config_handle_ptr->is_counter1_periodic_enabled) {
      *config_value |= SL_COUNTER1_PERIODIC_ENABLE;
    }
    if (config_handle_ptr->is_counter1_trigger_enabled) {
      *config_value |= SL_COUNTER1_TRIGGER_ENABLE;
    }
    if (config_handle_ptr->is_counter1_sync_trigger_enabled) {
      *config_value |= SL_COUNTER1_SYNC_TRIGGER_ENABLE;
    }
    if (config_handle_ptr->is_counter1_buffer_enabled) {
      *config_value |= SL_COUNTER1_BUFFER_ENABLE;
    }
    if ((config_handle_ptr->counter1_direction) == SL_COUNTER1_UP) {
      *config_value |= SL_COUNTER1_UP_DIRECTION;
    }
    if ((config_handle_ptr->counter1_direction) == SL_COUNTER1_DOWN) {
      *config_value |= SL_COUNTER1_DOWN_DIRECTION;
    }
    if ((config_handle_ptr->counter1_direction) == SL_COUNTER1_UP_DOWN) {
      *config_value |= SL_COUNTER1_UP_DOWN_DIRECTION;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief:Function to calculate ct ocu config register value 
* 
*******************************************************************************/
static sl_status_t evaluate_ocu_params(sl_config_timer_ocu_config_t *ocu_config_handle_ptr, uint32_t *ocu_config_value)
{
  sl_status_t status;
  status            = SL_STATUS_OK;
  *ocu_config_value = 0;
  do {
    // Validates the null pointer, if true returns error code
    if (ocu_config_handle_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validates the null pointer, if true returns error code
    if (ocu_config_value == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (ocu_config_handle_ptr->is_counter0_ocu_output_enabled) {
      *ocu_config_value = SL_COUNTER0_OCU_OUTPUT_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter0_ocu_dma_enabled) {
      *ocu_config_value |= SL_COUNTER0_OCU_DMA_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter0_ocu_8bit_mode_enabled) {
      *ocu_config_value |= SL_COUNTER0_OCU_8BIT_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter0_ocu_sync_enabled) {
      *ocu_config_value |= SL_COUNTER0_OCU_SYNC_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter1_ocu_output_enabled) {
      *ocu_config_value |= SL_COUNTER1_OCU_OUTPUT_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter1_ocu_dma_enabled) {
      *ocu_config_value |= SL_COUNTER1_OCU_DMA_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter1_ocu_8bit_mode_enabled) {
      *ocu_config_value |= SL_COUNTER1_OCU_8BIT_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter1_ocu_sync_enabled) {
      *ocu_config_value |= SL_COUNTER1_OCU_SYNC_ENABLE;
    }
    if (ocu_config_handle_ptr->is_counter0_toggle_output_high_enabled) {
      *ocu_config_value |= SL_OCU_OUTPUT0_TOGGLE_HIGH;
    }
    if (ocu_config_handle_ptr->is_counter0_toggle_output_low_enabled) {
      *ocu_config_value |= SL_OCU_OUTPUT0_TOGGLE_LOW;
    }
    if (ocu_config_handle_ptr->is_counter1_toggle_output_high_enabled) {
      *ocu_config_value |= SL_OCU_OUTPUT1_TOGGLE_HIGH;
    }
    if (ocu_config_handle_ptr->is_counter1_toggle_output_low_enabled) {
      *ocu_config_value |= SL_OCU_OUTPUT1_TOGGLE_LOW;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * CT DMA transfer complete callback function
 ******************************************************************************/
static void ct_dma_transfer_complete_callback(uint32_t channel, void *data)
{
  (void)data;
  if (channel == CT_DMA_CHANNEL_0) {
    ct_dma_transfer_flag_channel_0 = 1; // Set flag for Channel 1
  }
  if (channel == (CT_DMA_CHANNEL_8 - 1)) {
    ct_dma_transfer_flag_channel_1 = 1; // Set flag for Channel 8
  }
}

/*******************************************************************************
 * CT DMA error callback function
 ******************************************************************************/
static void ct_dma_error_callback(uint32_t channel, void *data)
{
  (void)data;
  callback_function_ptr((uint32_t *)(channel));
}
