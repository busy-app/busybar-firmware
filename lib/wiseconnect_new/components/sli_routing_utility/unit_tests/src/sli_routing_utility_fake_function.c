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

#include "sli_routing_utility_fake_function.h"
#include "sli_queue_manager_types.h"

DEFINE_FFF_GLOBALS
DEFINE_FAKE_VALUE_FUNC4(sl_status_t,
                        destination_packet_handler,
                        void *,
                        uint32_t,
                        sli_routing_utility_packet_status_handler_t,
                        void *);
DEFINE_FAKE_VALUE_FUNC2(uint32_t, osEventFlagsSet, void *, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, CORE_EnterAtomic);
DEFINE_FAKE_VOID_FUNC1(CORE_ExitAtomic, uint32_t);
DEFINE_FAKE_VALUE_FUNC2(sl_status_t, sli_queue_manager_enqueue, sli_queue_t *, void *);
DEFINE_FAKE_VALUE_FUNC2(sl_status_t, sli_queue_manager_enqueue_node, sli_queue_t *, sli_queue_node_t *);