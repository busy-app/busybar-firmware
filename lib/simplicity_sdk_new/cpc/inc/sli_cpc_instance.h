/***************************************************************************/ /**
 * @file
 * @brief CPC Instance Internal Definitions
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_CPC_INSTANCE_H
#define SLI_CPC_INSTANCE_H

#include "sl_slist.h"

#include "sli_cpc.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_memory.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
#include "sl_cmsis_os2_common.h"
#endif
#include "sl_cpc_kernel_config.h"
#endif

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
#include "sli_cpc_system_primary.h"
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
#include "sli_cpc_system_secondary.h"
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
#include "sl_memory_manager.h"
#else
#include "sli_mem_pool.h"
#endif

// Derive name of sli_cpc_instance_t based on the peripheral name
#define SL_CPC_INSTANCE_NAME(_peripheral)       SL_CONCAT_PASTER_2(sli_cpc_instance_, _peripheral)

#define SL_CPC_INSTANCE_PTR(_peripheral)        SL_CONCAT_PASTER_2(&sli_cpc_instance_, _peripheral)

// This set of macro allows to compute different values
// based on the user configuration stored in the instance
#define SLI_CPC_RX_DATA_MAX_LENGTH_INST(_inst)             (SLI_CPC_RX_DATA_MAX_LENGTH_CFG((_inst)->rx_user_payload_max_length))
#define SLI_CPC_HDLC_REJECT_MAX_COUNT_INST(_inst)          (SLI_CPC_HDLC_REJECT_MAX_COUNT_CFG((_inst)->rx_buffer_max_count))
#define SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_INST(_inst)        (SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_CFG((_inst)->rx_buffer_max_count))
#define SLI_CPC_TX_QUEUE_ITEM_SFRAME_MAX_COUNT_INST(_inst) (SLI_CPC_TX_QUEUE_ITEM_SFRAME_MAX_COUNT_CFG(SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_INST(_inst)))
#define SLI_CPC_BUFFER_HANDLE_MAX_COUNT_INST(_inst)        (SLI_CPC_BUFFER_HANDLE_MAX_COUNT_CFG( \
                                                              (_inst)->tx_queue_item_max_count,  \
                                                              (_inst)->rx_buffer_max_count,      \
                                                              SLI_CPC_TX_QUEUE_ITEM_SFRAME_MAX_COUNT_INST(_inst)))
#define SLI_CPC_HDLC_HEADER_MAX_COUNT_INST(_inst)          (SLI_CPC_HDLC_HEADER_MAX_COUNT_CFG(SLI_CPC_BUFFER_HANDLE_MAX_COUNT_INST(_inst)))
#define SLI_CPC_RX_FRAME_MAX_LENGTH_INST(_inst)            (SLI_CPC_RX_FRAME_MAX_LENGTH_CFG(SLI_CPC_RX_DATA_MAX_LENGTH_INST(_inst)))
#define SLI_CPC_RX_BUFFER_MAX_LENGTH_INST(_inst)           (SLI_CPC_RX_BUFFER_MAX_LENGTH_CFG(SLI_CPC_RX_FRAME_MAX_LENGTH_INST(_inst)))
#define SLI_CPC_EVENT_SIGNAL_MAX_COUNT_INST(_inst)         (SLI_CPC_EVENT_SIGNAL_MAX_COUNT_CFG( \
                                                              (_inst)->tx_queue_item_max_count, \
                                                              SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_INST(_inst)))

/** @brief Struct representing a memory pool handle. */
typedef struct sl_cpc_mem_pool_handle_t {
  // support having the memory manager or not
#if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
  sl_memory_pool_t      *handle;
#else
  sli_mem_pool_handle_t handle;
#endif

#if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  // counter of allocated blocks in the pool
  uint32_t used_block_cnt;      ///< Number of block in use
#endif
} sl_cpc_mem_pool_handle_t;

struct sli_cpc_instance {
  sl_slist_node_t *endpoints;
  sl_slist_node_t *defer_endpoint_free_list;
  sl_slist_node_t *transmit_queue;
  sl_slist_node_t *pending_on_security_ready_queue;
  sl_slist_node_t *write_complete_callback_list;
  sl_slist_node_t *expired_retransmit_list;

  sl_cpc_dispatcher_handle_t callback_dispatcher_handle;
  sl_cpc_dispatcher_handle_t retransmit_dispatcher_handle;

  // dispatcher holding variables
  sl_slist_node_t *process_queue;
  uint8_t event_counter;

#if defined(SL_CATALOG_KERNEL_PRESENT)
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
  /*
   * __ALIGNED is part of CMSIS-Core, but it's possible that an CMSIS-RTOS2 OS
   * is used without CMSIS-Core. In that case, instead of providing statically
   * allocated memory, let the OS dynamically allocate it.
   */
#define THREAD_STACK_SIZE   (SL_CPC_TASK_STACK_SIZE * sizeof(void *)) & 0xFFFFFFF8u

  __ALIGNED(4) uint8_t endpoints_list_mutex_cb[osMutexCbSize];
  __ALIGNED(4) uint8_t event_semaphore_cb[osSemaphoreCbSize];
  __ALIGNED(4) uint8_t thread_cb[osThreadCbSize];
  __ALIGNED(8) uint8_t thread_stack[THREAD_STACK_SIZE];
  __ALIGNED(4) uint8_t tx_item_availability_semaphore_cb[osSemaphoreCbSize];
#endif

  osThreadId_t thread_id;                       ///< thread ID
  osSemaphoreId_t event_signal;                 ///< event signal
  osMutexId_t endpoints_list_lock;              ///< endpoints list lock
  osSemaphoreId_t tx_item_availability_signal;  ///< transmit queue item availability signal
#else
  volatile uint32_t rx_process_flag;
#endif

  sli_cpc_drv_t *driver;
  sli_cpc_drv_capabilities_t sli_cpc_driver_capabilities;

  uint32_t rx_user_payload_max_length;          ///< Maximum length of a user payload (USER CONFIG)
  uint16_t rx_buffer_max_count;                 ///< Maximum count of RX buffer (USER CONFIG)
  uint16_t tx_queue_item_max_count;             ///< Maximum TX queue item (USER CONFIG)

  // The default value of 0 means that there is no maximum length.
  // This will stay at 0 with a Linux host and will be set to a
  // certain value with a MCU Primary.
  uint16_t remote_tx_payload_max_length;

  sl_slist_node_t *postponed_free_rx_queue_item;
#if !defined(SL_CATALOG_KERNEL_PRESENT)
  uint16_t tx_item_used_count;
#endif
  uint16_t sframe_tx_item_used_count;

  sl_cpc_mem_pool_handle_t mempool_buffer_handle;
  sl_cpc_mem_pool_handle_t mempool_hdlc_header;
  sl_cpc_mem_pool_handle_t mempool_hdlc_reject;
  sl_cpc_mem_pool_handle_t mempool_rx_buffer;
  sl_cpc_mem_pool_handle_t mempool_rx_queue_item;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  sl_cpc_mem_pool_handle_t mempool_command_handle;
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  sl_cpc_mem_pool_handle_t mempool_system_command;
#endif

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  sl_cpc_mem_pool_handle_t mempool_tx_security_tag;
#endif

#if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  uint32_t endpoints_used_block_cnt;
#endif

#if ((SL_CPC_DEBUG_CORE_EVENT_COUNTERS == 1) \
  || (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1))
  sl_cpc_core_debug_t core_debug;
#endif

  sli_cpc_system_endpoint_t system_ep;

  bool cpc_core_in_reset;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  bool cpc_initialized;

  bool secondary_protocol_version_mismatch;
  bool secondary_capabilities_mismatch;
  bool secondary_application_version_mismatch;
#else
  bool soft_reset;
#endif // SL_CATALOG_CPC_PRIMARY_PRESENT
};

typedef struct sli_cpc_instance sli_cpc_instance_t;

/***************************************************************************/ /**
 * Get back a CPC instance from a system endpoint pointer.
 ******************************************************************************/
static inline sli_cpc_instance_t* sli_cpc_instance_from_system_ep(sli_cpc_system_endpoint_t *system_ep)
{
  return container_of(system_ep, sli_cpc_instance_t, system_ep);
}

/***************************************************************************/ /**
 * Initialize a CPC instance.
 *
 * @param[in] instance      CPC instance pointer.
 *
 * @retval  SL_STATUS_OK    Initialized endpoint successfully.
 * @retval  Other sl_status_t if error occurred.
 ******************************************************************************/
sl_status_t sli_cpc_instance_init(sli_cpc_instance_t *inst);

/***************************************************************************/ /**
 * Deinitialize a CPC instance.
 *
 * @param[in] instance      CPC instance pointer.
 ******************************************************************************/
void sli_cpc_instance_deinit(sli_cpc_instance_t *inst);

/***************************************************************************/ /**
 * Start an instance. It will start processing ingress and egress packets.
 *
 * @param[in] instance      CPC instance pointer.
 *
 * @retval  SL_STATUS_OK    Initialized endpoint successfully.
 * @retval  Other sl_status_t if error occurred.
 ******************************************************************************/
sl_status_t sli_cpc_instance_start(sli_cpc_instance_t *inst);

/***************************************************************************/ /**
 * Return the bitrate of the bus associated with that instance.
 *
 * @param[in] instance      CPC instance pointer.
 *
 * @retval  Bus Bitrate
 ******************************************************************************/
uint32_t sli_cpc_instance_bus_get_bitrate(sli_cpc_instance_t *inst);

/***************************************************************************/ /**
 * Return the maximum bitrate of the bus associated with that instance.
 *
 * @param[in] instance      CPC instance pointer.
 *
 * @retval  Maximum Bus Bitrate
 ******************************************************************************/
uint32_t sli_cpc_instance_bus_get_max_bitrate(sli_cpc_instance_t *inst);

/***************************************************************************//**
 * Allocate kernel-related resources for a CPC instance.
 *
 * @retval  SL_STATUS_OK    Successfully initialized kernel resources
 * @retval  Other sl_status_t if error occurred.
 ******************************************************************************/
sl_status_t sli_cpc_instance_kernel_init(sli_cpc_instance_t *inst);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************/ /**
 * Checks if CPC has been initialized
 *
 * @retval  True if initialized, false otherwise.
 ******************************************************************************/
bool sli_cpc_instance_is_initialized(sli_cpc_instance_t *inst);
#endif
#endif /* SLI_CPC_INSTANCE_H */
