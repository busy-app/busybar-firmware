/*******************************************************************************
* @file  event_loop.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
/**
 * @file    event_loop.c
 * @version 0.1
 * @date    29 April 2022
 *
 *
 *
 *  @brief : This file contains event handling functions of module which required by different demo application
 *
 *  @section Description  This file contains event handling functions of module which required by different demo applications
 */

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/
#include <stdio.h>
#include "event_loop.h"
#include "rsi_common_utils.h"
#include "stdint.h"
#include "rsi_bt_common_apis.h"
#include "rsi_common_apis.h"

/*==============================================*/
/**
 * @fn         rsi_app_common_event_loop
 * @brief      Initilize the specific module 
 * @param[in]  generic_task_cb, hold the information of events and commands
 * @return     int32_t, status of the function
 * @section description
 * This function is used to initilize the specific module and handle the commands and events
 */
int32_t rsi_app_common_event_loop(generic_task_cb_t *generic_task_cb)
{
  int32_t status = RSI_SUCCESS;

  //! creating module specific mutex

  generic_task_cb->event_mutex = osMutexNew(NULL);
  if (generic_task_cb->event_mutex == NULL) {
    printf("failed to create event mutex object, error = %lx \r\n", status);
    return status;
  }
  generic_task_cb->cmd_mutex = osMutexNew(NULL);
  if (generic_task_cb->cmd_mutex == NULL) {
    printf("failed to create cmd mutex object, error = %lx \r\n", status);
    return status;
  }

  //! creating module specific semaphore
  generic_task_cb->semaphore = osSemaphoreNew(1, 0, NULL);
  if (status != RSI_SUCCESS) {
    printf("\r\n failed to create semaphore object, error = %lx  \r\n", status);
    return status;
  }

  //! check for module initlization
  if (generic_task_cb->init_hook != NULL) {
    //! Initialize the specifi module
    status = generic_task_cb->init_hook();
    if (status != RSI_SUCCESS) {
      printf("\r\n Module init hook fails with error status = 0X%lx  \r\n", status);
      return status;
    }
  }

  //! event loop
  while (1) {
    //! waiting for the events
    osSemaphoreAcquire(generic_task_cb->semaphore, osWaitForever);

    //! handling pending command
    rsi_app_pending_command_handling(generic_task_cb);

    //! handling pending events
    rsi_app_pending_event_handling(generic_task_cb);
  }
}

/*==============================================*/
/**
 * @fn         rsi_app_pending_command_handling
 * @brief      Handling pending commands 
 * @param[in]  generic_task_cb, hold the information of events and commands
 * @return     int32_t, status of the function
 * @section description
 * This function handle the pending commands which are their in the queue
 */
void rsi_app_pending_command_handling(generic_task_cb_t *generic_task_cb)
{
  generic_command_message_t *generic_command_message;
  generic_command_handler_lut_entry_t cmd_hdlr;

  //! get the command packet and dequeue it
  generic_command_message = (generic_command_message_t *)rsi_app_dequeue_pkt_with_mutex(&generic_task_cb->command_queue,
                                                                                        &generic_task_cb->cmd_mutex);

  //! check command queue is empty
  if (generic_command_message == NULL) {
    return;
  }

  //! check for command id is within range
  if (generic_command_message->command_id < generic_task_cb->max_num_commands) {
    cmd_hdlr = generic_task_cb->commands_handler_lut[generic_command_message->command_id];
    //! Invoke look up table of command handler
    cmd_hdlr.handler(generic_command_message);
  }

  //! check semaphore has been utilizie or not
  if (generic_command_message->semaphore != NULL) {
    //! Post the semaphore which is waiting on command message
    osSemaphoreRelease(*(generic_command_message->semaphore));
  }

  //! check for free handler
  if (generic_command_message->free_handler != NULL) {
    //! invoke free handler of command
    //generic_command_message->free_handler(generic_command_message);
  }
}

/*==============================================*/
/**
 * @fn         rsi_app_pending_event_handling
 * @brief      Handling pending events 
 * @param[in]  generic_task_cb, hold the information of events and commands
 * @return     int32_t, status of the function
 * @section description
 * This function handle the pending events which are their in the queue
 */
int32_t rsi_app_pending_event_handling(generic_task_cb_t *generic_task_cb)
{
  int32_t status = RSI_SUCCESS;
  rsi_app_pkt_t *event_pkt;
  generic_event_message_t *event_message = NULL;
  while (1) {
    //! get the event from the highest priority queue
    event_pkt     = rsi_app_get_pending_msg_highest_pri_que(generic_task_cb->event_queues, generic_task_cb);
    event_message = (generic_event_message_t *)event_pkt;
    //! check event packet
    if (event_pkt == NULL) {
      //! packet are not availabe in any of the event queue
      break;
    } else {
      if (event_message->event_id < generic_task_cb->max_num_events) {
        //! process the message
        rsi_app_process_message(generic_task_cb, (generic_event_message_t *)event_pkt);
      } else {
        free(event_pkt);
      }
    }
  }

  return status;
}

/*==============================================*/
/**
 * @fn         rsi_app_get_pending_msg_highest_pri_que
 * @brief      Gets the packet from the queue
 * @param[in]  event_queue, hold the queue list of different priority events
 * @param[in]  generic_task_cb, hold the information of events and commands
 * @return     rsi_app_pkt_t, returns selected packet from the queue 
 * @section description
 * This function is used to get packet from the highest priority queue
 */
rsi_app_pkt_t *rsi_app_get_pending_msg_highest_pri_que(app_queue_t *event_queue, generic_task_cb_t *generic_task_cb)
{
  uint8_t ix               = 0;
  rsi_app_pkt_t *event_pkt = NULL;

  while (ix < MAX_NUM_PRIORITY_QUEUES) {
    //! get the event packet from one of the event queue and dequeue it
    event_pkt = rsi_app_dequeue_pkt_with_mutex(&event_queue[ix], &generic_task_cb->event_mutex);

    //! check for packet is empty
    if (event_pkt != NULL) {
      //! return packet address which is found in the queue
      return event_pkt;
    } else {
      //! Increment queue index so that to check for available packet in the next queue
      ix++;
    }
  }

  return event_pkt;
}

/*==============================================*/
/**
 * @fn         rsi_app_process_message
 * @brief      process the event message
 * @param[in]  generic_task_cb, hold the information of events and commands
 * @param[in]  generic_event_message, hold the information of generic event data 
 * @return     none.
 * @section description
 * This function is used to process the event message
 */
void rsi_app_process_message(generic_task_cb_t *generic_task_cb, generic_event_message_t *generic_event_message)
{
  generic_event_handler_lut_entry_t event_hdlr;
  //! check for event id is within the range
  if (generic_event_message->event_id < generic_task_cb->max_num_events) {
    event_hdlr = generic_task_cb->events_handler_lut[generic_event_message->event_id];
    if (event_hdlr.handler != NULL) {
      //! Invoke look up table of command handler
      event_hdlr.handler(generic_event_message->status, generic_event_message->event_data);
    }
  }

  //! Check for free handler
  if (generic_event_message->free_callback != NULL) {
    //! Invoke free handler of event message
    generic_event_message->free_callback(generic_event_message);
  }
}
