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
#pragma once
#include <stdlib.h>
#include <stdint.h>
#include "sl_status.h"
#include "sl_net_si91x.h"
#include "sl_net_constants.h"
#include "sli_net_common_utility.h"
#include "sl_net_si91x_integration_handler.h"
#include "sl_ip_types.h"
#include "sl_si91x_socket_utility.h"
#include "sl_constants.h"
#include "fff.h"

DECLARE_FAKE_VALUE_FUNC4(sl_status_t, sl_si91x_wifi_set_certificate_index, uint8_t, uint8_t, const void *, uint32_t);
DECLARE_FAKE_VALUE_FUNC1(uint8_t, sli_get_certificate_index, sl_net_credential_id_t);
DECLARE_FAKE_VALUE_FUNC1(int, sli_si91x_check_cred_type, sl_net_credential_type_t);
DECLARE_FAKE_VALUE_FUNC2(sli_si91x_cert_type_t,
                         convert_to_si91x_cert_type,
                         sl_net_credential_id_t,
                         sl_net_credential_type_t);
DECLARE_FAKE_VALUE_FUNC2(sl_status_t, sli_si91x_flush_socket_command_queues_based_on_queue_type, uint8_t, uint16_t);
DECLARE_FAKE_VALUE_FUNC1(sl_status_t, sli_si91x_flush_socket_data_queues_based_on_queue_type, uint8_t);
DECLARE_FAKE_VALUE_FUNC0(sl_wifi_operation_mode_t, sli_wifi_get_opermode);
DECLARE_FAKE_VALUE_FUNC1(sl_status_t, sli_si91x_flush_select_request_table, uint16_t);
DECLARE_FAKE_VALUE_FUNC1(sl_status_t, sli_si91x_flush_all_tx_wifi_queues, uint16_t);
DECLARE_FAKE_VALUE_FUNC1(sl_status_t, sli_si91x_flush_all_socket_data_queues, uint8_t);
DECLARE_FAKE_VALUE_FUNC2(sl_status_t, sli_si91x_flush_all_socket_command_queues, uint16_t, uint8_t);
DECLARE_FAKE_VALUE_FUNC2(sl_status_t, sli_convert_si91x_event_to_sl_net_event, const uint16_t, sl_net_event_t);
DECLARE_FAKE_VALUE_FUNC2(sl_status_t, sli_si91x_vap_shutdown, uint8_t, sli_si91x_bsd_disconnect_reason_t);
DECLARE_FAKE_VALUE_FUNC1(bool, sli_wifi_is_ip_address_zero, const sl_ip_address_t *);
DECLARE_FAKE_VALUE_FUNC2(sl_status_t, sl_si91x_default_handler, sl_net_event_t, sl_wifi_buffer_t *);