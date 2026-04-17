/*******************************************************************************
 * @file  code_classifier.c
 * @brief
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
/**============================================================================
 * @section Description
 * This files contains example code to demonstrate the GPIO/LED toggle functionality.
 ============================================================================**/

// Include Files
#include "sl_sleeptimer.h"
#include "sl_si91x_led_instances.h"
#include "rsi_debug.h"
#include "sl_si91x_led.h"

// Code classifier feature for memory section placement
#ifdef SL_SI91X_CODE_CLASSIFIER_ENABLE
#include "sl_si91x_code_classifier.h"
#endif

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef LED_INSTANCE
#define LED_INSTANCE led_led0
#endif

#ifndef TOGGLE_DELAY_MS
#define TOGGLE_DELAY_MS 1000
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

sl_sleeptimer_timer_handle_t timer;
volatile bool toggle_timeout = false;

// Large array placed in PSRAM to save on RAM usage
SL_SI91X_CODE_CLASSIFY(bss_to_psram, psram_variable, SL_CODE_CLASS_FORCE_PSRAM)
volatile int psram_large_buffer[1024] __attribute__((used));

// Frequently accessed variable placed in RAM for fast access
SL_SI91X_CODE_CLASSIFY(data_to_ram, ram_variable, SL_CODE_CLASS_FORCE_RAM)
volatile int fast_access_counter = 0;

// Dummy DMA Accessible Buffer for data transfer without CPU intervention
SL_SI91X_CODE_CLASSIFY(data_dma_accessible, dma_variable, SL_CODE_CLASS_DMA_ACCESSIBLE)
volatile int dma_transfer_buffer[2] = { 0, 1 };

// Time-Critical Flag: Stored in a fast-access section to ensure minimal delay
SL_SI91X_CODE_CLASSIFY(data_time_critical, time_critical_variable, SL_CODE_CLASS_TIME_CRITICAL)
volatile bool time_critical_event_triggered = false;

// Loop Counter to track the buffer index
volatile int buffer_index = 0;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void on_timeout(sl_sleeptimer_timer_handle_t *handle, void *data);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

// Function to initialize the periodic timer
void code_classifier_example_init(void)
{
  sl_sleeptimer_start_periodic_timer_ms(&timer,
                                        TOGGLE_DELAY_MS,
                                        on_timeout,
                                        NULL,
                                        0,
                                        SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
}

// ==================== Time-Critical Function ====================
SL_SI91X_CODE_CLASSIFY(text, app_api, SL_CODE_CLASS_TIME_CRITICAL)
void code_classifier_example_process_action(void)
{
  if (toggle_timeout == true) {
    sl_si91x_led_toggle(LED_INSTANCE.pin);
    toggle_timeout = false;

    // Indicate that a time-critical event has occurred
    time_critical_event_triggered = !time_critical_event_triggered;

    // PSRAM Usage Example
    // Storing large data in PSRAM helps reduce RAM usage
    if (buffer_index < 1024) {
      psram_large_buffer[buffer_index] = buffer_index;
      DEBUGOUT("PSRAM Buffer [%d]: %d\n", buffer_index, psram_large_buffer[buffer_index]);
      buffer_index++;
    }

    // Fast-Access RAM Example
    // This variable is placed in RAM because it is frequently updated
    fast_access_counter++;
    DEBUGOUT("Fast RAM Counter: %d\n", fast_access_counter);

    // DMA Accessible Buffer
    // This buffer is used for DMA-based data transfer.
    // Updated in an efficient way to optimize memory access.
    if (buffer_index % 2 == 0) {
      dma_transfer_buffer[0]++;
      dma_transfer_buffer[1]++;
      DEBUGOUT("DMA Transfer Buffer: {%d, %d}\n", dma_transfer_buffer[0], dma_transfer_buffer[1]);
    }

    // Time-Critical Event Logging
    // Indicating whether a time-sensitive event has been triggered.
    DEBUGOUT("Time-Critical Event Triggered: %s\n", time_critical_event_triggered ? "Yes" : "No");
  }
}

/***************************************************************************/
/**
 * Sleeptimer timeout callback.
 * This function sets the flag to indicate the periodic timer event has occurred.
 ******************************************************************************/
static void on_timeout(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)&handle;
  (void)&data;
  toggle_timeout = true;
}
