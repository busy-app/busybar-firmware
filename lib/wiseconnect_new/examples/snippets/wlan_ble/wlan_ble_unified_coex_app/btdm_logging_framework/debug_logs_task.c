/*******************************************************************************
 * @file  debug_logs_task.c
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

#include <rsi_common_app.h>
#include "event_loop.h"
#include "debug_logs_event_hdlr_auto_gen.h"
#include "cmsis_os2.h"
#include "rsi_ble.h"
#include <stdio.h>
int32_t debug_logs_init_hook(void);

void rsi_task_bt_debug_logs(void);

extern generic_task_cb_t debug_logs_generic_cb;
extern osSemaphoreId_t bt_debug_logs_sem;
void rsi_task_bt_debug_logs(void)
{

  //int32_t status = RSI_SUCCESS;
  debug_logs_generic_cb.init_hook          = debug_logs_init_hook;
  debug_logs_generic_cb.events_handler_lut = debug_logs_events_hdlr_lut;
  debug_logs_generic_cb.max_num_events     = max_debug_logs_event_id;
  osSemaphoreAcquire(bt_debug_logs_sem, 0);
  osSemaphoreDelete(bt_debug_logs_sem);
  rsi_app_common_event_loop(&debug_logs_generic_cb);

  while (1)
    ;
}

int32_t debug_logs_init_hook(void)
{
  int32_t status = RSI_SUCCESS;
  status =
    rsi_bt_debug_logs_register_callbacks(RSI_BT_ON_CONTROLLER_LOGS, (void *)rsi_bt_on_controller_logs_driver_callback);
  if (status != RSI_SUCCESS) {
    printf("rsi_bt_debug_logs_register_callbacks failed \n");
  }
  return 0;
}