/***************************************************************************//**
 * @file sl_wisun_coap_collector.c
 * @brief Wi-SUN CoAP Collector
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sl_assert.h"
#include "sl_string.h"
#include "sl_mempool.h"
#include "sl_sleeptimer.h"
#include "sl_wisun_coap.h"
#include "sl_wisun_coap_rhnd.h"
#include "sl_wisun_coap_collector.h"
#include "sli_wisun_meter_collector.h"
#include "sl_wisun_meter_collector_config.h"
#include "sl_wisun_trace_util.h"
#include "sl_memory_manager.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Free CoAP packet and buffer, release CoAP Collector mutex and return with value helper macro function
#define sl_wisun_cleanup_release_mtx_ret_val(__packet, __buff, __retval) \
  do {                                                                   \
    sl_free((__packet));                                                 \
    sl_free((__buff));                                                   \
    _mutex_release();                                                    \
    return (__retval);                                                   \
  } while (0)

/// Release CoAP Collector mutex and return with value helper macro function
#define sl_wisun_release_mtx_and_ret_val(__retval) \
  do {                                             \
    _mutex_release();                              \
    return (__retval);                             \
  } while (0)

/// CoAP Collector measurement resource type ALL
#define SL_WISUN_COAP_COLLECTOR_RESOURCE_RT_ALL     "all"

/// CoAP Collector measurement interface sensor
#define SL_WISUN_COAP_COLLECTOR_RESOURCE_IF_SENSOR  "sensor"

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Mutex acquire
 *****************************************************************************/
__STATIC_INLINE void _mutex_acquire(void);

/**************************************************************************//**
 * @brief Mutex release
 *****************************************************************************/
__STATIC_INLINE void _mutex_release(void);

/**************************************************************************//**
 * @brief CoAP Collector print async meters
 *****************************************************************************/
static void _print_async_meters(void);

/**************************************************************************//**
 * @brief CoAP Collector print registered meters
 *****************************************************************************/
static void _print_registered_meters(void);

/**************************************************************************//**
 * @brief CoAP Collector send request
 * @details CoAP Collector send request
 * @param[in] addr Meter address
 * @param[in] req The request to send
 * @return SL_STATUS_OK On success
 * @return SL_STATUS_FAIL On failure
 *****************************************************************************/
static sl_status_t _send_request(const sockaddr_in6_t * addr,
                                 const sl_wisun_meter_request_t * const req);

/**************************************************************************//**
 * @brief Prepare CoAP request.
 * @details Should be used in thread init part
 * @param[in] req_type Request type
 * @param[out] req Pointer to the request
 * @return SL_STATUS_OK Success
 * @return SL_STATUS_FAIL Failure
 *****************************************************************************/
static sl_status_t _prepare_request(const sl_wisun_request_type_t req_type,
                                    sl_wisun_meter_request_t * const req);

/**************************************************************************//**
 * @brief Get Payload string by LED ID.
 * @details Helper function
 * @param[in] led_id LED ID
 * @return const char* Payload string
 *****************************************************************************/
static const char * _get_led_payload_by_id(const uint8_t led_id);

/**************************************************************************//**
 * @brief CoAP Collector get meter entry by address from a specific mempool
 * @details Helper function
 * @param remote_addr Remote address
 * @param block Pointer to the first block in the mempool
 * @return sl_wisun_meter_entry_t * Meter entry or NULL on error
 *****************************************************************************/
static sl_wisun_meter_entry_t * _get_meter_entry_by_address_from_mempool(const sockaddr_in6_t * const remote_addr,
                                                                         sl_mempool_block_hnd_t * block);

/**************************************************************************//**
 * @brief Process response received from meters
 * @details Callback for CoAP Resource Handler Service. It should be registered.
 * @param[in] src_addr Source address
 * @param[in] req_packet Request packet
 * @return sl_wisun_coap_packet_t * Response packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t * _response_received_cb(const sockaddr_in6_t * const src_addr,
                                                      const sl_wisun_coap_packet_t * const req_packet);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief CoAP Collector mutex
 *****************************************************************************/
static osMutexId_t _mutex;

/**************************************************************************//**
 * @brief CoAP Collector mutex attribute
 *****************************************************************************/
static const osMutexAttr_t _mutex_attr = {
  .name      = "CoAPCollectorMutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0U
};

/**************************************************************************//**
 * @brief CoAP Collector requests to meter
 *****************************************************************************/
static sl_wisun_meter_request_t _async_meas_req     = { 0 };
static sl_wisun_meter_request_t _registration_req   = { 0 };
static sl_wisun_meter_request_t _removal_req        = { 0 };
static sl_wisun_meter_request_t _led_req            = { 0 };

/**************************************************************************//**
 * @brief CoAP Collector request packets
 *****************************************************************************/
static sl_wisun_coap_packet_t _async_meas_packet    = { 0 };
static sl_wisun_coap_packet_t _registration_packet  = { 0 };
static sl_wisun_coap_packet_t _removal_packet       = { 0 };
static sl_wisun_coap_packet_t _led_packet           = { 0 };

/**************************************************************************//**
 * @brief Mempool for storing registered meters
 *****************************************************************************/
static sl_mempool_t _reg_meters_mempool = { 0 };

/**************************************************************************//**
 * @brief Mempool for storing async meters
 *****************************************************************************/
static sl_mempool_t _async_meters_mempool = { 0 };

/**************************************************************************//**
 * @brief Registered meter internal storage
 *****************************************************************************/
static sl_wisun_meter_entry_t _reg_meters[SL_WISUN_COAP_COLLECTOR_MAX_REG_METER] = { 0 };

/**************************************************************************//**
 * @brief Async meter internal storage
 *****************************************************************************/
static sl_wisun_meter_entry_t _async_meters[SL_WISUN_COAP_COLLECTOR_MAX_ASYNC_METER] = { 0 };

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* CoAP Collector component init */
void sl_wisun_coap_collector_init(void)
{
  sl_status_t stat = SL_STATUS_FAIL;
  static sl_wisun_coap_rhnd_resource_t coap_resource = { 0 };

  // Resource init sensor all
  sl_wisun_coap_rhnd_resource_init(&coap_resource);
  coap_resource.data.uri_path = SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH;
  coap_resource.data.resource_type = SL_WISUN_COAP_COLLECTOR_RESOURCE_RT_ALL;
  coap_resource.data.interface = SL_WISUN_COAP_COLLECTOR_RESOURCE_IF_SENSOR;
  coap_resource.auto_response_ext = _response_received_cb;
  coap_resource.redirect_response = NULL;
  coap_resource.discoverable = true;
  EFM_ASSERT(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);

  // Init collector mutex
  _mutex = osMutexNew(&_mutex_attr);
  EFM_ASSERT(_mutex != NULL);

  // Create mempool for registered meters
  stat = sl_mempool_create(&_reg_meters_mempool,
                           SL_WISUN_COAP_COLLECTOR_MAX_REG_METER,
                           sizeof(sl_wisun_meter_entry_t),
                           _reg_meters,
                           sizeof(_reg_meters));
  EFM_ASSERT(stat == SL_STATUS_OK);

  // Create mempool for async meters
  stat = sl_mempool_create(&_async_meters_mempool,
                           SL_WISUN_COAP_COLLECTOR_MAX_ASYNC_METER,
                           sizeof(sl_wisun_meter_entry_t),
                           _async_meters,
                           sizeof(_async_meters));
  EFM_ASSERT(stat == SL_STATUS_OK);

  // Prepare requests
  stat = _prepare_request(SL_WISUN_MC_REQ_ASYNC, &_async_meas_req);
  EFM_ASSERT(stat == SL_STATUS_OK);
  stat = _prepare_request(SL_WISUN_MC_REQ_REGISTER, &_registration_req);
  EFM_ASSERT(stat == SL_STATUS_OK);
  stat = _prepare_request(SL_WISUN_MC_REQ_REMOVE, &_removal_req);
  EFM_ASSERT(stat == SL_STATUS_OK);
}

/* Prepare led toggle request */
sl_status_t sl_wisun_coap_collector_prepare_led_toggle_request(const uint8_t led_id)
{
  sl_wisun_coap_packet_t *packet  = NULL;
  const char *payload             = NULL;
  uint16_t payload_len            = 0;
  int16_t builder_result          = -1;

  // mutex lock
  _mutex_acquire();

  // Free request buffers
  sl_free(_led_req.buff);
  _led_req.buff = NULL;

  // Allocate CoAP packet buffer
  packet = (sl_wisun_coap_packet_t *)sl_malloc(sizeof(sl_wisun_coap_packet_t));
  if (packet == NULL) {
    printf("[Coap packet buffer cannot be allocated]\n");
    sl_wisun_cleanup_release_mtx_ret_val(packet, _led_req.buff, SL_STATUS_FAIL);
  }

  // Calculate payload
  payload = _get_led_payload_by_id(led_id);
  payload_len = (uint16_t)(sl_strnlen((char *)payload, SL_WISUN_METER_LED_TOGGLE_PAYLOAD_STR_MAX_LEN) + 1);

  // Fill CoAP packet fields
  memset(packet, 0, sizeof(sl_wisun_coap_packet_t));
  packet->uri_path_ptr     = (uint8_t *) SL_WISUN_COAP_METER_COLLECTOR_LED_TOGGLE_URI_PATH;
  packet->uri_path_len     = sl_strlen(SL_WISUN_COAP_METER_COLLECTOR_LED_TOGGLE_URI_PATH);
  packet->msg_code         = COAP_MSG_CODE_REQUEST_PUT;
  packet->content_format   = COAP_CT_TEXT_PLAIN;
  packet->payload_ptr      = (uint8_t *) payload;
  packet->payload_len      = payload_len;
  packet->options_list_ptr = NULL;
  packet->msg_id           = SL_WISUN_COAP_METER_COLLECTOR_DEFAULT_MESSAGE_ID;

  // Allocate request buffer for meters
  _led_req.length = sl_wisun_coap_builder_calc_size(packet);
  _led_req.buff = sl_malloc(_led_req.length);
  if (_led_req.buff == NULL) {
    printf("[Coap message buffer cannot be allocated]\n");
    sl_wisun_cleanup_release_mtx_ret_val(packet, _led_req.buff, SL_STATUS_FAIL);
  }

  // Build CoAP request packet
  builder_result = sl_wisun_coap_builder(_led_req.buff, packet);
  if (builder_result < 0) {
    printf("[Coap builder error: %s]\n", builder_result == -1 ? "Message Header structure" : "NULL ptr arg");
    sl_wisun_cleanup_release_mtx_ret_val(packet, _led_req.buff, SL_STATUS_FAIL);
  }

  _led_packet = *packet;

  // Only allocated CoAP packet should be freed
  sl_free(packet);

  // mutex unlock
  _mutex_release();

  return SL_STATUS_OK;
}

/* Send led toggle request */
sl_status_t sl_wisun_coap_collector_send_led_toggle_request(const sockaddr_in6_t * meter_addr)
{
  sl_status_t res = _send_request(meter_addr, &_led_req);

  if (res == SL_STATUS_OK) {
    sl_wisun_coap_print_packet(&_led_packet, false);
  }

  return res;
}

/* Register meter */
sl_status_t sl_wisun_coap_collector_register_meter(const sockaddr_in6_t * meter_addr)
{
  const sl_mempool_block_hnd_t *block     = NULL;
  sl_wisun_meter_entry_t *tmp_meter_entry = NULL;
  sl_status_t res                         = SL_STATUS_FAIL;
  const char *ip_addr                     = NULL;

  // mutex lock
  _mutex_acquire();

  if (meter_addr == NULL) {
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  }

  // Check if meter is already registered
  block = _reg_meters_mempool.blocks;
  while (block != NULL) {
    tmp_meter_entry = (sl_wisun_meter_entry_t *) block->start_addr;
    if (sli_wisun_compare_addresses(&tmp_meter_entry->addr, meter_addr)) {
      printf("[Meter is already registered]\n");
      sl_wisun_release_mtx_and_ret_val(SL_STATUS_ALREADY_EXISTS);
    }
    block = block->next;
  }

  tmp_meter_entry = sl_mempool_alloc(&_reg_meters_mempool);
  if (tmp_meter_entry == NULL) {
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  }

  tmp_meter_entry->type = SL_WISUN_MC_REQ_REGISTER;
  tmp_meter_entry->resp_recv_timestamp = 0U;
  tmp_meter_entry->req_sent_timestamp = sl_sleeptimer_get_tick_count();
  memcpy(&tmp_meter_entry->addr, meter_addr, sizeof(sockaddr_in6_t));

  // Send a registration request to the meter
  res = _send_request(&tmp_meter_entry->addr, &_registration_req);
  if (res != SL_STATUS_OK) {
    ip_addr = app_wisun_trace_util_get_ip_str(&meter_addr->sin6_addr);
    printf("[CoAP Collector cannot send registration request to the meter: %s]\n", ip_addr);
    app_wisun_trace_util_destroy_ip_str(ip_addr);
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  } else {
    sl_wisun_coap_print_packet(&_registration_packet, false);
  }

  // mutex unlock
  _mutex_release();

  return SL_STATUS_OK;
}

/* Remove meter */
sl_status_t sl_wisun_coap_collector_remove_meter(const sockaddr_in6_t * meter_addr)
{
  const sl_mempool_block_hnd_t *block           = NULL;
  const sl_wisun_meter_entry_t *tmp_meter_entry = NULL;
  sl_status_t res                               = SL_STATUS_FAIL;
  const char *ip_addr                           = NULL;
  bool found                                    = false;

  // mutex lock
  _mutex_acquire();
  if (meter_addr == NULL) {
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  }

  // Check if registered meters contain the given removable meter
  block = _reg_meters_mempool.blocks;
  while (block != NULL) {
    tmp_meter_entry = (sl_wisun_meter_entry_t *) block->start_addr;
    if (sli_wisun_compare_addresses(&tmp_meter_entry->addr, meter_addr)) {
      found = true;
      break;
    }
    block = block->next;
  }

  if (!found) {
    printf("[Attempt to remove non registered meter]\n");
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  }

  // Send a remove request to the meter
  res = _send_request(&tmp_meter_entry->addr, &_removal_req);
  if (res != SL_STATUS_OK) {
    ip_addr = app_wisun_trace_util_get_ip_str(&meter_addr->sin6_addr);
    printf("[CoAP Collector cannot send removal request to the meter: %s]\n", ip_addr);
    app_wisun_trace_util_destroy_ip_str(ip_addr);
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  } else {
    sl_wisun_coap_print_packet(&_removal_packet, false);
  }

  sl_mempool_free(&_reg_meters_mempool, tmp_meter_entry);

  // mutex unlock
  _mutex_release();

  return SL_STATUS_OK;
}

/* Send async request */
sl_status_t sl_wisun_coap_collector_send_async_request(const sockaddr_in6_t * meter_addr)
{
  const sl_mempool_block_hnd_t *block     = NULL;
  sl_wisun_meter_entry_t *tmp_meter_entry = NULL;
  sl_status_t res                         = SL_STATUS_FAIL;
  const char *ip_addr                     = NULL;

  if (meter_addr == NULL) {
    return SL_STATUS_FAIL;
  }

  // mutex lock
  _mutex_acquire();

  // Check if async request has already been sent to the given meter
  block = _async_meters_mempool.blocks;
  while (block != NULL) {
    tmp_meter_entry = (sl_wisun_meter_entry_t *) block->start_addr;
    if (sli_wisun_compare_addresses(&tmp_meter_entry->addr, meter_addr)) {
      printf("[Async request has been already sent]\n");
      sl_wisun_release_mtx_and_ret_val(SL_STATUS_OK);
    }
    block = block->next;
  }

  tmp_meter_entry = sl_mempool_alloc(&_async_meters_mempool);
  if (tmp_meter_entry == NULL) {
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  }

  tmp_meter_entry->req_sent_timestamp = sl_sleeptimer_get_tick_count();
  tmp_meter_entry->resp_recv_timestamp = 0U;
  memcpy(&tmp_meter_entry->addr, meter_addr, sizeof(sockaddr_in6_t));

  // Send an async measurement request to the meter
  res = _send_request(&tmp_meter_entry->addr, &_async_meas_req);
  if (res != SL_STATUS_OK) {
    ip_addr = app_wisun_trace_util_get_ip_str(&meter_addr->sin6_addr);
    printf("[CoAP Collector cannot send async measurement request to the meter: %s]\n", ip_addr);
    app_wisun_trace_util_destroy_ip_str(ip_addr);
    sl_wisun_release_mtx_and_ret_val(SL_STATUS_FAIL);
  } else {
    sl_wisun_coap_print_packet(&_async_meas_packet, false);
  }

  // mutex unlock
  _mutex_release();

  return SL_STATUS_OK;
}

/* Print registered and async meters */
void sl_wisun_coap_collector_print_meters(void)
{
  _print_async_meters();
  _print_registered_meters();
}

/* Remove broken meters */
void sl_wisun_coap_collector_remove_broken_meters(sl_wisun_request_type_t req)
{
  const sl_mempool_block_hnd_t *block           = NULL;
  const sl_wisun_meter_entry_t *tmp_meter_entry = NULL;
  uint32_t timestamp                            = 0U;
  const char *ip_addr                           = NULL;
  uint32_t elapsed_ms                           = 0U;

  // mutex lock
  _mutex_acquire();

  switch (req) {
    case SL_WISUN_MC_REQ_ASYNC:
      block = _async_meters_mempool.blocks;
      break;

    case SL_WISUN_MC_REQ_REGISTER:
      block = _reg_meters_mempool.blocks;
      break;

    default:
      _mutex_release();
      return;
  }

  while (block != NULL) {
    tmp_meter_entry = (sl_wisun_meter_entry_t *) block->start_addr;

    timestamp = sl_sleeptimer_get_tick_count();
    elapsed_ms = sl_sleeptimer_tick_to_ms(timestamp - tmp_meter_entry->req_sent_timestamp);

    block = block->next;
    if ((tmp_meter_entry->resp_recv_timestamp) || (elapsed_ms <= SL_WISUN_COAP_COLLECTOR_REQUEST_TIMEOUT)) {
      continue;
    }

    ip_addr = app_wisun_trace_util_get_ip_str(&tmp_meter_entry->addr.sin6_addr);
    printf("[%s not responded for the %s request in time, therefore has been removed]\n",
           ip_addr,
           tmp_meter_entry->type == SL_WISUN_MC_REQ_ASYNC ? "async" : "registration");
    app_wisun_trace_util_destroy_ip_str(ip_addr);

    switch (req) {
      case SL_WISUN_MC_REQ_ASYNC:
        sl_mempool_free(&_async_meters_mempool, tmp_meter_entry);
        break;

      case SL_WISUN_MC_REQ_REGISTER:
        sl_mempool_free(&_reg_meters_mempool, tmp_meter_entry);
        break;

      default:
        // Nothing to do
        break;
    }
  }

  // mutex unlock
  _mutex_release();
}

/* CoAP Resource Handler weak function implementation */
void sl_wisun_coap_rhnd_service_resp_received_ext_hnd(const sockaddr_in6_t * const src_addr,
                                                      sl_wisun_coap_packet_t * req_packet)
{
  sl_wisun_meter_entry_t *meter = NULL;
  uint32_t response_time_ms = 0U;
  (void) req_packet;
#if !SL_WISUN_COAP_RESOURCE_HND_VERBOSE_MODE_ENABLE
  char *payload_str = NULL;
#endif

#if !SL_WISUN_COAP_RESOURCE_HND_VERBOSE_MODE_ENABLE
  printf("[CoAP-RHND-Service: Response packet received]\n");
#endif

  // mutex lock
  _mutex_acquire();

  // Retrieve meter address (based on async or register request)
  meter = _get_meter_entry_by_address_from_mempool(src_addr, _async_meters_mempool.blocks);

  if (meter == NULL) {
    meter = _get_meter_entry_by_address_from_mempool(src_addr, _reg_meters_mempool.blocks);
  }

  // mutex unlock
  _mutex_release();

#if !SL_WISUN_COAP_RESOURCE_HND_VERBOSE_MODE_ENABLE
  // Print packet payload
  payload_str = sl_wisun_coap_get_payload_str(req_packet);
  if (payload_str != NULL) {
    printf("%s\n", payload_str);
    sl_wisun_coap_destroy_payload_str(payload_str);
  }
#endif

  // Remove async meter from mempool
  if (meter != NULL) {
    meter->resp_recv_timestamp = sl_sleeptimer_get_tick_count();
    if (meter->type == SL_WISUN_MC_REQ_ASYNC) {
      response_time_ms = sl_sleeptimer_tick_to_ms(meter->resp_recv_timestamp - meter->req_sent_timestamp);
      printf("[Response time: %ldms]\n", response_time_ms);
      sl_mempool_free(&_async_meters_mempool, meter);
    }
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/* Mutex acquire */
__STATIC_INLINE void _mutex_acquire(void)
{
  if (osKernelGetState() == osKernelRunning) {
    EFM_ASSERT(osMutexAcquire(_mutex, osWaitForever) == osOK);
  }
}

/* Mutex release */
__STATIC_INLINE void _mutex_release(void)
{
  if (osKernelGetState() == osKernelRunning) {
    EFM_ASSERT(osMutexRelease(_mutex) == osOK);
  }
}

/* Print async meters */
static void _print_async_meters(void)
{
  uint32_t timestamp                            = 0U;
  const sl_mempool_block_hnd_t *block           = NULL;
  const sl_wisun_meter_entry_t *tmp_meter_entry = NULL;
  uint32_t remaining_ms                         = 0U;
  uint32_t elapsed_ms                           = 0U;
  const char *ip_addr                           = NULL;

  // mutex lock
  _mutex_acquire();

  printf("[Async meters:]\n");
  timestamp = sl_sleeptimer_get_tick_count();

  block = _async_meters_mempool.blocks;
  while (block != NULL) {
    tmp_meter_entry = (sl_wisun_meter_entry_t *)block->start_addr;
    elapsed_ms = sl_sleeptimer_tick_to_ms(timestamp - tmp_meter_entry->req_sent_timestamp);

    if (elapsed_ms > SL_WISUN_COAP_COLLECTOR_REQUEST_TIMEOUT) {
      remaining_ms = 0U;
    } else {
      remaining_ms = SL_WISUN_COAP_COLLECTOR_REQUEST_TIMEOUT - elapsed_ms;
    }

    ip_addr = app_wisun_trace_util_get_ip_str(&tmp_meter_entry->addr.sin6_addr);
    printf("[%s - time to live: %lu ms]\n", ip_addr, remaining_ms);
    app_wisun_trace_util_destroy_ip_str(ip_addr);

    block = block->next;
  }

  // mutex unlock
  _mutex_release();
}

/* Print registered meters */
static void _print_registered_meters(void)
{
  uint32_t timestamp                            = 0U;
  const sl_mempool_block_hnd_t *block           = NULL;
  const sl_wisun_meter_entry_t *tmp_meter_entry = NULL;
  uint32_t elapsed_ms                           = 0U;
  const char *ip_addr                           = NULL;

  // mutex lock
  _mutex_acquire();

  printf("[Registered meters:]\n");
  timestamp = sl_sleeptimer_get_tick_count();

  block = _reg_meters_mempool.blocks;
  while (block != NULL) {
    tmp_meter_entry = (sl_wisun_meter_entry_t *)block->start_addr;
    elapsed_ms = sl_sleeptimer_tick_to_ms(timestamp - tmp_meter_entry->req_sent_timestamp);
    ip_addr = app_wisun_trace_util_get_ip_str(&tmp_meter_entry->addr.sin6_addr);
    printf("[%s - registered %lu seconds ago]\n", ip_addr, elapsed_ms / 1000);
    app_wisun_trace_util_destroy_ip_str(ip_addr);

    block = block->next;
  }

  // mutex unlock
  _mutex_release();
}

/* Send request */
static sl_status_t _send_request(const sockaddr_in6_t * addr,
                                 const sl_wisun_meter_request_t * const req)
{
  // Create socket
  int32_t sockid = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (sockid == SOCKET_RETVAL_ERROR) {
    return SL_STATUS_FAIL;
  }

  if (sendto(sockid,
             req->buff,
             req->length,
             0,
             (const struct sockaddr *)addr,
             (socklen_t)sizeof(*addr)) == SOCKET_RETVAL_ERROR) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/* Prepare requests */
static sl_status_t _prepare_request(const sl_wisun_request_type_t req_type,
                                    sl_wisun_meter_request_t * const req)
{
  sl_wisun_coap_packet_t *packet  = NULL;
  int16_t builder_result          = -1;

  if ((req_type >= SL_WISUN_MC_REQ_UNKNOWN) || (req == NULL)) {
    return SL_STATUS_FAIL;
  }

  // mutex lock
  _mutex_acquire();

  // Free request buffers
  sl_free(req->buff);
  req->buff = NULL;

  // Allocate CoAP packet buffer
  packet = (sl_wisun_coap_packet_t *)sl_malloc(sizeof(sl_wisun_coap_packet_t));
  if (packet == NULL) {
    printf("[Coap packet buffer cannot be allocated]\n");
    sl_wisun_cleanup_release_mtx_ret_val(packet, req->buff, SL_STATUS_FAIL);
  }

  // Fill CoAP packet fields
  memset(packet, 0, sizeof(sl_wisun_coap_packet_t));
  packet->uri_path_ptr      = (uint8_t *) SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH;
  packet->uri_path_len      = sl_strlen(SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH);
  packet->content_format    = COAP_CT_TEXT_PLAIN;
  packet->payload_ptr       = NULL;
  packet->payload_len       = 0;
  packet->options_list_ptr  = NULL;
  packet->msg_id            = SL_WISUN_COAP_METER_COLLECTOR_DEFAULT_MESSAGE_ID;

  // Fill CoAP packet payload
  switch (req_type) {
    case SL_WISUN_MC_REQ_ASYNC:
      packet->msg_code    = COAP_MSG_CODE_REQUEST_GET;
      // Setting payload is not needed
      break;

    case SL_WISUN_MC_REQ_REGISTER:
      packet->msg_code    = COAP_MSG_CODE_REQUEST_PUT;
      packet->payload_ptr = (uint8_t *) SL_WISUN_METER_REQUEST_TYPE_STR_REGISTER;
      packet->payload_len = (uint16_t)sl_strlen(SL_WISUN_METER_REQUEST_TYPE_STR_REGISTER);
      break;

    case SL_WISUN_MC_REQ_REMOVE:
      packet->msg_code    = COAP_MSG_CODE_REQUEST_PUT;
      packet->payload_ptr = (uint8_t *) SL_WISUN_METER_REQUEST_TYPE_STR_REMOVE;
      packet->payload_len = (uint16_t)sl_strlen(SL_WISUN_METER_REQUEST_TYPE_STR_REMOVE);
      break;

    default:
      // Nothing to do
      break;
  }

  // Allocate request buffer for meters
  req->length = sl_wisun_coap_builder_calc_size(packet);
  req->buff = sl_malloc(req->length);
  if (req->buff == NULL) {
    printf("[Coap message buffer cannot be allocated]\n");
    sl_wisun_cleanup_release_mtx_ret_val(packet, req->buff, SL_STATUS_FAIL);
  }

  // Build CoAP request packet
  builder_result = sl_wisun_coap_builder(req->buff, packet);
  if (builder_result < 0) {
    printf("[Coap builder error: %s]\n", builder_result == -1 ? "Message Header structure" : "NULL ptr arg");
    sl_wisun_cleanup_release_mtx_ret_val(packet, req->buff, SL_STATUS_FAIL);
  }

  // Set requests based on type
  switch (req_type) {
    case SL_WISUN_MC_REQ_ASYNC:
      memcpy(&_async_meas_req, req, sizeof(_async_meas_req));
      _async_meas_packet = *packet;
      break;

    case SL_WISUN_MC_REQ_REGISTER:
      memcpy(&_registration_req, req, sizeof(_registration_req));
      _registration_packet = *packet;
      break;

    case SL_WISUN_MC_REQ_REMOVE:
      memcpy(&_removal_req, req, sizeof(_removal_req));
      _removal_packet = *packet;
      break;

    default:
      // Nothing to do
      break;
  }

  // Only allocated CoAP packet should be freed
  sl_free(packet);

  // mutex unlock
  _mutex_release();

  return SL_STATUS_OK;
}

/* Get LED payload by ID */
static const char * _get_led_payload_by_id(const uint8_t led_id)
{
  switch (led_id) {
    case 0:
      return SL_WISUN_METER_LED0_TOGGLE_PAYLOAD_STR;
    case 1:
      return SL_WISUN_METER_LED1_TOGGLE_PAYLOAD_STR;
    default:
      return "Unknown";
  }
}

/* Get meter entry by address from mempool */
static sl_wisun_meter_entry_t * _get_meter_entry_by_address_from_mempool(const sockaddr_in6_t * const remote_addr,
                                                                         sl_mempool_block_hnd_t * block)
{
  sl_wisun_meter_entry_t *tmp_meter_entry = NULL;

  if (remote_addr == NULL) {
    return NULL;
  }

  while (block != NULL) {
    tmp_meter_entry = (sl_wisun_meter_entry_t *) block->start_addr;
    if (sli_wisun_compare_addresses(&tmp_meter_entry->addr, remote_addr)) {
      return tmp_meter_entry;
    }
    block = block->next;
  }

  return NULL;
}

/* Resource handler auto response callback */
static sl_wisun_coap_packet_t * _response_received_cb(const sockaddr_in6_t * const src_addr,
                                                      const sl_wisun_coap_packet_t * const req_packet)
{
  const sl_wisun_meter_entry_t *meter = NULL;
#if !SL_WISUN_COAP_RESOURCE_HND_VERBOSE_MODE_ENABLE
  char *payload_str = NULL;
#endif

  // Check request packet
  if ((req_packet->msg_code != COAP_MSG_CODE_REQUEST_PUT) && (req_packet->msg_code != COAP_MSG_CODE_REQUEST_POST)) {
    printf("[Unknown request message received!]\n");
    return NULL;
  }

  // Retrieve registered meter
  _mutex_acquire();
  meter = _get_meter_entry_by_address_from_mempool(src_addr, _reg_meters_mempool.blocks);
  _mutex_release();

  if (meter == NULL) {
    printf("[Unknown remote message received!]\n");
    return NULL;
  }

#if !SL_WISUN_COAP_RESOURCE_HND_VERBOSE_MODE_ENABLE
  // Print packet payload
  payload_str = sl_wisun_coap_get_payload_str(req_packet);
  if (payload_str != NULL) {
    printf("%s\n", payload_str);
    sl_wisun_coap_destroy_payload_str(payload_str);
  }
#endif

  return NULL;
}
