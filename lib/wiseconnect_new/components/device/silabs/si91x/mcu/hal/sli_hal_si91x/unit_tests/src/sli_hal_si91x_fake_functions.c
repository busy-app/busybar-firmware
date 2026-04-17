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
#include "sli_hal_si91x_fake_functions.h"
DEFINE_FFF_GLOBALS

DEFINE_FAKE_VALUE_FUNC(uint32_t, CORE_EnterAtomic);
DEFINE_FAKE_VOID_FUNC1(CORE_ExitAtomic, uint32_t);
DEFINE_FAKE_VALUE_FUNC3(osThreadId_t, osThreadNew, osThreadFunc_t, void *, const osThreadAttr_t *);
DEFINE_FAKE_VALUE_FUNC1(osEventFlagsId_t, osEventFlagsNew, const osEventFlagsAttr_t *);
DEFINE_FAKE_VALUE_FUNC1(osStatus_t, osThreadTerminate, osThreadId_t);
DEFINE_FAKE_VALUE_FUNC1(osStatus_t, osEventFlagsDelete, osEventFlagsId_t);
DEFINE_FAKE_VALUE_FUNC2(sl_status_t, sli_queue_manager_init, sli_queue_t *, sli_buffer_manager_pool_types_t);
DEFINE_FAKE_VALUE_FUNC4(uint32_t, osEventFlagsWait, osEventFlagsId_t, uint32_t, uint32_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC2(uint32_t, osEventFlagsSet, osEventFlagsId_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC0(uint32_t, osKernelGetTickFreq);
DEFINE_FAKE_VALUE_FUNC3(void *, sli_wifi_host_get_buffer_data, sl_wifi_buffer_t *, uint16_t, uint16_t *);
DEFINE_FAKE_VALUE_FUNC2(sl_status_t, sli_queue_manager_dequeue, sli_queue_t *, void **);
DEFINE_FAKE_VOID_FUNC(sl_si91x_host_clear_sleep_indicator);
DEFINE_FAKE_VALUE_FUNC2(sl_status_t, sli_queue_manager_enqueue, sli_queue_t *, void *);
DEFINE_FAKE_VALUE_FUNC2(sl_status_t, sli_queue_manager_deinit, sli_queue_t *, sli_queue_manager_flush_handler_t);
DEFINE_FAKE_VALUE_FUNC1(sl_status_t, sli_si91x_host_free_buffer, sl_wifi_buffer_t *);
DEFINE_FAKE_VALUE_FUNC3(sl_status_t, sli_si91x_bus_write_frame, sl_wifi_buffer_t *, uint8_t *, uint32_t);
DEFINE_FAKE_VALUE_FUNC5(sl_status_t,
                        sli_routing_utility_route_packet,
                        sli_routing_table_t *,
                        uint16_t,
                        void *,
                        uint16_t,
                        void *);
DEFINE_FAKE_VALUE_FUNC4(sl_status_t,
                        sli_si91x_set_rx_event,
                        void *,
                        uint32_t,
                        sli_routing_utility_packet_status_handler_t,
                        void *);
DEFINE_FAKE_VALUE_FUNC4(sl_status_t,
                        sli_buffer_manager_allocate_buffer,
                        const sli_buffer_manager_pool_types_t,
                        const sli_buffer_manager_allocation_types_t,
                        const uint32_t,
                        sli_buffer_t *);
DEFINE_FAKE_VALUE_FUNC1(sl_status_t, sli_buffer_manager_free_buffer, sli_buffer_t);
DEFINE_FAKE_VALUE_FUNC0(sl_status_t, sli_si91x_req_wakeup);
DEFINE_FAKE_VALUE_FUNC1(sl_status_t, sli_si91x_bus_read_interrupt_status, uint16_t *);
DEFINE_FAKE_VOID_FUNC1(sli_si91x_update_flash_command_status, bool);
DEFINE_FAKE_VOID_FUNC1(sli_si91x_update_tx_command_status, bool);