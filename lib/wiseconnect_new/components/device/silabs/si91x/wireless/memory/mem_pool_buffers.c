/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sli_mem_pool.h"
#include "sl_status.h"
#include "sl_wifi_host_interface.h"
#include "sl_constants.h"
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"
//#include <stdlib.h>
//#include <stddef.h>

#ifndef SLI_WIFI_BUFFER_BLOCK_SIZE
#ifdef SPI_EXTENDED_TX_LEN_2K
#define SLI_WIFI_BUFFER_BLOCK_SIZE 2300
#else
#define SLI_WIFI_BUFFER_BLOCK_SIZE 1616
#endif
#endif

#ifndef SLI_WIFI_BUFFER_SIZE
#define SLI_WIFI_BUFFER_SIZE (8 * SLI_WIFI_BUFFER_BLOCK_SIZE)
#endif

static sli_mem_pool_handle_t mem_pool;
static void *allocated_wifi_buffer;
uint32_t sli_allocated_buffers = 0;
uint32_t sli_freed_buffers     = 0;
sl_status_t sli_si91x_host_init_buffer_manager(void)
{
  allocated_wifi_buffer = malloc(SLI_WIFI_BUFFER_SIZE);
  if (allocated_wifi_buffer == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  sli_mem_pool_create(&mem_pool,
                      SLI_WIFI_BUFFER_BLOCK_SIZE,
                      SLI_WIFI_BUFFER_SIZE / SLI_WIFI_BUFFER_BLOCK_SIZE,
                      allocated_wifi_buffer,
                      SLI_WIFI_BUFFER_SIZE);
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_host_deinit_buffer_manager(void)
{
  free(allocated_wifi_buffer);
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_host_allocate_buffer(sl_wifi_buffer_t **buffer,
                                           sl_wifi_buffer_type_t type,
                                           uint32_t buffer_size,
                                           uint32_t wait_duration_ms)
{
  UNUSED_PARAMETER(type);
  UNUSED_PARAMETER(buffer_size);
  uint32_t start = osKernelGetTickCount();
  do {
    *buffer = sli_mem_pool_alloc(&mem_pool);
    if (*buffer != NULL) {
      break;
    } else {
      osDelay(SLI_SYSTEM_MS_TO_TICKS(1));
    }
  } while ((osKernelGetTickCount() - start) < wait_duration_ms);

  if (*buffer == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  ++sli_allocated_buffers;
  (*buffer)->node.node = NULL;
  (*buffer)->length    = SLI_WIFI_BUFFER_BLOCK_SIZE - sizeof(sl_wifi_buffer_t);
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
  ++sli_freed_buffers;
  sli_mem_pool_free(&mem_pool, buffer);
}
