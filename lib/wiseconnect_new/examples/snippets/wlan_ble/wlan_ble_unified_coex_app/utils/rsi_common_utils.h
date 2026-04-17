

/*******************************************************************************
* @file  rsi_common_utils.h
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

#ifndef RSI_COMMON_UTILS_H
#define RSI_COMMON_UTILS_H

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/

#include "stdint.h"
#include "cmsis_os2.h"

/*=======================================================================*/
//   ! Defines
/*=======================================================================*/

/*=======================================================================*/
//   ! Structures
/*=======================================================================*/
//! generic packet structure.first element in packet buffer should always be pointer to next element
typedef struct rsi_app_pkt_s {
  struct rsi_app_pkt_s *next; //! pointer to next element in the queue
  uint8_t pkt_data[1];        //! transparent data structure
} rsi_app_pkt_t;
//! generic queue structure. first element in packet buffer should always be pointer to next element to use this queue
typedef struct rsi_app_queue_s {
  rsi_app_pkt_t *head;     //! generic pointer to a packet buffer
  rsi_app_pkt_t *tail;     //! generic pointer to a packet buffer
  uint32_t pkt_cnt;        //! packet count in queue
  osMutexId_t queue_mutex; //! mutex for accessing queue
} rsi_app_queue_t;

typedef struct app_queue_s {
  rsi_app_pkt_t *head; //! generic pointer to a packet buffer
  rsi_app_pkt_t *tail; //! generic pointer to a packet buffer
  uint32_t pkt_cnt;    //! packet count in queue
} app_queue_t;

typedef struct debug_log_s {
  uint32_t debug_log_id;
  uint32_t debug_systick;
} debug_log_t;
/*=======================================================================*/
//   ! Fucntion declarations
/*=======================================================================*/

void rsi_app_init_event_parameters(uint32_t *async_event_map,
                                   uint32_t *async_event_map_mask,
                                   osSemaphoreId_t *task_sem,
                                   osMutexId_t *event_map_mutex);
void rsi_app_deinit_event_parameters(osSemaphoreId_t *task_sem, osMutexId_t *event_map_mutex);
void rsi_app_set_event(uint32_t event_num,
                       uint32_t *async_event_map,
                       osSemaphoreId_t *task_sem,
                       osMutexId_t *event_map_mutex);
void rsi_app_set_event_from_isr(uint32_t event_num,
                                uint32_t *async_event_map,
                                osSemaphoreId_t *task_sem,
                                osMutexId_t *event_map_mutex);
void rsi_app_clear_event(uint32_t event_num, uint32_t *async_event_map, osMutexId_t *event_map_mutex);
void rsi_app_mask_event(uint32_t event_num, uint32_t *async_event_map_mask, osMutexId_t *event_map_mutex);
void rsi_app_unmask_event_from_isr(uint32_t event_num,
                                   uint32_t *async_event_map_mask,
                                   osSemaphoreId_t *task_sem,
                                   osMutexId_t *event_map_mutex);
void rsi_app_unmask_event(uint32_t event_num,
                          uint32_t *async_event_map_mask,
                          osSemaphoreId_t *task_sem,
                          osMutexId_t *event_map_mutex);
int32_t rsi_app_get_event(uint32_t *async_event_map, uint32_t *async_event_map_mask);
void rsi_app_enqueue_pkt(rsi_app_queue_t *queue, rsi_app_pkt_t *pkt);
void rsi_app_enqueue_pkt_with_mutex(app_queue_t *queue, rsi_app_pkt_t *pkt, osMutexId_t *queue_mutex);
rsi_app_pkt_t *rsi_app_dequeue_pkt(rsi_app_queue_t *queue);
rsi_app_pkt_t *rsi_app_dequeue_pkt_with_mutex(app_queue_t *queue, osMutexId_t *queue_mutex);
void rsi_app_enqueue_pkt_from_isr(rsi_app_queue_t *queue, rsi_app_pkt_t *pkt);
rsi_app_pkt_t *rsi_app_dequeue_pkt_from_isr(rsi_app_queue_t *queue);
void rsi_app_init_queue(rsi_app_queue_t *queue);
void _assert(uint8_t *msg, uint16_t line_number);
#endif
