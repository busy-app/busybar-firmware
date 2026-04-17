/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_host_interface.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"
#include <string.h>
extern osMutexId_t malloc_free_mutex;
sl_status_t sli_si91x_host_init_buffer_manager(void);
sl_status_t sli_si91x_host_deinit_buffer_manager(void);
sl_status_t sli_si91x_host_allocate_buffer(sl_wifi_buffer_t **buffer,
                                           sl_wifi_buffer_type_t type,
                                           uint32_t buffer_size,
                                           uint32_t wait_duration_ms);
void *sl_si91x_host_get_buffer_data(sl_wifi_buffer_t *buffer, uint16_t offset, uint16_t *data_length);
void sli_si91x_host_free_buffer(sl_wifi_buffer_t *buffer);

sl_status_t sli_si91x_host_init_buffer_manager(void)
{
  if (malloc_free_mutex == NULL) {
    malloc_free_mutex = osMutexNew(NULL);
  }
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_host_deinit_buffer_manager(void)
{
  if (malloc_free_mutex != NULL) {
    osMutexDelete(malloc_free_mutex);
    malloc_free_mutex = NULL;
  }
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_host_allocate_buffer(sl_wifi_buffer_t **buffer,
                                           sl_wifi_buffer_type_t type,
                                           uint32_t buffer_size,
                                           uint32_t wait_duration_ms)
{
  (void)type;
  osMutexAcquire(malloc_free_mutex, 0xFFFFFFFFUL);
  uint32_t start         = osKernelGetTickCount();
  sl_wifi_buffer_t *temp = NULL;
  do {
    temp = (sl_wifi_buffer_t *)malloc(buffer_size + sizeof(*temp));
    if (temp != NULL) {
      break;
    } else {
      osDelay(SLI_SYSTEM_MS_TO_TICKS(1));
    }
  } while ((osKernelGetTickCount() - start) < wait_duration_ms);

  osMutexRelease(malloc_free_mutex);
  if (temp == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  temp->length    = buffer_size;
  temp->node.node = NULL;
  *buffer         = temp;
  return SL_STATUS_OK;
}

void *sl_si91x_host_get_buffer_data(sl_wifi_buffer_t *buffer, uint16_t offset, uint16_t *data_length)
{
  if (offset >= buffer->length) {
    return NULL;
  }
  if (data_length) {
    *data_length = (uint16_t)(buffer->length) - offset;
  }
  return (void *)&buffer->data[offset];
}

void sli_si91x_host_free_buffer(sl_wifi_buffer_t *buffer)
{
  if (buffer == NULL) {
    return;
  }
  osMutexAcquire(malloc_free_mutex, 0xFFFFFFFFUL);
  free((void *)buffer);
  osMutexRelease(malloc_free_mutex);
}
