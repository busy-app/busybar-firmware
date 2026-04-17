/***************************************************************************//**
 * @file
 * @brief spidrv master freeRTOS examples functions
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
#include <string.h>
#include <stdio.h>
#include "spidrv_master_freertos.h"
#include "spidrv.h"
#include "sl_spidrv_instances.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define SPIDRV_TASK_STACK_SIZE      256
#define SPIDRV_TASK_PRIORITY        (tskIDLE_PRIORITY + 2)

// use SPI handle for EXP header (configured in project settings)
#define SPI_HANDLE                  sl_spidrv_eusart_exp_handle

// size of transmission and reception buffers
#define APP_BUFFER_SIZE             16

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static TaskHandle_t spidrvTaskHandle = NULL;

// Semaphore to signal that transfer is complete
static SemaphoreHandle_t tx_semaphore;

// Transmission and reception buffers
static char rx_buffer[APP_BUFFER_SIZE];
static char tx_buffer[APP_BUFFER_SIZE];

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

// Application task
static void spidrv_app_task(void *pvParameters);

// Callback fired when data is transmitted
void transfer_callback(SPIDRV_HandleData_t *handle,
                       Ecode_t transfer_status,
                       int items_transferred)
{
  (void)handle;
  (void)items_transferred;

  // Post semaphore to signal to application task that transfer is successful
  if (transfer_status == ECODE_EMDRV_SPIDRV_OK) {
    xSemaphoreGiveFromISR(tx_semaphore, NULL);
  }
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize example.
 ******************************************************************************/
void spidrv_app_init(void)
{
  // Create binary semaphore for transfer complete signaling
  tx_semaphore = xSemaphoreCreateBinary();

  // Create the SPI app task
  xTaskCreate(spidrv_app_task,
              "spidrv_app_task",
              SPIDRV_TASK_STACK_SIZE,
              NULL,
              SPIDRV_TASK_PRIORITY,
              &spidrvTaskHandle);
}

/***************************************************************************//**
 * spidrv task.
 ******************************************************************************/
static void spidrv_app_task(void *pvParameters)
{
  (void)pvParameters;
  int counter = 0;

  // stdout is redirected to VCOM in project configuration
  printf("Welcome to the SPIDRV example application, master mode\r\n");

  while (1) {
    // Delay to allow slave to start (10s)
    vTaskDelay(pdMS_TO_TICKS(10000));

    // send a string that includes an incrementing counter
    sprintf(tx_buffer, "ping %03d", counter);
    counter++;
    printf("Sending %s to slave...\r\n", tx_buffer);

    // Non-blocking data transfer to slave. When complete, rx buffer will be filled.
    Ecode_t ecode = SPIDRV_MTransfer(SPI_HANDLE, tx_buffer, rx_buffer, APP_BUFFER_SIZE, transfer_callback);

    configASSERT(ecode == ECODE_OK);

    // Wait for semaphore indicating that transfer is complete
    if (xSemaphoreTake(tx_semaphore, portMAX_DELAY) == pdTRUE) {
      printf("Got message from slave: %s\r\n", rx_buffer);
    }
  }
}