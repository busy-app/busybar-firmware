/***************************************************************************//**
 * @file
 * @brief DMADRV FreeRTOS application
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

#include <stdio.h>
#include <string.h>
#include "dmadrv.h"
#include "dmadrv_app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef DMADRV_TASK_STACK_SIZE
#define DMADRV_TASK_STACK_SIZE             configMINIMAL_STACK_SIZE
#endif

#ifndef DMADRV_TASK_PRIO
#define DMADRV_TASK_PRIO                   20
#endif

#ifndef EXAMPLE_USE_STATIC_ALLOCATION
#define EXAMPLE_USE_STATIC_ALLOCATION   1
#endif

#define RX_BUFFER_SIZE                     (8)
#define TX_BUFFER_SIZE                     (RX_BUFFER_SIZE + 64)

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static unsigned int tx_channel, rx_channel;      // DMA channels
static char tx_buffer[TX_BUFFER_SIZE + 1];       // Transmit buffer
static char rx_buffer[RX_BUFFER_SIZE + 1];       // Receive buffer
static SemaphoreHandle_t dma_rx_complete;        // Semaphore for DMA completion

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void dmadrv_task(void *pvParameters);
static bool rx_callback(unsigned int channel, unsigned int sequence_no, void *user_param);
static void transmit_data(const char *message);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 * Initialize DMADRV application.
 ******************************************************************************/
void dmadrv_app_init(void)
{
  // Initialize DMADRV
  DMADRV_Init();

  // Allocate DMA channels for TX and RX
  Ecode_t status = DMADRV_AllocateChannel(&tx_channel, NULL);
  configASSERT(status == ECODE_EMDRV_DMADRV_OK);

  status = DMADRV_AllocateChannel(&rx_channel, NULL);
  configASSERT(status == ECODE_EMDRV_DMADRV_OK);

  // Create semaphore for DMA reception completion
  dma_rx_complete = xSemaphoreCreateBinary();
  configASSERT(dma_rx_complete != NULL);

  // Start FreeRTOS task for DMADRV
  BaseType_t task_status = xTaskCreate(dmadrv_task,
                                       "DMADRV_Task",
                                       DMADRV_TASK_STACK_SIZE,
                                       NULL,
                                       DMADRV_TASK_PRIO,
                                       NULL);
  configASSERT(task_status == pdPASS);
}

/*******************************************************************************
 ***************************  LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 * FreeRTOS task for DMADRV processing.
 ******************************************************************************/
static void dmadrv_task(void *pvParameters)
{
  (void)pvParameters; // Suppress unused parameter warning

  // Welcome message
  transmit_data("Welcome to the DMADRV FreeRTOS app\r\nEnter data:\r\n");

  while (1) {
    // Start RX DMA transfer if not already active
    bool active;
    DMADRV_TransferActive(rx_channel, &active);
    if (!active) {
      DMADRV_PeripheralMemory(rx_channel,
                              dmadrvPeripheralSignal_EUSART0_RXDATAV,
                              rx_buffer,
                              (void *)&(EUSART0->RXDATA),
                              true,
                              RX_BUFFER_SIZE,
                              dmadrvDataSize1,
                              rx_callback,
                              NULL);
    }

    // Wait for DMA reception to complete
    if (xSemaphoreTake(dma_rx_complete, portMAX_DELAY) == pdTRUE) {
      // Send back the received message
      char response[TX_BUFFER_SIZE];
      snprintf(response, sizeof(response), "You wrote: %s\r\nEnter data:\r\n", rx_buffer);
      transmit_data(response);
    }

    // Small delay to avoid tight looping
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/*******************************************************************************
 * DMA RX callback function.
 ******************************************************************************/
static bool rx_callback(unsigned int channel, unsigned int sequence_no, void *user_param)
{
  (void)channel;
  (void)sequence_no;
  (void)user_param;

  // Notify the task that DMA reception is complete
  BaseType_t task_woken = pdFALSE;
  xSemaphoreGiveFromISR(dma_rx_complete, &task_woken);
  return task_woken == pdTRUE;
}

/*******************************************************************************
 * Transmit data using DMA.
 ******************************************************************************/
static void transmit_data(const char *message)
{
  // Wait for any active DMA transfers to complete
  bool active;
  DMADRV_TransferActive(tx_channel, &active);
  while (active) {
    vTaskDelay(pdMS_TO_TICKS(1));
    DMADRV_TransferActive(tx_channel, &active);
  }

  // Copy message to transmit buffer
  strncpy(tx_buffer, message, TX_BUFFER_SIZE);
  tx_buffer[TX_BUFFER_SIZE] = '\0'; // Ensure null termination

  // Start TX DMA transfer
  DMADRV_MemoryPeripheral(tx_channel,
                          dmadrvPeripheralSignal_EUSART0_TXBL,
                          (void *)&(EUSART0->TXDATA),
                          tx_buffer,
                          true,
                          strlen(tx_buffer),
                          dmadrvDataSize1,
                          NULL,
                          NULL);
}
