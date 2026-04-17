/*******************************************************************************
* @file  event_loop.h
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
 * @file    event_loop.h
 * @version 0.1
 * @date    5 May 2022
 *
 *
 *
 *  @brief : This file contains structures and macros of event and command of a module
 *
 *  @section Description  This file contain structures and macros
 */

#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__
#include "rsi_common_utils.h"

//! defines
#define MAX_NAME_TASK                 8
#define MAX_NUM_PRIORITY_QUEUES       8
#define FIXED_LEN_RESPONSE_BUFFER_LEN 16
#define BD_ADDR_NOT_FOUND             -1

//! structures

typedef struct generic_command_message_s {
  void *next;
  uint32_t command_id;
  uint8_t argc;
  uint8_t **argv;
  osSemaphoreId_t *semaphore; //! semaphore to allow command task to proceed
  uint8_t response_status;
  uint8_t fixed_length_response_buffer[FIXED_LEN_RESPONSE_BUFFER_LEN];
  //! optional variable length buffer for response.
  uint8_t *variable_length_response_buffer;
  //! free_handler for variable len buffer
  void (*free_handler)(void *); //! handler to free command message
} generic_command_message_t;

typedef struct generic_event_message_s {
  void *next;
  void (*free_callback)(void *); //! handler to free command message
  uint16_t status;
  uint32_t event_id;
  char event_data[1]; //! Variable length event data. To be replaced with structure corresponding to each event
} generic_event_message_t;

typedef struct generic_command_handler_lut_entry_s {
  void (*handler)(generic_command_message_t *generic_command_message); //! generic function pointer to create LUTs
} generic_command_handler_lut_entry_t;
typedef struct generic_event_handler_lut_entry_s {
  void (*handler)(uint16_t status, void *event_data); //! generic function pointer to create LUTs
} generic_event_handler_lut_entry_t;

typedef struct generic_task_cb_s {
  uint8_t name[MAX_NAME_TASK]; //! for general debugging
  osMutexId_t event_mutex;     //! mutex lock to be used while enqueuing or dequeuing packets to events queue
  osMutexId_t cmd_mutex;       //! mutex lock to be used while enqueuing or dequeuing packets to commads queue
  osSemaphoreId_t semaphore;   //! task semaphore to be used while task is idle
  uint32_t max_num_commands;   //! Number of commands supported by module
  uint32_t max_num_events;     //! maximum number of events supported by module
  app_queue_t command_queue;   //! queue to enqueue commands to the task from other context
  app_queue_t event_queues[MAX_NUM_PRIORITY_QUEUES]; //! queue to hold event messages received from SAPI/Driver context
  generic_command_handler_lut_entry_t *commands_handler_lut; //! LUT for commands handlers for the module
  generic_event_handler_lut_entry_t *events_handler_lut;     //! LUT for event handler for the module
  int32_t (*init_hook)(void); //! function to perform module specific inits before entering into event loop
  void *private_cb;           //! pointer to module specific control bench
} generic_task_cb_t;

/*=======================================================================*/
//   ! Fucntion declarations
/*=======================================================================*/
int32_t rsi_app_pending_event_handling(generic_task_cb_t *generic_task_cb);
void rsi_app_pending_command_handling(generic_task_cb_t *generic_task_cb);
void rsi_app_process_message(generic_task_cb_t *generic_task_cb, generic_event_message_t *generic_event_message);
rsi_app_pkt_t *rsi_app_get_pending_msg_highest_pri_que(app_queue_t *event_queue, generic_task_cb_t *generic_task_cb);
int32_t rsi_app_common_event_loop(generic_task_cb_t *generic_task_cb);

/*=======================================================================*/
//   ! Extern Fucntions
/*=======================================================================*/
extern int32_t rsi_app_common_event_loop(generic_task_cb_t *generic_task_cb);
#endif
