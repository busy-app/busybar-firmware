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
#include "lwip/pbuf.h"
#include <stdlib.h>
#include <stddef.h>

sl_status_t sli_si91x_host_allocate_buffer(sl_wifi_buffer_t **buffer,
                                           sl_si91x_buffer_type_t type,
                                           uint32_t buffer_size,
                                           uint32_t wait_duration_ms)
{
  struct pbuf **buf = (struct pbuf **)buffer;
  *buf              = pbuf_alloc(0, buffer_size, 0);
  return SL_STATUS_OK;
}

void *sl_si91x_host_get_buffer_data(sl_wifi_buffer_t *buffer, uint16_t offset, uint16_t *data_length)
{
}

void sli_si91x_host_free_buffer(sl_wifi_buffer_t *buffer)
{
  pbuf_free((struct pbuf *)buffer);
}
