/******************************************************************************
* @file sl_si91x_config_timer.h
* @brief Config-timer API implementation
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

#ifndef SL_SI91X_CONFIG_TIMER_H
#define SL_SI91X_CONFIG_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "rsi_ct.h"
#include "sl_si91x_dma.h"
/***************************************************************************/
/**
 * @addtogroup CONFIG-TIMER Configurable Timers
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{ 
 * 
 ******************************************************************************/

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define CT                   CT0 ///< pointer to CT base address
#define CT_DMA_CHANNEL_0     0   ///< DMA channel for Counter 0
#define CT_DMA_CHANNEL_8     8   ///< DMA channel for Counter 1
#define CT_DMA_NUMBER        0   ///< DMA number for Config Timer
#define CT_CHANNEL_PRIORITY  1   ///< DMA Channel Priority
#define CT_DMA_TRANSFER_SIZE 1   ///< DMA transfer size
// -----------------------------------------------------------------------------
// Data Types

typedef OCU_PARAMS_T sl_config_timer_ocu_params_t; ///< Renaming OCU (Output Compare Unit) parameters structure type
typedef WFG_PARAMS_T sl_config_timer_wfg_config_t; ///< Renaming WFG (Waveform Generator) parameters structure type
typedef RSI_CT_CALLBACK_T sl_config_timer_pwm_callback_t; ///< Renaming MCPWM callback structure
extern sl_dma_xfer_t ct_dma_transfer_channel_0;           ///< DMA transfer configuration for Channel 0
extern sl_dma_xfer_t ct_dma_transfer_channel_1;           ///< DMA transfer configuration for Channel 1
extern volatile uint8_t ct_dma_transfer_flag_channel_0;   ///< Transfer flag for Channel 0
extern volatile uint8_t ct_dma_transfer_flag_channel_1;   ///< Transfer flag for Channel 1
extern volatile uint32_t ct_dma_transfer_index_channel_0; ///< DMA transfer index Channel 0
extern volatile uint32_t ct_dma_transfer_index_channel_1; ///< DMA transfer index Channel 1
/***************************************************************************/ /**
 * Typedef for the function pointer of the interrupt callback function
 *
 * @param callback_flag (void *) parameter for updating flag values
 ******************************************************************************/
typedef void (*sl_config_timer_callback_t)(void *callback_flag);

/**
 * @brief Enumeration to represent timer modes.
 */
typedef enum {
  SL_COUNTER_16BIT,     ///< 16-bit counter mode.
  RESERVED,             ///< Reserved
  SL_COUNTER_MODE_LAST, ///< Last member of the enumeration for validation.
} sl_config_timer_mode_t;

/**
 * @brief Enumeration to represent timer counter numbers.
 */
typedef enum {
  SL_COUNTER_0,           ///< Enum for CT counter-0.
  SL_COUNTER_1,           ///< Enum for CT counter-1.
  SL_COUNTER_NUMBER_LAST, ///< Last member of the enumeration for validation.
} sl_counter_number_t;

/**
 * @brief Enumeration to represent timer counter 0 directions.
 */
typedef enum {
  SL_COUNTER0_UP,             ///< CT counter-0 up-counting operation.
  SL_COUNTER0_DOWN,           ///< CT counter-0 down-counting operation.
  SL_COUNTER0_UP_DOWN,        ///< CT counter-0 up-down counting operation.
  SL_COUNTER0_DIRECTION_LAST, ///< Last member of the enumeration for validation.
} sl_counter0_direction_t;

/**
 * @brief Enumeration to represent timer counter 1 directions.
 */
typedef enum {
  SL_COUNTER1_UP,             ///< CT counter-1 up-counting operation.
  SL_COUNTER1_DOWN,           ///< CT counter-1 down-counting operation.
  SL_COUNTER1_UP_DOWN,        ///< CT counter-1 up-down counting operation.
  SL_COUNTER1_DIRECTION_LAST, ///< Last member of the enumeration for validation.
} sl_counter1_direction_t;

/**
 * @brief Enumeration to represent timer configuration parameter values.
 */
typedef enum {
  SL_COUNTER0_SOFT_RESET_ENABLE   = SOFTRESET_COUNTER_0,  ///< Soft reset enable value for counter-0.
  SL_COUNTER0_PERIODIC_ENABLE     = PERIODIC_ENCOUNTER_0, ///< Periodic mode enable value for counter-0.
  SL_COUNTER0_TRIGGER_ENABLE      = COUNTER0_TRIG,        ///< Software trigger enable value for counter-0.
  SL_COUNTER0_SYNC_TRIGGER_ENABLE = COUNTER0_SYNC_TRIG,   ///< Sync trigger enable value for counter-0.
  SL_COUNTER0_BUFFER_ENABLE       = BUF_REG0EN,           ///< Buffer enable value for counter-0.
  SL_COUNTER0_UP_DIRECTION        = COUNTER0_UP,          ///< Up-direction enable value for counter-0.
  SL_COUNTER0_DOWN_DIRECTION      = COUNTER0_DOWN,        ///< Down-direction enable value for counter-0.
  SL_COUNTER0_UP_DOWN_DIRECTION   = COUNTER0_UP_DOWN,     ///< Up-down direction enable value for counter-0.
  SL_COUNTER1_SOFT_RESET_ENABLE   = SOFTRESET_COUNTER_1,  ///< Soft reset enable value for counter-1.
  SL_COUNTER1_PERIODIC_ENABLE     = PERIODIC_ENCOUNTER_1, ///< Periodic mode enable value for counter-1.
  SL_COUNTER1_TRIGGER_ENABLE      = COUNTER1_TRIG,        ///< Software trigger enable value for counter-1.
  SL_COUNTER1_SYNC_TRIGGER_ENABLE = COUNTER1_SYNC_TRIG,   ///< Sync trigger enable value for counter-1.
  SL_COUNTER1_BUFFER_ENABLE       = BUF_REG1EN,           ///< Buffer enable value for counter-1.
  SL_COUNTER1_UP_DIRECTION        = COUNTER1_UP,          ///< Up-direction enable value for counter-1.
  SL_COUNTER1_DOWN_DIRECTION      = COUNTER1_DOWN,        ///< Down-direction enable value for counter-1.
  SL_COUNTER1_UP_DOWN_DIRECTION   = COUNTER1_UP_DOWN,     ///< Up-down direction enable value for counter-1.
  SL_COUNTER_PARAM_LAST,                                  ///< Last member of the enumeration for validation.
} sl_config_timer_config_values_t;

/**
 * @brief Enumeration to represent timer OCU configuration parameter values.
 */
typedef enum {
  SL_COUNTER0_OCU_OUTPUT_ENABLE = OUTPUT_OCU_0,             ///< Counter-0 output enable value.
  SL_COUNTER0_OCU_DMA_ENABLE    = OCU_DMA_0,                ///< Counter-0 OCU-DMA mode enable value.
  SL_COUNTER0_OCU_8BIT_ENABLE   = OCU_8_MODE_0,             ///< Counter-0 OCU 8-bit mode enable value.
  SL_COUNTER0_OCU_SYNC_ENABLE   = OCU_SYNC_WITH_0,          ///< Counter-0 OCU sync mode enable value.
  SL_COUNTER1_OCU_OUTPUT_ENABLE = OUTPUT_OCU_1,             ///< Counter-1 output enable value.
  SL_COUNTER1_OCU_DMA_ENABLE    = OCU_DMA_1,                ///< Counter-1 OCU-DMA mode enable value.
  SL_COUNTER1_OCU_8BIT_ENABLE   = OCU_8_MODE_1,             ///< Counter-1 OCU 8-bit mode enable value.
  SL_COUNTER1_OCU_SYNC_ENABLE   = OCU_SYNC_WITH_1,          ///< Counter-1 OCU sync mode enable value.
  SL_OCU_OUTPUT0_TOGGLE_HIGH    = MAKE_OUTPUT_0_HIGH_SEL_0, ///< Counter-0 output toggle high select value.
  SL_OCU_OUTPUT0_TOGGLE_LOW     = MAKE_OUTPUT_0_LOW_SEL_0,  ///< Counter-0 output toggle low select value.
  SL_OCU_OUTPUT1_TOGGLE_HIGH    = MAKE_OUTPUT_1_HIGH_SEL_1, ///< Counter-1 output toggle high select value.
  SL_OCU_OUTPUT1_TOGGLE_LOW     = MAKE_OUTPUT_1_LOW_SEL_1,  ///< Counter-1 output toggle low select value.
  SL_OCU_PARAM_LAST,                                        ///< Last member of the enumeration for validation.
} sl_config_timer_ocu_config_values_t;

/**
 * @brief Enumeration to represent various timer input events.
 */
typedef enum {
  SL_NO_EVENT,                                ///< No input event.
  SL_EVENT0_RISING_EDGE,                      ///< Input-0 rising edge event.
  SL_EVENT1_RISING_EDGE,                      ///< Input-1 rising edge event.
  SL_EVENT2_RISING_EDGE,                      ///< Input-2 rising edge event.
  SL_EVENT3_RISING_EDGE,                      ///< Input-3 rising edge event.
  SL_EVENT0_FALLING_EDGE,                     ///< Input-0 falling edge event.
  SL_EVENT1_FALLING_EDGE,                     ///< Input-1 falling edge event.
  SL_EVENT2_FALLING_EDGE,                     ///< Input-2 falling edge event.
  SL_EVENT3_FALLING_EDGE,                     ///< Input-3 falling edge event.
  SL_EVENT0_RISING_FALLING_EDGE,              ///< Input-0 rising-falling edge event.
  SL_EVENT1_RISING_FALLING_EDGE,              ///< Input-1 rising-falling edge event.
  SL_EVENT2_RISING_FALLING_EDGE,              ///< Input-2 rising-falling edge event.
  SL_EVENT3_RISING_FALLING_EDGE,              ///< Input-3 rising-falling edge event.
  SL_EVENT0_LEVEL0,                           ///< Input-0 Level-0 event.
  SL_EVENT1_LEVEL0,                           ///< Input-1 Level-0 event.
  SL_EVENT2_LEVEL0,                           ///< Input-2 Level-0 event.
  SL_EVENT3_LEVEL0,                           ///< Input-3 Level-0 event.
  SL_EVENT0_LEVEL1,                           ///< Input-0 Level-1 event.
  SL_EVENT1_LEVEL1,                           ///< Input-1 Level-1 event.
  SL_EVENT2_LEVEL1,                           ///< Input-2 Level-1 event.
  SL_EVENT3_LEVEL1,                           ///< Input-3 Level-1 event.
  SL_AND_EVENT,                               ///< AND event.
  SL_OR_EVENT,                                ///< OR event.
  SL_EVENT0_RISING_EDGE_AND_EVENT,            ///< Input-0 rising edge AND event.
  SL_EVENT0_RISING_EDGE_OR_EVENT,             ///< Input-0 rising edge OR event.
  SL_EVENT1_RISING_EDGE_AND_EVENT,            ///< Input-1 rising edge AND event.
  SL_EVENT1_RISING_EDGE_OR_EVENT,             ///< Input-1 rising edge OR event.
  SL_EVENT2_RISING_EDGE_AND_EVENT,            ///< Input-2 rising edge AND event.
  SL_EVENT2_RISING_EDGE_OR_EVENT,             ///< Input-2 rising edge OR event.
  SL_EVENT3_RISING_EDGE_AND_EVENT,            ///< Input-3 rising edge AND event.
  SL_EVENT3_RISING_EDGE_OR_EVENT,             ///< Input-3 rising edge OR event.
  SL_EVENT0_RISING_EDGE_REGISTERED_AND_EVENT, ///< Input-0 rising edge registered AND event.
  SL_EVENT0_RISING_EDGE_REGISTERED_OR_EVENT,  ///< Input-0 rising edge registered OR event.
  SL_EVENT1_RISING_EDGE_REGISTERED_AND_EVENT, ///< Input-1 rising edge registered AND event.
  SL_EVENT1_RISING_EDGE_REGISTERED_OR_EVENT,  ///< Input-1 rising edge registered OR event.
  SL_EVENT2_RISING_EDGE_REGISTERED_AND_EVENT, ///< Input-2 rising edge registered AND event.
  SL_EVENT2_RISING_EDGE_REGISTERED_OR_EVENT,  ///< Input-2 rising edge registered OR event.
  SL_EVENT3_RISING_EDGE_REGISTERED_AND_EVENT, ///< Input-3 rising edge registered AND event.
  SL_EVENT3_RISING_EDGE_REGISTERED_OR_EVENT,  ///< Input-3 rising edge registered OR event.
  SL_CT_EVENT_LAST,                           ///< Last member of the enumeration for validation.
} sl_config_timer_event_t;

/**
 * @brief Enumeration to represent various timer actions.
 */
typedef enum {
  START,       ///< Enum for timer-start action.
  STOP,        ///< Enum for timer-stop action.
  CONTINUE,    ///< Enum for timer-continue action.
  HALT,        ///< Enum for timer-halt action.
  INCREMENT,   ///< Enum for timer-increment action.
  CAPTURE,     ///< Enum for timer-capture action.
  INTERRUPT,   ///< Enum for timer-interrupt action.
  OUTPUT,      ///< Enum for timer-output action.
  ACTION_LAST, ///< Last member of the enumeration for validation.
} sl_config_timer_action_t;

/**
 * @brief Enumeration to represent various timer interrupt flag values.
 */
typedef enum {
  SL_CT_EVENT_INTR_0_FLAG = RSI_CT_EVENT_INTR_0_l,      ///< Enum for counter-0 event interrupt enable value.
  SL_CT_FIFO_0_FULL_FLAG  = RSI_CT_EVENT_FIFO_0_FULL_l, ///< Enum for counter-0 FIFO full interrupt enable value.
  SL_CT_COUNTER_0_IS_ZERO_FLAG =
    RSI_CT_EVENT_COUNTER_0_IS_ZERO_l, ///< Enum for counter-0 zero-count-value interrupt enable value.
  SL_CT_COUNTER_0_IS_PEAK_FLAG =
    RSI_CT_EVENT_COUNTER_0_IS_PEAK_l,                   ///< Enum for counter-0 match-value interrupt enable value.
  SL_CT_EVENT_INTR_1_FLAG = RSI_CT_EVENT_INTR_1_l,      ///< Enum for counter-1 event interrupt enable value.
  SL_CT_FIFO_1_FULL_FLAG  = RSI_CT_EVENT_FIFO_1_FULL_l, ///< Enum for counter-1 FIFO full interrupt enable value.
  SL_CT_COUNTER_1_IS_ZERO_FLAG =
    RSI_CT_EVENT_COUNTER_1_IS_ZERO_l, ///< Enum for counter-1 zero-count-value interrupt enable value.
  SL_CT_COUNTER_1_IS_PEAK_FLAG =
    RSI_CT_EVENT_COUNTER_1_IS_PEAK_l, ///< Enum for counter-1 match-value interrupt enable value.
} sl_config_timer_interrupt_flags_values_t;

/**
 * @brief Structure to hold the parameters of timer configurations.
 */
typedef struct {
  boolean_t is_counter_mode_32bit_enabled;    ///< CT mode, see \ref sl_config_timer_mode_t for possible values.
  boolean_t is_counter0_soft_reset_enabled;   ///< Counter-0 soft reset, true to enable, false to disable.
  boolean_t is_counter0_periodic_enabled;     ///< Counter-0 periodic mode, true to enable, false to disable.
  boolean_t is_counter0_trigger_enabled;      ///< Counter-0 software trigger, true to enable, false to disable.
  boolean_t is_counter0_sync_trigger_enabled; ///< Counter-0 sync trigger, true to enable, false to disable.
  boolean_t is_counter0_buffer_enabled;       ///< Counter-0 buffer, true to enable, false to disable.
  boolean_t is_counter1_soft_reset_enabled;   ///< Counter-1 soft reset, true to enable, false to disable.
  boolean_t is_counter1_periodic_enabled;     ///< Counter-1 periodic mode, true to enable, false to disable.
  boolean_t is_counter1_trigger_enabled;      ///< Counter-1 software trigger, true to enable, false to disable.
  boolean_t is_counter1_sync_trigger_enabled; ///< Counter-1 sync trigger, true to enable, false to disable.
  boolean_t is_counter1_buffer_enabled;       ///< Counter-1 buffer, true to enable, false to disable.
  uint8_t counter0_direction; ///< Counter-0 direction, sl_config_timer_counter0_direction_t for possible values.
  uint8_t counter1_direction; ///< Counter-1 direction, sl_config_timer_counter1_direction_t for possible values.
} sl_config_timer_config_t;

/**
 * @brief Structure to hold the parameters of timer Output Compare Unit (OCU) configurations.
 */
typedef struct {
  boolean_t is_counter0_ocu_output_enabled;    ///< True to enable OCU output of counter-0, false to disable it.
  boolean_t is_counter0_ocu_dma_enabled;       ///< True to enable OCU DMA support of counter-0, false to disable it.
  boolean_t is_counter0_ocu_8bit_mode_enabled; ///< True to enable OCU 8-bit mode of counter-0, false to disable it.
  boolean_t
    is_counter0_ocu_sync_enabled; ///< True to enable counter-0 output sync with other channels, false to disable it.
  boolean_t is_counter1_ocu_output_enabled;    ///< True to enable OCU output of counter-1, false to disable it.
  boolean_t is_counter1_ocu_dma_enabled;       ///< True to enable OCU DMA support of counter-1, false to disable it.
  boolean_t is_counter1_ocu_8bit_mode_enabled; ///< True to enable OCU 8-bit mode of counter-1, false to disable it.
  boolean_t
    is_counter1_ocu_sync_enabled; ///< True to enable counter-1 output sync with other channels, false to disable it.
  boolean_t
    is_counter0_toggle_output_high_enabled; ///< True to enable counter-0 output toggle high, false to disable it.
  boolean_t is_counter0_toggle_output_low_enabled; ///< True to enable counter-0 output toggle low, false to disable it.
  boolean_t
    is_counter1_toggle_output_high_enabled; ///< True to enable counter-1 output toggle high, false to disable it.
  boolean_t is_counter1_toggle_output_low_enabled; ///< True to enable counter-1 output toggle low, false to disable it.
} sl_config_timer_ocu_config_t;

/**
 * @brief Structure to hold the parameters of timer OCU control, such as compare values and DMA state.
 */
typedef struct {
  boolean_t is_counter_number_1;            ///< CT counter number, see \ref sl_counter_number_t.
  boolean_t is_dma_state_enabled;           ///< DMA state for counter, true to enable, false to disable.
  sl_config_timer_ocu_params_t *params;     ///< Pointer to OCU control parameters structure.
  sl_config_timer_pwm_callback_t *callback; ///< Pointer to MCPWM callback structure.
} sl_config_timer_ocu_control_t;

/**
 * @brief Structure to hold the parameters of timer action's AND-event and OR-event configurations.
 */
typedef struct {
  uint8_t action;                        ///< CT action, see \ref sl_config_timer_action_t.
  uint8_t and_event_counter0;            ///< AND-event for counter-0 action, see \ref sl_config_timer_event_t.
  uint8_t or_event_counter0;             ///< OR-event for counter-0 action, see \ref sl_config_timer_event_t.
  uint8_t and_event_valid_bits_counter0; ///< Valid bits for counter-0 action AND-event, possible values 0 to 16.
  uint8_t or_event_valid_bits_counter0;  ///< Valid bits for counter-0 action OR-event, possible values 0 to 16.
  uint8_t and_event_counter1;            ///< AND-event for counter-1 action, see \ref sl_config_timer_event_t.
  uint8_t or_event_counter1;             ///< OR-event for counter-1 action, see \ref sl_config_timer_event_t.
  uint8_t and_event_valid_bits_counter1; ///< Valid bits for counter-1 action AND-event, possible values 0 to 16.
  uint8_t or_event_valid_bits_counter1;  ///< Valid bits for counter-1 action OR-event, possible values 0 to 16.
} sl_config_action_event_t;

/**
 * @brief Structure to hold various interrupt flags of the config-timer.
 */
typedef struct {
  boolean_t
    is_counter0_event_interrupt_enabled; ///< True to enable counter-0 interrupt on event occurrence, false to disable it.
  boolean_t
    is_counter0_fifo_full_interrupt_enabled; ///< True to enable counter-0 interrupt on FIFO full, false to disable it.
  boolean_t
    is_counter0_hit_zero_interrupt_enabled; ///< True to enable counter-0 interrupt on zero count value, false to disable it.
  boolean_t
    is_counter0_hit_peak_interrupt_enabled; ///< True to enable counter-0 interrupt on match-value, false to disable it.
  boolean_t
    is_counter1_event_interrupt_enabled; ///< True to enable counter-1 interrupt on event occurrence, false to disable it.
  boolean_t
    is_counter1_fifo_full_interrupt_enabled; ///< True to enable counter-1 interrupt on FIFO full, false to disable it.
  boolean_t
    is_counter1_hit_zero_interrupt_enabled; ///< True to enable counter-1 interrupt on zero count value, false to disable it.
  boolean_t
    is_counter1_hit_peak_interrupt_enabled; ///< True to enable counter-1 interrupt on match-value, false to disable it.
} sl_config_timer_interrupt_flags_t;

/**
 * @brief Structure to hold the versions of the peripheral API.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_config_timer_version_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To initialize the config-timer peripheral.
 * @details Configures its output GPIO pins and clock to 16 MHz.
 *******************************************************************************/
void sl_si91x_config_timer_init(void);

/***************************************************************************/
/**
 * @brief Configures DMA for Config Timer.
 * 
 * @details This API performs the following operations:
 *          - Initializes the DMA for Config Timer.
 *          - Configures the DMA transfer parameters, including source and destination addresses.
 *          - Registers DMA callback functions for transfer completion and error handling.
 *          - Allocates a DMA channel for the transfer.
 *          - Enables the DMA controller for operation.
 * 
 * @param[in] compare_values Pointer to the array containing compare values for DMA transfer.
 *                           This array is used to transfer data to the Config Timer registers.
 * @param[in] channel DMA channel to be used for the transfer (e.g., CT_DMA_CHANNEL_0 or CT_DMA_CHANNEL_8).
 * 
 * @return Status code indicating the result of the operation:
 *         - SL_STATUS_OK - DMA configuration was successful.
 *         - SL_STATUS_FAIL - DMA configuration failed.
 * 
 * @pre Pre-conditions:
 *      - The Config Timer must be initialized using `sl_si91x_config_timer_init()`.
 *      - DMA resources must be available for allocation.
 * 
 * @see For more information on status codes, refer to:
 *      [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_set_dma_configuration(uint32_t *compare_values, uint32_t channel);

/***************************************************************************/
/**
 * @brief Starts DMA transfer for Config Timer and enables the DMA channel.
 * 
 * @details This API performs the following operations:
 *          - Initiates the DMA transfer for the specified channel.
 *          - Enables the DMA channel after the transfer is configured.
 * 
 * @param[in] compare_values Pointer to the array containing compare values for DMA transfer.
 *                           This array is used to transfer data to the Config Timer registers.
 * @param[in] channel DMA channel to be used for the transfer (e.g., CT_DMA_CHANNEL_0 or CT_DMA_CHANNEL_8).
 * @param[in] dma_transfer Pointer to the DMA transfer configuration structure.
 *                         This structure contains details such as source and destination addresses,
 *                         transfer size, and transfer type.
 * 
 * @return Status code indicating the result of the operation:
 *         - SL_STATUS_OK - DMA transfer and channel enable were successful.
 *         - SL_STATUS_FAIL - DMA transfer or channel enable failed.
 * 
 * @pre Pre-conditions:
 *      - DMA must be configured using `sl_si91x_config_timer_dma_configure()`.
 *      - The Config Timer must be initialized using `sl_si91x_config_timer_init()`.
 * 
 * @see For more information on status codes, refer to:
 *      [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_dma_transfer(uint32_t *compare_values, uint32_t channel, sl_dma_xfer_t *dma_transfer);

/***************************************************************************/
/**
 * @brief To set the config-timer mode as 16-bit counters.
 * 
 * @details 
 * In this mode, Counter_0 modes/triggers/enables are used.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * 
 * @param[in] mode \ref sl_config_timer_mode_t for possible values.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer mode is set properly.
 *         - SL_STATUS_INVALID_MODE  - 'mode' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_set_mode(sl_config_timer_mode_t mode);

/***************************************************************************/
/**
 * @brief To configure Config-timer parameters.
 * 
 * @details This API sets the Config-timer parameters such as 16-bit mode, periodic mode, counter trigger enable, 
 * soft reset enable, buffer enable, sync trigger enable, and
 * sets direction for counter0 and counter1.
 * Counter trigger enable starts the counter.
 * Sync trigger enables the counter to run/activate when sync is found.
 * Buffer enable copies buffer data to the Counter Match register.
 * Soft reset is valid only when the counter is in two 16-bit counters mode,
 * and resets the counter on the write.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * 
 * @param[in] timer_config_ptr Pointer to CT config structure @ref sl_config_timer_config_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer configurations are set properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Counter direction parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * @note 
 * Config Timer supports only 16-bit mode, and the SL_CT_MODE_32BIT_ENABLE_MACRO in the configuration file is defined to 16-bit mode by default.
 *
 * If UC is enabled, the application can only use the configuration set in the Universal Configurator (UC).
 * If your application needs to change the configuration at run-time, disable UC configuration and pass user-defined configuration.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_set_configuration(sl_config_timer_config_t *timer_config_ptr);

/***************************************************************************/
/**
 * @brief To reset config-timer parameters and set default values.
 * 
 * @details This API sets the config-timer to 16-bit mode, up-counter direction,
 * and disables periodic mode, soft reset, buffer, sync, and software trigger of counters.
 *******************************************************************************/
void sl_si91x_config_timer_reset_configuration(void);

/***************************************************************************/
/**
 * @brief To set Config-timer OCU (Output Compare Unit) configurations.
 * 
 * @details This API enables outputs in OCU mode, OCU-DMA mode,
 * channel sync with OCU outputs, and 8-bit mode for OCU outputs for both counters.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_configuration();
 * 
 * @param[in] ocu_config_ptr Pointer to CT OCU-config structure \ref sl_config_timer_ocu_config_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, OCU configurations are set properly.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_set_ocu_configuration(sl_config_timer_ocu_config_t *ocu_config_ptr);

/***************************************************************************/
/**
 * @brief To reset config-timer OCU (Output Compare Unit) parameters.
 * 
 * @details This API sets the config-timer to 16-bit mode, sets the up-counter direction,
 * and disables DMA mode, channel sync, and 8-bit mode for OCU outputs.
 *******************************************************************************/
void sl_si91x_config_timer_reset_ocu_configuration(void);

/***************************************************************************/
/**
 * @brief To set Config-timer OCU mode control parameters.
 * 
 * @details This API sets Config-timer OCU mode control parameters such as first and next threshold 
 * values for counter outputs. It also enables the DMA support for counters.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_ocu_configuration();
 *
 * @param[in] ocu_params Pointer to CT OCU-config structure \ref sl_config_timer_ocu_control_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, OCU mode control parameters are set properly.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_set_ocu_control(sl_config_timer_ocu_control_t *ocu_params);

/***************************************************************************/
/**
 * @brief To set Config-timer WFG (Waveform Generator) mode configurations.
 * 
 * @details This API configures the Config-timer WFG mode settings such as selecting toggle high, low, 
 * and peak for counter-0 and counter-1 outputs.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 *
 * @param[in] wfg_config_ptr Pointer to CT WFG-config structure \ref sl_config_timer_wfg_config_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, WFG mode configurations are set properly.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_set_wfg_configuration(sl_config_timer_wfg_config_t *wfg_config_ptr);

/***************************************************************************/
/**
 * @brief To set the Config-timer initial count based on the timer mode.
 * 
 * @details Configurable timer modes are mentioned below:
 *  - For 16-bit mode counters, the ORed value of both initial values is passed to the register.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_configuration();
 *
 * @param[in] mode \ref sl_config_timer_mode_t for possible values
 * @param[in] counter0_initial_value (uint32_t)
 * @param[in] counter1_initial_value (uint32_t)
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, initial count is set properly.
 *         - SL_STATUS_INVALID_MODE  - 'mode' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_set_initial_count(sl_config_timer_mode_t mode,
                                                    uint32_t counter0_initial_value,
                                                    uint32_t counter1_initial_value);

/***************************************************************************/
/**
 * @brief To set the Config-timer match-count based on the timer mode and counter number.
 * 
 * @details 
 * If the mode is 16-bit, as per the passed counter number, it updates the match value.
 * For 16-bit mode, it takes a 16-bit match-value only, else throws an error.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_configuration();
 * 
 * @param[in] counter_number \ref sl_counter_number_t for possible values
 * @param[in] mode \ref sl_config_timer_mode_t for possible values
 * @param[in] match_value (uint32_t)
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, match-value is set properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'counter_number' parameter value is invalid.
 *         - SL_STATUS_INVALID_MODE  - 'mode' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_set_match_count(sl_config_timer_mode_t mode,
                                                  sl_counter_number_t counter_number,
                                                  uint32_t match_value);

/***************************************************************************/
/**
 * @brief To calculate and return the match value of the timer for the desired time period.
 * 
 * @details This API calculates the match value based on the provided time period in microseconds.
 * 
 * @param[in] time_period_in_us Time period in microseconds.
 * @param[out] match_value Gets the match value of the timer for the desired time period.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_COUNT  - Count is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_get_match_value(uint32_t time_period_in_us, uint32_t *match_value);

/***************************************************************************/
/**
 * @brief To get the Config-timer current count.
 * 
 * @details This API retrieves the Config-timer current count based on the timer mode and counter number,
 * and updates the count value to the count_value variable input parameter.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_configuration();
 *
 * @param[in]  counter_number \ref sl_counter_number_t for possible values
 * @param[in]  mode \ref sl_config_timer_mode_t for possible values
 * @param[out] count_value Pointer to the variable to store the count value
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, count value is read properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'counter_number' parameter value is invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter 'count_value' is a null pointer.
 *         - SL_STATUS_INVALID_MODE  - 'mode' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_get_count(sl_config_timer_mode_t mode,
                                            sl_counter_number_t counter_number,
                                            uint32_t *count_value);

/***************************************************************************/
/**
 * @brief To reset the Config-timer counter register value.
 * 
 * @details This API resets the count values of the selected counter.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_configuration();
 *
 * @param[in] counter_number \ref sl_counter_number_t for possible values
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, counter resets properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'counter_number' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_reset_counter(sl_counter_number_t counter_number);

/***************************************************************************/
/**
 * @brief To start the selected config-timer counter on a software trigger.
 * 
 * @note This API triggers the 'counter_number' counter.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_configuration(), keep software trigger disabled here
 *
 * @param[in] counter_number \ref sl_counter_number_t for possible values
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer started properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'counter_number' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_start_on_software_trigger(sl_counter_number_t counter_number);

/***************************************************************************/
/**
 * @brief To select config timer input events for triggering selected timer actions.
 * 
 * @details This API selects config timer input events for triggering selected timer actions. 
 * Different timer actions include start, stop, continue, halt, increment, capture, 
 * interrupt, and output. ORed value of both events is passed to the register.    
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_config_timer_init();
 *      - \ref sl_si91x_config_timer_set_configuration(), keep software trigger disabled here
 *      - \ref sl_si91x_config_timer_register_callback(), keep event interrupt flag enabled for
 *      respective counter
 * 
 * @param[in]  action \ref sl_config_timer_action_t for possible values
 * @param[in]  select_event_counter0 \ref sl_config_timer_event_t for possible values,
 *             (selects input event for triggering counter-0 action)
 * @param[in]  select_event_counter1 \ref sl_config_timer_event_t for possible values,
 *             (selects input event for triggering counter-1 action)
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, input event is set properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Selected input event or action parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_select_action_event(sl_config_timer_action_t action,
                                                      sl_config_timer_event_t select_event_counter0,
                                                      sl_config_timer_event_t select_event_counter1);

/***************************************************************************/
/**
 * @brief To configure the config timer action events.
 * 
 * @details This API configures the config timer input-event's AND-event and OR-event for triggering
 * selected timer actions. Different timer actions include start, stop, continue, halt, increment, capture, 
 * interrupt, and output.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_config_timer_init();
 *      - \ref sl_si91x_config_timer_set_configuration(), keep software trigger disabled here
 *      - \ref sl_si91x_config_timer_register_callback(), keep event interrupt flag enabled for
 *        respective counter
 *      - \ref sl_si91x_config_timer_select_action_event(), first selects the input event for
 *        respective action for respective counter
 * 
 * @param[in] event_config_handle Pointer to CT configure action events structure
 *            \ref sl_config_action_event_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, AND event & OR event set properly.
 *         - SL_STATUS_INVALID_PARAMETER  - and-event or or-event or event-valid-bits value is invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_configure_action_event(sl_config_action_event_t *event_config_handle);

/***************************************************************************/
/**
 * @brief To register a callback for config timer interrupts.
 * 
 * @details This API registers a callback function for config timer interrupts by enabling 
 * the respective interrupts as per the selected interrupt flag.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_init();
 * - \ref sl_si91x_config_timer_set_configuration(), keep software trigger disabled here
 * - sl_si91x_config_timer_unregister_timeout_callback(), if already registered for any interrupt
 *
 * @param[in]  on_config_timer_callback (function pointer) Callback function pointer
 *             to be called when a timer interrupt occurs \ref sl_config_timer_callback_t
 * @param[in]  callback_flag (void *) Pointer to the interrupt flag value variable \ref sl_config_timer_callback_t
 * @param[in]  interrupt_flags Pointer to the interrupt flags structure \ref sl_config_timer_interrupt_flags_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully registered timer timeout callback.
 *         - SL_STATUS_BUSY  - The callback is already registered, unregister
 *                                    the previous callback before registering a new one.
 *         - SL_STATUS_NULL_POINTER  - Parameter is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_register_callback(sl_config_timer_callback_t on_config_timer_callback,
                                                    void *callback_flag,
                                                    sl_config_timer_interrupt_flags_t *interrupt_flags);

/***************************************************************************/
/**
 * @brief To unregister the timer interrupt callback.
 * 
 * @details This API disables interrupts as per the selected interrupt flag and unregisters 
 * the previously registered timer interrupt callback.
 *
 * @pre Pre-conditions:
 * - \ref sl_si91x_config_timer_register_callback(), first register a particular interrupt flag.
 * 
 * @param[in] interrupt_flags Pointer to the interrupt flags structure \ref sl_config_timer_interrupt_flags_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully unregistered timer interrupt callback.
 *         - SL_STATUS_NULL_POINTER  - Parameter is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_unregister_callback(sl_config_timer_interrupt_flags_t *interrupt_flags);

/***************************************************************************/
/**
 * @brief Configures the timer to resume or halt on a specific event.
 * 
 * This API configures the timer to either resume or halt based on a specified event.
 * 
 * @param[in] counter_number The counter number that triggers the timer to resume or halt.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK (0x0000) - Success.
 *         - SL_STATUS_FAIL (0x0001) - Function failed.
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - Invalid parameter is passed.
 *         - SL_STATUS_NULL_POINTER (0x0022) - Null pointer is passed.
 * 
 * For more information on status codes, see
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_resume_halt_event(sl_counter_number_t counter_number);

/***************************************************************************/
/**
 * @brief To get the Config-timer count value when a capture event occurs.
 * 
 * @details This API updates the capture count value of the timer to the capture_value variable input parameter.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_config_timer_init();
 *      - \ref sl_si91x_config_timer_set_configuration();
 *      - \ref sl_si91x_config_timer_select_action_event(), first select events for capture action
 * 
 * @param[in]  counter_number \ref sl_counter_number_t for possible values
 * @param[out] capture_value Pointer to the variable to store the count value at capture event (uint16_t*).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, capture count value is read properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'counter_number' parameter value is invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter 'capture_value' is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_read_capture(sl_counter_number_t counter_number, uint16_t *capture_value);

/***************************************************************************/
/**
 * @brief To synchronize counter output with other channels.
 * 
 * @details This API synchronizes the counter output with other channels as per the sync_counter_value.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_config_timer_init();
 *      - \ref sl_si91x_config_timer_set_configuration();
 *
 * @param[in] counter_number \ref sl_counter_number_t for possible values
 * @param[in] sync_counter_value The value to synchronize the counter output (uint8_t).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, counter output synced properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'counter_number' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_set_counter_sync(sl_counter_number_t counter_number, uint8_t sync_counter_value);

/***************************************************************************/
/**
 * @brief To configure config timer output events for ADC trigger.
 * 
 * @details Configures events for counter0 and counter1 outputs to trigger ADC.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_config_timer_init();
 *      - \ref sl_si91x_config_timer_set_configuration();
 *
 * @param[in] pin1 Counter0 output event for ADC trigger (0 to 31) \ref sl_config_timer_event_t
 * @param[in] pin2 Counter1 output event for ADC trigger (0 to 31) \ref sl_config_timer_event_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, ADC trigger events set properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'pin1' or 'pin2' parameter value is invalid.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_set_output_adc_pin(uint8_t pin1, uint8_t pin2);

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
//TODO: usecase not clear
/***************************************************************************/
/**
 * @brief To set waveform generation compare values.
 * 
 * @details This API sets the compare values for waveform generation based on the provided 
 * OCU parameters structure.
 *
 * @param[in] counter_number \ref sl_counter_number_t for possible values
 * @param[in] ocu_params Pointer to the OCU parameters structure (sl_config_timer_ocu_params_t*).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success, compare values set properly.
 *         - SL_STATUS_INVALID_PARAMETER  - 'counter_number' parameter value is invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter 'ocu_params' is a null pointer.
 * 
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_config_timer_set_wfg_compare_values(sl_counter_number_t counter_number,
                                                         sl_config_timer_ocu_params_t *ocu_params);
/** @endcond */

/***************************************************************************/
/**
 * @brief To de-initialize the config-timer by disabling its clock.
 * 
 * @details This API de-initializes the config-timer by disabling its clock. It also unregisters 
 * the callback and disables all config timer interrupts.
 *
 * @note Ensure that no timer operations are pending before calling this API.
 ******************************************************************************/
void sl_si91x_config_timer_deinit(void);

/***************************************************************************/
/**
 * @brief To de-initialize the config-timer DMA.
 * 
 * @details This API de-initializes the config-timer DMA. 
 * 
 * @param[in] channel DMA channel to be used for the transfer (e.g., CT_DMA_CHANNEL_0 or CT_DMA_CHANNEL_8).
 * @return Status code indicating the result of the operation:
 *         - SL_STATUS_OK - DMA deinit success
 *         - SL_STATUS_FAIL - DMA deinit failure
 * 
 * @see For more information on status codes, refer to:
 *      [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_config_timer_dma_deinit(uint32_t channel);

/***************************************************************************/
/**
 * @brief To get the config timer API version.
 * 
 * @details This API retrieves the version information of the config timer API.
 * 
 * @return Version structure ( @ref sl_config_timer_version_t) containing the version details.
 ******************************************************************************/
sl_config_timer_version_t sl_si91x_config_timer_get_version(void);

/** @} end group CONFIG-TIMER */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/** @addtogroup CONFIG-TIMER Configurable Timers
 * @{
 *   @details
 *   @n @section CONFIG-TIMER_Intro Introduction
 *   Configurable timers are versatile peripherals that can be used for a variety of tasks, such as:
 *   - Modulated signal output
 *   - Event capture on GPIOs in input mode
 *   - Clock and event counting
 *   These timers can operate in Pulse Width Modulation (PWM) mode, driving a PWM wave on the outputs based on
 *   programmed ON and OFF periods.
 *   Timers are typically built-in peripherals found in microcontrollers and microprocessors.
 *   They function independently of the CPU and other peripherals, allowing them to keep track of time and perform
 *   timing-related tasks without requiring CPU intervention.
 *
 *   @n @section CONFIG-TIMER_Config Configuration
 *   To configure the config-timer, several parameters need to be defined:
 *   - Timer mode (16-bit) @ref sl_config_timer_mode_t
 *   - Periodic mode (Enable/Disable)
 *   - Counter trigger (Enable/Disable)
 *   - Soft reset (Enable/Disable)
 *   - Buffer enable (Enable/Disable)
 *   - Counter direction for counter0 @ref sl_counter0_direction_t
 *   - Counter direction for counter1 @ref sl_counter1_direction_t
 *   These configurations are set in the @ref sl_config_timer_config_t structure, and the API @ref sl_si91x_config_timer_set_configuration() must be
 *   called to apply them.
 *   For more detailed information on configuring the available parameters, see the respective peripheral example README document.
 *
 *   @n @section CONFIG-TIMER_Use Usage
 *   After defining the Config Timer (CT) structures, the following common Config Timer functions can be called, using an instance of @ref sl_config_timer_config_t structure:
 *   1. @ref sl_si91x_config_timer_init - Initializes the config timer.
 *   2. @ref sl_si91x_config_timer_set_configuration - Sets the configuration for the config timer.
 *   3. @ref sl_si91x_config_timer_set_initial_count (Optional) - Sets the initial count value.
 *   4. @ref sl_si91x_config_timer_set_match_count - Sets the match count value.
 *   5. @ref sl_si91x_config_timer_register_callback - Registers a callback for timer events.
 *   6. @ref sl_si91x_config_timer_start_on_software_trigger - Starts the timer with a software trigger.
 *   7. @ref sl_si91x_config_timer_deinit - De-initializes the config timer.
 *   These functions will initiate and configure the Config Timer according to the specified parameters, following the implementation pipeline.
 */
/** @} end group CONFIG-TIMER */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_CONFIG_TIMER_H */
