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
#include "fff.h"
#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_types.h"
#include "sl_constants.h"

// Define sl_si91x_host_timestamp_t if not already defined
#ifndef SL_SI91X_HOST_TIMESTAMP_T
typedef uint32_t sl_si91x_host_timestamp_t;
#endif

typedef uint8_t sli_task_register_id_t;
typedef struct os_mutex_cb *osMutexId;

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
DECLARE_FAKE_VALUE_FUNC2(sl_status_t, sli_si91x_flush_socket_command_queues_based_on_queue_type, uint8_t, uint16_t);
DECLARE_FAKE_VALUE_FUNC1(sl_status_t, sli_si91x_flush_socket_data_queues_based_on_queue_type, uint8_t);
DECLARE_FAKE_VOID_FUNC2(sli_si91x_append_to_buffer_queue, sli_wifi_buffer_queue_t *, sl_wifi_buffer_t *);
DECLARE_FAKE_VOID_FUNC1(sli_si91x_set_event, uint32_t);
DECLARE_FAKE_VALUE_FUNC6(sl_status_t,
                         sli_wifi_wait_for_response_packet,
                         sli_wifi_buffer_queue_t *,
                         osEventFlagsId_t,
                         uint32_t,
                         uint16_t,
                         sli_wifi_wait_period_t,
                         sl_wifi_buffer_t **);
DECLARE_FAKE_VALUE_FUNC4(sl_status_t,
                         sli_si91x_host_allocate_buffer,
                         sl_wifi_buffer_t **,
                         sl_wifi_buffer_type_t,
                         uint32_t,
                         uint32_t);
DECLARE_FAKE_VALUE_FUNC1(sl_si91x_host_timestamp_t, sl_si91x_host_elapsed_time, uint32_t);
DECLARE_FAKE_VALUE_FUNC4(sl_status_t,
                         sli_si91x_driver_send_async_command,
                         uint32_t,
                         sli_wifi_command_type_t,
                         void *,
                         uint32_t);
DECLARE_FAKE_VALUE_FUNC4(sl_status_t,
                         sli_si91x_allocate_command_buffer,
                         sl_wifi_buffer_t **,
                         void **,
                         uint32_t,
                         uint32_t);
DECLARE_FAKE_VALUE_FUNC0(bool, sli_is_tcp_auto_close_enabled);
DECLARE_FAKE_VALUE_FUNC3(sl_status_t,
                         sli_osTaskRegisterSetValue,
                         const osThreadId_t,
                         const sli_task_register_id_t,
                         const uint32_t);
DECLARE_FAKE_VALUE_FUNC1(osMutexId_t, osMutexNew, const osMutexAttr_t *);
DECLARE_FAKE_VALUE_FUNC1(osStatus_t, osEventFlagsDelete, osEventFlagsId_t);
DECLARE_FAKE_VALUE_FUNC2(osStatus_t, osMutexAcquire, osMutexId_t, uint32_t);
DECLARE_FAKE_VALUE_FUNC1(osEventFlagsId_t, osEventFlagsNew, const osEventFlagsAttr_t *);
DECLARE_FAKE_VALUE_FUNC1(osStatus_t, osDelay, uint32_t);
DECLARE_FAKE_VALUE_FUNC2(uint32_t, osEventFlagsSet, void *, uint32_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, osKernelGetTickCount);
DECLARE_FAKE_VALUE_FUNC4(uint32_t, osEventFlagsWait, osEventFlagsId_t, uint32_t, uint32_t, uint32_t);
DECLARE_FAKE_VOID_FUNC1(CORE_ExitAtomic, uint32_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, CORE_EnterAtomic);
DECLARE_FAKE_VALUE_FUNC1(osStatus_t, osMutexDelete, osMutexId_t);
DECLARE_FAKE_VALUE_FUNC1(osStatus_t, osMutexRelease, osMutexId_t);
DECLARE_FAKE_VOID_FUNC_VARARG(sl_redirect_log, const char *, ...);
DECLARE_FAKE_VOID_FUNC1(sli_wifi_set_event, uint32_t);
DECLARE_FAKE_VOID_FUNC2(sli_wifi_append_to_buffer_queue, sli_wifi_buffer_queue_t *, sl_wifi_buffer_t *);
DECLARE_FAKE_VALUE_FUNC2(size_t, sl_strnlen, char *, size_t);
DECLARE_FAKE_VALUE_FUNC0(uint32_t, osKernelGetTickFreq);
