/********************************************************************************
 * @file  app_xapi_cmd_event.h
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef APP_XAPI_CMD_EVENT_H
#define APP_XAPI_CMD_EVENT_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define APP_NET_UP_EVENT 0

typedef struct {
  uint8_t event_id; // Store the event ID (as a uint8_t)
  void *data;       // Store a pointer to any data (void pointer for flexibility)
} event_data_t;

void wifi_cmd_event_init(void);
void cmd_event_thread(void *args);
QueueHandle_t wifi_cmd_get_queue_handle(void);

#endif // APP_XAPI_CMD_EVENT_H
