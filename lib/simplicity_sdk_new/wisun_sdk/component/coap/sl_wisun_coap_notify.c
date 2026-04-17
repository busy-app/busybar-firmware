/***************************************************************************//**
 * @file sl_wisun_coap_notify.c
 * @brief Wi-SUN CoAP notification service
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include <string.h>

#include "sl_assert.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_mempool.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_coap_notify.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Max ID string length
#define SL_WISUN_COAP_NOTIFY_ID_STR_MAX_LEN            128U

/// Default schedule time
#define SL_WISUN_COAP_NOTIFY_DEFAULT_SCHEDULE_TIME_MS  10000UL

/// Default URI Path
#define SL_WISUN_COAP_NOTIFY_DEFAULT_URI_PATH          "coap/notification"

/// Default URI Path length
#define SL_WISUN_COAP_NOTIFY_DEFAULT_URI_PATH_LEN      (sizeof(SL_WISUN_COAP_NOTIFY_DEFAULT_URI_PATH) - 1)

/// Default notification payload
#define SL_WISUN_COAP_NOTIFY_DEFAULT_PAYLOAD           "notification"

/// Default notification payload length
#define SL_WISUN_COAP_NOTIFY_DEFAULT_PAYLOAD_LEN       (sizeof(SL_WISUN_COAP_NOTIFY_DEFAULT_PAYLOAD) - 1)

/// Default CoAP packet ID
#define SL_WISUN_COAP_NOTIFY_DEFAULT_PACKET_ID         4000U

/// Timequanta for sending notification
#define SL_WISUN_COAP_NOTIFY_TIMEQUANTA                1000U

/// Unlock the CoAP notification mutex and return with value
#define _coap_notify_mutex_release_and_return_val(__val) \
  do {                                                   \
    _coap_notify_mtx_release();                          \
    return (__val);                                      \
  } while (0)

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/// Notify schedule parameters
typedef struct sl_wisun_coap_notify_schedule {
  /// Time quanta for synchronous notification
  const uint32_t timequanta;
  /// Tick event enable flag
  bool tick_evt_enable;
  /// Tick event ID
  osEventFlagsId_t evt;
  /// Tick event mask
  const uint32_t evt_msk;
} sl_wisun_coap_notify_schedule_t;
#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Notification mutex lock and return
 * @details Helper function
 *****************************************************************************/
__STATIC_INLINE void _coap_notify_mtx_acquire(void);

/**************************************************************************//**
 * @brief Notification mutex unlock and return
 * @details Helper function
 *****************************************************************************/
__STATIC_INLINE void _coap_notify_mtx_release(void);

/**************************************************************************//**
 * @brief Get notify instance
 * @details Helper function
 * @param id ID string
 *****************************************************************************/
static sl_wisun_coap_notify_t * _get_notify(const char * const id);

/**************************************************************************//**
 * @brief Default condition check callback
 * @details Default callback implementation, it returns always true
 * @param notify Notify instance (not used)
 * @return bool Always true
 *****************************************************************************/
static bool _default_condition_cb(const sl_wisun_coap_notify_t * notify);

/**************************************************************************//**
 * @brief Default notify packet handler callback
 * @details Default callback implementation, it returns a default CoAP packet
 * @param notify Notify instance (not used)
 * @return sl_wisun_coap_packet_t* Default CoAP packet ptr
 *****************************************************************************/
static sl_wisun_coap_packet_t * _default_hnd(const sl_wisun_coap_notify_t * notify);

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/**************************************************************************//**
 * @brief Thread function of Notification Service
 * @details Thread function
 * @param[in] args Arguments (not used)
 *****************************************************************************/
static void _notify_thr_fnc(void * args);
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Notification mutex attribute
static const osMutexAttr_t _notify_mtx_attr = {
  .name      = "WisunCoapNotifyMutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0U
};

/// Notification service mutex
static osMutexId_t _notify_mtx = NULL;

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
/// Thread ID
static osThreadId_t _notify_thr = NULL;

/// Notificaion thread attribute
static const osThreadAttr_t _notify_thr_attr = {
  .name        = "CoAP-Notification-Service",
  .attr_bits   = osThreadDetached,
  .cb_mem      = NULL,
  .cb_size     = 0U,
  .stack_mem   = NULL,
  .stack_size  = app_stack_size_word_to_byte(SL_WISUN_COAP_NOTIFY_STACK_SIZE_WORD),
  .priority    = osPriorityNormal7,
  .tz_module   = 0U
};

/// Schedule parameters
static sl_wisun_coap_notify_schedule_t _schd = {
  .timequanta = 1UL,
  .tick_evt_enable = false,
  .evt_msk = 0x00000001U,
  .evt = NULL
};

/// Notify event attribute
static const osEventFlagsAttr_t _notify_evt_attr = {
  .name = "CoAP-Notify-Tick-Event",
  .attr_bits = 0U,
  .cb_mem = NULL,
  .cb_size = 0U
};

#endif

/// Internal notification buffer
static sl_wisun_coap_notify_t _notify_table[SL_WISUN_COAP_NOTIFY_MAX_NOTIFICATION] = { 0U };

/// Notification storage mempool
static sl_mempool_t _notifications = { 0U };

/// Socket communication buffer to receive/send
static uint8_t _notify_sock_buff[SL_WISUN_COAP_NOTIFY_SOCK_BUFF_SIZE] = { 0 };

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_wisun_coap_notify_init(void)
{
  sl_status_t stat = SL_STATUS_FAIL;
  // init mutex
  _notify_mtx = osMutexNew(&_notify_mtx_attr);
  EFM_ASSERT(_notify_mtx != NULL);

  // create mempool for notifications
  stat = sl_mempool_create(&_notifications,
                           SL_WISUN_COAP_NOTIFY_MAX_NOTIFICATION,
                           sizeof(sl_wisun_coap_notify_t),
                           _notify_table,
                           sizeof(_notify_table));
  EFM_ASSERT(stat == SL_STATUS_OK);

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE

  // init schedule parameters
  _schd.tick_evt_enable = false;
  _schd.evt = osEventFlagsNew(&_notify_evt_attr);
  EFM_ASSERT(_schd.evt != NULL);

  // init thread
  _notify_thr = osThreadNew(_notify_thr_fnc, NULL, &_notify_thr_attr);
  EFM_ASSERT(_notify_thr != NULL);
#endif
}

sl_status_t sl_wisun_coap_notify_init_default_inst(sl_wisun_coap_notify_t * const notify, const char *id)
{
  if (id == NULL) {
    return SL_STATUS_FAIL;
  }

  notify->id = id;

  // convert default IP String to byte address
  if (inet_pton(AF_INET6,
                SL_WISUN_COAP_NOTIFY_DEFAULT_REMOTE_ADDR,
                &notify->remote_addr.sin6_addr) != 1) {
    return SL_STATUS_FAIL;
  }
  notify->remote_addr.sin6_port = htons(SL_WISUN_COAP_NOTIFY_DEFAULT_REMOTE_PORT);

  notify->schedule_time_ms = SL_WISUN_COAP_NOTIFY_DEFAULT_SCHEDULE_TIME_MS;
  notify->tick_ms = 0UL;
  notify->condition_cb = _default_condition_cb;
  notify->hnd_cb = _default_hnd;

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_coap_notify_add(const sl_wisun_coap_notify_t * const src_notify)
{
  sl_wisun_coap_notify_t *notify = NULL;
  sl_mempool_block_hnd_t *slot = _notifications.blocks;

  _coap_notify_mtx_acquire();

  // Check if resource already exists
  while (slot != NULL) {
    notify = (sl_wisun_coap_notify_t *)slot->start_addr;
    if (!strncmp(notify->id,
                 src_notify->id,
                 SL_WISUN_COAP_NOTIFY_ID_STR_MAX_LEN)) {
      _coap_notify_mutex_release_and_return_val(SL_STATUS_FAIL);
    }
    slot = slot->next;
  }

  notify = sl_mempool_alloc(&_notifications);

  // resource cannot be allocated
  if (notify == NULL) {
    _coap_notify_mutex_release_and_return_val(SL_STATUS_FAIL);
  }

  // Copy resource data
  memcpy(notify, src_notify, sizeof(sl_wisun_coap_notify_t));

  _coap_notify_mtx_release();
  return SL_STATUS_OK;
}

const sl_wisun_coap_notify_t *sl_wisun_coap_notify_get_by_id(const char* id)
{
  const sl_wisun_coap_notify_t *notify = NULL;
  _coap_notify_mtx_acquire();
  notify = _get_notify(id);
  _coap_notify_mtx_release();

  return notify;
}

sl_status_t sl_wisun_coap_notify_remove_by_id(const char* id)
{
  const sl_wisun_coap_notify_t *notify = NULL;
  _coap_notify_mtx_acquire();

  notify = _get_notify(id);

  if (notify == NULL) {
    _coap_notify_mutex_release_and_return_val(SL_STATUS_FAIL);
  }

  sl_mempool_free(&_notifications, notify);

  _coap_notify_mtx_release();
  return SL_STATUS_OK;
}

void sl_wisun_coap_notify_print_notifications(void)
{
  const sl_wisun_coap_notify_t *notify = NULL;
  sl_mempool_block_hnd_t *slot         = _notifications.blocks;

  _coap_notify_mtx_acquire();

  while (slot != NULL) {
    notify = (sl_wisun_coap_notify_t *)slot->start_addr;
    printf("%s (%lu/%lu)\n", notify->id, notify->tick_ms, notify->schedule_time_ms);
    slot = slot->next;
  }

  _coap_notify_mtx_release();
}

sl_status_t sl_wisun_coap_notify_send_notification(const sl_wisun_coap_notify_t * const notify)
{
  sl_wisun_coap_packet_t *pkt = NULL;
  uint16_t payload_size = 0U;
  int32_t sockid = SOCKET_INVALID_ID;
  bool is_confirmable = false;
  int32_t r = SOCKET_RETVAL_ERROR;
  bool finished = false;
  static uint8_t _ack_buff[128] = { 0U };
  uint8_t max_retransmit = 1U;
  sl_wisun_coap_packet_t *ack_pkt = NULL;

  // Check if the notification handler callback is valid
  if (notify->hnd_cb == NULL) {
    return SL_STATUS_FAIL;
  }

  // Prepare CoAP packet
  pkt = notify->hnd_cb(notify);
  if (pkt == NULL) {
    return SL_STATUS_FAIL;
  }
  // Check if the packet is confirmable
  if (pkt->msg_type == COAP_MSG_TYPE_CONFIRMABLE) {
    is_confirmable = true;
    max_retransmit = SL_WISUN_COAP_NOTIFY_MAX_RETRANSMIT;
  }

  // Check condition to send
  if (notify->condition_cb != NULL
      && !notify->condition_cb(notify)) {
    return SL_STATUS_OK;
  }

  // Create non-blocking socket
  sockid = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
  if (sockid == SOCKET_RETVAL_ERROR) {
    return SL_STATUS_FAIL;
  }

  // Calculate CoAP payload size
  payload_size = sl_wisun_coap_builder_calc_size(pkt);
  if (!payload_size || payload_size > SL_WISUN_COAP_NOTIFY_SOCK_BUFF_SIZE) {
    return SL_STATUS_FAIL;
  }

  // Acquire mutex to protect common buffer
  _coap_notify_mtx_acquire();

  // Build CoAP payload packet
  if (sl_wisun_coap_builder(_notify_sock_buff, pkt) < 0L) {
    _coap_notify_mutex_release_and_return_val(SL_STATUS_FAIL);
  }

  // Send packet to the remote host
  for (uint8_t retry_cnt = 0U; retry_cnt < max_retransmit; ++retry_cnt) {
    if (sendto(sockid,
               _notify_sock_buff,
               (uint32_t)payload_size, 0L,
               (const struct sockaddr *) &notify->remote_addr,
               sizeof(sockaddr_in6_t)) == SOCKET_RETVAL_ERROR) {
      _coap_notify_mutex_release_and_return_val(SL_STATUS_FAIL);
    }
    for (uint8_t ack_timeout = 0U; ack_timeout < max_retransmit - 1U; ++ack_timeout) {
      osDelay(SL_WISUN_COAP_NOTIFY_TIMEQUANTA);
      memset(_ack_buff, 0U, sizeof(_ack_buff));
      // Read the socket buffer in case of confirmable message
      r = recvfrom(sockid,
                   _ack_buff,
                   sizeof(_ack_buff) - 1,
                   0, NULL, NULL);
      if (r > 0L) {
        // Parse the packet. If it is an ACK, break the loop.
        ack_pkt = sl_wisun_coap_parser((uint16_t) r, _ack_buff);
        // Check packet
        if (ack_pkt == NULL) {
          continue;
        }
        if (ack_pkt->msg_type == COAP_MSG_TYPE_ACKNOWLEDGEMENT
            && ack_pkt->msg_id == pkt->msg_id) {
          // ACK received, stop retransmission
          sl_wisun_coap_destroy_packet(ack_pkt);
          finished = true;
          break;
        }
      }
    }
    if (finished) {
      break;
    }
  }
  // Destroy packet
  sl_wisun_coap_destroy_packet(pkt);

  // Release mutex
  _coap_notify_mtx_release();

  // Close socket
  close(sockid);

  if (is_confirmable && !finished) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE
SL_WEAK void sl_wisun_coap_notify_error_hnd(const sl_wisun_coap_notify_t * const notify)
{
  printf("[CoAP Notify Error: %s (%lu/%lu)]\n", notify->id, notify->tick_ms, notify->schedule_time_ms);
}
#endif
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

__STATIC_INLINE void _coap_notify_mtx_acquire(void)
{
  if (osKernelGetState() == osKernelRunning) {
    EFM_ASSERT(osMutexAcquire(_notify_mtx, osWaitForever) == osOK);
  }
}

__STATIC_INLINE void _coap_notify_mtx_release(void)
{
  if (osKernelGetState() == osKernelRunning) {
    EFM_ASSERT(osMutexRelease(_notify_mtx) == osOK);
  }
}

static sl_wisun_coap_notify_t * _get_notify(const char * const id)
{
  sl_wisun_coap_notify_t *notify = NULL;
  sl_mempool_block_hnd_t *slot   = _notifications.blocks;

  while (slot != NULL) {
    notify = (sl_wisun_coap_notify_t *)slot->start_addr;
    if (!strncmp(id, notify->id, SL_WISUN_COAP_NOTIFY_ID_STR_MAX_LEN)) {
      return notify;
    }
    slot = slot->next;
  }

  return NULL;
}

#if SL_WISUN_COAP_NOTIFY_SERVICE_ENABLE

static void _notify_thr_fnc(void * args)
{
  sl_wisun_coap_notify_t *notify = NULL;

  (void) args;

  SL_COAP_SERVICE_LOOP() {
    #if !defined(SL_CATALOG_WISUN_BR_STACK_PRESENT)
    // wait for network connected state
    sl_wisun_app_core_util_wait_for_connection();
    #endif

    if (_schd.tick_evt_enable) {
      (void) osEventFlagsWait(_schd.evt, _schd.evt_msk, osFlagsWaitAny, osWaitForever);
    }

    _coap_notify_mtx_acquire();

    for (sl_mempool_block_hnd_t *slot = _notifications.blocks; slot != NULL; slot = slot->next) {
      // get notification instance
      notify = (sl_wisun_coap_notify_t *)slot->start_addr;

      // check schedule
      if (!_schd.tick_evt_enable && (notify->schedule_time_ms != notify->tick_ms)) {
        ++notify->tick_ms;
        continue;
      }

      // sendto
      if (sl_wisun_coap_notify_send_notification(notify) != SL_STATUS_OK) {
        // error handling
        sl_wisun_coap_notify_error_hnd(notify);
      }

      // reset time counter
      notify->tick_ms = 0UL;
    }

    _coap_notify_mtx_release();

    if (!_schd.tick_evt_enable) {
      // 1 ms time quanta if event trigger is not enabled
      osDelay(_schd.timequanta);
    }
  }
}

void sl_wisun_coap_notify_tick_evt_enable(const bool enable)
{
  _coap_notify_mtx_acquire();
  _schd.tick_evt_enable = enable;
  // If it's disabled, unblock event flags wait in the notification thread.
  if (!enable) {
    osEventFlagsSet(_schd.evt, _schd.evt_msk);
  }
  _coap_notify_mtx_release();
}

bool sl_wisun_coap_notify_tick_evt_is_enabled(void)
{
  bool ret = false;
  _coap_notify_mtx_acquire();
  ret = _schd.tick_evt_enable;
  _coap_notify_mtx_release();
  return ret;
}

sl_status_t sl_wisun_coap_notify_tick(void)
{
  uint32_t stat = 0UL;
  stat =  osEventFlagsSet(_schd.evt, _schd.evt_msk);
  // if high bit is set, it means error
  return (stat & (1UL << 31UL)) ? SL_STATUS_FAIL : SL_STATUS_OK;
}
#endif

static bool _default_condition_cb(const sl_wisun_coap_notify_t * notify)
{
  (void) notify;
  return true;
}

static sl_wisun_coap_packet_t * _default_hnd(const sl_wisun_coap_notify_t * notify)
{
  static sl_wisun_coap_packet_t pkt = {
    .msg_code = COAP_MSG_CODE_REQUEST_PUT,
    .msg_id = SL_WISUN_COAP_NOTIFY_DEFAULT_PACKET_ID,
    .msg_type = COAP_MSG_TYPE_NON_CONFIRMABLE,
    .content_format = COAP_CT_TEXT_PLAIN,
    .uri_path_ptr = (uint8_t *) SL_WISUN_COAP_NOTIFY_DEFAULT_URI_PATH,
    .uri_path_len = SL_WISUN_COAP_NOTIFY_DEFAULT_URI_PATH_LEN,
    .payload_ptr = (uint8_t *) SL_WISUN_COAP_NOTIFY_DEFAULT_PAYLOAD,
    .payload_len = SL_WISUN_COAP_NOTIFY_DEFAULT_PAYLOAD_LEN,
    .token_ptr = NULL,
    .token_len = 0U,
    .options_list_ptr = NULL
  };

  (void) notify;

  return &pkt;
}
