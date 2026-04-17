/*******************************************************************************
 * @file cpc_app.c
 * @brief Secondary firmware for the CPC sample application (FreeRTOS variant)
 * @version 1.0.0
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

#include "sl_cpc.h"
#include "sl_assert.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "cpc_app.h"
#include "sl_memory_manager.h"

/*******************************************************************************
 *******************************  DEFINES   ************************************
 ******************************************************************************/
#ifndef TASK_STACK_SIZE
#define TASK_STACK_SIZE       256
#endif

#ifndef TASK_PRIO
#define TASK_PRIO             (tskIDLE_PRIORITY + 1)
#endif

#define ENDPOINT_0_CONNECTED  (1 << 0)
#define ENDPOINT_1_CONNECTED  (1 << 1)

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
static uint8_t static_write_array[SL_CPC_RX_PAYLOAD_MAX_LENGTH];
static sl_cpc_endpoint_handle_t user_endpoint_handle_0;
static sl_cpc_endpoint_handle_t user_endpoint_handle_1;

static TaskHandle_t task_handle_0;
static TaskHandle_t task_handle_1;
static TaskHandle_t task_handle_connections;

static EventGroupHandle_t endpoint_connected_events;
static SemaphoreHandle_t  tx_complete_semaphore;

/*******************************************************************************
 *************************  LOCAL FUNCTIONS   **********************************
 ******************************************************************************/

// Callback triggered on error
static void on_endpoint_error(uint8_t endpoint_id,
                              void *arg)
{
  (void) arg;

  if ((endpoint_id == SL_CPC_ENDPOINT_USER_ID_0)
      && (sl_cpc_get_endpoint_state(&user_endpoint_handle_0) == SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE)) {
    // clear connected flag
    xEventGroupClearBits(endpoint_connected_events, ENDPOINT_0_CONNECTED);
  } else if ((endpoint_id == SL_CPC_ENDPOINT_USER_ID_1)
             && (sl_cpc_get_endpoint_state(&user_endpoint_handle_1) == SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE)) {
    // clear connected flag
    xEventGroupClearBits(endpoint_connected_events, ENDPOINT_1_CONNECTED);
  } else {
    configASSERT(0);
  }
}

// Callback triggered on write complete
static void on_write_completed(sl_cpc_user_endpoint_id_t endpoint_id,
                               void *buffer,
                               void *arg,
                               sl_status_t status)
{
  (void) arg;
  (void) buffer;

  configASSERT(status == SL_STATUS_OK);

  if (endpoint_id == SL_CPC_ENDPOINT_USER_ID_0) {
    sl_memory_free(buffer);
  } else if (endpoint_id == SL_CPC_ENDPOINT_USER_ID_1) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(tx_complete_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    configASSERT(0);
  }
}

/*******************************************************************************
 * Open and configure endpoint
 ******************************************************************************/
static void open_endpoint(sl_cpc_endpoint_handle_t *ep, uint8_t endpoint_id)
{
  sl_status_t status;

  status = sl_cpc_open_user_endpoint(ep, endpoint_id, 0, 1);
  configASSERT(status == SL_STATUS_OK);

  status = sl_cpc_set_endpoint_option(ep, SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED,
                                      (void *)on_write_completed);
  configASSERT(status == SL_STATUS_OK);

  status = sl_cpc_set_endpoint_option(ep, SL_CPC_ENDPOINT_ON_ERROR,
                                      (void *)on_endpoint_error);
  configASSERT(status == SL_STATUS_OK);
}

/***************************************************************************//**
 * CPC task for user endpoint 0
 ******************************************************************************/
static void cpc_app_task_endpoint_0(void *arg)
{
  sl_status_t status;
  void *read_array;
  uint16_t size;
  uint8_t *dynamic_write_array;
  EventBits_t flags;
  (void) arg;

  for (;; ) {
    flags = xEventGroupWaitBits(endpoint_connected_events,
                                ENDPOINT_0_CONNECTED,
                                pdFALSE,
                                pdFALSE,
                                portMAX_DELAY);
    if (flags & ENDPOINT_0_CONNECTED) {
      status = sl_cpc_read(&user_endpoint_handle_0, &read_array, &size, 0, 0u);
      configASSERT(status == SL_STATUS_OK || status == SL_STATUS_EMPTY);
      if (size > 0) {
        status = sl_memory_alloc(size, BLOCK_TYPE_SHORT_TERM, (void **)&dynamic_write_array);
        configASSERT(status == SL_STATUS_OK);
        memcpy(dynamic_write_array, read_array, size);
        sl_cpc_free_rx_buffer(read_array);
        status = sl_cpc_write(&user_endpoint_handle_0, dynamic_write_array, size, 0u, NULL);
        configASSERT(status == SL_STATUS_OK);
      }
    }
  }
}

/*******************************************************************************
 * CPC task for user endpoint 1
 ******************************************************************************/
static void cpc_app_task_endpoint_1(void *arg)
{
  sl_status_t status;
  void *read_array;
  uint16_t size;
  EventBits_t flags;
  (void) arg;

  for (;; ) {
    flags = xEventGroupWaitBits(endpoint_connected_events,
                                ENDPOINT_1_CONNECTED,
                                pdFALSE,
                                pdFALSE,
                                portMAX_DELAY);
    if (flags & ENDPOINT_1_CONNECTED) {
      status = sl_cpc_read(&user_endpoint_handle_1, &read_array, &size, 0u, 0u);
      configASSERT(status == SL_STATUS_OK || status == SL_STATUS_EMPTY);
      if (size > 0) {
        configASSERT(size <= sizeof(static_write_array));
        memcpy(static_write_array, read_array, size);
        sl_cpc_free_rx_buffer(read_array);
        status = sl_cpc_write(&user_endpoint_handle_1, static_write_array, size, 0, NULL);
        configASSERT(status == SL_STATUS_OK);
        // Wait for tx to complete
        xSemaphoreTake(tx_complete_semaphore, pdMS_TO_TICKS(1000));
      }
    }
  }
}

/*******************************************************************************
 * CPC task for monitoring connections
 ******************************************************************************/
static void cpc_app_task_connections(void *arg)
{
  EventBits_t flags;
  sl_status_t status;
  (void) arg;

  open_endpoint(&user_endpoint_handle_0, SL_CPC_ENDPOINT_USER_ID_0);
  xEventGroupSetBits(endpoint_connected_events, ENDPOINT_0_CONNECTED);

  open_endpoint(&user_endpoint_handle_1, SL_CPC_ENDPOINT_USER_ID_1);
  xEventGroupSetBits(endpoint_connected_events, ENDPOINT_1_CONNECTED);

  for (;; ) {
    // Wait for one or both endpoints to become disconnected (flags cleared)
    flags = xEventGroupWaitBits(endpoint_connected_events,
                                ENDPOINT_0_CONNECTED | ENDPOINT_1_CONNECTED,
                                pdFALSE,
                                pdFALSE,
                                portMAX_DELAY);

    if (!(flags & ENDPOINT_0_CONNECTED)) {
      status = sl_cpc_close_endpoint(&user_endpoint_handle_0);
      configASSERT(status == SL_STATUS_OK || status == SL_STATUS_BUSY);

      if (sl_cpc_get_endpoint_state(&user_endpoint_handle_0) == SL_CPC_STATE_FREED) {
        open_endpoint(&user_endpoint_handle_0, SL_CPC_ENDPOINT_USER_ID_0);
        xEventGroupSetBits(endpoint_connected_events, ENDPOINT_0_CONNECTED);
      } else {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }

    if (!(flags & ENDPOINT_1_CONNECTED)) {
      status = sl_cpc_close_endpoint(&user_endpoint_handle_1);
      configASSERT(status == SL_STATUS_OK || status == SL_STATUS_BUSY);

      if (sl_cpc_get_endpoint_state(&user_endpoint_handle_1) == SL_CPC_STATE_FREED) {
        open_endpoint(&user_endpoint_handle_1, SL_CPC_ENDPOINT_USER_ID_1);
        xEventGroupSetBits(endpoint_connected_events, ENDPOINT_1_CONNECTED);
      } else {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }
  }
}
/*******************************************************************************
 ************************  GLOBAL FUNCTIONS   **********************************
 ******************************************************************************/

/*******************************************************************************
 * Initialize CPC application (FreeRTOS)
 ******************************************************************************/
void cpc_app_init(void)
{
  // Create semaphore for transmission complete
  tx_complete_semaphore = xSemaphoreCreateBinary();
  configASSERT(tx_complete_semaphore != NULL);

  // Create event group for endpoint connect/disconnect events
  endpoint_connected_events = xEventGroupCreate();
  configASSERT(endpoint_connected_events != NULL);

  // Create Tasks
  xTaskCreate(cpc_app_task_endpoint_0,
              "cpc task endpoint 0",
              TASK_STACK_SIZE,
              NULL,
              TASK_PRIO,
              &task_handle_0);

  xTaskCreate(cpc_app_task_endpoint_1,
              "cpc task endpoint 1",
              TASK_STACK_SIZE,
              NULL,
              TASK_PRIO,
              &task_handle_1);

  xTaskCreate(cpc_app_task_connections,
              "cpc task connection monitor",
              TASK_STACK_SIZE,
              NULL,
              TASK_PRIO,
              &task_handle_connections);
}
