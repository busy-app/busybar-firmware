/*******************************************************************************
 * @file
 * @brief
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

#include "sli_aes_fake_functions.h"
#include "sl_status.h"

typedef struct {
  union {
    struct {
      uint16_t length;
      uint16_t command;
      uint8_t unused[12];
    };
    uint8_t desc[16];
  };
  uint8_t data[16];
} fake_packet_t;

static fake_packet_t fake_packet;

void *my_host_get_buffer_data_fake(sl_wifi_buffer_t *buffer, uint16_t offset, uint16_t *out_length)
{
  fake_packet.length  = 16;
  fake_packet.command = 0; // can be 0 or any value
  for (int i = 0; i < 12; ++i)
    fake_packet.unused[i] = 0;
  for (int i = 0; i < 16; ++i)
    fake_packet.data[i] = 0xAB;
  if (out_length) {
    *out_length = fake_packet.length;
  }
  return &fake_packet;
}

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC7(sl_status_t,
                        sli_si91x_driver_send_command,
                        uint32_t,
                        sli_wifi_command_type_t,
                        const void *,
                        uint32_t,
                        sli_wifi_wait_period_t,
                        void *,
                        sl_wifi_buffer_t **);
DEFINE_FAKE_VOID_FUNC1(sli_si91x_host_free_buffer, sl_wifi_buffer_t *);
DEFINE_FAKE_VALUE_FUNC3(void *, sli_wifi_host_get_buffer_data, sl_wifi_buffer_t *, uint16_t, uint16_t *);