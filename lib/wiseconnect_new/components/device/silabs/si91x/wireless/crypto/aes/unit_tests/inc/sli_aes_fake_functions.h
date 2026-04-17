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

#ifndef SL_AES_FAKE_FUNCTIONS_H
#define SL_AES_FAKE_FUNCTIONS_H

#include "fff.h"
#include "sl_status.h"
#include "sl_si91x_aes.h"
#include "sl_si91x_driver.h"

void *my_host_get_buffer_data_fake(sl_wifi_buffer_t *buffer, uint16_t offset, uint16_t *out_length);
DECLARE_FAKE_VALUE_FUNC7(sl_status_t,
                         sli_si91x_driver_send_command,
                         uint32_t,
                         sli_wifi_command_type_t,
                         const void *,
                         uint32_t,
                         sli_wifi_wait_period_t,
                         void *,
                         sl_wifi_buffer_t **);
DECLARE_FAKE_VOID_FUNC1(sli_si91x_host_free_buffer, sl_wifi_buffer_t *);
DECLARE_FAKE_VALUE_FUNC3(void *, sli_wifi_host_get_buffer_data, sl_wifi_buffer_t *, uint16_t, uint16_t *);

#endif // SL_AES_FAKE_FUNCTIONS_H