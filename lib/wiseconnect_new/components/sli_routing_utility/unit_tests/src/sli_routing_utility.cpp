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

#include <gtest/gtest.h>
extern "C" {
#include "sli_routing_utility.h"
#include "sl_wifi_device.h"
#include "sli_routing_utility_types.h"
#include "sli_routing_utility_fake_function.h"
}

static sli_queue_t queue_handle = {0};

TEST(sli_routing_utility, sli_routing_utility_route_queue_node_InvalidPacketType) {
    sli_routing_entry_t entry = {nullptr, nullptr, 0, nullptr, nullptr, 0};
    sli_routing_table_t routing_table = {&entry, 1};
    sli_queue_node_t packet;
    destination_packet_handler_reset();
    osEventFlagsSet_reset();
    sl_status_t status = sli_routing_utility_route_queue_node(&routing_table, 1, &packet, 10, nullptr); 
    EXPECT_EQ(status, SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_routing_utility, sli_routing_utility_route_queue_node_ValidPacketTypeWithHandler) {
    sli_routing_entry_t entry = {destination_packet_handler, nullptr, 0, nullptr, nullptr, 0};
    sli_routing_table_t routing_table = {&entry, 1};
    sli_queue_node_t packet;
    destination_packet_handler_reset();
    osEventFlagsSet_reset();
    destination_packet_handler_fake.return_val = SL_STATUS_OK;

    sl_status_t status = sli_routing_utility_route_queue_node(&routing_table, 0, &packet, 10, nullptr);
    EXPECT_EQ(status, SL_STATUS_OK);
    EXPECT_EQ(destination_packet_handler_fake.call_count, 1);
}

TEST(sli_routing_utility, sli_routing_utility_route_queue_node_ValidPacketTypeWithQueue) {
    sli_routing_entry_t entry = {nullptr, nullptr, 0, &queue_handle, (osEventFlagsId_t)1, 1};
    sli_routing_table_t routing_table = {&entry, 1};
    sli_queue_node_t packet;
    destination_packet_handler_reset();
    osEventFlagsSet_reset();
    sli_queue_manager_enqueue_node_reset();
    osEventFlagsSet_fake.return_val = 0;

    sl_status_t enqueue_status = sli_queue_manager_enqueue_node(&queue_handle, &packet);
    EXPECT_EQ(enqueue_status, SL_STATUS_OK);

    sl_status_t status = sli_routing_utility_route_queue_node(&routing_table, 0, &packet, 10, nullptr);
    EXPECT_EQ(status, SL_STATUS_OK);
    EXPECT_EQ(osEventFlagsSet_fake.call_count, 1);
}

TEST(sli_routing_utility, sli_routing_utility_route_queue_node_ValidPacketTypeWithHandlerAndQueue) {
    sli_routing_entry_t entry = {destination_packet_handler, nullptr, 1, &queue_handle, (osEventFlagsId_t)1, 1};
    sli_routing_table_t routing_table = {&entry, 1};
    sli_queue_node_t packet;
    destination_packet_handler_reset();
    osEventFlagsSet_reset();
    sli_queue_manager_enqueue_node_reset();
    destination_packet_handler_fake.return_val = SL_STATUS_OK;
    osEventFlagsSet_fake.return_val = 0;

    sl_status_t enqueue_status = sli_queue_manager_enqueue_node(&queue_handle, &packet);
    EXPECT_EQ(enqueue_status, SL_STATUS_OK);
    sl_status_t status = sli_routing_utility_route_queue_node(&routing_table, 0, &packet, 10, nullptr);
    EXPECT_EQ(status, SL_STATUS_OK);
    EXPECT_EQ(destination_packet_handler_fake.call_count, 1);
    EXPECT_EQ(osEventFlagsSet_fake.call_count, 1);
}

TEST(sli_routing_utility, sli_routing_utility_route_packet_InvalidPacketType) {
    sli_routing_entry_t entry = {nullptr, nullptr, 0, nullptr, nullptr, 0};
    sli_routing_table_t routing_table = {&entry, 1};
    destination_packet_handler_reset();    
    sl_status_t status = sli_routing_utility_route_packet(&routing_table, 1, nullptr, 10, nullptr); 
    EXPECT_EQ(status, SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_routing_utility, sli_routing_utility_route_packet_ValidPacketTypeWithHandler) {
    sli_routing_entry_t entry = {destination_packet_handler, nullptr, 1, nullptr, nullptr, 0};
    sli_routing_table_t routing_table = {&entry, 1};
    destination_packet_handler_reset();
    destination_packet_handler_fake.return_val = SL_STATUS_OK;

    sl_status_t status = sli_routing_utility_route_packet(&routing_table, 0, nullptr, 10, nullptr);
    EXPECT_EQ(status, SL_STATUS_OK);
    EXPECT_EQ(destination_packet_handler_fake.call_count, 1);
}

TEST(sli_routing_utility, sli_routing_utility_route_packet_ValidPacketTypeWithQueue) {
    sli_routing_entry_t entry = {nullptr, nullptr, 0, &queue_handle, (osEventFlagsId_t)1, 1};
    sli_routing_table_t routing_table = {&entry, 1};
    destination_packet_handler_reset();
    sli_queue_manager_enqueue_reset();
    osEventFlagsSet_reset();
    osEventFlagsSet_fake.return_val = 0;

    // Actually call the real function to test integration
    void *test_packet = (void*)0x1234;  // Use a test pointer
    sl_status_t enqueue_status = sli_queue_manager_enqueue(&queue_handle, test_packet);
    EXPECT_EQ(enqueue_status, SL_STATUS_OK);

    sl_status_t status = sli_routing_utility_route_packet(&routing_table, 0, nullptr, 10, nullptr);
    EXPECT_EQ(status, SL_STATUS_OK);
    EXPECT_EQ(osEventFlagsSet_fake.call_count, 1);
}

TEST(sli_routing_utility, sli_routing_utility_route_packet_ValidPacketTypeWithHandlerAndQueue) {
    sli_routing_entry_t entry = {destination_packet_handler, nullptr, 1, &queue_handle, (osEventFlagsId_t)1, 1};
    sli_routing_table_t routing_table = {&entry, 1};
    destination_packet_handler_reset();
    osEventFlagsSet_reset();
    sli_queue_manager_enqueue_reset();
    destination_packet_handler_fake.return_val = SL_STATUS_OK;
    osEventFlagsSet_fake.return_val = 0;

    // Actually call the real function to test integration  
    void *test_packet = (void*)0x1234;  // Use a test pointer
    sl_status_t enqueue_status = sli_queue_manager_enqueue(&queue_handle, test_packet);
    EXPECT_EQ(enqueue_status, SL_STATUS_OK);

    sl_status_t status = sli_routing_utility_route_packet(&routing_table, 0, nullptr, 10, nullptr);
    EXPECT_EQ(status, SL_STATUS_OK);
    EXPECT_EQ(destination_packet_handler_fake.call_count, 1);
    EXPECT_EQ(osEventFlagsSet_fake.call_count, 1);
}