/***************************************************************************/ /**
 * @file
 * @brief CPC API implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_cpc.h"
#include "sli_cpc_timer.h"
#include "sli_cpc.h"
#include "sl_atomic.h"
#include "sl_status.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "sli_cpc_assert.h"
#include "sli_cpc_trace.h"
#include "sli_cpc_hdlc.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_crc.h"
#include "sli_cpc_debug.h"
#include "sl_cpc_config.h"
#include "sli_cpc_instance.h"
#include "sli_cpc_memory.h"
#include "sli_cpc_system_common.h"
#include "sli_cpc_types.h"

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
#include "sl_cpc_system_primary.h"
#include "sli_cpc_reboot_sequence.h"
#include "sli_cpc_system_primary.h"
#include "sli_cpc_fwu.h"
#include "sli_cpc_trace.h"
#include "sl_cpc_primary_config.h"
#endif
#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
#include "sli_cpc_system_secondary.h"
#endif

#include "sl_cpc_instance_handles.h"

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
#define LOCK_ENDPOINTS_LIST(instance)    if (osKernelGetState() == osKernelRunning) \
  { SLI_CPC_ASSERT(osMutexAcquire((instance)->endpoints_list_lock, osWaitForever) == osOK); }

#define RELEASE_ENDPOINTS_LIST(instance) if (osKernelGetState() == osKernelRunning) \
  { SLI_CPC_ASSERT(osMutexRelease((instance)->endpoints_list_lock) == osOK); }

#define LOCK_ENDPOINT(ep)                SLI_CPC_ASSERT(ep->state != SL_CPC_STATE_FREED); \
  if (osKernelGetState() == osKernelRunning)                                              \
  { SLI_CPC_ASSERT(osMutexAcquire(ep->lock, osWaitForever) == osOK); }

#define RELEASE_ENDPOINT(ep)             SLI_CPC_ASSERT(ep->state != SL_CPC_STATE_FREED); \
  if (osKernelGetState() == osKernelRunning)                                              \
  { SLI_CPC_ASSERT(osMutexRelease(ep->lock) == osOK); }
#else
// silence warnings where instance is passed just for locking purpose
#define LOCK_ENDPOINTS_LIST(instance) ((void)instance)
#define RELEASE_ENDPOINTS_LIST(instance)
#define LOCK_ENDPOINT(ep)
#define RELEASE_ENDPOINT(ep)
#endif

#define ABS(a) (unsigned) ((a) < 0 ? -(a) : (a))

/*******************************************************************************
 ***************************  GLOBAL VARIABLES   *******************************
 ******************************************************************************/

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
bool secondary_protocol_version_mismatch = false;
bool secondary_capabilities_mismatch = false;
bool secondary_application_version_mismatch = false;

sli_cpc_instance_t g_instance = {
  .rx_user_payload_max_length = SL_CPC_RX_PAYLOAD_MAX_LENGTH,
  .tx_queue_item_max_count = SL_CPC_TX_QUEUE_ITEM_MAX_COUNT,
  .rx_buffer_max_count = SL_CPC_RX_BUFFER_MAX_COUNT,
};

sli_cpc_instance_t* sl_cpc_instances[] = {
  &g_instance,
};

const uint8_t sl_cpc_instances_count = 1;
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static sl_status_t handle_to_ep(sl_cpc_endpoint_handle_t *handle,
                                sl_cpc_endpoint_t **endpoint);
static sl_status_t handle_to_ep_enter_api(sl_cpc_endpoint_handle_t *handle,
                                          sl_cpc_endpoint_t **endpoint);

#if defined(SL_CATALOG_KERNEL_PRESENT)
static bool endpoint_abort_read(sl_cpc_endpoint_t *endpoint);
#endif

static sl_status_t init_instance(sli_cpc_instance_t *inst);
static sl_status_t start_instance(sli_cpc_instance_t *inst);

static sl_status_t init_endpoint(sl_cpc_endpoint_handle_t *endpoint,
                                 sli_cpc_instance_t *inst,
                                 uint8_t id,
                                 uint8_t flags);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) || (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
static sl_cpc_endpoint_t *find_endpoint(uint8_t id);
#endif
static sl_cpc_endpoint_t* find_instance_endpoint(sli_cpc_instance_t *inst, uint8_t id);

static bool sort_endpoints(sl_slist_node_t *item_l,
                           sl_slist_node_t *item_r);

static sl_status_t write(sl_cpc_endpoint_t *ep,
                         void* data,
                         uint16_t data_length,
                         uint8_t flags,
                         void *on_write_completed_arg);

static void notify_write_complete(void *data);

static bool notify_state_change(sl_cpc_endpoint_t *endpoint);

static void submit_write_completion(sl_cpc_buffer_handle_t *buffer_handle, sl_status_t write_status);

static void decode_packet(sli_cpc_instance_t *inst);

static void receive_ack(sl_cpc_endpoint_t *endpoint,
                        uint8_t ack);

static void receive_iframe(sl_cpc_endpoint_t *endpoint,
                           sl_cpc_buffer_handle_t *rx_handle,
                           uint8_t control,
                           uint16_t data_length);

static void receive_sframe(sl_cpc_endpoint_t *endpoint,
                           sl_cpc_buffer_handle_t *rx_handle,
                           uint8_t control,
                           uint16_t data_length);

static void receive_uframe(sl_cpc_endpoint_t *endpoint,
                           sl_cpc_buffer_handle_t *rx_handle,
                           uint8_t control,
                           uint16_t data_length);

static sl_status_t transmit_ack(sl_cpc_endpoint_t *endpoint);

static sl_status_t re_transmit_frame(sl_cpc_endpoint_t *endpoint);

static void transmit_reject(sli_cpc_instance_t *inst,
                            sl_cpc_endpoint_t *endpoint,
                            uint8_t address,
                            uint8_t ack,
                            sl_cpc_reject_reason_t reason);

static void queue_for_transmission(sl_cpc_endpoint_t *ep,
                                   sl_cpc_buffer_handle_t *buffer_handle,
                                   bool signal_tx_event);

static sl_status_t process_tx_queue(sli_cpc_instance_t *inst);

static void process_free(sli_cpc_instance_t *inst);

static void defer_endpoint_free(sl_cpc_endpoint_t *ep);

static bool is_endpoint_tx_done(const sl_cpc_endpoint_t *ep);

static bool is_endpoint_empty(const sl_cpc_endpoint_t *ep);

static bool is_endpoint_connection_active(const sl_cpc_endpoint_t *ep);

static bool free_closed_endpoint_if_empty(sl_cpc_endpoint_t *ep);

static void notify_endpoint_is_closed(void);

static sl_status_t terminate_endpoint(sl_cpc_endpoint_t *endpoint, bool queue_free, uint8_t flags);

static void clean_tx_queues(sl_cpc_endpoint_t *endpoint);

static void clean_rx_queues(sl_cpc_endpoint_t *endpoint);

static void re_transmit_timeout_callback(sli_cpc_timer_handle_t *handle, void *data);

static void notify_error(sl_cpc_endpoint_t *endpoint);

static bool is_seq_valid(uint8_t seq, uint8_t ack);

static bool cpc_enter_api(sl_cpc_endpoint_handle_t *endpoint_handle);

static void cpc_exit_api(sl_cpc_endpoint_handle_t *endpoint_handle);

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
static void on_state_change(sl_cpc_security_state_t old, sl_cpc_security_state_t new_state);
#endif

static bool notify_ep_on_connect(sl_cpc_endpoint_t * ep, sl_status_t status);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
static sl_status_t cpc_core_is_ready(void);

static void on_connection_request_reply(sli_cpc_property_id_t property_id,
                                        void *property_value,
                                        size_t property_length,
                                        void *on_reply_arg,
                                        sl_status_t status);

static void on_endpoint_terminate_reply(sli_cpc_property_id_t property_id,
                                        void *property_value,
                                        size_t property_length,
                                        void *on_reply_arg,
                                        sl_status_t status);

static void on_endpoint_shutdown_reply(sli_cpc_property_id_t property_id,
                                       void *property_value,
                                       size_t property_length,
                                       void *on_reply_arg,
                                       sl_status_t status);
#endif

bool sli_cpc_endpoints_are_all_closed(sli_cpc_instance_t *inst);

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
static sli_cpc_system_endpoint_t* endpoint_get_system_ep(sl_cpc_endpoint_t *ep);

static sl_status_t open_endpoint(sl_cpc_endpoint_handle_t *endpoint,
                                 sli_cpc_instance_t *inst,
                                 uint8_t id,
                                 uint8_t flags,
                                 uint8_t tx_window_size);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize permanent memory for CPC
 ******************************************************************************/
sl_status_t sl_cpc_init_permanent_allocations(void)
{
  sl_status_t status;

  for (uint8_t i = 0; i < sl_cpc_instances_count; i++) {
    status = sli_cpc_memory_init(sl_cpc_instances[i]);
    if (status != SL_STATUS_OK) {
      SLI_CPC_ASSERT(false);
      return status;
    }
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  for (uint8_t i = 0; i < sl_cpc_instances_count; i++) {
    status = sli_cpc_instance_kernel_init(sl_cpc_instances[i]);
    if (status != SL_STATUS_OK) {
      SLI_CPC_ASSERT(false);
      return status;
    }
  }
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Initialize CPC module.
 ******************************************************************************/
sl_status_t sl_cpc_init(void)
{
  sl_status_t status = SL_STATUS_OK;

#if !defined(SL_CATALOG_SL_MAIN_PRESENT)
  status = sl_cpc_init_permanent_allocations();
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(false);
    return status;
  }
#endif

  for (uint8_t i = 0; i < sl_cpc_instances_count; i++) {
    status = init_instance(sl_cpc_instances[i]);
    SLI_CPC_ASSERT(status == SL_STATUS_OK);
  }

  return status;
}

/***************************************************************************/ /**
 * Return instance associated with endpoint.
 ******************************************************************************/
sli_cpc_instance_t* sli_cpc_get_instance(uint8_t ep_id)
{
#if defined(g_instance) || defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  (void)ep_id;

  return &g_instance;
#else

  if (ep_id == 0) {
    SLI_CPC_ASSERT(0);
    return NULL;
  }
  #if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_SECURITY_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_SECURITY) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_SECURITY_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_BLUETOOTH_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_BLUETOOTH_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_BLUETOOTH) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_BLUETOOTH_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_RAIL_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_RAIL_INSTANCE)
  else if (ep_id == SL_CPC_SLI_CPC_ENDPOINT_RAIL_DOWNSTREAM
           || ep_id == SL_CPC_SLI_CPC_ENDPOINT_RAIL_UPSTREAM) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_RAIL_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_ZIGBEE_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_ZIGBEE_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_ZIGBEE) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_ZIGBEE_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_ZWAVE_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_ZWAVE_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_ZWAVE) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_ZWAVE_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_CONNECT_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_CONNECT_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_CONNECT) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_CONNECT_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_GPIO_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_GPIO_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_GPIO) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_GPIO_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_OPENTHREAD_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_OPENTHREAD_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_OPENTHREAD) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_OPENTHREAD_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_WISUN_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_WISUN_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_WISUN) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_WISUN_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_WIFI_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_WIFI_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_WIFI) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_WIFI_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_15_4_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_15_4_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_15_4) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_15_4_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_CLI_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_CLI_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_CLI) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_CLI_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_BLUETOOTH_RCP_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_BLUETOOTH_RCP_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_BLUETOOTH_RCP) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_BLUETOOTH_RCP_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_ACP_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_ACP_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_ACP) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_ACP_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_SE_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_SE_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_SE) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_SE_INSTANCE);
  }
  #endif
  #if (SLI_CPC_ENDPOINT_NVM3_ENABLED >= 1) && defined(SL_CPC_ENDPOINT_NVM3_INSTANCE)
  else if (ep_id == SL_CPC_ENDPOINT_NVM3) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_NVM3_INSTANCE);
  }
  #endif
  #if defined(SL_CPC_ENDPOINT_USER_INSTANCE)
  else if (ep_id >= SL_CPC_ENDPOINT_USER_ID_0) {
    return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_USER_INSTANCE);
  }
  #endif

  return SL_CPC_INSTANCE_PTR(SL_CPC_ENDPOINT_DEFAULT_INSTANCE);
#endif
}

/***************************************************************************/ /**
 * Initialize a CPC user endpoint
 ******************************************************************************/
sl_status_t sl_cpc_init_user_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                      sl_cpc_user_endpoint_id_t id,
                                      uint8_t flags)
{
  sli_cpc_instance_t *inst;

  // Validate input parameters
  if (endpoint_handle == NULL) {
    return SL_STATUS_INVALID_HANDLE;
  }

  if ((uint8_t) id < SL_CPC_USER_ENDPOINT_ID_START
      || (uint8_t) id > SL_CPC_USER_ENDPOINT_ID_END) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (flags & ~(SL_CPC_ENDPOINT_FLAG_IFRAME_DISABLE
                | SL_CPC_ENDPOINT_FLAG_UFRAME_ENABLE
                | SL_CPC_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE
                | SL_CPC_ENDPOINT_FLAG_DISABLE_ENCRYPTION)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  inst = sli_cpc_get_instance(id);
  if (inst == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return init_endpoint(endpoint_handle, inst, id, flags);
}

/***************************************************************************//**
 * Allocate a CPC service endpoint
 ******************************************************************************/
sl_status_t sli_cpc_init_service_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                          sl_cpc_service_endpoint_id_t id,
                                          uint8_t flags)
{
  sli_cpc_instance_t *inst;

  // Validate input parameters
  if (endpoint_handle == NULL) {
    return SL_STATUS_INVALID_HANDLE;
  }

  if (id > SLI_CPC_SERVICE_ENDPOINT_ID_END) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (flags & ~(SL_CPC_ENDPOINT_FLAG_DISABLE_ENCRYPTION
                | SL_CPC_ENDPOINT_FLAG_IFRAME_DISABLE
                | SL_CPC_ENDPOINT_FLAG_UFRAME_ENABLE
                | SL_CPC_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  inst = sli_cpc_get_instance(id);
  if (inst == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return init_endpoint(endpoint_handle, inst, id, flags);
}

/***************************************************************************//**
 * Initialize a CPC temporary endpoint
 ******************************************************************************/
sl_status_t sli_cpc_init_temporary_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                            uint8_t *id,
                                            uint8_t flags)
{
  sli_cpc_instance_t *inst;
  sl_status_t status;
  uint8_t id_free = 0;

  // Validate input parameters
  if (endpoint_handle == NULL) {
    return SL_STATUS_INVALID_HANDLE;
  }

  if (id == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // one instance is responsible for handling all temporary endpoints
  inst = sli_cpc_get_instance(SLI_CPC_TEMPORARY_ENDPOINT_ID_START);
  if (inst == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  LOCK_ENDPOINTS_LIST(inst);

  for (uint16_t id_iterator = SLI_CPC_TEMPORARY_ENDPOINT_ID_START;
       id_iterator <= SLI_CPC_TEMPORARY_ENDPOINT_ID_END;
       id_iterator++) {
    const sl_cpc_endpoint_t *endpoint = find_instance_endpoint(inst, (uint8_t)id_iterator);
    if (endpoint == NULL) {
      id_free = (uint8_t)id_iterator;
      break;
    }
  }

  if (id_free == 0) {
    status = SL_STATUS_NO_MORE_RESOURCE;
    goto release_ep_list;
  }

  SLI_CPC_ASSERT(id_free >= SLI_CPC_TEMPORARY_ENDPOINT_ID_START);
  *id = id_free;

  status = init_endpoint(endpoint_handle, inst, *id, flags);

  release_ep_list:
  RELEASE_ENDPOINTS_LIST(inst);

  return status;
}

/***************************************************************************//**
 * Set endpoint option
 ******************************************************************************/
sl_status_t sl_cpc_set_endpoint_option(sl_cpc_endpoint_handle_t *endpoint_handle,
                                       sl_cpc_endpoint_option_t option,
                                       void *value)
{
  sl_cpc_endpoint_t *ep;
  sl_status_t status = SL_STATUS_OK;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
  if (sli_cpc_is_fwu_in_progress() && (endpoint_handle->id != SL_CPC_ENDPOINT_SYSTEM)) {
    return SL_STATUS_BUSY;
  }
#endif

  if (endpoint_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }

  LOCK_ENDPOINT(ep);
  switch (option) {
    case SL_CPC_ENDPOINT_ON_IFRAME_RECEIVE:
      ep->on_iframe_data_reception = (sl_cpc_on_data_reception_t)value;
      break;
    case SL_CPC_ENDPOINT_ON_IFRAME_RECEIVE_ARG:
      ep->on_iframe_data_reception_arg = value;
      break;
    case SL_CPC_ENDPOINT_ON_UFRAME_RECEIVE:
      ep->on_uframe_data_reception = (sl_cpc_on_data_reception_t)value;
      break;
    case SL_CPC_ENDPOINT_ON_UFRAME_RECEIVE_ARG:
      ep->on_uframe_data_reception_arg = value;
      break;
    case SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED:
      ep->on_iframe_write_completed = (sl_cpc_on_write_completed_t)value;
      break;
    case SL_CPC_ENDPOINT_ON_UFRAME_WRITE_COMPLETED:
      ep->on_uframe_write_completed = (sl_cpc_on_write_completed_t)value;
      break;
    case SL_CPC_ENDPOINT_ON_FINAL:
#if defined(SL_CPC_ON_FINAL_PRESENT)
      ep->poll_final.on_final = (sl_cpc_on_final_t)value;
#else
      status = SL_STATUS_INVALID_PARAMETER;
#endif
      break;
    case SL_CPC_ENDPOINT_ON_FINAL_ARG:
      ep->poll_final.on_fnct_arg = value;
      break;
    case SL_CPC_ENDPOINT_ON_ERROR:
      ep->on_error = (sl_cpc_on_error_callback_t)value;
      break;
    case SL_CPC_ENDPOINT_ON_ERROR_ARG:
      ep->on_error_arg = value;
      break;
    case SL_CPC_ENDPOINT_ON_CONNECT:
      ep->on_connect = (sl_cpc_on_connect_callback_t)value;
      break;
    case SL_CPC_ENDPOINT_ON_CONNECT_ARG:
      ep->on_connect_arg = value;
      break;
#if defined(SL_CATALOG_KERNEL_PRESENT)
    case SL_CPC_ENDPOINT_SHUTDOWN_TIMEOUT:
      ep->shutdown_timeout = *(uint32_t*)value;
      break;
    case SL_CPC_ENDPOINT_WRITE_TIMEOUT:
      ep->write_timeout = *(uint32_t*)value;
      break;
#endif
    default:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  RELEASE_ENDPOINT(ep);

  cpc_exit_api(endpoint_handle);
  return status;
}

/***************************************************************************//**
 * Shutdown endpoint's connection
 ******************************************************************************/
sl_status_t sl_cpc_shutdown_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                     uint8_t flags)
{
  sl_cpc_endpoint_t *ep;
  sl_cpc_endpoint_state_t state;
  sl_status_t status = SL_STATUS_FAIL;
  #if defined(SL_CATALOG_KERNEL_PRESENT)
  osSemaphoreAttr_t semaphore_attr;
  osStatus_t sem_status;

  #if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
  uint32_t semaphore_cb[osSemaphoreCbSize / 4];
  #endif
  #endif
  (void) flags;
  (void) state;

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }

  LOCK_ENDPOINT(ep);

  if (ep->state == SL_CPC_STATE_DISCONNECTED) {
    // Endpoint disconnected, shutdown sequence done.
    status = SL_STATUS_OK;
    goto fail;
  }

  if (!is_endpoint_connection_active(ep)) {
    // Can only be called when actively connected to remote
    status = SL_STATUS_INVALID_STATE;
    goto fail;
  }

  if (ep->state == SL_CPC_STATE_CONNECTED) {
    if (is_endpoint_tx_done(ep)) {
      // Endpoint is done transmitting all its frames, send shutdown request immediately
      ep->state = SL_CPC_STATE_SHUTDOWN;
      #if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
      state = SL_CPC_STATE_DISCONNECTED;
      status = sli_cpc_system_cmd_property_set(on_endpoint_shutdown_reply,
                                               (void*)ep,
                                               EP_ID_TO_PROPERTY_STATE(ep->id),
                                               0,       // try only once
                                               SLI_CPC_SYSTEM_CMD_TIMEOUT_MS,
                                               &state,
                                               sizeof(state),
                                               SYSTEM_EP_IFRAME);
      #else
      status = sli_cpc_system_send_shutdown_request(endpoint_get_system_ep(ep), ep->id);
      #endif
      if (status != SL_STATUS_OK) {
        // System endpoint command failed.
        goto fail;
      }
    } else {
      // Endpoint still has frames pending a transmission. Once all pending frames
      // have been sent and their on_write_complete callbacks called, the system
      // will automatically send a shutdown request to the remote without having
      // to call this API again.
      ep->state = SL_CPC_STATE_SHUTTING_DOWN;
    }
  }

  #if defined(SL_CATALOG_KERNEL_PRESENT)
  // Wait for all pending TX frames to be sent
  if (flags & SL_CPC_FLAG_NO_BLOCK) {
    status = SL_STATUS_IN_PROGRESS;
    goto release_ep;
  }

  // Allocate semaphore for blocking mode
  if (osKernelGetState() != osKernelRunning) {
    status = SL_STATUS_NOT_READY;
    goto fail;
  }
  // Allocate endpoint semaphore
  #if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
  semaphore_attr.cb_mem = semaphore_cb;
  semaphore_attr.cb_size = osSemaphoreCbSize;
  #else
  semaphore_attr.cb_mem = NULL;
  semaphore_attr.cb_size = 0U;
  #endif // SL_CATALOG_CMSIS_OS_COMMON_PRESENT

  if (ep->state_event_signal != NULL) {
    // Another thread is already shutting down
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_IS_WAITING;
    goto fail;
  }

  ep->state_event_signal = osSemaphoreNew(1, 0u, &semaphore_attr);
  if (ep->state_event_signal == NULL) {
    status = SL_STATUS_ALLOCATION_FAILED;
    goto fail;
  }

  RELEASE_ENDPOINT(ep);
  // acquire semaphore, it will be released when remote responds to SHUTDOWN
  // or command times out (on_endpoint_shutdown_reply)
  sem_status = osSemaphoreAcquire(ep->state_event_signal, ep->shutdown_timeout);
  LOCK_ENDPOINT(ep);

  // delete semaphore
  if (osSemaphoreDelete(ep->state_event_signal) != osOK) {
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_FAIL;
    goto fail;
  }
  ep->state_event_signal = NULL;

  if (sem_status != osOK && sem_status != osErrorTimeout) {
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_FAIL;
    goto fail;
  }

  if (sem_status == osErrorTimeout) {
    // Failed to disconnect before timeout. Abort shutdown procedure.
    status = SL_STATUS_TIMEOUT;
    goto fail;
  }

  // Semaphore successfully acquired
  if (ep->state == SL_CPC_STATE_DISCONNECTED) {
    // Shutdown proceedure successful
    status = SL_STATUS_OK;
  } else {
    // Shutdown procedure aborted by error or terminate. Do not update endpoint
    // state, as it needs to retain its error state so the user can read it.
    status = SL_STATUS_ABORT;
    SLI_CPC_ASSERT(is_endpoint_tx_done(ep));
    SLI_CPC_ASSERT(!is_endpoint_connection_active(ep));
  }

  #else // SL_CATALOG_KERNEL_PRESENT
  // Shutdown procedure in progress.
  status = SL_STATUS_IN_PROGRESS;
  #endif
  goto release_ep;

  // Failure handling
  fail:
  ep->state = SL_CPC_STATE_DISCONNECTED;
  clean_tx_queues(ep);

  release_ep:
  RELEASE_ENDPOINT(ep);
  cpc_exit_api(endpoint_handle);
  return status;
}

/***************************************************************************//**
 * Empty all endpoint RX queues.
 ******************************************************************************/
static void clean_rx_queues(sl_cpc_endpoint_t *endpoint)
{
  // Drop all RX I-Frames
  while (endpoint->iframe_receive_queue != NULL) {
    sl_cpc_receive_queue_item_t *item;
    item = SL_SLIST_ENTRY(sl_slist_pop(&endpoint->iframe_receive_queue), sl_cpc_receive_queue_item_t, node);

    sli_cpc_free_receive_queue_item(endpoint->instance, item);
  }

  // Drop all RX U-Frames
  while (endpoint->uframe_receive_queue != NULL) {
    sl_cpc_receive_queue_item_t *item;
    item = SL_SLIST_ENTRY(sl_slist_pop(&endpoint->uframe_receive_queue), sl_cpc_receive_queue_item_t, node);

    sli_cpc_free_receive_queue_item(endpoint->instance, item);
  }
}

/***************************************************************************//**
 * Terminate endpoint (This function must be called on a locked endpoint).
 ******************************************************************************/
static sl_status_t terminate_endpoint(sl_cpc_endpoint_t *endpoint, bool queue_free, uint8_t flags)
{
  sl_status_t status;
  #if defined (SL_CATALOG_CPC_PRIMARY_PRESENT)
  sl_cpc_endpoint_state_t ep_state;
  #endif
  (void) flags;

  // Notify the host that we want to close an endpoint. If the connection was
  // active with the remote, it's obvious we must send the notification. Being
  // in CONNECTING state means that primary and secondary started updating the
  // state of this endpoint.
  // For instance, on the secondary, being CONNECTING means we have received a
  // connection requested, we replied to it and we're waiting for the HDLC ack
  // to become CONNECTED. If a user terminates the endpoint, we must send a
  // terminate notification to the primary to let it know of this state change.
  if (is_endpoint_connection_active(endpoint)
      || endpoint->state == SL_CPC_STATE_CONNECTING) {
    // Send terminate notification to remote
    if (endpoint->id == SL_CPC_ENDPOINT_SYSTEM) {
      // System endpoint sends no notification when it terminates, it only cleans up.
    } else {
      #if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
      ep_state = SL_CPC_STATE_CLOSED;
      // Return status is irrelevant. If the command fails, the endpoint will
      // still get terminated, and incoming frames will simply get rejected with
      // SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE.
      status = sli_cpc_system_cmd_property_set(on_endpoint_terminate_reply,
                                               (void*)endpoint,
                                               EP_ID_TO_PROPERTY_STATE(endpoint->id),
                                               0,     // try only once
                                               SLI_CPC_SYSTEM_CMD_TIMEOUT_MS, // 10 second timeout
                                               &ep_state,
                                               sizeof(ep_state),
                                               SYSTEM_EP_IFRAME);
    #else
      // Return status is irrelevant. If the command fails, the endpoint will
      // still get terminated, and incoming frames will simply get rejected with
      // SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE.
      status = sli_cpc_system_send_terminate_notification(endpoint_get_system_ep(endpoint), endpoint->id);
    #endif
    }
  }

  // Endpoint is now in the process of cleaning its queues
  endpoint->state = SL_CPC_STATE_CLOSING;

  // Drop any pending RX frame
  clean_rx_queues(endpoint);

  // Drop any pending TX frame
  clean_tx_queues(endpoint);

  // reset various counters
  endpoint->seq = 0;
  endpoint->ack = 0;
  endpoint->re_transmit_timeout = sli_cpc_timer_ms_to_tick(SLI_CPC_INIT_RE_TRANSMIT_TIMEOUT_MS);
  endpoint->last_iframe_sent_timestamp = 0;
  endpoint->smoothed_rtt = 0;
  endpoint->rtt_variation = 0;
#if (SECURITY_ENABLED >= 1)
  endpoint->frame_counter_rx = 0;
  endpoint->frame_counter_tx = 0;
#endif

  if (is_endpoint_empty(endpoint)) {
    // Endpoint is empty, set it to CLOSED
    endpoint->state = SL_CPC_STATE_CLOSED;
    status = SL_STATUS_OK;

    if (queue_free) {
      // defer_endpoint_free already notifies the core that
      // the endpoint is being freed, so no need to notify
      // that endpoint is being closed in that case.
      defer_endpoint_free(endpoint);
    } else {
      notify_endpoint_is_closed();
    }
  } else {
    // If endpoint is not yet empty, it means the driver still owns one of its buffer
    // handles. The endpoint will be CLOSED once the buffer handle is returned
    // (see notify_write_complete)
    status = SL_STATUS_IN_PROGRESS;
  }

  #if defined(SL_CATALOG_KERNEL_PRESENT)
  // Unlock any pending read
  endpoint_abort_read(endpoint);
  #endif
  // Return state is irrelevant since call was made by the user. Simply unblock
  // any blocking APIs.
  notify_state_change(endpoint);

  return status;
}

/***************************************************************************//**
 * Terminate endpoint
 ******************************************************************************/
sl_status_t sl_cpc_terminate_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                      uint8_t flags)
{
  sl_status_t status;
  sl_cpc_endpoint_t *ep;

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }
  LOCK_ENDPOINT(ep);
  if (ep->flags & SL_CPC_ENDPOINT_FLAG_LEGACY_API) {
    RELEASE_ENDPOINT(ep);
    cpc_exit_api(endpoint_handle);
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (ep->state) {
    case SL_CPC_STATE_CLOSED:
      // Endpoint already terminated
      status = SL_STATUS_OK;
      goto exit;
      break;
    case SL_CPC_STATE_CLOSING:
      // Termination in progress, endpoint will go to CLOSED when all on_write_completed
      // callbacks have been called (see notify_write_complete)
      status = SL_STATUS_IN_PROGRESS;
      goto exit;
      break;
    case SL_CPC_STATE_FREED:
      SLI_CPC_ASSERT(0);
      status = SL_STATUS_INVALID_STATE;
      goto exit;
      break;
    default:
      // Proceed to terminate
      break;
  }

  status = terminate_endpoint(ep, false, flags);

  exit:
  RELEASE_ENDPOINT(ep);
  cpc_exit_api(endpoint_handle);
  return status;
}

/***************************************************************************//**
 * Free endpoint
 ******************************************************************************/
sl_status_t sl_cpc_free_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle)
{
  sl_cpc_endpoint_t *ep;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();

  if (endpoint_handle->ep == NULL) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_INVALID_HANDLE;
  }

  ep = endpoint_handle->ep;

  if (ep->state != SL_CPC_STATE_CLOSED) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_INVALID_STATE;
  }

  if (endpoint_handle->ref_count > 1) {
    // Another API has been called at the same time
    MCU_EXIT_ATOMIC();
    return SL_STATUS_BUSY;
  }

  // Invalidate handle
  endpoint_handle->ref_count = 0u;

  MCU_EXIT_ATOMIC();
  LOCK_ENDPOINT(ep);

  // Endpoint should have been freed on termination
  if (is_endpoint_empty(ep) == false) {
    RELEASE_ENDPOINT(ep);
    SLI_CPC_ASSERT(0);
    return SL_STATUS_FAIL;
  }
  // Cleanup the endpoint
  defer_endpoint_free(ep);
  RELEASE_ENDPOINT(ep);

  // Invalidate handle endpoint pointer
  endpoint_handle->ep = NULL;
  return SL_STATUS_OK;
}

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
/***************************************************************************//**
 * Close an endpoint
 ******************************************************************************/
sl_status_t sl_cpc_close_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle)
{
  sl_status_t status;
  sl_cpc_endpoint_t *ep;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  ep = (sl_cpc_endpoint_t *)endpoint_handle->ep;
  if (ep != NULL && (ep->flags & SL_CPC_ENDPOINT_FLAG_LEGACY_API) == 0) {
    // Close only allowed on an endpoint that was initialized with the legacy
    // open API.
    MCU_EXIT_ATOMIC();
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (endpoint_handle->ref_count > 1) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_BUSY;
  }

  endpoint_handle->ref_count = 0u;
  MCU_EXIT_ATOMIC();

  if (ep == NULL) {
    return SL_STATUS_OK;
  }

  LOCK_ENDPOINT(ep);

  // Clear queues and queue for a free
  status = terminate_endpoint(ep, true, 0);
  if (status == SL_STATUS_OK || status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  }

  RELEASE_ENDPOINT(ep);

  // Set endpoint to null, so we cannot read and send data anymore or
  // closing the endpoint again
  endpoint_handle->ep = NULL;

  return status;
}
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
/***************************************************************************//**
 * Open a user endpoint
 ******************************************************************************/
sl_status_t sl_cpc_open_user_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                      sl_cpc_user_endpoint_id_t id,
                                      uint8_t flags,
                                      uint8_t tx_window_size)
{
  SLI_CPC_ASSERT((uint8_t)id >= SL_CPC_USER_ENDPOINT_ID_START);
  SLI_CPC_ASSERT((uint8_t)id <= SL_CPC_USER_ENDPOINT_ID_END);

  return open_endpoint(endpoint_handle, NULL, (uint8_t)id, flags, tx_window_size);
}

/***************************************************************************//**
 * Open a service endpoint explicitely on an instance. Is reserved for system endpoint.
 ******************************************************************************/
sl_status_t sli_cpc_open_service_endpoint_in_instance(sl_cpc_endpoint_handle_t *endpoint_handle,
                                                      sli_cpc_instance_t *inst,
                                                      sl_cpc_service_endpoint_id_t id,
                                                      uint8_t flags,
                                                      uint8_t tx_window_size)
{
  SLI_CPC_ASSERT((uint8_t)id == SL_CPC_ENDPOINT_SYSTEM);

  return open_endpoint(endpoint_handle, inst, (uint8_t)id, flags, tx_window_size);
}

/***************************************************************************//**
 * Open a service endpoint
 ******************************************************************************/
sl_status_t sli_cpc_open_service_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                          sl_cpc_service_endpoint_id_t id,
                                          uint8_t flags,
                                          uint8_t tx_window_size)
{
  SLI_CPC_ASSERT((uint8_t)id <= SLI_CPC_SERVICE_ENDPOINT_ID_END);

  return open_endpoint(endpoint_handle, NULL, (uint8_t)id, flags, tx_window_size);
}

/***************************************************************************//**
 * Open a temporary endpoint
 ******************************************************************************/
sl_status_t sli_cpc_open_temporary_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                            uint8_t *id,
                                            uint8_t flags,
                                            uint8_t tx_window_size)
{
  sli_cpc_instance_t *inst;
  uint8_t id_free = 0;

  // one instance is responsible for handling all temporary endpoints
  inst = sli_cpc_get_instance(SLI_CPC_TEMPORARY_ENDPOINT_ID_START);
  if (inst == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  LOCK_ENDPOINTS_LIST(inst);

  for (uint16_t id_iterator = SLI_CPC_TEMPORARY_ENDPOINT_ID_START;
       id_iterator <= SLI_CPC_TEMPORARY_ENDPOINT_ID_END;
       id_iterator++) {
    sl_cpc_endpoint_t *endpoint = find_instance_endpoint(inst, (uint8_t)id_iterator);
    if (endpoint == NULL) {
      id_free = (uint8_t)id_iterator;
      break;
    }
  }

  RELEASE_ENDPOINTS_LIST(inst);

  if (id_free == 0) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  SLI_CPC_ASSERT(id_free >= SLI_CPC_TEMPORARY_ENDPOINT_ID_START);
  *id = id_free;

  return open_endpoint(endpoint_handle, inst, id_free, flags, tx_window_size);
}

/***************************************************************************//**
 * Set endpoint in listening mode, waiting for remote to connect
 ******************************************************************************/
sl_status_t sl_cpc_listen_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle, uint8_t flags)
{
  sl_cpc_endpoint_t *ep;
  sl_status_t status;

  // Semaphore resource when using an RTOS
#if defined(SL_CATALOG_KERNEL_PRESENT)
  bool blocking = (flags & SL_CPC_FLAG_NO_BLOCK) != SL_CPC_FLAG_NO_BLOCK;
  osSemaphoreAttr_t semaphore_attr;
  osStatus_t sem_status;

#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
  uint32_t semaphore_cb[osSemaphoreCbSize / 4];
#endif

#else
  (void) flags;
#endif

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }

  if (ep->configured_tx_window_size != ep->current_tx_window_space
      || ep->configured_tx_window_size < SL_CPC_TRANSMIT_WINDOW_MIN_SIZE
      || ep->configured_tx_window_size > SL_CPC_TRANSMIT_WINDOW_MAX_SIZE) {
    cpc_exit_api(endpoint_handle);
    return SL_STATUS_INVALID_PARAMETER;
  }

  LOCK_ENDPOINT(ep);

  // if the endpoint is already in the open state, it means
  // this function was already called before, return that we're in progress
  if (ep->state == SL_CPC_STATE_OPEN || ep->state == SL_CPC_STATE_CONNECTING) {
    status = SL_STATUS_IN_PROGRESS;
    goto release_ep;
  }

  // if connected, let the user know
  if (ep->state == SL_CPC_STATE_CONNECTED) {
    status = SL_STATUS_OK;
    goto release_ep;
  }

  if (ep->state != SL_CPC_STATE_CLOSED) {
    status = SL_STATUS_INVALID_STATE;
    goto release_ep;
  }

  if (ep->id == SL_CPC_ENDPOINT_SYSTEM) {
    ep->state = SL_CPC_STATE_CONNECTED;
    status = SL_STATUS_OK;
    goto release_ep;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (blocking) {
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
    // static allocation
    semaphore_attr.cb_mem = semaphore_cb;
    semaphore_attr.cb_size = osSemaphoreCbSize;
#else
    // dynamic allocation
    semaphore_attr.cb_mem = NULL;
    semaphore_attr.cb_size = 0U;
#endif
    // check the semaphore is not used, it should
    // not be coming from CLOSED state
    SLI_CPC_ASSERT(ep->state_event_signal == NULL);
    if (ep->state_event_signal != NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
      goto release_ep;
    }

    // create semaphore and check it's valid
    ep->state_event_signal = osSemaphoreNew(1, 0u, &semaphore_attr);
    SLI_CPC_ASSERT(ep->state_event_signal != NULL);
    if (ep->state_event_signal == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
      goto release_ep;
    }
  }
#endif

  if (sli_cpc_reboot_is_sequence_done(endpoint_get_system_ep(ep))) {
    status = sli_cpc_system_send_opening_notification(endpoint_get_system_ep(ep), ep->id);
    if (status != SL_STATUS_OK) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
      if (blocking) {
        SLI_CPC_ASSERT(osSemaphoreDelete(ep->state_event_signal) == osOK);
      }
#endif
      goto release_ep;
    }
  }

  // Endpoint is now waiting for remote to connect.
  //  - in baremetal, the completion will be done via a callback, so the
  //    function is basically done here
  //  - with a RTOS, this call is blocking until the remote connects,
  //    so allocate a semaphore and wait on it; except if timeout is 0
  ep->state = SL_CPC_STATE_OPEN;

  // Indicate that the operation is not done yet
  status = SL_STATUS_IN_PROGRESS;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (!blocking) {
    // Don't block
    goto release_ep;
  }

  RELEASE_ENDPOINT(ep);
  // acquire semaphore, it will be released when remote connects to endpoint
  sem_status = osSemaphoreAcquire(ep->state_event_signal, osWaitForever);
  SLI_CPC_ASSERT(sem_status == osOK);
  LOCK_ENDPOINT(ep);

  // delete semaphore
  SLI_CPC_ASSERT(osSemaphoreDelete(ep->state_event_signal) == osOK);
  ep->state_event_signal = NULL;

  // if the semaphore can be acquired, it means that the remote connected
  // successfully, the state is already updated to CONNECTED at this point.
  if (sem_status == osOK) {
    if (ep->state == SL_CPC_STATE_CONNECTED) {
      status = SL_STATUS_OK;
    } else {
      status = SL_STATUS_ABORT;
    }
  } else {
    ep->state = SL_CPC_STATE_CLOSED;
    status = SL_STATUS_INVALID_STATE;
  }
#endif

  release_ep:
  RELEASE_ENDPOINT(ep);
  cpc_exit_api(endpoint_handle);

  return status;
}
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
/***************************************************************************//**
 * Abort read from an endpoint
 ******************************************************************************/
static bool endpoint_abort_read(sl_cpc_endpoint_t *endpoint)
{
  MCU_DECLARE_IRQ_STATE;
  osSemaphoreId_t receive_signal;
  bool read_aborted = false;

  // Enter atomic to avoid context switch if read task is higher priority.
  MCU_ENTER_ATOMIC();
  receive_signal = endpoint->receive_signal;

  if (receive_signal != NULL) {
    SLI_CPC_ASSERT(osSemaphoreRelease(receive_signal) == osOK);
    read_aborted = true;
  }
  MCU_EXIT_ATOMIC();
  return read_aborted;
}
/***************************************************************************//**
 * Abort read from an endpoint handle
 ******************************************************************************/
sl_status_t sl_cpc_abort_read(sl_cpc_endpoint_handle_t *endpoint_handle)
{
  sl_cpc_endpoint_t *ep;
  sl_status_t status;
  bool read_aborted;

  if (osKernelGetState() != osKernelRunning) {
    return SL_STATUS_INVALID_STATE;
  }

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }

  read_aborted = endpoint_abort_read(ep);

  cpc_exit_api(endpoint_handle);
  return read_aborted ? SL_STATUS_OK : SL_STATUS_NONE_WAITING;
}
#endif

/***************************************************************************//**
 * Checks if an endpoint is connected to a remote
 ******************************************************************************/
static inline bool is_endpoint_read_state(const sl_cpc_endpoint_t* ep)
{
  // Using legacy APIs, both OPEN and CONNECTING states are valid
  if (ep->flags & SL_CPC_ENDPOINT_FLAG_LEGACY_API) {
    if (ep->state == SL_CPC_STATE_OPEN
        || ep->state == SL_CPC_STATE_CONNECTING) {
      return true;
    }
  }
  /*
   * Can still receive in all three states:
   *   Connected:     we are actively connected to remote
   *   Shutting down: we are draining our TX queue to the bus.
   *   Shutdown:      we will no longer transmit.
   */
  return ep->state == SL_CPC_STATE_CONNECTED
         || ep->state == SL_CPC_STATE_SHUTTING_DOWN
         || ep->state == SL_CPC_STATE_SHUTDOWN;
}

/***************************************************************************//**
 * Read data from an endpoint
 ******************************************************************************/
sl_status_t sl_cpc_read(sl_cpc_endpoint_handle_t *endpoint_handle,
                        void **data,
                        uint16_t *data_length,
                        uint32_t timeout,
                        uint8_t flags)
{
  sl_slist_node_t **receive_queue = NULL;
  sl_cpc_endpoint_t *ep = NULL;
  sl_cpc_receive_queue_item_t *item = NULL;
  sl_slist_node_t *node = NULL;
  sl_status_t status;
  #if defined(SL_CATALOG_KERNEL_PRESENT)
  bool blocking = (flags & SL_CPC_FLAG_NO_BLOCK) != SL_CPC_FLAG_NO_BLOCK;
  osStatus_t  sem_status = osOK;
  osSemaphoreId_t receive_signal;

  if (osKernelGetState() != osKernelRunning) {
    return SL_STATUS_INVALID_STATE;
  }
#endif

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
  if (sli_cpc_is_fwu_in_progress() && (endpoint_handle->id != SL_CPC_ENDPOINT_SYSTEM)) {
    return SL_STATUS_BUSY;
  }
#endif

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = SL_STATUS_EMPTY;

  LOCK_ENDPOINT(ep);

  if (flags & SL_CPC_FLAG_UNNUMBERED_INFORMATION) {
    receive_queue = &ep->uframe_receive_queue;
  } else {
    receive_queue = &ep->iframe_receive_queue;
  }

  if (*receive_queue == NULL) {
    if (!is_endpoint_read_state(ep)) {
      // Return error only when the receive queue is empty
      RELEASE_ENDPOINT(ep);
      cpc_exit_api(endpoint_handle);
      return SL_STATUS_INVALID_STATE;
    }

#if defined(SL_CATALOG_KERNEL_PRESENT)
    if (!blocking) {
      RELEASE_ENDPOINT(ep);
      cpc_exit_api(endpoint_handle);
      return status;
    } else {
      // Valid state, block until data is received
      osSemaphoreAttr_t semaphore_attr;
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
      uint32_t semaphore_cb[osSemaphoreCbSize / 4];
#endif

      if (ep->receive_signal != NULL) {
        // A task is already pending on this endpoint
        RELEASE_ENDPOINT(ep);
        cpc_exit_api(endpoint_handle);
        return SL_STATUS_IS_WAITING;
      }

#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
      semaphore_attr.cb_mem = semaphore_cb;
      semaphore_attr.cb_size = osSemaphoreCbSize;
#else
      semaphore_attr.cb_mem = NULL;
      semaphore_attr.cb_size = 0U;
#endif
      ep->receive_signal = osSemaphoreNew(255, 0u, &semaphore_attr);
      SLI_CPC_ASSERT(ep->receive_signal != NULL);
      if (ep->receive_signal == NULL) {
        RELEASE_ENDPOINT(ep);
        cpc_exit_api(endpoint_handle);
        return SL_STATUS_ALLOCATION_FAILED;
      }

      RELEASE_ENDPOINT(ep);
      if (timeout == 0) {
        timeout = osWaitForever;
      }
      sem_status = osSemaphoreAcquire(ep->receive_signal, timeout);
      SLI_CPC_ASSERT(sem_status == osOK || sem_status == osErrorTimeout);
      LOCK_ENDPOINT(ep);

      receive_signal = ep->receive_signal;
      MCU_ATOMIC_STORE(ep->receive_signal, NULL);
      SLI_CPC_ASSERT(osSemaphoreDelete(receive_signal) == osOK);

      if (sem_status == osErrorTimeout) {
        RELEASE_ENDPOINT(ep);
        cpc_exit_api(endpoint_handle);
        return SL_STATUS_TIMEOUT;
      }
    }
#else
    (void)timeout;
    (void)flags;
    RELEASE_ENDPOINT(ep);
    cpc_exit_api(endpoint_handle);
    return status;
#endif
  }

  // Allow read even if the state is "error".
  // Error will be returned only when the queue is empty.
  node = sl_slist_pop(receive_queue);
  if (node != NULL) {
    item = SL_SLIST_ENTRY(node, sl_cpc_receive_queue_item_t, node);
    sli_cpc_push_receive_queue_item_to_postponed_list(ep->instance, item, data, data_length);
  } else {
    *data = NULL;
    *data_length = 0;

    #if defined(SL_CATALOG_KERNEL_PRESENT)
    if (blocking) {
      // Check if read was aborted by state change that prevents read, or by
      // abort API.
      sl_status_t ret = is_endpoint_read_state(ep) ? SL_STATUS_ABORT : SL_STATUS_INVALID_STATE;
      RELEASE_ENDPOINT(ep);
      cpc_exit_api(endpoint_handle);
      return ret;
    }
    #endif

    RELEASE_ENDPOINT(ep);
    cpc_exit_api(endpoint_handle);
    return SL_STATUS_EMPTY;
  }

  RELEASE_ENDPOINT(ep);
  cpc_exit_api(endpoint_handle);
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Helper function to validate the tx_payload length against the maximum
 ******************************************************************************/
static sl_status_t sli_cpc_validate_tx_payload_length(sli_cpc_instance_t *inst, uint16_t tx_length, bool security_enabled)
{
  uint16_t tx_max_length = SL_CPC_TX_PAYLOAD_MAX_LENGTH;
#if !(SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  (void) security_enabled;
#endif

  if (inst->remote_tx_payload_max_length != 0) {
    tx_max_length = inst->remote_tx_payload_max_length;
  }

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  if (security_enabled) {
    tx_max_length -= SLI_CPC_SECURITY_TAG_LENGTH;
  }
#endif

  if (tx_length > tx_max_length) {
    return SL_STATUS_INVALID_COUNT;
  } else {
    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
 * Write data from an endpoint
 ******************************************************************************/
sl_status_t sl_cpc_write(sl_cpc_endpoint_handle_t *endpoint_handle,
                         void* data,
                         uint16_t data_length,
                         uint8_t flags,
                         void *on_write_completed_arg)
{
  sl_cpc_endpoint_t *ep;
  sl_status_t status;

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  // Secondary cannot send poll message
  // Can answer only using the on_poll callback
  SLI_CPC_ASSERT((flags & SL_CPC_FLAG_UNNUMBERED_POLL_FINAL) == 0);
#endif

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
  if (sli_cpc_is_fwu_in_progress() && (endpoint_handle->id != SL_CPC_ENDPOINT_SYSTEM)) {
    return SL_STATUS_BUSY;
  }
#endif

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }

  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (data_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Validate data_length against the maximum TX payload length
  {
    #if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
    const bool security_enabled = ep->encrypted;
    #else
    const bool security_enabled = false;
    #endif

    status = sli_cpc_validate_tx_payload_length(ep->instance, data_length, security_enabled);
    if (status != SL_STATUS_OK) {
      cpc_exit_api(endpoint_handle);
      return status;
    }
  }

  status = write(ep, data, data_length, flags, on_write_completed_arg);
  cpc_exit_api(endpoint_handle);

  return status;
}

/***************************************************************************//**
 * Get endpoint status
 ******************************************************************************/
sl_cpc_endpoint_state_t sl_cpc_get_endpoint_state(sl_cpc_endpoint_handle_t *endpoint_handle)
{
  sl_cpc_endpoint_state_t state;
  sli_cpc_instance_t *inst;
  sl_cpc_endpoint_t *ep;

  MCU_ATOMIC_LOAD(ep, (sl_cpc_endpoint_t *)endpoint_handle->ep);

  if (ep == NULL) {
    inst = sli_cpc_get_instance(endpoint_handle->id);
    if (inst == NULL) {
      SLI_CPC_ASSERT(0);

      return SL_CPC_STATE_FREED;
    }

    LOCK_ENDPOINTS_LIST(inst);

    ep = find_instance_endpoint(inst, endpoint_handle->id);
    if (ep == NULL) {
      state = SL_CPC_STATE_FREED;
    } else {
      LOCK_ENDPOINT(ep);
      state = ep->state;
      RELEASE_ENDPOINT(ep);
    }

    RELEASE_ENDPOINTS_LIST(inst);
  } else {
    LOCK_ENDPOINT(ep);
    state = ep->state;
    RELEASE_ENDPOINT(ep);
  }

  return state;
}

/***************************************************************************//**
 * Get endpoint encryption
 ******************************************************************************/
bool sl_cpc_get_endpoint_encryption(sl_cpc_endpoint_handle_t *endpoint_handle)
{
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  sl_cpc_endpoint_t *ep;
  bool encrypted;

  MCU_ATOMIC_LOAD(ep, (sl_cpc_endpoint_t *)endpoint_handle->ep);

  if (ep == NULL) {
    LOCK_ENDPOINTS_LIST(&g_instance);

    ep = find_endpoint(endpoint_handle->id);

    if (ep == NULL) {
      encrypted = false;
    } else {
      LOCK_ENDPOINT(ep);
      encrypted = ep->encrypted;
      RELEASE_ENDPOINT(ep);
    }

    RELEASE_ENDPOINTS_LIST(&g_instance)
  } else {
    LOCK_ENDPOINT(ep);
    encrypted = ep->encrypted;
    RELEASE_ENDPOINT(ep);
  }

  return encrypted;
#else
  (void)endpoint_handle;

  return false;
#endif
}

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
/***************************************************************************//**
 * Notify remote is connecting to endpoint.
 ******************************************************************************/
sl_status_t sli_cpc_instance_on_remote_ep_connecting(sli_cpc_instance_t *inst,
                                                     uint8_t endpoint_id,
                                                     sl_cpc_endpoint_state_t *reply_state)
{
  sl_cpc_endpoint_t *ep;
  sl_status_t status;

  LOCK_ENDPOINTS_LIST(inst);
  ep = find_instance_endpoint(inst, endpoint_id);

  // Only accept the connection request if the endpoint is in a valid state.
  *reply_state = SL_CPC_STATE_CLOSED;

  if (ep == NULL) {
    RELEASE_ENDPOINTS_LIST(inst);

    return SL_STATUS_INVALID_STATE;
  }

  LOCK_ENDPOINT(ep);
  RELEASE_ENDPOINTS_LIST(inst);

  switch (ep->state) {
    case SL_CPC_STATE_OPEN:
    case SL_CPC_STATE_CONNECTING:
      // Reply with CONNECTED to accept the connection
      ep->state = SL_CPC_STATE_CONNECTING;
      *reply_state = SL_CPC_STATE_CONNECTED;
      status = SL_STATUS_OK;
      break;
    default:
      status = SL_STATUS_INVALID_STATE;
      break;
  }
  RELEASE_ENDPOINT(ep);

  return status;
}

/***************************************************************************//**
 * Notify remote has connected, unlocked version, operating directly on an
 * endpoint instead of an endpoint ID.
 ******************************************************************************/
static void sli_cpc_remote_connected_endpoint(sli_cpc_instance_t *inst,
                                              sl_cpc_endpoint_t *ep,
                                              sl_status_t status)
{
  (void)inst;

  // only notify when the endpoint is connecting, as it's not valid
  // to become connected from any other state
  if (ep->state == SL_CPC_STATE_CONNECTING) {
    bool skip_callback = false;

    if (status == SL_STATUS_OK) {
      ep->state = SL_CPC_STATE_CONNECTED;
    } else {
      // Connection failed. Go back to CLOSED state. Connection sequence will
      // have to be restarted.
      ep->state = SL_CPC_STATE_CLOSED;
    }

    skip_callback = notify_state_change(ep);

    if (!skip_callback && ep->on_connect != NULL) {
      RELEASE_ENDPOINT(ep);
      notify_ep_on_connect(ep, status);
      LOCK_ENDPOINT(ep);
    }
  }
}

/***************************************************************************//**
 * Notify remote has connected
 ******************************************************************************/
void sli_cpc_instance_on_remote_ep_connected(sli_cpc_instance_t *inst,
                                             uint8_t endpoint_id,
                                             sl_status_t status)
{
  sl_cpc_endpoint_t *ep;

  LOCK_ENDPOINTS_LIST(inst);

  ep = find_instance_endpoint(inst, endpoint_id);
  if (ep == NULL) {
    // This can happen is the endpoint was terminated + freed while waiting
    // for the connection handshake to complete. Simply ignore the command.
    RELEASE_ENDPOINTS_LIST(inst);

    return;
  }

  LOCK_ENDPOINT(ep);
  RELEASE_ENDPOINTS_LIST(inst);

  sli_cpc_remote_connected_endpoint(inst, ep, status);

  RELEASE_ENDPOINT(ep);
}

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
/***************************************************************************//**
 * Update security counters for an endpoint.
 ******************************************************************************/
sl_status_t sli_cpc_set_security_counters(uint8_t endpoint_id,
                                          uint32_t primary_rx_counter,
                                          uint32_t primary_tx_counter)
{
  sl_cpc_endpoint_t *ep;
  sl_status_t status;

  LOCK_ENDPOINTS_LIST(&g_instance);

  ep = find_endpoint(endpoint_id);
  if (ep == NULL) {
    status = SL_STATUS_NOT_READY;
    goto release_ep_list;
  }

  LOCK_ENDPOINT(ep);

  if (ep->state == SL_CPC_STATE_OPEN) {
    // make sure to map primary's TX to secondary's RX and vice versa
    ep->frame_counter_rx = primary_tx_counter;
    ep->frame_counter_tx = primary_rx_counter;

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_INVALID_STATE;
  }

  RELEASE_ENDPOINT(ep);

  release_ep_list:
  RELEASE_ENDPOINTS_LIST(&g_instance);

  return status;
}
#endif // SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1
#endif // SL_CATALOG_CPC_SECONDARY_PRESENT

/***************************************************************************//**
 * Completed endpoint shutdown handshake. Endpoint must be locked by caller.
 ******************************************************************************/
void sli_cpc_instance_on_remote_ep_disconnected(sli_cpc_instance_t *inst, uint8_t endpoint_id)
{
  sl_cpc_endpoint_t *ep;

  LOCK_ENDPOINTS_LIST(inst);

  ep = find_instance_endpoint(inst, endpoint_id);
  if (ep == NULL) {
    // This can happen if the endpoint is terminated + freed while waiting for
    // the shutdown handshake to complete. Simply ignore the request.
    RELEASE_ENDPOINTS_LIST(inst);

    return;
  }

  LOCK_ENDPOINT(ep);
  RELEASE_ENDPOINTS_LIST(inst);

  if (ep->state == SL_CPC_STATE_SHUTDOWN
      || ep->state == SL_CPC_STATE_REMOTE_SHUTDOWN) {
    // Both sides are done transmitting and the endpoint shutdown handshake
    // completed successfully, endpoint has cleanly disconnected.
    SLI_CPC_ASSERT(is_endpoint_tx_done(ep) == true);
    ep->state = SL_CPC_STATE_DISCONNECTED;
  } else {
    // Endpoint was either terminated or went in error while waiting for the
    // shutdown handshake to complete. Don't update the state, as it is no longer
    // related to the shutdown handshake
  }

  RELEASE_ENDPOINT(ep);
  // Return state is irrelevant since there is not disconnection callback
  notify_state_change(ep);
}

/***************************************************************************//**
 * Received shutdown request from remote.
 ******************************************************************************/
sl_status_t sli_cpc_instance_on_remote_ep_shutdown(sli_cpc_instance_t *inst,
                                                   uint8_t endpoint_id,
                                                   sl_cpc_endpoint_state_t *reply_state)
{
  sl_cpc_endpoint_t *ep;
  sl_status_t status;

  LOCK_ENDPOINTS_LIST(inst);
  ep = find_instance_endpoint(inst, endpoint_id);
  if (ep == NULL) {
    RELEASE_ENDPOINTS_LIST(inst);

    // Endpoint was not allocated, simply accept the shutdown request silently.
    *reply_state = SL_CPC_STATE_DISCONNECTED;
    return SL_STATUS_NOT_FOUND;
  }

  LOCK_ENDPOINT(ep);
  RELEASE_ENDPOINTS_LIST(inst);

  switch (ep->state) {
    case SL_CPC_STATE_SHUTDOWN:
      // Endpoint was already shutdown, accept the request.
      // The local endpoint state will be updated to DISCONNECTED once the reply
      // has been succesfully sent to the remote (on_write_completed)
      *reply_state = SL_CPC_STATE_DISCONNECTED;
      status = SL_STATUS_OK;
      break;
    case SL_CPC_STATE_SHUTTING_DOWN:
      // Endpoint still has frames pending a transmission. Refuse shutdown request
      // for now. The local system will request a shutdown once all frames have
      // been sent.
      ep->state = SL_CPC_STATE_REMOTE_SHUTDOWN;
      *reply_state = ep->state;
      status = SL_STATUS_IN_PROGRESS;
      break;
    case SL_CPC_STATE_REMOTE_SHUTDOWN:
    case SL_CPC_STATE_CONNECTED:
      // Remote requests a shutdown
      ep->state = SL_CPC_STATE_REMOTE_SHUTDOWN;

      if (is_endpoint_tx_done(ep) == true) {
        // The local endpoint state will be updated once the reply has been succesfully
        // sent to the remote (on_write_completed)
        *reply_state = SL_CPC_STATE_DISCONNECTED;
        status = SL_STATUS_OK;
      } else {
        *reply_state = ep->state;
        status = SL_STATUS_IN_PROGRESS;
      }
      break;
    default:
      // Endpoint was not in an active connection, simply accept the shutdown
      // request silently.
      *reply_state = SL_CPC_STATE_DISCONNECTED;
      status = SL_STATUS_INVALID_STATE;
      break;
  }
  RELEASE_ENDPOINT(ep);

  return status;
}

/***************************************************************************//**
 * Notify remote has terminated endpoint connection
 ******************************************************************************/
void sli_cpc_instance_on_remote_ep_terminated(sli_cpc_instance_t *inst,
                                              uint8_t endpoint_id,
                                              sl_cpc_endpoint_state_t *reply_state)
{
  sl_cpc_endpoint_t *ep;

  SL_CPC_JOURNAL_RECORD_INFO("Remote terminated endpoint connection", endpoint_id);

  LOCK_ENDPOINTS_LIST(inst);
  ep = find_instance_endpoint(inst, endpoint_id);

  // Always reply CLOSED to a terminate command, regardless of the internal state
  // of the endpoint.
  *reply_state = SL_CPC_STATE_CLOSED;

  if (ep == NULL) {
    // Endpoint is not being used, terminate command has no effect.
    RELEASE_ENDPOINTS_LIST(inst);

    return;
  }

  LOCK_ENDPOINT(ep);
  RELEASE_ENDPOINTS_LIST(inst);

  if (is_endpoint_connection_active(ep) == true) {
    // Connection was unexpectedly terminated, notify error
    ep->state = SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE;

    // Stop sending, remote has left
    notify_error(ep);
  } else if (ep->state == SL_CPC_STATE_CONNECTING) {
    // handle specifically the case where a termination request is received
    // while in the CONNECTING test. That can happen if the HDLC ACK coming
    // from the remote is lost during the connection sequence.
    //  1. remote sends SET STATE CONNECTED
    //  2. local acknowledges the command with HDLC ack
    //  3. local sends IS STATE CONNECTED
    //  4. remote acknowledges the command with HDLC ack but it's never
    //     received because of a checksum error
    //  5. remote sends SET STATE DISCONNECTED

    // Set endpoint is disconnected state and notify user
    ep->state = SL_CPC_STATE_DISCONNECTED;

    // try to notify the endpoint that connection failed using either
    // the semaphore or registered callback
    bool ep_notified = notify_ep_on_connect(ep, SL_STATUS_ABORT);

    // if no semaphore and no callback are present, and if the endpoint
    // was opened in legacy mode, then fallback to signaling with the
    // error callback.
    if (!ep_notified && ep->flags & SL_CPC_ENDPOINT_FLAG_LEGACY_API) {
      notify_error(ep);
    }
  }
  RELEASE_ENDPOINT(ep);
}

/***************************************************************************//**
 * Calculate the re transmit timeout
 * Implemented using Karn's algorithm
 * Based off of RFC 2988 Computing TCP's Retransmission Timer
 ******************************************************************************/
static void sli_compute_re_transmit_timeout(sl_cpc_endpoint_t *endpoint)
{
  // Implemented using Karn's algorithm
  // Based off of RFC 2988 Computing TCP's Retransmission Timer
  static bool first_rtt_measurement = true;

  uint64_t round_trip_time = 0;
  uint64_t rto = 0;
  uint64_t delta = 0;

  const unsigned k = 4;   // This value is recommended by the Karn's algorithm

  SLI_CPC_ASSERT(endpoint != NULL);

  round_trip_time = sli_cpc_timer_get_tick_count64() - endpoint->last_iframe_sent_timestamp;

  if (first_rtt_measurement) {
    endpoint->smoothed_rtt = round_trip_time;
    endpoint->rtt_variation = round_trip_time / 2;
    first_rtt_measurement = false;
  } else {
    // RTTVAR <- (1 - beta) * RTTVAR + beta * |SRTT - R'| where beta is 0.25
    delta = ABS((int64_t)endpoint->smoothed_rtt - (int64_t)round_trip_time);
    endpoint->rtt_variation = 3 * (endpoint->rtt_variation / 4) +  delta / 4;

    //SRTT <- (1 - alpha) * SRTT + alpha * R' where alpha is 0.125
    endpoint->smoothed_rtt = 7 * (endpoint->smoothed_rtt / 8) + round_trip_time / 8;
  }

  // Impose a lowerbound on the variation, we don't want the RTO to converge too close to the RTT
  if (endpoint->rtt_variation < sli_cpc_timer_ms_to_tick(SLI_CPC_MIN_RE_TRANSMIT_TIMEOUT_MINIMUM_VARIATION_MS)) {
    endpoint->rtt_variation = sli_cpc_timer_ms_to_tick(SLI_CPC_MIN_RE_TRANSMIT_TIMEOUT_MINIMUM_VARIATION_MS);
  }

  rto = endpoint->smoothed_rtt + k * endpoint->rtt_variation;

  if (rto > sli_cpc_timer_ms_to_tick(SLI_CPC_MAX_RE_TRANSMIT_TIMEOUT_MS)) {
    rto = sli_cpc_timer_ms_to_tick(SLI_CPC_MAX_RE_TRANSMIT_TIMEOUT_MS);
  } else if (rto < sli_cpc_timer_ms_to_tick(SLI_CPC_MIN_RE_TRANSMIT_TIMEOUT_MS)) {
    rto = sli_cpc_timer_ms_to_tick(SLI_CPC_MIN_RE_TRANSMIT_TIMEOUT_MS);
  }

  endpoint->re_transmit_timeout = (uint32_t) rto;
}

/***************************************************************************//**
 * Signal processing is required
 ******************************************************************************/
void sli_cpc_signal_event(sli_cpc_instance_t *inst, sl_cpc_signal_type_t signal_type)
{
#if defined(SL_CATALOG_KERNEL_PRESENT)
  (void)signal_type;

  SLI_CPC_ASSERT(osSemaphoreRelease(inst->event_signal) == osOK);
#else
  if (signal_type == SL_CPC_SIGNAL_RX) {
    inst->rx_process_flag++;
  }
#endif
}

/***************************************************************************//**
 * Notify the core that an endpoint is terminated, it is used to unlock the
 * semaphore in CPC core task.
 ******************************************************************************/
static void notify_endpoint_is_closed(void)
{
#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (g_instance.cpc_core_in_reset) {
    sli_cpc_signal_event(&g_instance, SL_CPC_SIGNAL_SYSTEM);
  }
#endif
#endif
}

/***************************************************************************//**
 * This function checks the frame type and notifies the appropriate callback.
 * After notification, it clears the data and releases the buffer handle.
 ******************************************************************************/
static void notify_write_complete(void *data)
{
  sli_cpc_instance_t *inst = (sli_cpc_instance_t*)data;
  uint8_t type;
  sl_cpc_buffer_handle_t *buffer_handle;
  sl_cpc_endpoint_t * endpoint;
  sl_status_t status;
  #if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  sl_cpc_endpoint_state_t state;
  #endif

  MCU_DECLARE_IRQ_STATE;

  while (inst->write_complete_callback_list != NULL) {
    MCU_ENTER_ATOMIC();
    buffer_handle = sli_cpc_pop_core_buffer_handle(&inst->write_complete_callback_list);
    endpoint = buffer_handle->endpoint;
    if (endpoint->state == SL_CPC_STATE_FREED
        || endpoint->on_write_complete_pending_count == 0) {
      MCU_EXIT_ATOMIC();
      SLI_CPC_ASSERT(0);
      return;
    }
    endpoint->on_write_complete_pending_count--;
    MCU_EXIT_ATOMIC();

    SL_CPC_JOURNAL_RECORD_DEBUG("Write Complete", buffer_handle->endpoint->id);

    type = sli_cpc_hdlc_get_frame_type(buffer_handle->control);

    if (type == SLI_CPC_HDLC_FRAME_TYPE_UNNUMBERED && buffer_handle->endpoint->on_uframe_write_completed != NULL) {
      buffer_handle->endpoint->on_uframe_write_completed(buffer_handle->endpoint->id, buffer_handle->data, buffer_handle->arg, buffer_handle->write_status);
    } else if (type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION && buffer_handle->endpoint->on_iframe_write_completed != NULL) {
      buffer_handle->endpoint->on_iframe_write_completed(buffer_handle->endpoint->id, buffer_handle->data, buffer_handle->arg, buffer_handle->write_status);
    }

    LOCK_ENDPOINT(endpoint);
    switch (endpoint->state) {
      case SL_CPC_STATE_CLOSING:
        // Endpoint pending a cleanup
        if (is_endpoint_empty(endpoint)) {
          // Cleanup done, set endpoint to initial state
          endpoint->state = SL_CPC_STATE_CLOSED;

          // sl_cpc_close_endpoint called, automatically free the endpoint
          if (endpoint->flags & SL_CPC_ENDPOINT_FLAG_LEGACY_API) {
            defer_endpoint_free(endpoint);
          } else {
            // defer_endpoint_free already notifies the core that
            // the endpoint is being freed, so no need to notify
            // that endpoint is being closed in that case.
            notify_endpoint_is_closed();
          }
        }
        break;

      case SL_CPC_STATE_REMOTE_SHUTDOWN:
      case SL_CPC_STATE_SHUTTING_DOWN:
        if (!is_endpoint_tx_done(endpoint)) {
          // Frames yet to be sent
          break;
        }
        // Finished sending pending frames, send notification to remote
        #if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
        state = SL_CPC_STATE_DISCONNECTED;
        status = sli_cpc_system_cmd_property_set(on_endpoint_shutdown_reply,
                                                 (void*)endpoint,
                                                 EP_ID_TO_PROPERTY_STATE(endpoint->id),
                                                 0,     // try only once
                                                 SLI_CPC_SYSTEM_CMD_TIMEOUT_MS,
                                                 &state,
                                                 sizeof(state),
                                                 SYSTEM_EP_IFRAME);
        #else
        status = sli_cpc_system_send_shutdown_request(&endpoint->instance->system_ep, endpoint->id);
        #endif
        if (status != SL_STATUS_OK) {
          // System endpoint command failed, handshake aborted
          endpoint->state = SL_CPC_STATE_DISCONNECTED;
          // Return value is irrelevant since there is no disconnection callback
          notify_state_change(endpoint);
        }

        // Update endpoint state
        if (endpoint->state == SL_CPC_STATE_SHUTTING_DOWN) {
          // Signaled remote that we are done TX. Once remote responds, will
          // go to DISCONNECTED
          endpoint->state = SL_CPC_STATE_SHUTDOWN;
        } else {
          // Stay in REMOTE_SHUTDOWN until the remote replies to our shutdown
          // handshake, or times out.
        }
        break;
      default:
        // Nothing to be done
        break;
    }
    RELEASE_ENDPOINT(endpoint);

    buffer_handle->data = NULL;
    sli_cpc_free_buffer_handle(buffer_handle);
  }
}

/***************************************************************************//**
 * This function is called upon the completion of a write operation. It sets
 * the write status of the buffer handle and dispatches a routine that will
 * be executed later to inform the endpoint user via the appropriate registered
 * callback.
 ******************************************************************************/
static void submit_write_completion(sl_cpc_buffer_handle_t *buffer_handle, sl_status_t write_status)
{
  sl_cpc_endpoint_t *ep = buffer_handle->endpoint;

  // When submitting for write completion, the buffer handle should not be referenced
  SLI_CPC_ASSERT(buffer_handle->ref_count == 0);

  buffer_handle->write_status = write_status;

  SL_CPC_JOURNAL_RECORD_DEBUG("Submit write complete", ep->id);
  MCU_ATOMIC_SECTION(sli_cpc_push_back_core_buffer_handle(&ep->instance->write_complete_callback_list, buffer_handle); );
  sli_cpc_dispatcher_push(&ep->instance->callback_dispatcher_handle,
                          notify_write_complete,
                          ep->instance);   //Use to push status

  if (ep->on_write_complete_pending_count == UINT16_MAX) {
    // Count would overflow.
    SLI_CPC_ASSERT(0);
    return;
  }

  ep->on_write_complete_pending_count++;
}

/***************************************************************************//**
 * Notify Transmit completed
 ******************************************************************************/
void sli_cpc_notify_tx_data_by_drv(sl_cpc_buffer_handle_t *buffer_handle)
{
  uint8_t control_byte;
  uint8_t frame_type;
  sl_status_t status = SL_STATUS_OK;
  sl_cpc_endpoint_t *endpoint;
  MCU_DECLARE_IRQ_STATE;

#if defined(CPC_DEBUG_TRACE)
  if (buffer_handle->endpoint != NULL) {
    SLI_CPC_DEBUG_TRACE_ENDPOINT_FRAME_TRANSMIT_COMPLETED(buffer_handle->endpoint);
    SLI_CPC_DEBUG_TRACE_CORE_TXD_TRANSMIT_COMPLETED(buffer_handle->endpoint->instance);
  }
#endif

  // Recover what type is the frame
  control_byte = sli_cpc_hdlc_get_control(buffer_handle->hdlc_header);
  frame_type = sli_cpc_hdlc_get_frame_type(control_byte);

  MCU_ENTER_ATOMIC();
  endpoint = buffer_handle->endpoint;
  if (endpoint == NULL && frame_type != SLI_CPC_HDLC_FRAME_TYPE_SUPERVISORY) {
    // Only a REJECT frame for a closed endpoint may have a NULL endpoint
    MCU_EXIT_ATOMIC();
    SLI_CPC_ASSERT(0);
    return;
  }

  if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
    if (buffer_handle->on_write_complete_pending) {
      // Frame has already been acked by remote (this was a retransmission)
      buffer_handle->on_write_complete_pending = false;
      submit_write_completion(buffer_handle, SL_STATUS_OK);
    } else if (is_endpoint_connection_active(endpoint)) {
      status = sli_cpc_free_buffer_handle(buffer_handle);
      if (status == SL_STATUS_BUSY) {
        // Buffer waiting for an acknowledge, add to retransmit queue and start timer
        SLI_CPC_DEBUG_TRACE_ENDPOINT_DATA_FRAME_TRANSMIT_COMPLETED(endpoint);
        status = sli_cpc_timer_restart_timer(&endpoint->re_transmit_timer,
                                             endpoint->re_transmit_timeout,
                                             re_transmit_timeout_callback,
                                             buffer_handle);
        SLI_CPC_ASSERT(status == SL_STATUS_OK);
      } else if (status == SL_STATUS_OK) {
        // Buffer is not in the endpoint's queues anymore. This can happen when
        // the remote closes the endpoint unexpectedly, which clears the TX queues
        // (see sli_cpc_remote_terminated).
      } else {
        SLI_CPC_ASSERT(0);
      }
    } else {
      // Endpoint disconnected from remote, notify transmit failed
      endpoint->current_tx_window_space++;
      submit_write_completion(buffer_handle, SL_STATUS_TRANSMIT_INCOMPLETE);
    }
  } else if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_UNNUMBERED) {
    if (endpoint->uframe_tx_complete_pending == false) {
      // U-Frame TX complete should always be pending upon finishing a U-Frame TX
      MCU_EXIT_ATOMIC();
      SLI_CPC_ASSERT(0);
      return;
    }

    // Reset uframe_tx_complete_pending flag
    endpoint->uframe_tx_complete_pending = false;
    submit_write_completion(buffer_handle, SL_STATUS_OK);

    if (endpoint->uframe_holding_list != NULL) {
      // Put pending uframe in the tx queue
      sl_cpc_buffer_handle_t *frame = sli_cpc_pop_core_buffer_handle(&endpoint->uframe_holding_list);
      queue_for_transmission(endpoint, frame, true);
    }
  } else if (SLI_CPC_HDLC_FRAME_TYPE_SUPERVISORY) {
    if (endpoint != NULL) {
      // Reject frame for an endpoint that is not allocated results in a frame
      // with a NULL endpoint.
      SLI_CPC_DEBUG_TRACE_ENDPOINT_SUPERVISORY_FRAME_TRANSMIT_COMPLETED(endpoint);
    }

    sli_cpc_free_buffer_handle(buffer_handle);
  } else {
    // Unknown frame type
    MCU_EXIT_ATOMIC();
    SLI_CPC_ASSERT(0);
    return;
  }

  // Notify task if transmit frames are still queued in endpoints
  // Endpoint can be NULL if the notification was for an S-Frame
  if (endpoint && endpoint->instance->transmit_queue != NULL) {
    sli_cpc_signal_event(endpoint->instance, SL_CPC_SIGNAL_TX);
  }

  MCU_EXIT_ATOMIC();
}

/***************************************************************************//**
 * Notify Packet has been received and it is ready to be processed
 ******************************************************************************/
void sli_cpc_notify_rx_data_from_drv(sli_cpc_instance_t *inst)
{
  sli_cpc_signal_event(inst, SL_CPC_SIGNAL_RX);
}

/***************************************************************************/ /**
 * Determines if CPC is ok to enter sleep mode.
 ******************************************************************************/
#if !defined(SL_CATALOG_KERNEL_PRESENT) && defined(SL_CATALOG_POWER_MANAGER_PRESENT)
bool sl_cpc_is_ok_to_sleep(void)
{
  return (g_instance.rx_process_flag == 0
          && (g_instance.transmit_queue == NULL || !g_instance.driver->ops.is_transmit_ready(g_instance.driver)));
}
#endif

/***************************************************************************//**
 * Determines if CPC is ok to return to sleep mode on ISR exit.
 ******************************************************************************/
#if !defined(SL_CATALOG_KERNEL_PRESENT) && defined(SL_CATALOG_POWER_MANAGER_PRESENT)
sl_power_manager_on_isr_exit_t sl_cpc_sleep_on_isr_exit(void)
{
  return (sl_cpc_is_ok_to_sleep() ? SL_POWER_MANAGER_IGNORE : SL_POWER_MANAGER_WAKEUP);
}
#endif

/***************************************************************************/ /**
 * Convert a sl_cpc_endpoint_handle_t, owned by the user, to internal CPC
 * structure sl_cpc_endpoint_t.
 *
 * @param[in] endpoint_handle  Endpoint handle.
 * @param[out] endpoint        Pointer to a sl_cpc_endpoint_t pointer.
 *
 * @retval  SL_STATUS_OK            Successfully converted a handle to an endpoint.
 * @retval  SL_STATUS_INVALID_STATE Handle or endpoint is NULL
 ******************************************************************************/
static sl_status_t handle_to_ep(sl_cpc_endpoint_handle_t *handle,
                                sl_cpc_endpoint_t **endpoint)
{
  sl_cpc_endpoint_t *ep;

  if (!endpoint) {
    return SL_STATUS_INVALID_STATE;
  }

  MCU_ATOMIC_LOAD(ep, (sl_cpc_endpoint_t*)handle->ep);
  if (!ep) {
    return SL_STATUS_INVALID_STATE;
  }

  *endpoint = ep;

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Convert a sl_cpc_endpoint_handle_t, owned by the user, to internal CPC
 * structure sl_cpc_endpoint_t, entering the API (increase handle's ref_count).
 *
 * @param[in] endpoint_handle  Endpoint handle.
 * @param[out] endpoint        Pointer to a sl_cpc_endpoint_t pointer.
 *
 * @retval  SL_STATUS_OK            Successfully converted a handle to an endpoint.
 * @retval  SL_STATUS_INVALID_STATE Handle or endpoint is NULL, or failed to enter API
 ******************************************************************************/
static sl_status_t handle_to_ep_enter_api(sl_cpc_endpoint_handle_t *handle,
                                          sl_cpc_endpoint_t **endpoint)
{
  sl_status_t status;

  if (!cpc_enter_api(handle)) {
    return SL_STATUS_INVALID_STATE;
  }

  status = handle_to_ep(handle, endpoint);
  if (status != SL_STATUS_OK) {
    cpc_exit_api(handle);
  }

  return status;
}

/***************************************************************************//**
 * Initialize an instance
 ******************************************************************************/
static sl_status_t init_instance(sli_cpc_instance_t *inst)
{
  sl_status_t status;

  status = sli_cpc_instance_init(inst);
  if (status != SL_STATUS_OK) {
    goto exit;
  }

#if !defined(SL_CATALOG_KERNEL_PRESENT)
  status = start_instance(inst);
  if (status != SL_STATUS_OK) {
    goto deinit;
  }
#endif // SL_CATALOG_KERNEL_PRESENT

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  // mark primary as being initialzed
  inst->cpc_initialized = true;
#endif

  goto exit;

#if !defined(SL_CATALOG_KERNEL_PRESENT)
  deinit:
  sli_cpc_instance_deinit(inst);
#endif

  exit:
  return status;
}

/***************************************************************************//**
 * Get the whole instance ready for processing packets
 ******************************************************************************/
static sl_status_t start_instance(sli_cpc_instance_t *inst)
{
  sl_status_t status;

  status = sli_cpc_instance_start(inst);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  status = sli_cpc_security_init(on_state_change);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    return status;
  }
#endif // SL_CPC_ENDPOINT_SECURITY_ENABLED

  return status;
}

/***************************************************************************//**
 * CPC Task
 ******************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
void sl_cpc_task(void *arg)
{
  sli_cpc_instance_t *inst = (sli_cpc_instance_t*)arg;

  if (start_instance(inst) != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    osThreadExit();
  }

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && !defined(SLI_CPC_DEVICE_UNDER_TEST)
  // need to kickstart the transmission of the first packet to get the
  // event_signal semaphore to work as expected
  sli_cpc_reboot_sequence_process();
#endif

  while (1) {
    // Wait for signal indicating frame to receive or frame to transmit
    SLI_CPC_ASSERT(osSemaphoreAcquire(inst->event_signal, osWaitForever) == osOK);

    if (inst->cpc_core_in_reset) {
      if (sli_cpc_endpoints_are_all_closed(inst)) {
        sl_status_t status;

        inst->cpc_core_in_reset = false;

        // the system endpoint should never fail to connect
        status = sli_cpc_system_start(&inst->system_ep);
        SLI_CPC_ASSERT(status == SL_STATUS_OK);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
        sli_cpc_reboot_sequence_start();
#endif
      }
    }

    // Check for frame to receive and demux it
    decode_packet(inst);

    sli_cpc_system_process(&inst->system_ep);

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
    sli_cpc_security_process();
#endif

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && !defined(SLI_CPC_DEVICE_UNDER_TEST)
    sli_cpc_reboot_sequence_process();
#endif

    // Check for frame to transmit
    process_tx_queue(inst);

    process_free(inst);

    sli_cpc_dispatcher_process(inst);
  }
}
#endif

/***************************************************************************//**
 * Tick step function for a single instance.
 ******************************************************************************/
#if !defined(SL_CATALOG_KERNEL_PRESENT)
static void sl_cpc_process_action_for_instance(sli_cpc_instance_t *inst)
{
  uint32_t processed;
  uint32_t flag;
  MCU_DECLARE_IRQ_STATE;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
#if (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
  sli_cpc_fwu_process_action();

  if (sli_cpc_is_bootloader_running()) {
    return;
  }
#endif
#endif

  if (inst->cpc_core_in_reset) {
    if (sli_cpc_endpoints_are_all_closed(inst)) {
      sl_status_t status;

      inst->cpc_core_in_reset = false;

      // the system endpoint should never fail to connect
      status = sli_cpc_system_start(&inst->system_ep);
      SLI_CPC_ASSERT(status == SL_STATUS_OK);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
      sli_cpc_reboot_sequence_start();
#endif
    }
  }

  MCU_ATOMIC_LOAD(flag, inst->rx_process_flag);
  processed = flag;

  // Reception frame process action
  while (flag > 0) {
    decode_packet(inst);
    flag--;
  }

  MCU_ENTER_ATOMIC();
  inst->rx_process_flag -= processed;   // Do not process packet received during processing
  MCU_EXIT_ATOMIC();

  sli_cpc_system_process(&inst->system_ep);
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  sli_cpc_security_process();
#endif

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) && !defined(SLI_CPC_DEVICE_UNDER_TEST)
  sli_cpc_reboot_sequence_process();
#endif

  // Transmit frame process action
  while (inst->transmit_queue != NULL || inst->pending_on_security_ready_queue != NULL) {
    sl_status_t status;

    status = process_tx_queue(inst);
    if (status != SL_STATUS_OK) {
      break;
    }
  }

  process_free(inst);

  sli_cpc_dispatcher_process(inst);
}

/***************************************************************************//**
 * Tick step function, iterate over all instances and process them one by one.
 ******************************************************************************/
void sl_cpc_process_action(void)
{
  for (uint8_t i = 0; i < sl_cpc_instances_count; i++) {
    sl_cpc_process_action_for_instance(sl_cpc_instances[i]);
  }
}
#endif

/***************************************************************************//**
 * Initialize CPC endpoint
 ******************************************************************************/
static sl_status_t init_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                 sli_cpc_instance_t *inst,
                                 uint8_t id,
                                 uint8_t flags)
{
  MCU_DECLARE_IRQ_STATE;
  sl_cpc_endpoint_t *ep;
  sl_status_t status;

  LOCK_ENDPOINTS_LIST(inst);

  //  Check that endpoint doesn't exist
  ep = find_instance_endpoint(inst, id);
  if (ep != NULL) {
    status = SL_STATUS_ALREADY_INITIALIZED;
    goto release_ep_list;
  }

  // Allocate and setup the endpoint
  status = sli_cpc_get_endpoint(inst, &ep);
  if (status != SL_STATUS_OK) {
    goto release_ep_list;
  }

  SLI_CPC_DEBUG_ENDPOINT_INIT(ep);

  // Initialize the re-transmit timer
  status = sli_cpc_timer_init(&ep->re_transmit_timer);
  if (status != SL_STATUS_OK) {
    SLI_CPC_ASSERT(0);
    goto release_ep_list;
  }

  // Set default options
  ep->configured_tx_window_size = 1;

  // Initialize endpoint
  ep->current_tx_window_space = ep->configured_tx_window_size;
  ep->flags = flags;   // Flags should be set using the set_option API.
  ep->id = id;
  ep->seq = 0;
  ep->ack = 0;
  ep->frames_count_re_transmit_queue = 0;
  ep->state = SL_CPC_STATE_CLOSED;
  ep->node.node = NULL;
  ep->re_transmit_timeout = sli_cpc_timer_ms_to_tick(SLI_CPC_INIT_RE_TRANSMIT_TIMEOUT_MS);
  ep->instance = inst;

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  if (id == SL_CPC_ENDPOINT_SYSTEM
      || id == SL_CPC_ENDPOINT_SECURITY) {
    ep->encrypted = false;
  } else {
    if (flags & SL_CPC_ENDPOINT_FLAG_DISABLE_ENCRYPTION) {
      ep->encrypted = false;
    } else {
      ep->encrypted = true;
    }
  }
  ep->frame_counter_tx = 0;
  ep->frame_counter_rx = 0;
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
  ep->shutdown_timeout = osWaitForever;
  ep->write_timeout = osWaitForever;
  osMutexAttr_t mutex_attr;
  mutex_attr.attr_bits = osMutexRecursive;
  mutex_attr.name = "CPC Endpoint Lock";
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
  mutex_attr.cb_mem = ep->lock_cb;
  mutex_attr.cb_size = osMutexCbSize;
#else
  mutex_attr.cb_mem = NULL;
  mutex_attr.cb_size = 0U;
#endif
  ep->lock = osMutexNew(&mutex_attr);
  if (ep->lock == 0) {
    ep->state = SL_CPC_STATE_FREED;
    sli_cpc_free_endpoint(inst, ep);
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_NO_MORE_RESOURCE;
    goto release_ep_list;
  }
#endif

  sl_slist_init(&ep->iframe_receive_queue);
  sl_slist_init(&ep->uframe_receive_queue);
  sl_slist_init(&ep->re_transmit_queue);
  sl_slist_init(&ep->iframe_holding_list);
  sl_slist_init(&ep->uframe_holding_list);

  sl_slist_push(&inst->endpoints, &ep->node);
  sl_slist_sort(&inst->endpoints, sort_endpoints);

#if defined(SL_CATALOG_KERNEL_PRESENT)
  ep->receive_signal = NULL;
#endif

  MCU_ENTER_ATOMIC();
  // Update the handle
  endpoint_handle->id = id;
  endpoint_handle->ref_count = 1u;
  endpoint_handle->ep = (void*) ep;
  MCU_EXIT_ATOMIC();

  status = SL_STATUS_OK;

  release_ep_list:
  RELEASE_ENDPOINTS_LIST(inst);
  SLI_CPC_DEBUG_TRACE_CORE_OPEN_ENDPOINT(inst);

  return status;
}

/***************************************************************************//**
 * Wrapper function to signal the state_event_signal semaphore.
 * Returns if the state change was signaled or not.
 ******************************************************************************/
static inline bool notify_state_change(sl_cpc_endpoint_t *endpoint)
{
  bool signaled = false;
  #if defined(SL_CATALOG_KERNEL_PRESENT)
  if (endpoint->state_event_signal != NULL) {
    SLI_CPC_ASSERT(osSemaphoreRelease(endpoint->state_event_signal) == osOK);
    signaled = true;
  }
  #else
  (void) endpoint;
  #endif
  return signaled;
}

/***************************************************************************//**
 * Wrapper function to call the `on_connect` callback on an endpoint with the
 * given `status`. This function checks if `on_connect` is defined befor calling it
 ******************************************************************************/
static bool notify_ep_on_connect(sl_cpc_endpoint_t * ep, sl_status_t status)
{
  bool skip_callback = notify_state_change(ep);

  if (!skip_callback && ep->on_connect) {
    ep->on_connect(ep->id, ep->on_connect_arg, status);

    return true;
  }

  return false;
}

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************//**
 * Callback called when the secondary replies to a connection request command
 ******************************************************************************/
static void on_connection_request_reply(sli_cpc_property_id_t property_id,
                                        void *property_value,
                                        size_t property_length,
                                        void *on_reply_arg,
                                        sl_status_t status)
{
  sl_cpc_endpoint_t *ep = (sl_cpc_endpoint_t*)on_reply_arg;
  sl_cpc_endpoint_state_t *reply_state;
  uint8_t endpoint_id;

  SLI_CPC_ASSERT(property_id >= PROP_ENDPOINT_STATE_0);
  SLI_CPC_ASSERT(property_id <= PROP_ENDPOINT_STATE_255);
  SLI_CPC_ASSERT(ep != NULL);

  LOCK_ENDPOINT(ep);

  // Command failed, go back to the closed state
  if (status != SL_STATUS_OK && status != SL_STATUS_IN_PROGRESS) {
    ep->state = SL_CPC_STATE_CLOSED;
    goto notify;
  }

  endpoint_id = PROPERTY_ID_TO_EP_ID(property_id);
  if (ep->id != endpoint_id) {
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_FAIL;
    goto notify;
  }

  status = SL_STATUS_OK;

  // Validate response
  if (property_length != sizeof(sl_cpc_endpoint_state_t)
      || property_value == NULL) {
    SLI_CPC_ASSERT(0);
    status = SL_STATUS_FAIL;
    goto notify;
  }

  reply_state = (sl_cpc_endpoint_state_t *)property_value;

  // Received valid response
  if (*reply_state == SL_CPC_STATE_CONNECTED) {
    if (ep->state != SL_CPC_STATE_CONNECTING) {
      SLI_CPC_ASSERT(0);
      status = SL_STATUS_FAIL;
      goto notify;
    }
    // Remote accepted connection
    ep->state = SL_CPC_STATE_CONNECTED;
  } else {
    // Remote refused connection, stay in the same state without notify
    goto release_ep;
  }

  notify:
  notify_ep_on_connect(ep, status);

  release_ep:
  RELEASE_ENDPOINT(ep);
}

/***************************************************************************//**
 * Callback called when the secondary replies to a terminate notification.
 * (terminate is SET ENDPOINT_STATE=CLOSED)
 ******************************************************************************/
static void on_endpoint_terminate_reply(sli_cpc_property_id_t property_id,
                                        void *property_value,
                                        size_t property_length,
                                        void *on_reply_arg,
                                        sl_status_t status)
{
  sl_cpc_endpoint_t *ep = (sl_cpc_endpoint_t*)on_reply_arg;
  uint8_t endpoint_id;

  (void) property_value;
  (void) property_length;

  // Validate the reply
  SLI_CPC_ASSERT(ep != NULL);
  SLI_CPC_ASSERT(property_id >= PROP_ENDPOINT_STATE_0);
  SLI_CPC_ASSERT(property_id <= PROP_ENDPOINT_STATE_255);

  if (status != SL_STATUS_OK && status != SL_STATUS_IN_PROGRESS) {
    SL_CPC_JOURNAL_RECORD_WARNING("Terminate failed on EP:", PROPERTY_ID_TO_EP_ID(property_id));
  }

  endpoint_id = PROPERTY_ID_TO_EP_ID(property_id);
  SLI_CPC_ASSERT(ep->id == endpoint_id);

  // A terminate command is a notification. No action is taken upon receiving
  // the reply.
}

/***************************************************************************//**
 * Callback called when the secondary replies to a shutdown command.
 * (shutdown is SET ENDPOINT_STATE=SHUTDOWN)
 ******************************************************************************/
static void on_endpoint_shutdown_reply(sli_cpc_property_id_t property_id,
                                       void *property_value,
                                       size_t property_length,
                                       void *on_reply_arg,
                                       sl_status_t status)
{
  sl_cpc_endpoint_t *ep = (sl_cpc_endpoint_t*)on_reply_arg;
  sl_cpc_endpoint_state_t *reply_state;
  uint8_t endpoint_id;

  LOCK_ENDPOINT(ep);

  // Endpoint connection was interrupted before command could complete.
  // Ignore the reply and leave the endpoint as-is.
  if (is_endpoint_connection_active(ep) == false) {
    goto release_ep;
  }

  // Command failed, disconnect the endpoint
  if (status != SL_STATUS_OK && status != SL_STATUS_IN_PROGRESS) {
    sli_cpc_instance_on_remote_ep_disconnected(ep->instance, ep->id);
    goto notify;
  }

  endpoint_id = PROPERTY_ID_TO_EP_ID(property_id);
  if (ep->id != endpoint_id) {
    SLI_CPC_ASSERT(0);
    goto notify;
  }

  // Validate response
  if (property_length != sizeof(sl_cpc_endpoint_state_t)
      || property_value == NULL) {
    SLI_CPC_ASSERT(0);
    goto notify;
  }

  reply_state = (sl_cpc_endpoint_state_t *)property_value;

  // Received valid response
  if (*reply_state == SL_CPC_STATE_DISCONNECTED) {
    // Remote is done transmitting, end connection
    SLI_CPC_ASSERT(ep->state == SL_CPC_STATE_REMOTE_SHUTDOWN || ep->state == SL_CPC_STATE_SHUTDOWN);
    sli_cpc_instance_on_remote_ep_disconnected(ep->instance, ep->id);
  } else {
    // Remote has pending TX frames. Stay in SHUTDOWN until notification.
    SLI_CPC_ASSERT(ep->state == SL_CPC_STATE_SHUTDOWN);
    goto release_ep;
  }

  notify:
  // Return value is irrelevant, since no callback is associated with a shutdown
  notify_state_change(ep);
  release_ep:
  RELEASE_ENDPOINT(ep);
}

/***************************************************************************//**
 * Connect endpoint to secondary
 ******************************************************************************/
sl_status_t sl_cpc_connect_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle, uint8_t flags)
{
  sl_cpc_endpoint_t *ep;
  sl_cpc_endpoint_state_t state;
  sl_status_t status;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  bool blocking = (flags & SL_CPC_FLAG_NO_BLOCK) != SL_CPC_FLAG_NO_BLOCK;
  osStatus_t os_status;
  osSemaphoreId_t state_event_signal;
  osSemaphoreAttr_t semaphore_attr;
  #if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
  uint32_t semaphore_cb[osSemaphoreCbSize / 4];
  #endif
#else
  (void) flags;
#endif

  // Validate input parameters
  if (endpoint_handle == NULL) {
    return SL_STATUS_INVALID_HANDLE;
  }

  status = handle_to_ep_enter_api(endpoint_handle, &ep);
  if (status != SL_STATUS_OK) {
    return status;
  }

  LOCK_ENDPOINT(ep);

  switch (ep->state) {
    case SL_CPC_STATE_CLOSED:
      // Endpoint closed, proceed with connection request
      break;
    case SL_CPC_STATE_CONNECTED:
      // Connection already established
      status = SL_STATUS_OK;
      goto exit;
      break;
    case SL_CPC_STATE_CONNECTING:
    case SL_CPC_STATE_OPEN:
      // Connection request sent previously
      status = SL_STATUS_IN_PROGRESS;
      goto exit;
      break;
    default:
      // Any other state is invalid
      status = SL_STATUS_INVALID_STATE;
      goto exit;
  }

  // System endpoint is always connected
  if (ep->id == SL_CPC_ENDPOINT_SYSTEM) {
    ep->state = SL_CPC_STATE_CONNECTED;
    status = SL_STATUS_OK;
    goto exit;
  }

  status = cpc_core_is_ready();
  if (status != SL_STATUS_OK) {
    goto exit;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (blocking) {
    // Allocate semaphore for blocking mode
    if (osKernelGetState() != osKernelRunning) {
      status = SL_STATUS_NOT_READY;
      goto exit;
    }

    if (ep->state_event_signal != NULL) {
      // Another thread is already connecting
      SLI_CPC_ASSERT(0);
      status = SL_STATUS_INVALID_STATE;
      goto exit;
    }

    // Allocate endpoint semaphore
    #if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
    semaphore_attr.cb_mem = semaphore_cb;
    semaphore_attr.cb_size = osSemaphoreCbSize;
    #else
    semaphore_attr.cb_mem = NULL;
    semaphore_attr.cb_size = 0U;
    #endif // defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)

    ep->state_event_signal = osSemaphoreNew(1, 0u, &semaphore_attr);
    if (ep->state_event_signal == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
      goto exit;
    }
  }
#endif // defined(SL_CATALOG_KERNEL_PRESENT)

  // Endpoint in connecting state while pending reply
  ep->state = SL_CPC_STATE_CONNECTING;

  // Send connection request
  state = SL_CPC_STATE_CONNECTED;
  status = sli_cpc_system_cmd_property_set(on_connection_request_reply,
                                           (void*)ep,
                                           EP_ID_TO_PROPERTY_STATE(ep->id),
                                           0,
                                           SLI_CPC_SYSTEM_CMD_TIMEOUT_MS,
                                           &state,   // State is copied in cmd_propery_set
                                           sizeof(state),
                                           SYSTEM_EP_IFRAME);
  if (status != SL_STATUS_OK) {
    ep->state = SL_CPC_STATE_CLOSED;
#if defined(SL_CATALOG_KERNEL_PRESENT)
    if (blocking) {
      // Cleanup semaphore
      state_event_signal = ep->state_event_signal;
      ep->state_event_signal =  NULL;
      os_status = osSemaphoreDelete(state_event_signal);
      if (os_status != osOK) {
        status = SL_STATUS_FAIL;
      }
    }
#endif
    goto exit;
  }

  status = SL_STATUS_IN_PROGRESS;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (!blocking) {
    // Don't block
    goto exit;
  }

  // Handle blocking mode
  // Wait for connection reply
  RELEASE_ENDPOINT(ep);
  os_status = osSemaphoreAcquire(ep->state_event_signal, osWaitForever);
  SLI_CPC_ASSERT(os_status == osOK);
  LOCK_ENDPOINT(ep);

  // Cleanup semaphore
  state_event_signal = ep->state_event_signal;
  ep->state_event_signal = NULL;
  SLI_CPC_ASSERT(osSemaphoreDelete(state_event_signal) == osOK);

  // EP state updated in the on_connection_request_reply callback
  if (ep->state == SL_CPC_STATE_CONNECTED) {
    // Connection successful
    status = SL_STATUS_OK;
  } else {
    // Connection failed
    SLI_CPC_ASSERT(ep->state == SL_CPC_STATE_CLOSED);
    status = SL_STATUS_TIMEOUT;
  }

#endif // defined(SL_CATALOG_KERNEL_PRESENT)

  exit:
  RELEASE_ENDPOINT(ep);
  cpc_exit_api(endpoint_handle);
  return status;
}

/***************************************************************************//**
 * Remote notified its endpoint is open.
 ******************************************************************************/
void sli_cpc_remote_open_unsolicited(uint8_t id)
{
  sl_cpc_endpoint_t *ep;
  sl_cpc_endpoint_state_t ep_state;
  sl_status_t status;

  LOCK_ENDPOINTS_LIST(&g_instance);

  // Verify input parameter
  ep = find_endpoint(id);
  if (ep == NULL) {
    // Freed endpoint, disregard
    RELEASE_ENDPOINTS_LIST(&g_instance);

    return;
  }

  LOCK_ENDPOINT(ep);
  RELEASE_ENDPOINTS_LIST(&g_instance);

  // Send another connection request as previous one was denied
  if (ep->state == SL_CPC_STATE_OPEN) {
    ep_state = SL_CPC_STATE_CONNECTED;
    status = sli_cpc_system_cmd_property_set(on_connection_request_reply,
                                             (void*)ep,
                                             EP_ID_TO_PROPERTY_STATE(ep->id),
                                             0,
                                             SLI_CPC_SYSTEM_CMD_TIMEOUT_MS,   // Allow 10 seconds for connection
                                             &ep_state,
                                             sizeof(ep_state),
                                             SYSTEM_EP_IFRAME);

    if (status == SL_STATUS_OK) {
      // System command success
      ep->state = SL_CPC_STATE_CONNECTING;
    } else {
      // System command failure
      ep->state = SL_CPC_STATE_CLOSED;
      notify_ep_on_connect(ep, status);
    }
  }
  RELEASE_ENDPOINT(ep);
}
/***************************************************************************//**
 * Remote is asking to shutdown its connection.
 ******************************************************************************/
void sli_cpc_remote_shutdown_unsolicited(uint8_t id)
{
  sl_cpc_endpoint_t *ep;
  sl_cpc_endpoint_state_t reply_state;
  sl_status_t status;

  LOCK_ENDPOINTS_LIST(&g_instance);
  ep = find_endpoint(id);
  if (ep == NULL) {
    // Remote tried to shutdown an endpoint that is not allocated. Ignore request.
    RELEASE_ENDPOINTS_LIST(&g_instance);

    return;
  }

  LOCK_ENDPOINT(ep);
  RELEASE_ENDPOINTS_LIST(&g_instance);

  status = sli_cpc_instance_on_remote_ep_shutdown(&g_instance, id, &reply_state);
  if (status != SL_STATUS_IN_PROGRESS) {
    // Endpoint was either done transmitting and the shutdown request should be
    // accepted, or the endpoint was not in a connected state to begin with.
    // In both cases, we want the remote to set its endpoint state to DISCONNECTED.
    status = sli_cpc_system_cmd_property_set(on_endpoint_shutdown_reply,
                                             (void*)ep,
                                             EP_ID_TO_PROPERTY_STATE(ep->id),
                                             0,         // try only once
                                             SLI_CPC_SYSTEM_CMD_TIMEOUT_MS,
                                             &reply_state,
                                             sizeof(reply_state),
                                             SYSTEM_EP_IFRAME);
    if (status != SL_STATUS_OK) {
      // System endpoint command failed.
      if (is_endpoint_connection_active(ep)) {
        // Enpoint had an active connection, but we failed to reply to the remote's
        // request. Go to the disconnected state without notifying the remote.
        ep->state = SL_CPC_STATE_DISCONNECTED;
        clean_tx_queues(ep);
      } else {
        // Endpoint was not connected upon receiving the shutdown request. Ignore
        // the command.
      }
    }
  } else {
    // Endpoint still has frames pending transmission. Once they have all been
    // transmitted, the system will automatically send the shutdown request.
  }
  RELEASE_ENDPOINT(ep);
}
#endif // SL_CATALOG_CPC_PRIMARY_PRESENT

/***************************************************************************//**
 * Remote has reset.
 ******************************************************************************/
void sli_cpc_reset(sli_cpc_instance_t *inst)
{
  sl_cpc_endpoint_t *ep;

  inst->cpc_core_in_reset = true;

  LOCK_ENDPOINTS_LIST(inst);
  SL_SLIST_FOR_EACH_ENTRY(inst->endpoints, ep, sl_cpc_endpoint_t, node) {
    LOCK_ENDPOINT(ep);

    switch (ep->state) {
      // if endpoint is waiting for connection, just notify the endpoint
      // that the connection failed, no extra clean-up required.
      case SL_CPC_STATE_OPEN:
      case SL_CPC_STATE_CONNECTING:
        ep->state = SL_CPC_STATE_CLOSED;
        notify_ep_on_connect(ep, SL_STATUS_ABORT);
        break;

      case SL_CPC_STATE_CONNECTED:
      case SL_CPC_STATE_SHUTTING_DOWN:
      case SL_CPC_STATE_SHUTDOWN:
      case SL_CPC_STATE_REMOTE_SHUTDOWN:
        ep->state = SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE;
        // notify error
        notify_error(ep);
        break;

      // If already in error, nothing to do
      case SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE:
      case SL_CPC_STATE_ERROR_SECURITY_INCIDENT:
      case SL_CPC_STATE_ERROR_FAULT:
        break;

      // If the endpoint is already closing/closed, nothing to do
      case SL_CPC_STATE_CLOSED:
      case SL_CPC_STATE_CLOSING:
      case SL_CPC_STATE_DISCONNECTED:
        break;

      case SL_CPC_STATE_FREED:
      default:
        // Endpoint in that state should not be in the list
        SLI_CPC_ASSERT(0);
        break;
    }

    RELEASE_ENDPOINT(ep);
  }
  RELEASE_ENDPOINTS_LIST(inst);
}

/***************************************************************************//**
 * Helper function to check if all endpoints are closed.
 ******************************************************************************/
bool sli_cpc_endpoints_are_all_closed(sli_cpc_instance_t* inst)
{
  sl_cpc_endpoint_t *ep;
  bool closed = true;

  LOCK_ENDPOINTS_LIST(inst);

  SL_SLIST_FOR_EACH_ENTRY(inst->endpoints, ep, sl_cpc_endpoint_t, node) {
    LOCK_ENDPOINT(ep);
    if (ep->state != SL_CPC_STATE_CLOSED) {
      closed = false;

      RELEASE_ENDPOINT(ep);
      break;
    }

    RELEASE_ENDPOINT(ep);
  }

  RELEASE_ENDPOINTS_LIST(inst);

  return closed;
}

/***************************************************************************//**
 * Check if endpoint ID is already in use for specified instance.
 ******************************************************************************/
static sl_cpc_endpoint_t* find_instance_endpoint(sli_cpc_instance_t *inst,
                                                 uint8_t id)
{
  sl_cpc_endpoint_t *endpoint;

  SL_SLIST_FOR_EACH_ENTRY(inst->endpoints, endpoint, sl_cpc_endpoint_t, node) {
    if (endpoint->id == id) {
      if (endpoint->state == SL_CPC_STATE_FREED) {
        // Should not be in the endpoint list if freed
        SLI_CPC_ASSERT(0);
        return NULL;
      }
      return endpoint;
    }
  }

  return NULL;
}

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT) || (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
/***************************************************************************//**
 * Check if endpoint ID is already in use in the global instance.
 ******************************************************************************/
static sl_cpc_endpoint_t* find_endpoint(uint8_t id)
{
  return find_instance_endpoint(&g_instance, id);
}
#endif

/***************************************************************************//**
 * Endpoint list sorting function
 ******************************************************************************/
static bool sort_endpoints(sl_slist_node_t *item_l,
                           sl_slist_node_t *item_r)
{
  sl_cpc_endpoint_t *ep_left = SL_SLIST_ENTRY(item_l, sl_cpc_endpoint_t, node);
  sl_cpc_endpoint_t *ep_right = SL_SLIST_ENTRY(item_r, sl_cpc_endpoint_t, node);

  if (ep_left->id < ep_right->id) {
    return true;
  } else {
    return false;
  }
}

/***************************************************************************//**
 * Queue a buffer_handle for transmission. This function will either put the
 * buffer in the transmit queue if there is enough room in the endpoint's TX
 * window, or in the endpoint holding list otherwise.
 * This function doesn't lock the endpoint, it's caller's responsibility.
 ******************************************************************************/
static void queue_for_transmission(sl_cpc_endpoint_t *ep,
                                   sl_cpc_buffer_handle_t *buffer_handle,
                                   bool signal_tx_event)
{
  uint8_t type = sli_cpc_hdlc_get_frame_type(buffer_handle->control);
  MCU_DECLARE_IRQ_STATE;
  MCU_ENTER_ATOMIC();
  // Check if room in TX Queue for endpoint
  if ((type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION &&  ep->current_tx_window_space > 0)
      || (type == SLI_CPC_HDLC_FRAME_TYPE_UNNUMBERED &&  ep->uframe_tx_complete_pending == false)) {
    if (type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
      ep->current_tx_window_space--;
    } else {
      ep->uframe_tx_complete_pending = true;
    }
    // Put frame in Tx Q so that it can be transmitted by CPC Core later
    sli_cpc_push_back_core_buffer_handle(&ep->instance->transmit_queue, buffer_handle);

    if (signal_tx_event) {
      // Signal task/process_action that frame is in Tx Queue
      sli_cpc_signal_event(ep->instance, SL_CPC_SIGNAL_TX);
    }
  }
  // No more room in tx_window, add to corresponding holding list
  else {
    sl_slist_node_t **holding_list = type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION
                                     ? &ep->iframe_holding_list : &ep->uframe_holding_list;
    sli_cpc_push_back_core_buffer_handle(holding_list, buffer_handle);
  }
  MCU_EXIT_ATOMIC();
}

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
static bool should_encrypt_frame(sl_cpc_buffer_handle_t *frame)
{
  sl_cpc_endpoint_t *ep = frame->endpoint;
  uint16_t data_length = frame->data_length;
  uint8_t control = frame->control;
  bool type = sli_cpc_hdlc_get_frame_type(control);
  bool encrypt;

  if (type != SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
    return false;
  }

  if (data_length == 0) {
    return false;
  }

  /* security buffer already exists, frame already encrypted */
  if (frame->security_tag != NULL) {
    return false;
  }

  LOCK_ENDPOINT(ep);
  encrypt = ep->encrypted;
  RELEASE_ENDPOINT(ep);

  return encrypt;
}

/***************************************************************************//**
 * Return true if security is ready to process packets on regular endpoints.
 ******************************************************************************/
static bool security_is_ready(void)
{
  sl_cpc_security_state_t security_state = sl_cpc_security_get_state();

  return security_state == SL_CPC_SECURITY_STATE_INITIALIZED
         || security_state == SL_CPC_SECURITY_STATE_DISABLED;
}
#endif

/***************************************************************************//**
 * Write data from an endpoint
 ******************************************************************************/
static sl_status_t write(sl_cpc_endpoint_t *ep,
                         void* data,
                         uint16_t data_length,
                         uint8_t flags,
                         void *on_write_completed_arg)
{
  sl_cpc_buffer_handle_t *frame_handle;
  sl_status_t status;
  bool is_final = flags & SL_CPC_FLAG_INFORMATION_POLL_FINAL;
  bool iframe = true;
  uint8_t type = 0;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (osKernelGetState() != osKernelRunning) {
    return SL_STATUS_INVALID_STATE;
  }
#endif
  // Get new frame handler
#if defined(SL_CATALOG_KERNEL_PRESENT)
  bool block = !(flags & SL_CPC_FLAG_NO_BLOCK);
  status = sli_cpc_get_write_buffer_handle(ep->instance, &frame_handle, block, ep->write_timeout);
#else
  status = sli_cpc_get_write_buffer_handle(ep->instance, &frame_handle, false, 0);
#endif
  if (status != SL_STATUS_OK) {
    return status;
  }

  LOCK_ENDPOINT(ep);

  if ((flags & SL_CPC_FLAG_UNNUMBERED_INFORMATION)
      || (flags & SL_CPC_FLAG_UNNUMBERED_ACKNOWLEDGE)
      || (flags & SL_CPC_FLAG_UNNUMBERED_POLL_FINAL)) {
    if (!(ep->flags & SL_CPC_ENDPOINT_FLAG_UFRAME_ENABLE)) {
      RELEASE_ENDPOINT(ep);

      status = sli_cpc_free_buffer_handle(frame_handle);
      SLI_CPC_ASSERT(status == SL_STATUS_OK);

      return SL_STATUS_INVALID_PARAMETER;
    }

    iframe = false;

    if (flags & SL_CPC_FLAG_UNNUMBERED_INFORMATION) {
      type = SLI_CPC_HDLC_CONTROL_UNNUMBERED_TYPE_INFORMATION;
    } else if ((flags & SL_CPC_FLAG_UNNUMBERED_POLL_FINAL)) {
      type = SLI_CPC_HDLC_CONTROL_UNNUMBERED_TYPE_POLL_FINAL;
    } else if ((flags & SL_CPC_FLAG_UNNUMBERED_ACKNOWLEDGE)) {
      type = SLI_CPC_HDLC_CONTROL_UNNUMBERED_TYPE_ACKNOWLEDGE;
    }
  } else if (ep->flags & SL_CPC_ENDPOINT_FLAG_IFRAME_DISABLE) {
    RELEASE_ENDPOINT(ep);

    status = sli_cpc_free_buffer_handle(frame_handle);
    SLI_CPC_ASSERT(status == SL_STATUS_OK);

    return SL_STATUS_INVALID_PARAMETER;
  }

  // Write only allowed when TX connection is active
  if (ep->state != SL_CPC_STATE_CONNECTED) {
    RELEASE_ENDPOINT(ep);

    status = sli_cpc_free_buffer_handle(frame_handle);
    SLI_CPC_ASSERT(status == SL_STATUS_OK);

    return SL_STATUS_INVALID_STATE;
  }

  // Link the data buffer inside the frame buffer
  frame_handle->data = data;
  frame_handle->data_length = data_length;
  frame_handle->endpoint = ep;
  frame_handle->address = ep->id;
  frame_handle->arg = on_write_completed_arg;

  if (iframe) {
    // Set the SEQ number and ACK number in the control byte
    frame_handle->control = sli_cpc_hdlc_create_control_data(ep->seq, ep->ack, is_final);
    // Update endpoint sequence number
    ep->seq++;
    ep->seq %= 8;
  } else {
    frame_handle->control = sli_cpc_hdlc_create_control_unnumbered(type);
  }

  // Compute payload CRC

  frame_handle->fcs = sli_cpc_get_crc_sw(data, data_length);

  queue_for_transmission(ep, frame_handle, true);

  RELEASE_ENDPOINT(ep);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * De-multiplex receive frame and put it in right endpoint queue.
 ******************************************************************************/
static void decode_packet(sli_cpc_instance_t *inst)
{
  sl_cpc_buffer_handle_t *rx_handle;
  sl_cpc_endpoint_t *endpoint;
  uint16_t data_length;
  uint8_t control;
  uint8_t address;
  uint8_t type;

  if (inst->driver->ops.read(inst->driver, &rx_handle) != SL_STATUS_OK) {
    return;
  }

  SLI_CPC_DEBUG_TRACE_CORE_RXD_FRAME(inst);

#if !defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
  if (!inst->sli_cpc_driver_capabilities.preprocess_hdlc_header) {
    // Validate header checksum. In case it is invalid, drop the packet.
    uint16_t hcs = sli_cpc_hdlc_get_hcs(rx_handle->hdlc_header);
    if (!sli_cpc_validate_crc_sw(rx_handle->hdlc_header, SLI_CPC_HDLC_HEADER_SIZE, hcs)
        || ((uint8_t*)rx_handle->hdlc_header)[0] != SLI_CPC_HDLC_FLAG_VAL) {
      // If HCS is invalid, we drop the packet as we cannot NAK it
      sli_cpc_free_buffer_handle(rx_handle);
      SLI_CPC_DEBUG_TRACE_CORE_INVALID_HEADER_CHECKSUM(inst);

      return;
    }
  }
#endif

  address = sli_cpc_hdlc_get_address(rx_handle->hdlc_header);
  control = sli_cpc_hdlc_get_control(rx_handle->hdlc_header);
  data_length = sli_cpc_hdlc_get_length(rx_handle->hdlc_header);
  type = sli_cpc_hdlc_get_frame_type(control);

  // Check if driver dropped the payload due to buffer starvation
  // rx_handle->data_length is populated by the driver
  // data_length is what the header says the payload length will be
  // both of these can differ if the driver had to drop the payload
  bool payload_dropped = (rx_handle->data_length == 0 && data_length > 0);

  if (data_length >= SLI_CPC_RX_DATA_MAX_LENGTH_INST(inst)) {
    rx_handle->data_length = SLI_CPC_RX_DATA_MAX_LENGTH_INST(inst);
  }

  LOCK_ENDPOINTS_LIST(inst);
  endpoint = find_instance_endpoint(inst, address);

  // lock endpoint before releasing endpoint list
  if (endpoint != NULL) {
    LOCK_ENDPOINT(endpoint);
  }

  RELEASE_ENDPOINTS_LIST(inst);

  if (endpoint == NULL) {
    if (type != SLI_CPC_HDLC_FRAME_TYPE_SUPERVISORY) {
      // Received data on a unallocated endpoint, reject
      transmit_reject(inst, NULL, address, 0, SL_CPC_REJECT_UNREACHABLE_ENDPOINT);
    }
    sli_cpc_free_buffer_handle(rx_handle);
    return;
  }

  SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_FRAME(endpoint)

  #if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  // endpoint is in CONNECTING state until it receives the ack to its
  // STATE IS CONNECTED frame. But the ack might get lost because of a
  // CRC error. In that case, the primary sees the endpoint as CONNECTED
  // and start transmitting while the secondary is still CONNECTING.
  // So it's a valid scenario to receive an I-Frame in that state.
  if (endpoint->state == SL_CPC_STATE_CONNECTING && type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
    sli_cpc_remote_connected_endpoint(inst, endpoint, SL_STATUS_OK);
  }
  #endif

  if (!is_endpoint_connection_active(endpoint)
      || (endpoint->state == SL_CPC_STATE_REMOTE_SHUTDOWN
          && type != SLI_CPC_HDLC_FRAME_TYPE_SUPERVISORY)) {
    // The connection is not considered active in reception when the endpoint
    // is in REMOTE_SHUTDOWN, since this indicates that the remote signaled that
    // it was done transmitting. Hence, we should reject the frame because it
    // should not have been sent. Also reject any frame if the endpoint does not
    // have an active connection.
    transmit_reject(endpoint->instance, endpoint, address, 0, SL_CPC_REJECT_UNREACHABLE_ENDPOINT);
    sli_cpc_free_buffer_handle(rx_handle);
    SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_SUPERVISORY_DROPPED(endpoint);
    RELEASE_ENDPOINT(endpoint);
    return;
  }

  rx_handle->endpoint = endpoint;

  if ((type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION
       || type == SLI_CPC_HDLC_FRAME_TYPE_SUPERVISORY)
      && rx_handle->reason == SL_CPC_REJECT_NO_ERROR) {
    // get ack from control byte
    uint8_t ack = sli_cpc_hdlc_get_ack(control);

    // Clean Tx queue
    receive_ack(endpoint, ack);
  }

  // Early exit if payload was dropped - handle invalid combinations safely
  if (payload_dropped) {
    if (data_length > 0 && rx_handle->data != NULL) {
      // Invalid state: payload marked as dropped but data pointer exists
      SL_CPC_JOURNAL_RECORD_DEBUG("Invalid frame : payload_dropped but data exists", endpoint->id);
      sli_cpc_free_buffer_handle(rx_handle);
      RELEASE_ENDPOINT(endpoint);
      return;
    }
    SL_CPC_JOURNAL_RECORD_DEBUG("Frame with dropped payload - ACK processed, ignoring payload", endpoint->id);
    sli_cpc_free_buffer_handle(rx_handle);
    RELEASE_ENDPOINT(endpoint);
    return;
  }

  // Check for invalid combination: header says there's payload but driver didn't provide data
  if (data_length > 0 && rx_handle->data == NULL) {
    SL_CPC_JOURNAL_RECORD_DEBUG("Invalid frame: data_length > 0 but no data pointer", endpoint->id);
    sli_cpc_free_buffer_handle(rx_handle);
    RELEASE_ENDPOINT(endpoint);
    return;
  }

  if (rx_handle->reason != SL_CPC_REJECT_NO_ERROR) {
    transmit_reject(endpoint->instance, endpoint, address, endpoint->ack, rx_handle->reason);
    rx_handle->reason = SL_CPC_REJECT_NO_ERROR;
    sli_cpc_free_buffer_handle(rx_handle);
    SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_SUPERVISORY_DROPPED(endpoint);
  }

  if (data_length > 0) {
    // make sure the payload is at least 3 bytes (FCS + 1 byte payload)
    if (data_length <= SLI_CPC_HDLC_FCS_SIZE) {
      transmit_reject(endpoint->instance, endpoint, address, endpoint->ack, SL_CPC_REJECT_ERROR);

      sli_cpc_free_buffer_handle(rx_handle);
      RELEASE_ENDPOINT(endpoint);

      return;
    }

    SLI_CPC_ASSERT(rx_handle->data != NULL);

    data_length = data_length - SLI_CPC_HDLC_FCS_SIZE;
    uint16_t fcs = sli_cpc_hdlc_get_fcs(rx_handle->data, data_length);
    bool crc_valid;

    #if defined(SL_CATALOG_CPC_DRIVER_HW_CRC_PRESENT)
    crc_valid = fcs == rx_handle->fcs;
    #else
    crc_valid = sli_cpc_validate_crc_sw(rx_handle->data, data_length, fcs);
    #endif

    if (!crc_valid) {
      SLI_CPC_DEBUG_TRACE_CORE_INVALID_PAYLOAD_CHECKSUM(inst);

      if (type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
        transmit_reject(endpoint->instance, endpoint, address, endpoint->ack, SL_CPC_REJECT_CHECKSUM_MISMATCH);
      } else if (type == SLI_CPC_HDLC_FRAME_TYPE_UNNUMBERED) {
        SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_DROPPED(endpoint);
      }

      sli_cpc_free_buffer_handle(rx_handle);
      RELEASE_ENDPOINT(endpoint);

      return;
    }
  }

  if (type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
    SLI_CPC_DEBUG_TRACE_CORE_RXD_VALID_IFRAME(inst);
    receive_iframe(endpoint, rx_handle, control, data_length);
  } else if (type == SLI_CPC_HDLC_FRAME_TYPE_SUPERVISORY) {
    receive_sframe(endpoint, rx_handle, control, data_length);
    SLI_CPC_DEBUG_TRACE_CORE_RXD_VALID_SFRAME(inst);
  } else if (type == SLI_CPC_HDLC_FRAME_TYPE_UNNUMBERED) {
    receive_uframe(endpoint, rx_handle, control, data_length);
    SLI_CPC_DEBUG_TRACE_CORE_RXD_VALID_UFRAME(inst);
  } else {
    transmit_reject(endpoint->instance, endpoint, address, endpoint->ack, SL_CPC_REJECT_ERROR);
    sli_cpc_free_buffer_handle(rx_handle);
    SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_SUPERVISORY_DROPPED(endpoint);
  }
  RELEASE_ENDPOINT(endpoint);
}

/***************************************************************************//**
 * Process received ACK frame
 ******************************************************************************/
static void receive_ack(sl_cpc_endpoint_t * endpoint,
                        uint8_t ack)
{
  sl_cpc_buffer_handle_t *frame;
  uint8_t control_byte;
  uint8_t seq_number;
  uint8_t ack_range_min;
  uint8_t ack_range_max;
  uint8_t frames_count_ack = 0;
  MCU_DECLARE_IRQ_STATE;

  // Protect the re_transmit_queue from being popped in a re_transmit timeout
  MCU_ENTER_ATOMIC();

  // Return if no frame to acknowledge
  if (endpoint->re_transmit_queue == NULL) {
    MCU_EXIT_ATOMIC();
    return;
  }

  // Get the sequence number of the first frame in the re-transmission queue
  frame = SL_SLIST_ENTRY(endpoint->re_transmit_queue, sl_cpc_buffer_handle_t, core_node);

  control_byte = sli_cpc_hdlc_get_control(frame->hdlc_header);
  seq_number = sli_cpc_hdlc_get_seq(control_byte);

  // Calculate the acceptable ACK number range
  ack_range_min = seq_number + 1;
  ack_range_min %= 8;
  ack_range_max = seq_number + endpoint->frames_count_re_transmit_queue;
  ack_range_max %= 8;

  // Check that received ACK number is in range
  if (ack_range_max >= ack_range_min) {
    if (ack < ack_range_min
        || ack > ack_range_max) {
      // Invalid ack number
      MCU_EXIT_ATOMIC();
      return;
    }
  } else {
    if (ack > ack_range_max
        && ack < ack_range_min) {
      // Invalid ack number
      MCU_EXIT_ATOMIC();
      return;
    }
  }

  // Find number of frames acknowledged with ACK number
  if (ack > seq_number) {
    frames_count_ack = ack - seq_number;
  } else {
    frames_count_ack = 8 - seq_number;
    frames_count_ack += ack;
  }

  // Reset re-transmit counter
  endpoint->packet_re_transmit_count = 0u;

  // Calculate re_transmit_timeout
  sli_compute_re_transmit_timeout(endpoint);

  // Stop incomming re-transmit timeout
  (void)sli_cpc_timer_stop_timer(&endpoint->re_transmit_timer);

  // Remove all acknowledged frames in re-transmit queue
  for (uint8_t i = 0; i < frames_count_ack; i++) {
    frame = sli_cpc_pop_core_buffer_handle(&endpoint->re_transmit_queue);
    SLI_CPC_ASSERT(frame != NULL);

    // ref_count is > 0 indicates that the driver is still processing this frame
    if (frame->ref_count == 0) {
      submit_write_completion(frame, SL_STATUS_OK);
    } else {
      frame->on_write_complete_pending = true;
    }

    // Update transmit window
    endpoint->current_tx_window_space++;

    // Update number of frames in re-transmit queue
    endpoint->frames_count_re_transmit_queue--;
  }

  MCU_EXIT_ATOMIC();

  // Put data frames hold in the endpoint in the tx queue if space in transmit window
  while (endpoint->iframe_holding_list != NULL
         && endpoint->current_tx_window_space > 0) {
    MCU_ENTER_ATOMIC();
    frame = sli_cpc_pop_core_buffer_handle(&endpoint->iframe_holding_list);
    MCU_EXIT_ATOMIC();
    queue_for_transmission(endpoint, frame, false);
  }

  SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_ACK(endpoint);
}

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
/***************************************************************************//**
 * Decrypt a buffer
 *
 * This function just wraps the call to the security API.
 ******************************************************************************/
static sl_status_t decrypt_buffer_handle(sl_cpc_endpoint_t *endpoint,
                                         sl_cpc_buffer_handle_t *rx_handle,
                                         uint16_t *payload_len)
{
  size_t output_len;
  sl_status_t ret;
  uint8_t ack;

  ack = sli_cpc_hdlc_get_ack(sli_cpc_hdlc_get_control(rx_handle->hdlc_header));
  sli_cpc_hdlc_set_control_ack(&((uint8_t*)rx_handle->hdlc_header)[SLI_CPC_HDLC_CONTROL_POS], 0);

  ret = sli_cpc_security_decrypt(endpoint,
                                 rx_handle->hdlc_header, SLI_CPC_HDLC_HEADER_SIZE,
                                 rx_handle->data, *payload_len, *payload_len, &output_len);
  if (ret == SL_STATUS_OK) {
    SLI_CPC_ASSERT(output_len < UINT16_MAX);
    *payload_len = output_len;
  }

  sli_cpc_hdlc_set_control_ack(&((uint8_t*)rx_handle->hdlc_header)[SLI_CPC_HDLC_CONTROL_POS], ack);

  return ret;
}

/***************************************************************************//**
 * Decrypt an I-Frame.
 *
 * Takes into consideration the security setup to handle buffer accordingly:
 *  - if security is not used, this does nothing
 *  - if security is being setup (messages are being exchanged between primary
 *    and secondary), only allow messages on the security endpoint. All other
 *    endpoints cannot send I-frames.
 *  - if security is setup, assume payload is encrypted.
 ******************************************************************************/
static sl_status_t decrypt_iframe(sl_cpc_endpoint_t *endpoint,
                                  sl_cpc_buffer_handle_t *rx_handle,
                                  uint16_t *payload_len)
{
  sl_cpc_security_state_t security_state = sl_cpc_security_get_state();

  if (security_state == SL_CPC_SECURITY_STATE_NOT_READY) {
    /*
     * Security is not initialized yet. At this point we don't know if the
     * payload is encrypted or if plaintext payload on non-security endpoints
     * are allowed. Reject.
     */
    return SL_STATUS_NOT_READY;
  } else if (security_state == SL_CPC_SECURITY_STATE_INITIALIZING) {
    /*
     * During initialization, I-frames are only allowed on the security
     * endpoint. The content is still cleartext, so there is no need
     * for decryption just yet.
     */
    if (endpoint->id != SL_CPC_ENDPOINT_SECURITY
        && endpoint->id != SL_CPC_ENDPOINT_SYSTEM) {
      return SL_STATUS_PERMISSION;
    } else {
      return SL_STATUS_OK;
    }
  } else if (security_state == SL_CPC_SECURITY_STATE_DISABLED) {
    /* nothing to do here, just for completeness */
    return SL_STATUS_OK;
  } else if (security_state == SL_CPC_SECURITY_STATE_RESETTING
             || security_state == SL_CPC_SECURITY_STATE_WAITING_ON_TX_COMPLETE) {
    /*
     * During a reset, we might still receive frames encrypted with the previous
     * session key. In that case, decrypt them. If something is received on the
     * security endpoint, it should not be decrypted.
     */
    if (endpoint->id == SL_CPC_ENDPOINT_SECURITY) {
      return SL_STATUS_OK;
    }
  } else if (security_state != SL_CPC_SECURITY_STATE_INITIALIZED) {
    /* we should not be here */
    SLI_CPC_ASSERT(0);
  }

  if (!endpoint->encrypted) {
    /* nothing to do here */
    return SL_STATUS_OK;
  }

  return decrypt_buffer_handle(endpoint, rx_handle, payload_len);
}
#endif

/***************************************************************************//**
 * Process received iframe
 ******************************************************************************/
static void receive_iframe(sl_cpc_endpoint_t *endpoint,
                           sl_cpc_buffer_handle_t *rx_handle,
                           uint8_t control,
                           uint16_t data_length)
{
  uint8_t seq;
#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  sli_cpc_system_cmd_context_t *context = NULL;
  sl_status_t on_poll_status = ~SL_STATUS_OK;
  uint32_t reply_data_length = 0;
  void *reply_data = NULL;
#endif

  SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_DATA_FRAME(endpoint);

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  SLI_CPC_ASSERT(data_length <= endpoint->instance->rx_user_payload_max_length + SLI_SECURITY_TAG_LENGTH_BYTES);
#else
  SLI_CPC_ASSERT(data_length <= endpoint->instance->rx_user_payload_max_length);
#endif

  // If iframe are disabled, drop packet and send reject
  if (endpoint->flags & SL_CPC_ENDPOINT_FLAG_IFRAME_DISABLE) {
    transmit_reject(endpoint->instance, endpoint, endpoint->id, endpoint->ack, SL_CPC_REJECT_ERROR);
    sli_cpc_free_buffer_handle(rx_handle);
    return;
  }

  seq = sli_cpc_hdlc_get_seq(control);

  // If the sequence number doesn't march, reject the frame
  if (seq != endpoint->ack && !is_seq_valid(seq, endpoint->ack)) {
    transmit_reject(endpoint->instance, endpoint, endpoint->id, endpoint->ack, SL_CPC_REJECT_SEQUENCE_MISMATCH);
    sli_cpc_free_buffer_handle(rx_handle);
    return;
  }

  // If the packet was already received, re-send a ACK because the other side missed it the first time
  if (seq != endpoint->ack && is_seq_valid(seq, endpoint->ack)) {
    transmit_ack(endpoint);
    sli_cpc_free_buffer_handle(rx_handle);
    SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_DUPLICATE_DATA_FRAME(endpoint);
    return;
  }

  // data received, Push in Rx Queue and send Ack

  if (sli_cpc_hdlc_is_poll_final(control)) {
  #if (!defined(SLI_CPC_DEVICE_UNDER_TEST))
    // Only system endpoint can use poll/final
    if (endpoint->id != 0) {
      transmit_reject(endpoint->instance, endpoint, endpoint->id, endpoint->ack, SL_CPC_REJECT_ERROR);
      sli_cpc_free_buffer_handle(rx_handle);
      return;
    }
  #endif

  #if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
    sl_status_t ret = decrypt_iframe(endpoint, rx_handle, &data_length);
    if (ret != SL_STATUS_OK) {
      transmit_reject(endpoint->instance, endpoint, endpoint->id, endpoint->ack, SL_CPC_REJECT_SECURITY_ISSUE);
      sli_cpc_free_buffer_handle(rx_handle);
      return;
    }
  #endif

  #if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
    on_poll_status = sli_cpc_system_secondary_on_poll(&endpoint->instance->system_ep,
                                                      SYSTEM_EP_IFRAME,
                                                      rx_handle->data, data_length,
                                                      &reply_data, &reply_data_length, &context);
  #endif

  #if defined(SL_CPC_ON_FINAL_PRESENT)
    if (endpoint->poll_final.on_final != NULL) {
      endpoint->poll_final.on_final(endpoint->id, (void *)SLI_CPC_HDLC_FRAME_TYPE_INFORMATION,
                                    rx_handle->data, data_length);
    }
  #endif
    sli_cpc_free_rx_buffer(endpoint->instance, rx_handle->data);
    rx_handle->data = NULL;
    sli_cpc_free_buffer_handle(rx_handle);
  } else {
    sl_status_t ret;

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
    ret = decrypt_iframe(endpoint, rx_handle, &data_length);
    if (ret != SL_STATUS_OK) {
      transmit_reject(endpoint->instance, endpoint, endpoint->id, endpoint->ack, SL_CPC_REJECT_SECURITY_ISSUE);
      sli_cpc_free_buffer_handle(rx_handle);
      return;
    }
#endif

    ret = sli_cpc_push_back_rx_data_in_receive_queue(endpoint->instance,
                                                     rx_handle,
                                                     &endpoint->iframe_receive_queue,
                                                     data_length);

    if (ret != SL_STATUS_OK) {
      transmit_reject(endpoint->instance, endpoint, endpoint->id, endpoint->ack, SL_CPC_REJECT_OUT_OF_MEMORY);
      sli_cpc_free_buffer_handle(rx_handle);
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
      sli_cpc_security_rollback_decrypt(endpoint);
#endif
      return;
    }

  #if defined(SL_CATALOG_KERNEL_PRESENT)
    if (endpoint->receive_signal != NULL) {
      SLI_CPC_ASSERT(osSemaphoreRelease(endpoint->receive_signal) == osOK);
    }
  #endif

    // Notify the user if a callback is registered
    // We expect the users to not call sl_cpc_read from there
    if (endpoint->on_iframe_data_reception != NULL) {
      endpoint->on_iframe_data_reception(endpoint->id, endpoint->on_iframe_data_reception_arg);
    }
  }
  SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_DATA_FRAME_QUEUED(endpoint);

  // Update endpoint acknowledge number
  endpoint->ack++;
  endpoint->ack %= 8;

  // Send ack
  transmit_ack(endpoint);

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  // Send poll reply (final) if required
  if (on_poll_status == SL_STATUS_OK) {
    SLI_CPC_ASSERT(reply_data != NULL);
    SLI_CPC_ASSERT(reply_data_length > 0);

    sl_status_t status = write(endpoint,
                               reply_data,
                               (uint16_t)reply_data_length,
                               SL_CPC_FLAG_INFORMATION_POLL_FINAL | SL_CPC_FLAG_NO_BLOCK,
                               context);
    if (status != SL_STATUS_OK) {
      if (endpoint->on_iframe_write_completed != NULL) {
        endpoint->on_iframe_write_completed(endpoint->id, reply_data, context, SL_STATUS_TRANSMIT_INCOMPLETE);
      }
    }
  }
#endif
}

#if defined(SLI_CPC_ENABLE_TEST_FEATURES)
/***************************************************************************//**
 * Received an S-Frame rejecting a frame
 ******************************************************************************/
SL_WEAK void sli_cpc_on_frame_rejected(sl_cpc_endpoint_t *endpoint, sl_cpc_reject_reason_t reason)
{
  (void)endpoint;
  (void)reason;
}
#endif

/***************************************************************************//**
 * Process received sframe
 ******************************************************************************/
static void receive_sframe(sl_cpc_endpoint_t *endpoint,
                           sl_cpc_buffer_handle_t *rx_handle,
                           uint8_t control,
                           uint16_t data_length)
{
  SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_SUPERVISORY_FRAME(endpoint);
  // Supervisory packet received
  bool fatal_error = false;
  sl_cpc_endpoint_state_t new_state = endpoint->state;
  uint8_t supervisory_function = sli_cpc_hdlc_get_supervisory_function(control);

  switch (supervisory_function) {
    case SLI_CPC_HDLC_ACK_SUPERVISORY_FUNCTION:
      SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_SUPERVISORY_PROCESSED(endpoint);
      // ACK; already processed previously by receive_ack(), so nothing to do
      break;

    case SLI_CPC_HDLC_REJECT_SUPERVISORY_FUNCTION:
      SL_CPC_JOURNAL_RECORD_WARNING("RXD reject", *((sl_cpc_reject_reason_t *)rx_handle->data));
      SL_CPC_JOURNAL_RECORD_WARNING("On endpoint", endpoint->id);
      SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_SUPERVISORY_PROCESSED(endpoint);
      SLI_CPC_ASSERT(data_length  == SLI_CPC_HDLC_REJECT_PAYLOAD_SIZE);
      switch (*((sl_cpc_reject_reason_t *)rx_handle->data)) {
        case SL_CPC_REJECT_SEQUENCE_MISMATCH:
          SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_REJECT_SEQ_MISMATCH(endpoint);
          break;
        case SL_CPC_REJECT_CHECKSUM_MISMATCH:
          if (endpoint->re_transmit_queue != NULL) {
            re_transmit_frame(endpoint);
          }
          SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_REJECT_CHECSUM_MISMATCH(endpoint);
          break;
        case SL_CPC_REJECT_OUT_OF_MEMORY:
          // Re-transmit mechanism based on the timer will kick in at a later
          // time and attempt to retransmit the packet. Here we need just to
          // keep a trace of it and no additional operations are needed
          SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_REJECT_OUT_OF_MEMORY(endpoint);
          break;
        case SL_CPC_REJECT_SECURITY_ISSUE:
          fatal_error = true;
          new_state = SL_CPC_STATE_ERROR_SECURITY_INCIDENT;
          SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_REJECT_SECURITY_ISSUE(endpoint);
          break;
        case SL_CPC_REJECT_UNREACHABLE_ENDPOINT:
          fatal_error = true;
          new_state = SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE;
          SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_REJECT_DESTINATION_UNREACHABLE(endpoint);
          break;
        case SL_CPC_REJECT_ERROR:
        default:
          fatal_error = true;
          new_state = SL_CPC_STATE_ERROR_FAULT;
          SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_REJECT_FAULT(endpoint);
          break;
      }
#if defined(SLI_CPC_ENABLE_TEST_FEATURES)
      sli_cpc_on_frame_rejected(endpoint, *((sl_cpc_reject_reason_t *)rx_handle->data));
#endif
      break;
    default:
      // Should not reach this case
      SLI_CPC_ASSERT(0);
      break;   // Drop packet by executing the rest of the function
  }

  // Free buffers
  sli_cpc_free_buffer_handle(rx_handle);

  if (fatal_error) {
    // Stop incoming re-transmit timeout
    sli_cpc_timer_stop_timer(&endpoint->re_transmit_timer);
    endpoint->state = new_state;

    // Endpoint in error, should no longer transmit
    notify_error(endpoint);
  }
}

/***************************************************************************//**
 * Process received uframe
 ******************************************************************************/
static void receive_uframe(sl_cpc_endpoint_t *endpoint,
                           sl_cpc_buffer_handle_t *buffer_handle,
                           uint8_t control,
                           uint16_t data_length)
{
  sl_status_t status = SL_STATUS_FAIL;
  uint8_t type;
  void *reply_data = NULL;
  uint32_t reply_data_length = 0;
  sl_status_t reply_write_status = SL_STATUS_FAIL;
#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  sli_cpc_system_cmd_context_t *context = NULL;
#endif

  SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_FRAME(endpoint);

  if (!(endpoint->flags & SL_CPC_ENDPOINT_FLAG_UFRAME_ENABLE)) {
    SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_DROPPED(endpoint);
    sli_cpc_free_buffer_handle(buffer_handle);
    return;
  }

  type = sli_cpc_hdlc_get_unnumbered_type(control);
  if ((type == SLI_CPC_HDLC_CONTROL_UNNUMBERED_TYPE_INFORMATION)
      && !(endpoint->flags & SL_CPC_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE)) {
    if (buffer_handle->data_length > 0) {
      status = sli_cpc_push_back_rx_data_in_receive_queue(endpoint->instance,
                                                          buffer_handle,
                                                          &endpoint->uframe_receive_queue,
                                                          data_length);
      if (status != SL_STATUS_OK) {
        SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_DROPPED(endpoint);
        sli_cpc_free_buffer_handle(buffer_handle);
        return;
      }
    } else {
      SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_DROPPED(endpoint);
      sli_cpc_free_buffer_handle(buffer_handle);
    }

    // Notify the user if a callback is registered
    // We expect the users to not call sl_cpc_read from there
    if (endpoint->on_uframe_data_reception != NULL) {
      endpoint->on_uframe_data_reception(endpoint->id, endpoint->on_uframe_data_reception_arg);
    }

#if defined(SL_CATALOG_KERNEL_PRESENT)
    if (endpoint->receive_signal != NULL) {
      SLI_CPC_ASSERT(osSemaphoreRelease(endpoint->receive_signal) == osOK);
    }
#endif
  } else if (type == SLI_CPC_HDLC_CONTROL_UNNUMBERED_TYPE_POLL_FINAL) {
    #if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
    if (endpoint->id == SL_CPC_ENDPOINT_SYSTEM) {
      status = sli_cpc_system_secondary_on_poll(&endpoint->instance->system_ep,
                                                SYSTEM_EP_UFRAME,
                                                buffer_handle->data, data_length,
                                                &reply_data, &reply_data_length,
                                                &context);
      if (status == SL_STATUS_OK) {
        SLI_CPC_ASSERT(reply_data != NULL);
        SLI_CPC_ASSERT(reply_data_length > 0);

        reply_write_status = write(endpoint,
                                   reply_data,
                                   (uint16_t)reply_data_length,
                                   SL_CPC_FLAG_NO_BLOCK | SL_CPC_FLAG_UNNUMBERED_POLL_FINAL,
                                   context);
      } else {
        reply_data = NULL;
      }
    }
    #endif

    #if defined(SL_CPC_ON_FINAL_PRESENT)
    if (endpoint->poll_final.on_final != NULL) {
      endpoint->poll_final.on_final(endpoint->id, (void *)SLI_CPC_HDLC_FRAME_TYPE_INFORMATION,
                                    buffer_handle->data, data_length);
    }
    #endif

    sli_cpc_free_buffer_handle(buffer_handle);

    if (reply_data != NULL && reply_data_length > 0 && reply_write_status != SL_STATUS_OK) {
      if (endpoint->on_uframe_write_completed != NULL) {
        void *arg = NULL;

        #if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
        arg = context;
        #endif

        endpoint->on_uframe_write_completed(endpoint->id, reply_data, arg, SL_STATUS_TRANSMIT_INCOMPLETE);
      }
    }
  } else if (type == SLI_CPC_HDLC_CONTROL_UNNUMBERED_TYPE_RESET_SEQ) {
    sli_cpc_free_buffer_handle(buffer_handle);
    if (endpoint->id != 0) {
      // Can only reset sequence numbers on the system endpoint, drop the packet
      SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_DROPPED(endpoint);
      return;
    } else {
      // Reset sequence numbers on the system endpoint
      endpoint->seq = 0;
      endpoint->ack = 0;
      // Send an unnumbered acknowledgement
      status = write(endpoint, NULL, 0, SL_CPC_FLAG_UNNUMBERED_ACKNOWLEDGE | SL_CPC_FLAG_NO_BLOCK, NULL);
      if (status != SL_STATUS_OK) {
        SLI_CPC_ASSERT(0);
        SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_DROPPED(endpoint);
        return;
      }
    }
  } else {
    SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_DROPPED(endpoint);
    sli_cpc_free_buffer_handle(buffer_handle);
    return;
  }

  SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_UNNUMBERED_PROCESSED(endpoint);
}

/***************************************************************************//**
 * Transmit ACK frame
 ******************************************************************************/
static sl_status_t transmit_ack(sl_cpc_endpoint_t *endpoint)
{
  sl_status_t status;
  sl_cpc_buffer_handle_t *frame_handle;

  // Get new frame handler
  status = sli_cpc_get_buffer_handle(endpoint->instance, &frame_handle, SL_CPC_TX_SFRAME_BUFFER_HANDLE);
  if (status != SL_STATUS_OK) {
    return status;
  }

  frame_handle->endpoint = endpoint;
  frame_handle->address = endpoint->id;

  // Set ACK number in the supervisory control byte
  frame_handle->control = sli_cpc_hdlc_create_control_supervisory(endpoint->ack, 0);

  // Put frame in Tx Q so that it can be transmitted by CPC Core later
  MCU_ATOMIC_SECTION(sli_cpc_push_back_core_buffer_handle(&endpoint->instance->transmit_queue, frame_handle); );

  SLI_CPC_DEBUG_TRACE_ENDPOINT_TXD_ACK(endpoint);

  return SL_STATUS_OK;
}

#if defined(SLI_CPC_ENABLE_TEST_FEATURES)
/***************************************************************************//**
 * Called on expiration of retransmit timer
 ******************************************************************************/
SL_WEAK void sli_cpc_on_retransmit_timer_expiration(sl_cpc_buffer_handle_t *frame)
{
  (void)frame;
}

/***************************************************************************//**
 * Called on re-transmition of frame
 ******************************************************************************/
SL_WEAK void sli_cpc_on_frame_retransmit(sl_cpc_buffer_handle_t *frame)
{
  (void)frame;
}
#endif

/***************************************************************************//**
 * Re-transmit frame
 ******************************************************************************/
static sl_status_t re_transmit_frame(sl_cpc_endpoint_t* endpoint)
{
  sl_cpc_buffer_handle_t *buffer_handle;
  bool free_hdlc_header = true;
  MCU_DECLARE_IRQ_STATE;

  SLI_CPC_ASSERT(endpoint != NULL);

  MCU_ENTER_ATOMIC();
  buffer_handle = sli_cpc_pop_core_buffer_handle(&endpoint->re_transmit_queue);
  if (buffer_handle == NULL) {
    MCU_EXIT_ATOMIC();

    return SL_STATUS_NOT_AVAILABLE;
  }

  if (buffer_handle->ref_count > 0) {
    // If ref_count is greater than 0, it means that the buffer is still
    // currently owned by the SPI/UART driver. This is possible for instance
    // with the following scenario:
    //  1. buffer handle is transmitted
    //  2. tx complete callback is called, timeout timer is armed
    //  3. host sends a reject CRC error, re_transmit_frame is called
    //     immediately and the packet is put in the transmit_queue
    //  4. process_tx_queue is called, buffer handle is sent to the driver for
    //     transmission, and the core moves the buffer handle from the global
    //     transmit_queue to endpoint's re_transmit_queue for bookkeeping
    //  5. timeout callback, from timer armed during step 2, calls this
    //     function.
    //
    // Step 5 can happen before the tx complete callback of retransmission
    // done at step 4 is called, so it's necessary to check if the buffer
    // handle is still owned by the driver before going forward.

    // if that happened, put back buffer handle at the front of the retransmit queue
    sli_cpc_push_core_buffer_handle(&endpoint->re_transmit_queue, buffer_handle);
    MCU_EXIT_ATOMIC();

    return SL_STATUS_NOT_AVAILABLE;
  }
  // We are about to re-submit the frame for transmission. Make sure the re-transmit
  // timer is killed to avoid retransmitting twice.
  // Ignore return code, since we  get an error when we stop a timer that has elapsed.
  (void)sli_cpc_timer_stop_timer(&endpoint->re_transmit_timer);
  MCU_EXIT_ATOMIC();

#if defined(SLI_CPC_ENABLE_TEST_FEATURES)
  sli_cpc_on_frame_retransmit(buffer_handle);
#endif

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  // If a frame is encrypted, the header is part of the authenticated data
  // (they are in plaintext but signed), meaning that the header must *NOT*
  // change once it has been used to generate a security tag.
  if (buffer_handle->security_tag) {
    free_hdlc_header = false;
  }
#endif

  // Free the header buffer
  if (free_hdlc_header) {
    sli_cpc_free_hdlc_header(endpoint->instance, buffer_handle->hdlc_header);
    buffer_handle->hdlc_header = NULL;
  }

  // Put frame in Tx Q so that it can be transmitted by CPC Core later
  MCU_ENTER_ATOMIC();
  sli_cpc_push_core_buffer_handle(&endpoint->instance->transmit_queue, buffer_handle);

  endpoint->packet_re_transmit_count++;
  endpoint->frames_count_re_transmit_queue--;
  MCU_EXIT_ATOMIC();

  // Signal task/process_action that frame is in Tx Queue
  sli_cpc_signal_event(endpoint->instance, SL_CPC_SIGNAL_TX);

  SLI_CPC_DEBUG_TRACE_ENDPOINT_RETXD_DATA_FRAME(endpoint);
  SLI_CPC_DEBUG_TRACE_CORE_RE_TRANSMIT_FRAME(endpoint->instance);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Transmit REJECT frame
 ******************************************************************************/
static void transmit_reject(sli_cpc_instance_t *inst,
                            sl_cpc_endpoint_t *endpoint,
                            uint8_t address,
                            uint8_t ack,
                            sl_cpc_reject_reason_t reason)
{
  sl_cpc_buffer_handle_t *handle;
  sl_status_t status;

  status = sli_cpc_get_reject_buffer(inst, &handle);
  if (status != SL_STATUS_OK) {
    return;   // Try again when the primary will re-transmit
  }

  handle->endpoint = endpoint;
  handle->address = address;

  // Set the SEQ number and ACK number in the control byte
  handle->control = sli_cpc_hdlc_create_control_supervisory(ack, SLI_CPC_HDLC_REJECT_SUPERVISORY_FUNCTION);

  // Set in reason
  *((uint8_t *)handle->data) = (uint8_t)reason;

  // Compute payload CRC
  handle->fcs = sli_cpc_get_crc_sw(handle->data, 1);

  // Put frame in Tx Q so that it can be transmitted by CPC Core later
  MCU_ATOMIC_SECTION(sli_cpc_push_back_core_buffer_handle(&inst->transmit_queue, handle); );

#if defined(CPC_DEBUG_TRACE)
  if (endpoint != NULL) {
    SL_CPC_JOURNAL_RECORD_WARNING("TXD reject", reason);
    SL_CPC_JOURNAL_RECORD_WARNING("On endpoint", endpoint->id);
    switch (reason) {
      case SL_CPC_REJECT_CHECKSUM_MISMATCH:
        SLI_CPC_DEBUG_TRACE_ENDPOINT_TXD_REJECT_CHECKSUM_MISMATCH(endpoint);
        break;
      case SL_CPC_REJECT_SEQUENCE_MISMATCH:
        SLI_CPC_DEBUG_TRACE_ENDPOINT_TXD_REJECT_SEQ_MISMATCH(endpoint);
        break;
      case SL_CPC_REJECT_OUT_OF_MEMORY:
        SLI_CPC_DEBUG_TRACE_ENDPOINT_TXD_REJECT_OUT_OF_MEMORY(endpoint);
        break;
      case SL_CPC_REJECT_SECURITY_ISSUE:
        SLI_CPC_DEBUG_TRACE_ENDPOINT_TXD_REJECT_SECURITY_ISSUE(endpoint);
        break;
      case SL_CPC_REJECT_UNREACHABLE_ENDPOINT:
        SLI_CPC_DEBUG_TRACE_ENDPOINT_TXD_REJECT_DESTINATION_UNREACHABLE(endpoint);
        break;
      case SL_CPC_REJECT_ERROR:
      default:
        SLI_CPC_DEBUG_TRACE_ENDPOINT_TXD_REJECT_FAULT(endpoint);
        break;
    }
  }
#endif
}

/***************************************************************************//**
 * Transmit the next data frame queued in a endpoint's transmit queue.
 ******************************************************************************/
static sl_status_t process_tx_queue(sli_cpc_instance_t *inst)
{
  sl_status_t status;
  sl_cpc_buffer_handle_t *frame = NULL;
  uint8_t frame_type;
  uint16_t data_length;
  sl_slist_node_t *tx_queue;
  bool free_hdlc_header = true;
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  bool encrypt;
#endif

  MCU_DECLARE_IRQ_STATE;

  // Check if driver is ready or not
  if (inst->driver->ops.is_transmit_ready(inst->driver) == false) {
    return SL_STATUS_TRANSMIT_BUSY;
  }

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  // If there are packets queued up because they were about to be sent but
  // the security sub-system was not ready, send these packets first.
  // Otherwise, process the regular transmit_queue.
  MCU_ATOMIC_LOAD(tx_queue, inst->pending_on_security_ready_queue);
  if (tx_queue != NULL && security_is_ready()) {
    MCU_ATOMIC_SECTION(frame = sli_cpc_pop_core_buffer_handle(&inst->pending_on_security_ready_queue); );
  }
#endif

  // This condition is always true when security is not used.
  // It's just to keep the code a bit more clean.
  if (frame == NULL) {
    // Return if nothing to transmit
    MCU_ATOMIC_LOAD(tx_queue, inst->transmit_queue);
    if (tx_queue == NULL) {
      return SL_STATUS_EMPTY;
    }

    // Get first queued frame for transmission
    MCU_ATOMIC_SECTION(frame = sli_cpc_pop_core_buffer_handle(&inst->transmit_queue); );
  }

  SLI_CPC_ASSERT(frame != NULL);

  // set frame_type as it's used further down in the function
  frame_type = sli_cpc_hdlc_get_frame_type(frame->control);

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  // if frame is already encrypted
  if (frame->security_tag) {
    free_hdlc_header = false;
  }
#endif

  // free_hdlc_header is true when the buffer is allocated in this function
  // if it's false it means it was allocated in a previous run
  if (free_hdlc_header) {
    status = sli_cpc_get_hdlc_header_buffer(inst, &frame->hdlc_header);
    if (status != SL_STATUS_OK) {
      // Retry later on
      MCU_ATOMIC_SECTION(sli_cpc_push_core_buffer_handle(&inst->transmit_queue, frame); );
      return SL_STATUS_NO_MORE_RESOURCE;
    }
  }

  // Form the HDLC header
  data_length = (frame->data_length != 0) ? frame->data_length + 2 : 0;

  // The ACK must be set regardless if this is the first time a buffer
  // goes through this function or not
  if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
    // Update ACK cnt with latest
    LOCK_ENDPOINT(frame->endpoint);
    sli_cpc_hdlc_set_control_ack(&frame->control, frame->endpoint->ack);
    RELEASE_ENDPOINT(frame->endpoint);
  }

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  encrypt = should_encrypt_frame(frame);
  if (encrypt) {
    // If the frame should be encrypted but the security is not ready yet,
    // hold back the frame for later processing.
    if (!security_is_ready()) {
      sli_cpc_free_hdlc_header(inst, frame->hdlc_header);
      frame->hdlc_header = NULL;

      MCU_ATOMIC_SECTION(sli_cpc_push_core_buffer_handle(&inst->pending_on_security_ready_queue, frame); );

      return SL_STATUS_OK;
    }
  }

  if (encrypt || frame->security_tag) {
    data_length += SLI_SECURITY_TAG_LENGTH_BYTES;
  }
#endif

  /*
   * header must be created after it is known if the frame should be encrypted
   * or not, as it has an impact on the total length of the message.
   */
  sli_cpc_hdlc_create_header(frame->hdlc_header, frame->address, data_length, frame->control, true);

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  if (encrypt) {
    uint8_t ack;

    status = sli_cpc_get_security_tag_buffer(inst, &frame->security_tag);
    if (status != SL_STATUS_OK) {
      sli_cpc_free_hdlc_header(inst, frame->hdlc_header);
      frame->hdlc_header = NULL;

      return status;
    }

    /*
     * Set ACK to 0 before authenticating and encrypting data
     */
    ack = sli_cpc_hdlc_get_ack(sli_cpc_hdlc_get_control(frame->hdlc_header));
    sli_cpc_hdlc_set_control_ack(&((uint8_t*)frame->hdlc_header)[SLI_CPC_HDLC_CONTROL_POS], 0);

    status = sli_cpc_security_encrypt(frame->endpoint,
                                      frame->hdlc_header, SLI_CPC_HDLC_HEADER_SIZE,
                                      frame->data, frame->data_length,
                                      frame->security_tag, SLI_SECURITY_TAG_LENGTH_BYTES);

    /*
     * Restore ack
     */
    sli_cpc_hdlc_set_control_ack(&((uint8_t*)frame->hdlc_header)[SLI_CPC_HDLC_CONTROL_POS], ack);

    if (status != SL_STATUS_OK) {
      sli_cpc_free_security_tag_buffer(inst, frame->security_tag);
      frame->security_tag = NULL;

      sli_cpc_free_hdlc_header(inst, frame->hdlc_header);
      frame->hdlc_header = NULL;
      return status;
    }

    /*
     * Recompute FCS as the payload is now encrypted and there is
     * an additional security tag to take into account.
     */
    frame->fcs = sli_cpc_get_crc_sw_with_security(frame->data,
                                                  frame->data_length,
                                                  frame->security_tag,
                                                  SLI_SECURITY_TAG_LENGTH_BYTES);

    free_hdlc_header = false;
  }

#endif
  // Pass frame to driver for transmission
  MCU_ENTER_ATOMIC();
  status = inst->driver->ops.write(inst->driver, frame);
  if (status != SL_STATUS_OK) {
    // Retry later on
    sli_cpc_push_core_buffer_handle(&inst->transmit_queue, frame);

    // In case the driver returns an error we will wait for driver
    // notification before resuming transmission. If the security
    // is used, the HDLC header must not be freed as it's part of
    // the security tag (it's authenticated data), meaning that if
    // the header changes on a subsequent retransmit, the security
    // tag will be invalid and the other end will fail to decrypt.
    if (free_hdlc_header) {
      // free HDLC header
      sli_cpc_free_hdlc_header(inst, frame->hdlc_header);
      frame->hdlc_header = NULL;
    }

    MCU_EXIT_ATOMIC();
    return status;
  }

  if (frame->endpoint == NULL) {
    if (frame_type != SLI_CPC_HDLC_FRAME_TYPE_SUPERVISORY) {
      // Only a REJECT frame on a closed endpoint can have a NULL endpoint.
      MCU_EXIT_ATOMIC();
      SLI_CPC_ASSERT(0);
      return SL_STATUS_FAIL;
    }
  } else {
    SLI_CPC_DEBUG_TRACE_ENDPOINT_FRAME_TRANSMIT_SUBMITTED(frame->endpoint);
  }

  // Put frame in re-transmission queue if it's a I-frame type (with data)
  if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
    sli_cpc_push_back_core_buffer_handle(&frame->endpoint->re_transmit_queue, frame);
    frame->endpoint->frames_count_re_transmit_queue++;

    // Remember when we sent this i-frame in order to calculate round trip time
    // Only do so if this is not a re_transmit
    if (frame->endpoint->packet_re_transmit_count == 0u) {
      frame->endpoint->last_iframe_sent_timestamp = sli_cpc_timer_get_tick_count64();
    }
  }
  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Process endpoint that need to be closed
 ******************************************************************************/
static void process_free(sli_cpc_instance_t *inst)
{
  bool freed;

  if (inst->defer_endpoint_free_list != NULL) {
    sl_slist_node_t *node;

    MCU_ATOMIC_SECTION(node = sl_slist_pop(&inst->defer_endpoint_free_list); );
    do {
      sl_cpc_endpoint_t *endpoint = SL_SLIST_ENTRY(node, sl_cpc_endpoint_t, node_closing);
      LOCK_ENDPOINT(endpoint);
      freed = free_closed_endpoint_if_empty(endpoint);
      MCU_ATOMIC_SECTION(node = sl_slist_pop(&inst->defer_endpoint_free_list); );
      if (!freed) {
        // Endpoint should never be prevented from being freed once it has been
        // queued, as it should remain empty until freed.
        SLI_CPC_ASSERT(0);
        RELEASE_ENDPOINT(endpoint);
      }
    } while (node != NULL);
  }
}

/***************************************************************************//**
 * Add endpoint to closing list
 ******************************************************************************/
static void defer_endpoint_free(sl_cpc_endpoint_t * ep)
{
  MCU_ATOMIC_SECTION(sl_slist_push(&ep->instance->defer_endpoint_free_list, &ep->node_closing); );
  sli_cpc_signal_event(ep->instance, SL_CPC_SIGNAL_CLOSED);
}

static inline bool is_endpoint_tx_done(const sl_cpc_endpoint_t *ep)
{
  return ep->uframe_holding_list == NULL
         && ep->uframe_tx_complete_pending == false
         && ep->iframe_holding_list == NULL
         && ep->re_transmit_queue == NULL
         && ep->current_tx_window_space == ep->configured_tx_window_size;
}

/***************************************************************************//**
 * Checks if an endpoint is empty
 ******************************************************************************/
static inline bool is_endpoint_empty(const sl_cpc_endpoint_t *ep)
{
  return is_endpoint_tx_done(ep)
         && ep->uframe_receive_queue == NULL
         && ep->iframe_receive_queue == NULL
         && ep->on_write_complete_pending_count == 0;
}

/***************************************************************************//**
 * Checks if an endpoint is connected to a remote
 ******************************************************************************/
static inline bool is_endpoint_connection_active(const sl_cpc_endpoint_t *ep)
{
  return ep->state == SL_CPC_STATE_CONNECTED
         || ep->state == SL_CPC_STATE_REMOTE_SHUTDOWN
         || ep->state == SL_CPC_STATE_SHUTDOWN
         || ep->state == SL_CPC_STATE_SHUTTING_DOWN;
}

/***************************************************************************//**
 * Try to free endpoint in closed state (Must be called with the endpoint locked)
 ******************************************************************************/
static bool free_closed_endpoint_if_empty(sl_cpc_endpoint_t *ep)
{
  bool wait = false;
  bool freed = false;

  // This function must not be called if the endpoint is not in the closed state
  if (ep->state != SL_CPC_STATE_CLOSED) {
    SLI_CPC_ASSERT(0);
    return false;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (ep->receive_signal != NULL) {
    // Pending task has not yet been unblocked
    wait = true;
  }
#endif

  if (!is_endpoint_empty(ep)) {
    wait = true;
  }

  // Don't need to check `holding_list` because `receive_ack()` fills the
  // `tx_queue` and reduce `current_tx_window_space`. So it is impossible
  // to have something in the holding list and have `current_tx_window_space`
  // equal to `configured_tx_window_size`

  if (!wait) {
    sli_cpc_instance_t *inst = ep->instance;

    // Stop incoming re-transmit timeout
    (void)sli_cpc_timer_stop_timer(&ep->re_transmit_timer);

    LOCK_ENDPOINTS_LIST(inst);
    ep->state = SL_CPC_STATE_FREED;
    sl_slist_remove(&inst->endpoints, &ep->node);
    RELEASE_ENDPOINTS_LIST(inst);

#if defined(SL_CATALOG_KERNEL_PRESENT)
    osMutexDelete(ep->lock);
#endif

    sli_cpc_free_endpoint(inst, ep);
    SLI_CPC_DEBUG_TRACE_CORE_CLOSE_ENDPOINT(inst);
    freed = true;
  }

  return freed;
}

/***************************************************************************//**
 * Clean queue item
 ******************************************************************************/
static void clean_single_queue_item(sl_cpc_buffer_handle_t *queue_item,
                                    sl_slist_node_t **queue)
{
  sli_cpc_remove_core_buffer_handle(queue, queue_item);
  submit_write_completion(queue_item, SL_STATUS_TRANSMIT_INCOMPLETE);
}

/***************************************************************************//**
 * Function for freeing items in tx queues.
 * Endpoint must be locked before calling.
 ******************************************************************************/
static void clean_tx_queues(sl_cpc_endpoint_t * endpoint)
{
  sl_slist_node_t *node;
  MCU_DECLARE_IRQ_STATE;

  // Enter atomic region for the following reasons:
  // - Re-transmit timer callback is an ISR and will access the re_transmit_queue.
  // - Transmit completed callback is an ISR and will access the transmit_queue
  // - Transmit completed callback is an ISR and will access the holding_list
  MCU_ENTER_ATOMIC();
  node = endpoint->instance->transmit_queue;
  while (node != NULL) {
    sl_cpc_buffer_handle_t *queue_item = SL_SLIST_ENTRY(node, sl_cpc_buffer_handle_t, core_node);
    uint8_t frame_type = sli_cpc_hdlc_get_frame_type(queue_item->control);

    node = node->node;
    if (queue_item->endpoint == endpoint) {
      clean_single_queue_item(queue_item, &endpoint->instance->transmit_queue);
      if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
        endpoint->current_tx_window_space++;
      } else if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_UNNUMBERED) {
        endpoint->uframe_tx_complete_pending = false;
      }
    }
  }

  sli_cpc_timer_stop_timer(&endpoint->re_transmit_timer);

  // Cleanup expired re-transmit queues
  node = endpoint->instance->expired_retransmit_list;
  while (node != NULL) {
    sl_cpc_buffer_handle_t *queue_item = SL_SLIST_ENTRY(node, sl_cpc_buffer_handle_t, core_node);
    uint8_t frame_type = sli_cpc_hdlc_get_frame_type(queue_item->control);

    node = node->node;
    if (queue_item->endpoint == endpoint) {
      clean_single_queue_item(queue_item, &endpoint->instance->expired_retransmit_list);
      if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_INFORMATION) {
        SLI_CPC_ASSERT(endpoint->current_tx_window_space < endpoint->configured_tx_window_size);
        endpoint->current_tx_window_space++;
      } else if (frame_type == SLI_CPC_HDLC_FRAME_TYPE_UNNUMBERED) {
        endpoint->uframe_tx_complete_pending = false;
      }
    }
  }

  // Cleanup re-transmit queues
  node = endpoint->re_transmit_queue;
  while (node != NULL) {
    sl_cpc_buffer_handle_t *queue_item = SL_SLIST_ENTRY(node, sl_cpc_buffer_handle_t, core_node);

    node = node->node;
    sli_cpc_remove_core_buffer_handle(&endpoint->re_transmit_queue, queue_item);
    if (queue_item->ref_count == 0) {
      endpoint->current_tx_window_space++;
      submit_write_completion(queue_item, SL_STATUS_TRANSMIT_INCOMPLETE);
    } else {
      // Frame is still owned by the core. Callback will be called once the
      // the buffer is released.
    }
    endpoint->frames_count_re_transmit_queue--;
  }

  endpoint->packet_re_transmit_count = 0u;

  // Clean I-Frame holding list
  node = endpoint->iframe_holding_list;
  while (node != NULL) {
    sl_cpc_buffer_handle_t *queue_item = SL_SLIST_ENTRY(node, sl_cpc_buffer_handle_t, core_node);

    node = node->node;
    clean_single_queue_item(queue_item, &endpoint->iframe_holding_list);
  }

  // Clean U-Frame holding list
  node = endpoint->uframe_holding_list;
  while (node != NULL) {
    sl_cpc_buffer_handle_t *queue_item = SL_SLIST_ENTRY(node, sl_cpc_buffer_handle_t, core_node);

    node = node->node;
    clean_single_queue_item(queue_item, &endpoint->uframe_holding_list);
  }

  // pending_on_security_ready_queue list is not referenced by an ISR, an atomic section is not necessary
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  node = endpoint->instance->pending_on_security_ready_queue;
  while (node != NULL) {
    sl_cpc_buffer_handle_t *queue_item = SL_SLIST_ENTRY(node, sl_cpc_buffer_handle_t, core_node);

    node = node->node;
    if (queue_item->endpoint == endpoint) {
      clean_single_queue_item(queue_item, &endpoint->instance->pending_on_security_ready_queue);
      // Must be done in atomic context as we clean this flag on the first packet
      // that matches the condition, but there might be other packets queued up
      // for this endpoint
      endpoint->packets_held_for_security = false;
    }
  }
#endif

  MCU_EXIT_ATOMIC();
}

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
/***************************************************************************//**
 * Open a specialized endpoint
 ******************************************************************************/
static sl_status_t open_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                 sli_cpc_instance_t *inst,
                                 uint8_t id,
                                 uint8_t flags,
                                 uint8_t tx_window_size)
{
  sl_cpc_endpoint_state_t state;
  sl_cpc_endpoint_t *ep;
  sl_status_t status;

  if (inst == NULL) {
    inst = sli_cpc_get_instance(id);
    if (inst == NULL) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  if (inst->cpc_core_in_reset) {
    return SL_STATUS_BUSY;
  }

  status = init_endpoint(endpoint_handle, inst, id, flags);
  if (status != SL_STATUS_OK) {
    // Convert init_endpoint return codes to open_endpoint return codes
    switch (status) {
      case SL_STATUS_ALREADY_INITIALIZED:
        LOCK_ENDPOINTS_LIST(inst);

        ep = find_instance_endpoint(inst, id);
        LOCK_ENDPOINT(ep);
        state = ep->state;
        RELEASE_ENDPOINT(ep);

        RELEASE_ENDPOINTS_LIST(inst);

        if (state == SL_CPC_STATE_OPEN) {
          status = SL_STATUS_ALREADY_EXISTS;
        } else {
          status = SL_STATUS_BUSY;
        }
        break;
      case SL_STATUS_NO_MORE_RESOURCE:
        status = SL_STATUS_ALLOCATION_FAILED;
        break;
      default:
        break;
    }
    return status;
  }

  // Set TX windows size
  ep = (sl_cpc_endpoint_t *) endpoint_handle->ep;
  ep->configured_tx_window_size = tx_window_size;
  ep->current_tx_window_space = ep->configured_tx_window_size;

  // this part is specific to the secondary, the primary should use the connect API
  status = sl_cpc_listen_endpoint(endpoint_handle, SL_CPC_FLAG_NO_BLOCK);
  if (status == SL_STATUS_OK || status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  } else {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    osMutexDelete(ep->lock);
#endif
    sli_cpc_free_endpoint(ep->instance, ep);

    SL_CPC_JOURNAL_RECORD_INFO("Failed to start listening for ep", id);
    SL_CPC_JOURNAL_RECORD_INFO("Failed to start listening status", status);

    return status;
  }

  // mark this endpoint as using the legacy API
  ep->flags |= SL_CPC_ENDPOINT_FLAG_LEGACY_API;

  SL_CPC_JOURNAL_RECORD_INFO("Endpoint opened", ep->id);

  SLI_CPC_DEBUG_TRACE_CORE_OPEN_ENDPOINT(ep->instance);
  return status;
}
#endif

static void process_expired_retransmit(void *data)
{
  sl_cpc_buffer_handle_t *buffer_handle = NULL;
  sli_cpc_instance_t *inst = (sli_cpc_instance_t*)data;

  while (inst->expired_retransmit_list != NULL) {
    MCU_ATOMIC_SECTION(buffer_handle = sli_cpc_pop_core_buffer_handle(&inst->expired_retransmit_list); );

    sl_cpc_endpoint_t *endpoint = buffer_handle->endpoint;

    LOCK_ENDPOINT(endpoint);
    if (buffer_handle->on_write_complete_pending) {
      // This scenario is to handle a case where a ack
      // is received while the driver is processing a retransmit.
      // However it could also happen when the retransmit timer is
      // expired and the ack is received before this dispatched
      // function is called.
      //
      // The current implementation does not handle that.
      SL_CPC_JOURNAL_RECORD_DEBUG("Packet was acked while retransmit timer expired", endpoint->id);
      submit_write_completion(buffer_handle, SL_STATUS_OK);
    } else {
      sli_cpc_push_core_buffer_handle(&endpoint->re_transmit_queue, buffer_handle);

      if (endpoint->packet_re_transmit_count >= SLI_CPC_RE_TRANSMIT) {
        #if defined(SL_CATALOG_SECONDARY_PRESENT)
        if (endpoint->id != SL_CPC_ENDPOINT_SYSTEM) {
          endpoint->state = SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE;
          notify_error(endpoint);
        }
        #else
        endpoint->state = SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE;
        notify_error(endpoint);
        #endif
      } else {
        endpoint->re_transmit_timeout *= 2;   // RTO(new) = RTO(before retransmission) *2 )
        // this is explained in Karn's Algorithm
        if (endpoint->re_transmit_timeout > sli_cpc_timer_ms_to_tick(SLI_CPC_MAX_RE_TRANSMIT_TIMEOUT_MS)) {
          endpoint->re_transmit_timeout = sli_cpc_timer_ms_to_tick(SLI_CPC_MAX_RE_TRANSMIT_TIMEOUT_MS);
        }
        re_transmit_frame(endpoint);
      }
    }
    RELEASE_ENDPOINT(buffer_handle->endpoint);
  }
}

/***************************************************************************//**
 * Callback for re-transmit frame
 ******************************************************************************/
static void re_transmit_timeout_callback(sli_cpc_timer_handle_t *handle, void *data)
{
  sl_cpc_buffer_handle_t *popped_buffer_handle;
  sl_cpc_buffer_handle_t *buffer_handle = (sl_cpc_buffer_handle_t *)data;
  sl_cpc_endpoint_t *ep;

  (void)handle;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();

  ep = buffer_handle->endpoint;
  SLI_CPC_ASSERT(ep != NULL);

  SL_CPC_JOURNAL_RECORD_INFO("Retransmit timer expired", buffer_handle->endpoint->id);
  popped_buffer_handle = sli_cpc_pop_core_buffer_handle(&ep->re_transmit_queue);
  SLI_CPC_ASSERT(buffer_handle == popped_buffer_handle);
  sli_cpc_timer_stop_timer(&ep->re_transmit_timer);
#if defined(SLI_CPC_DEVICE_UNDER_TEST)
  sli_cpc_on_retransmit_timer_expiration(buffer_handle);
#endif
  sli_cpc_push_back_core_buffer_handle(&ep->instance->expired_retransmit_list, buffer_handle);
  sli_cpc_dispatcher_push(&ep->instance->retransmit_dispatcher_handle, process_expired_retransmit, ep->instance);   //Use to push status

  MCU_EXIT_ATOMIC();
}

/***************************************************************************//**
 * Notify app about endpoint error. Endpoint must be locked by caller.
 ******************************************************************************/
static void notify_error(sl_cpc_endpoint_t * endpoint)
{
  SL_CPC_JOURNAL_RECORD_INFO("Notify error on endpoint", endpoint->id);
  if (endpoint->on_error != NULL) {
    endpoint->on_error(endpoint->id, endpoint->on_error_arg);
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (endpoint->receive_signal != NULL) {
    SLI_CPC_ASSERT(osSemaphoreRelease(endpoint->receive_signal) == osOK);
  }
#endif
  // Return state is irrelevant, simply want to unlock any blocking API.
  notify_state_change(endpoint);

  // Return all pending write frames to the user
  clean_tx_queues(endpoint);
}

/***************************************************************************//**
 * Check if seq equal ack minus one
 ******************************************************************************/
static bool is_seq_valid(uint8_t seq, uint8_t ack)
{
  bool result = false;

  if (seq == (ack - 1u)) {
    result = true;
  } else if (ack == 0u && seq == 7u) {
    result = true;
  }

  return result;
}

/***************************************************************************//**
 * Increment the endpoint reference counter, returns false if it could not be incremented
 ******************************************************************************/
static bool cpc_enter_api(sl_cpc_endpoint_handle_t * endpoint_handle)
{
  MCU_DECLARE_IRQ_STATE;

  if (endpoint_handle == NULL) {
    return false;
  }

  MCU_ENTER_ATOMIC();

  if (endpoint_handle->ref_count == 0) {
    MCU_EXIT_ATOMIC();
    return false;
  }

  endpoint_handle->ref_count++;

  MCU_EXIT_ATOMIC();
  return true;
}

/***************************************************************************//**
 * Decrement the endpoint reference counter
 ******************************************************************************/
static void cpc_exit_api(sl_cpc_endpoint_handle_t * endpoint_handle)
{
  MCU_ATOMIC_SECTION(endpoint_handle->ref_count--; );
}

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************//**
 * Check if CPC core is ready to operate
 ******************************************************************************/
static sl_status_t cpc_core_is_ready(void)
{
  if (!sli_cpc_system_is_ready()) {
    return SL_STATUS_NOT_READY;
  }

  if (g_instance.secondary_protocol_version_mismatch || g_instance.secondary_application_version_mismatch) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  #if (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)
  if (sli_cpc_is_fwu_in_progress()) {
    return SL_STATUS_BUSY;
  }
  #endif

  return SL_STATUS_OK;
}
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
/***************************************************************************//**
 * Get the system endpoint associated to an endpoint.
 ******************************************************************************/
static sli_cpc_system_endpoint_t* endpoint_get_system_ep(sl_cpc_endpoint_t *ep)
{
  if (ep != NULL && ep->instance != NULL) {
    return &ep->instance->system_ep;
  }

  return NULL;
}
#endif

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
/***************************************************************************//**
 * Callback called when the security state changes.
 ******************************************************************************/
static void on_state_change(sl_cpc_security_state_t old, sl_cpc_security_state_t new_state)
{
  if (old == SL_CPC_SECURITY_STATE_INITIALIZING
      && new_state == SL_CPC_SECURITY_STATE_WAITING_ON_TX_COMPLETE) {
    // During the first initialization phase, the system endpoint is not
    // encrypted. Then, the endpoint must be encrypted for the rest of
    // the session.
    sl_cpc_endpoint_t *system_endpoint;

    LOCK_ENDPOINTS_LIST(&g_instance);

    system_endpoint = find_endpoint(SL_CPC_ENDPOINT_SYSTEM);
    LOCK_ENDPOINT(system_endpoint);
    system_endpoint->encrypted = true;
    RELEASE_ENDPOINT(system_endpoint);

    RELEASE_ENDPOINTS_LIST(&g_instance);
  } else if (old == SL_CPC_SECURITY_STATE_RESETTING
             && new_state == SL_CPC_SECURITY_STATE_WAITING_ON_TX_COMPLETE) {
    sl_cpc_endpoint_t *endpoint;

    LOCK_ENDPOINTS_LIST(&g_instance);
    SL_SLIST_FOR_EACH_ENTRY(g_instance.endpoints, endpoint, sl_cpc_endpoint_t, node) {
      LOCK_ENDPOINT(endpoint);
      endpoint->frame_counter_rx = 0;
      endpoint->frame_counter_tx = 0;
      RELEASE_ENDPOINT(endpoint);
    }
    RELEASE_ENDPOINTS_LIST(&g_instance);
  }
}
#endif

/***************************************************************************//**
 * Function for white testing
 ******************************************************************************/
#if (defined(SLI_CPC_DEVICE_UNDER_TEST))
sl_slist_node_t **sli_cpc_dut_get_endpoints_head(void)
{
  return &g_instance.endpoints->node;   // Give the next node since we want to skip the system endpoint
}
#endif

/***************************************************************************//**
 * In case sl_cpc_primary.slcc or sl_cpc_secondary.slcc is not included (for unity tests
 * for example), the definition of sli_cpc_system_init is missing. This is
 * allows to skip compile error
 ******************************************************************************/
SL_WEAK sl_status_t sli_cpc_system_init(sli_cpc_system_endpoint_t *system_ep)
{
  (void)system_ep;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Configure the maximum payload length. This is the number of bytes that can
 * be sent as a payload. With security enabled this will include the security tag
 ******************************************************************************/
void sli_cpc_set_remote_tx_max_payload_length(sli_cpc_instance_t *inst, uint16_t remote_tx_max_payload_length)
{
  inst->remote_tx_payload_max_length = remote_tx_max_payload_length;
}

/***************************************************************************//**
 * Retrieve the maximum payload length that can be transmitted
 *
 * @param endpoint_handle   handle of the endpoint for which the query is made
 *
 * @return the maximum tx payload length in function of whether the encryption
 *         is enabled on the endpoint or not
 ******************************************************************************/
uint16_t sl_cpc_get_tx_max_payload_length(sl_cpc_endpoint_handle_t *endpoint_handle)
{
  uint16_t max_tx_length = SL_CPC_TX_PAYLOAD_MAX_LENGTH;
  sl_cpc_endpoint_t *ep;

  MCU_ATOMIC_LOAD(ep, (sl_cpc_endpoint_t *)endpoint_handle->ep);

  if (ep == NULL) {
    return max_tx_length;
  }

  if (ep->instance->remote_tx_payload_max_length != 0) {
    max_tx_length = ep->instance->remote_tx_payload_max_length;
  } else {
    return max_tx_length;
  }

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  if (ep->encrypted) {
    max_tx_length -= SLI_CPC_SECURITY_TAG_LENGTH;
  }
#endif

  return max_tx_length;
}
