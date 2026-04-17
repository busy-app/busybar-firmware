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
#include "sli_buffer_manager.h"
#include "sl_slist.h"
#include "cmsis_os2.h"
#include "sli_buffer_manager_fake_function.h"
}
typedef struct {
  sl_slist_node_t next;          //< Next node(used only in case of common mempool).
  void *mempool_memory;          ///< Memory pool memory.
  sli_mem_pool_handle_t mempool; ///< Memory pool handler.

  uint16_t max_buffer_count;       ///< Maximum buffer count.
  uint16_t allocated_buffer_count; ///< Allocated buffer count.
  bool is_common_pool;             ///< Whether the buffer has been allocated from common mempool.
} sli_buffer_manager_mempool_handler_t;
typedef struct {
  sli_buffer_manager_mempool_handler_t
    *buffer_manager_mempool_handler; ///< pointer of the mempool from which the data has been allocated.
  uint8_t data[];                    ///< Data.
 } sli_internal_buffer_t;

TEST(sli_buffer_manager,sli_buffer_manager_init_null_configuration){
    sl_status_t status;
    status = sli_buffer_manager_init(NULL);
    EXPECT_TRUE(status == SL_STATUS_NULL_POINTER);
}

TEST(sli_buffer_manager,sli_buffer_manager_init_success){
  sl_status_t status; 
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL;i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_buffer_manager_deinit();
}


TEST(sli_buffer_manager,sli_buffer_manager_init_dedicated_pool_fail){
  sl_status_t status; 
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL;i++){
    dedicated_pool_info[i].block_count = 90000;
    dedicated_pool_info[i].block_size = 1640000; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_NO_MORE_RESOURCE);
  sli_buffer_manager_deinit();
}

TEST(sli_buffer_manager,sli_buffer_manager_init_common_pool_fail){
  sl_status_t status; 
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 90000;
  configuration.common_pool_info.block_size = 1640000;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL;i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_NO_MORE_RESOURCE);
  sli_buffer_manager_deinit();
}


TEST(sli_buffer_manager,sli_buffer_manager_init_common_pool_config_failure){
  sl_status_t status; 
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 0;
  configuration.common_pool_info.block_size = 0;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL;i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
  sli_buffer_manager_deinit();
}


TEST(sli_buffer_manager,sli_buffer_manager_init_dedicated_pool_config_failure){
  sl_status_t status; 
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL;i++){
    dedicated_pool_info[i].block_count = 0;
    dedicated_pool_info[i].block_size = 0; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_INVALID_PARAMETER);
  sli_buffer_manager_deinit();
}


TEST(sli_buffer_manager,sli_buffer_manager_free_buffer_null_pointer){
    sl_status_t status;
    status = sli_buffer_manager_free_buffer(NULL);
    EXPECT_TRUE(status == SL_STATUS_NULL_POINTER);
}

TEST(sli_buffer_manager,sli_buffer_manager_free_buffer_dedicated_pool){
  sl_status_t status;
  sli_buffer_t buffer;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer);
  EXPECT_TRUE(status==SL_STATUS_OK);                                   
  status = sli_buffer_manager_free_buffer(buffer);
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_buffer_manager_deinit();
}

TEST(sli_buffer_manager,sli_buffer_manager_free_buffer_common_pool_alive_allocations){
  sl_status_t status;
  sli_buffer_t buffer1,buffer2;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 2;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK); 
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);                                  
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);                                   
  status = sli_buffer_manager_free_buffer(buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_buffer_manager_deinit();                                   
}


TEST(sli_buffer_manager, sli_buffer_manager_free_buffer_common_pool_remove_from_queue_head){
  sl_status_t status;
  sli_buffer_t buffer1,buffer2,buffer3;
  sli_internal_buffer_t *internal_buffer;
  uint8_t *temp;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer3);
  EXPECT_TRUE(status==SL_STATUS_OK);

  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);
                                  
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);
                                   
  status = sli_buffer_manager_free_buffer(buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_buffer_manager_deinit();
}


TEST(sli_buffer_manager, sli_buffer_manager_free_buffer_common_pool_remove_from_queue_tail){
  sl_status_t status;
  sli_buffer_t buffer1,buffer2,buffer3,buffer4;
  sli_internal_buffer_t *internal_buffer;
  uint8_t *temp;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer3);
  EXPECT_TRUE(status==SL_STATUS_OK);

  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);
                                  
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);

  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer4);
  EXPECT_TRUE(status==SL_STATUS_OK);

  status = sli_buffer_manager_free_buffer(buffer4);
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_buffer_manager_deinit();
}


TEST(sli_buffer_manager, sli_buffer_manager_free_buffer_common_pool_remove_from_queue_invalid){
  sl_status_t status;
  sli_buffer_t buffer1,buffer2,buffer3;
  sli_internal_buffer_t *internal_buffer;
  uint8_t *temp;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer3);
  EXPECT_TRUE(status==SL_STATUS_OK);

  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);
                                  
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);
  internal_buffer =  (sli_internal_buffer_t *)malloc(1648); 
  internal_buffer->buffer_manager_mempool_handler = (sli_buffer_manager_mempool_handler_t *)malloc(sizeof(sli_buffer_manager_mempool_handler_t)); 
  internal_buffer->buffer_manager_mempool_handler->is_common_pool = true;
  internal_buffer->buffer_manager_mempool_handler->allocated_buffer_count = 1; 
  internal_buffer->buffer_manager_mempool_handler->max_buffer_count = 1;
  internal_buffer->buffer_manager_mempool_handler->mempool_memory = internal_buffer;
  internal_buffer->buffer_manager_mempool_handler->mempool = (sli_mem_pool_handle_t)malloc(sizeof(sli_mem_pool_handle_t));
  internal_buffer->buffer_manager_mempool_handler->mempool.block_count = 1;
  internal_buffer->buffer_manager_mempool_handler->mempool.block_size = 1648;
  internal_buffer->buffer_manager_mempool_handler->mempool.data = internal_buffer;
  internal_buffer->buffer_manager_mempool_handler->next.node=NULL;
  status = sli_buffer_manager_free_buffer(internal_buffer->data);
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_buffer_manager_deinit();
}


TEST(sli_buffer_manager, sli_buffer_manager_free_buffer_common_pool_remove_from_queue_head_last_used){
  sl_status_t status;
  sli_buffer_t buffer1,buffer2,buffer3,buffer4,buffer5;
  sli_internal_buffer_t *internal_buffer;
  uint8_t *temp;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  configuration.common_pool_info.block_count = 2;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer3);
  EXPECT_TRUE(status==SL_STATUS_OK);

  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);
                                  
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);

  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer4);
  EXPECT_TRUE(status==SL_STATUS_OK);
                                  
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer5);
  EXPECT_TRUE(status==SL_STATUS_OK);

  status = sli_buffer_manager_free_buffer(buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);

  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);

  status = sli_buffer_manager_free_buffer(buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK);

  status = sli_buffer_manager_free_buffer(buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);  
  sli_buffer_manager_deinit();
}



TEST(sli_buffer_manager,sli_buffer_manager_deinit_uninitialized){
  sl_status_t status;  
  status = sli_buffer_manager_deinit();
  EXPECT_TRUE(status==SL_STATUS_OK);
}

TEST(sli_buffer_manager, sli_buffer_manager_deinit_valid_mempool_handlers){
  sl_status_t status; 
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  status = sli_buffer_manager_deinit();
  EXPECT_TRUE(status==SL_STATUS_OK);  
}


TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_from_dedicated_pool_uninitialized_state){
  sl_status_t status;
  sli_buffer_t buffer;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer);
  EXPECT_TRUE(status == SL_STATUS_NOT_INITIALIZED);
  sli_buffer_manager_deinit();                                   
}

TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_from_common_mempool_uninitialized_state){
  sl_status_t status;
  sli_buffer_t buffer;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  
  status = sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer);
  sli_buffer_manager_deinit();                                
  EXPECT_TRUE(status == SL_STATUS_NOT_INITIALIZED);                                   
}

TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_with_dedicated_pool_pass_from_dedicated_pool){
  sl_status_t status;
  sli_buffer_t buffer;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer);
  sli_buffer_manager_deinit();                                   
  EXPECT_TRUE(status==SL_STATUS_OK);                                   
}

TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_with_common_pool_pass_from_common_pool){
  sl_status_t status;
  sli_buffer_t buffer;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer);
  sli_buffer_manager_deinit();                                   
  EXPECT_TRUE(status==SL_STATUS_OK);                                   
}

TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_with_common_pool_pass_from_dedicated_pool){
  sl_status_t status;
  sli_buffer_t buffer;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer);
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer);                    
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_buffer_manager_deinit();                                                                     
}

TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_with_common_pool_pass_from_new_common_pool){
  sl_status_t status;
  sli_buffer_t buffer1,buffer2,buffer3;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer3);
  EXPECT_TRUE(status == SL_STATUS_OK); 
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);                                  
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
  EXPECT_TRUE(status==SL_STATUS_OK); 
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer2);
  EXPECT_TRUE(status==SL_STATUS_OK);
  sli_buffer_manager_deinit();                                                                     
}

TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_with_common_pool_fail_from_new_common_pool){
  sl_status_t status;
  sli_buffer_t buffer1,buffer2,buffer3;
  sli_buffer_manager_pool_info_t dedicated_pool_info; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640000;
  dedicated_pool_info.block_count = 1;
  dedicated_pool_info.block_size = 1640;
  configuration.pool_info[0] = &dedicated_pool_info;
  for(int i = 1 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    configuration.pool_info[i] = NULL;
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer3);
  EXPECT_TRUE(status == SL_STATUS_OK);                                   
  while(1){
    sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
    status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                     1000,
                                     &buffer1);
    if(status == SL_STATUS_OK){
      continue;
    }
    else{
      break;
    }
  }
  EXPECT_TRUE(status==SL_STATUS_ALLOCATION_FAILED);  
  sli_buffer_manager_deinit();                                                                     
}

TEST(sli_buffer_manager,sli_buffer_manager_allocate_buffer_with_dedicated_pool_fail){
  sl_status_t status;
  sli_buffer_t buffer;
  sli_buffer_manager_pool_info_t dedicated_pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; 
  sli_buffer_manager_configuration_t configuration;
  configuration.common_pool_info.block_count = 1;
  configuration.common_pool_info.block_size = 1640;
  for(int i = 0 ; i < SLI_BUFFER_MANAGER_MAX_POOL; i++){
    dedicated_pool_info[i].block_count = 1;
    dedicated_pool_info[i].block_size = 1640; 
    configuration.pool_info[i] = &dedicated_pool_info[i];
  }
  status = sli_buffer_manager_init(&configuration);
  EXPECT_TRUE(status == SL_STATUS_OK);
  sli_mem_pool_alloc_fake.return_val = (void *)malloc(1648);
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer);
  EXPECT_TRUE(status==SL_STATUS_OK);
  osKernelGetTickCount_reset();
  sli_mem_pool_alloc_fake.return_val = NULL;
  uint32_t return_val_seq[2] = {0,1001};
  osKernelGetTickCount_fake.return_val_seq = return_val_seq;
  osKernelGetTickCount_fake.return_val_seq_len = 2;
  osKernelGetTickCount_fake.return_val_seq_idx = 0;
  status=sli_buffer_manager_allocate_buffer(SLI_BUFFER_MANAGER_CE_TX_POOL,
                                     SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED,
                                     1000,
                                     &buffer);
  osKernelGetTickCount_reset();
  EXPECT_TRUE(status == SL_STATUS_ALLOCATION_FAILED);
}