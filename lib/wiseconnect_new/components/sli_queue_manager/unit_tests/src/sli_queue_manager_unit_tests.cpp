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
#include "sli_queue_manager.h"
#include "sli_buffer_manager.h"
#include "sli_queue_manager_fake_function.h"
}
typedef struct {
  sl_slist_node_t node;
  int id;
} temp_node;

sl_status_t fake_buffer_manager_allocate_buffer(const sli_buffer_manager_pool_types_t p, 
                                                const sli_buffer_manager_allocation_types_t at,
                                                const uint32_t t,
                                                sli_buffer_t *buffer) {
  *buffer = (sli_buffer_t)malloc(128);
  return SL_STATUS_OK;
}

sl_status_t fake_buffer_manager_free_buffer(sli_buffer_t buffer) {
  free((void *)buffer);
  return SL_STATUS_OK;
}

bool sli_queue_manager_node_match_handler(sli_queue_t *handle, void *node, void *node_match_data)
{
  temp_node *buffer = (temp_node *)node;
  if (buffer->id == *((uint32_t *)node_match_data)) {
    return true;
  }
  return false;
}

void sli_queue_manager_flush_handler(sli_queue_t *handle, void *node, void *context)
{
  return;
}

TEST(sli_queue_manager, sli_queue_manager_init_null_handle)
{
  sl_status_t status;
  status = sli_queue_manager_init(NULL, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_init_valid_handle)
{
  sli_queue_t handle;
  sl_status_t status;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_enqueue_null_handle)
{
  sl_slist_node_t node;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_enqueue(NULL, &node);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_enqueue_valid_handle)
{
  sli_queue_t handle;
  sl_slist_node_t node;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_dequeue_null_handle)
{
  void *node;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_dequeue(NULL, &node);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_dequeue_empty_queue)
{
  sli_queue_t handle;
  void *node;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_dequeue(&handle, &node);
  EXPECT_TRUE(status == SL_STATUS_EMPTY);
}

TEST(sli_queue_manager, sli_queue_manager_dequeue_singleton_queue)
{
  sli_queue_t handle;
  sl_slist_node_t node;
  void *node_ptr;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_dequeue(&handle, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_dequeue_non_empty_queue)
{
  sli_queue_t handle;
  sl_slist_node_t node1;
  sl_slist_node_t node2;
  void *node_ptr;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node1);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node2);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_dequeue(&handle, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_add_to_queue_head_null_handle)
{
  sl_slist_node_t node;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_add_to_queue_head(NULL, &node);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_add_to_queue_head_empty_queue)
{
  sli_queue_t handle;
  sl_slist_node_t node;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_add_to_queue_head(&handle, &node);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_add_to_queue_head_non_empty_queue)
{
  sli_queue_t handle;
  sl_slist_node_t node1;
  sl_slist_node_t node2;
  sl_slist_node_t *node_ptr;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node1);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_add_to_queue_head(&handle, &node2);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_null_handle)
{
  void *node;
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_remove_node_from_queue(NULL, sli_queue_manager_node_match_handler, &id, &node);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_null_node)
{
  sli_queue_t handle;
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, sli_queue_manager_node_match_handler, &id, NULL);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_empty_queue)
{
  sli_queue_t handle;
  void *node;
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, sli_queue_manager_node_match_handler, &id, &node);
  EXPECT_TRUE(status == SL_STATUS_EMPTY);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_match_found_single_element)
{
  sli_queue_t handle;
  temp_node node1;
  node1.id = 1;
  sl_status_t status;
  void *node_ptr;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node1.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, sli_queue_manager_node_match_handler, &id, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_match_found_two_element_head)
{
  sli_queue_t handle;
  temp_node node1;
  node1.id = 1;
  temp_node node2;
  node2.id = 2;
  sl_status_t status;
  void *node_ptr;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node1.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node2.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, sli_queue_manager_node_match_handler, &id, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_match_found_two_element_tail)
{
  sli_queue_t handle;
  temp_node node1;
  node1.id = 1;
  temp_node node2;
  node2.id = 2;
  sl_status_t status;
  void *node_ptr;
  int id = 2;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node1.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node2.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, sli_queue_manager_node_match_handler, &id, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_match_found_neither_head_nor_tail)
{
  sli_queue_t handle;
  temp_node node1;
  node1.id = 1;
  temp_node node2;
  node2.id = 2;
  temp_node node3;
  node3.id = 3;
  sl_status_t status;
  void *node_ptr;
  int id = 2;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node1.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node2.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node3.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, sli_queue_manager_node_match_handler, &id, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_no_match_found)
{
  sli_queue_t handle;
  temp_node node1;
  node1.id = 1;
  temp_node node2;
  node2.id = 2;
  sl_status_t status;
  void *node_ptr;
  int id = 3;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node1.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node2.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, sli_queue_manager_node_match_handler, &id, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_NOT_FOUND);
}

TEST(sli_queue_manager, sli_queue_manager_remove_node_from_queue_null_id_handler)
{
  sli_queue_t handle;
  sl_status_t status;
  void *node_ptr;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_remove_node_from_queue(&handle, NULL, &id, &node_ptr);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_flush_nodes_from_queue_null_id_handler)
{
  sli_queue_t handle;
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_flush_nodes_from_queue(&handle, NULL, &id, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_flush_nodes_from_queue_null_handle)
{
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_flush_nodes_from_queue(NULL, NULL, &id, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_flush_nodes_from_queue_empty_queue)
{
  sli_queue_t handle;
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_flush_nodes_from_queue(&handle,
                                                    sli_queue_manager_node_match_handler,
                                                    &id,
                                                    sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_flush_nodes_from_queue_match_present_head)
{
  sli_queue_t handle;
  temp_node node1;
  node1.id = 1;
  temp_node node2;
  node2.id = 2;
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node1.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node2.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_flush_nodes_from_queue(&handle,
                                                    sli_queue_manager_node_match_handler,
                                                    &id,
                                                    sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_flush_nodes_from_queue_match_present_tail)
{
  sli_queue_t handle;
  temp_node node1;
  node1.id = 1;
  temp_node node2;
  node2.id = 2;
  sl_status_t status;
  int id = 2;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node1.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &(node2.node));
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_flush_nodes_from_queue(&handle,
                                                    sli_queue_manager_node_match_handler,
                                                    &id,
                                                    sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_flush_queue_empty_queue)
{
  sli_queue_t handle;
  sl_status_t status;
  int id = 1;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_flush_queue(&handle, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_flush_queue_single_element)
{
  sli_queue_t handle;
  sl_slist_node_t node1;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node1);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_flush_queue(&handle, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_flush_queue_two_element)
{
  sli_queue_t handle;
  sl_slist_node_t node1, node2;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node1);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node2);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_flush_queue(&handle, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_deinit_null_handle)
{
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_deinit(NULL, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
}

TEST(sli_queue_manager, sli_queue_manager_deinit_empty_queue)
{
  sli_queue_t handle;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_deinit(&handle, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}

TEST(sli_queue_manager, sli_queue_manager_deinit_non_empty_queue)
{
  sli_queue_t handle;
  sl_slist_node_t node1;
  sl_status_t status;
  sli_buffer_manager_allocate_buffer_fake.custom_fake = fake_buffer_manager_allocate_buffer;
  sli_buffer_manager_free_buffer_fake.custom_fake = fake_buffer_manager_free_buffer;
  status = sli_queue_manager_init(&handle, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_enqueue(&handle, &node1);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_queue_manager_deinit(&handle, sli_queue_manager_flush_handler);
  EXPECT_TRUE(status == SL_STATUS_OK);
}