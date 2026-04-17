/***************************************************************************/ /**
 * @file
 * @brief CPC NVM3 implementation.
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
#include "ecode.h"
#include "nvm3_default.h"
#include "nvm3_generic.h"
#include "sl_common.h"
#include "sl_cpc_config.h"
#include "sl_sleeptimer.h"
#include "sl_status.h"
#include "sl_sleeptimer.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_cpc.h"
#include "sli_cpc.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_cpc_nvm3_kernel_config.h"
#endif
#include "sl_cpc_nvm3.h"
#include "sl_cpc_nvm3_config.h"

#include "sl_assert.h"
#include "sl_enum.h"
#include "em_core.h"
#include "string.h"
#include "nvm3.h"
#include "nvm3_default_config.h"

/*******************************************************************************
 **************************   DEFINES   ****************************************
 ******************************************************************************/

#ifndef CPC_NVM3_OS_DELAY_MS
#define CPC_NVM3_OS_DELAY_MS 1
#endif

#define CPC_NVM3_API_VERSION_MAJOR 2
#define CPC_NVM3_API_VERSION_MINOR 0
#define CPC_NVM3_API_VERSION_PATCH 0

#ifndef CPC_NVM3_OS_DELAY_MS
#define CPC_NVM3_OS_DELAY_MS 1
#endif

#if (SL_CPC_NVM3_TX_PAYLOAD_MAX_LENGTH < 64) \
  || (SL_CPC_NVM3_TX_PAYLOAD_MAX_LENGTH > SL_CPC_TX_PAYLOAD_MAX_LENGTH)
#error Invalid configuration, SL_CPC_NVM3_TX_PAYLOAD_MAX_LENGTH is invalid. It must be configured between 64 and SL_CPC_TX_PAYLOAD_MAX_LENGTH.
#endif

/*******************************************************************************
 **************************   ENUMS   ******************************************
 ******************************************************************************/

SL_ENUM_GENERIC(status_is_response_type_t, uint8_t) {
  SL_STATUS, ECODE
};

/// @brief Enumeration representing endpoint state types.
SL_ENUM_GENERIC(endpoint_state_t, uint8_t) {
  ENDPOINT_STATE_INIT,
  ENDPOINT_STATE_RESTART,
  ENDPOINT_STATE_WRITE_COMPLETED,
  ENDPOINT_STATE_WRITE_PENDING,
  ENDPOINT_STATE_REPLY_READY,
};

/// @brief Enumeration representing command types
SL_ENUM_GENERIC(cmd_t, uint8_t) {
  CMD_GET_VERSION = 0x00,
  CMD_VERSION_IS = 0x01,
  CMD_STATUS_IS = 0x02,
  CMD_NOOP = 0x03,
  CMD_PROP_VALUE_GET = 0x04,
  CMD_PROP_VALUE_IS = 0x05,
  CMD_WRITE_DATA = 0x06,
  CMD_WRITE_DATA_COMPLETED = 0x07,
  CMD_READ_DATA = 0x08,
  CMD_READ_DATA_IS  = 0x09,
  CMD_GET_OBJECT_INFO = 0x0A,
  CMD_OBJECT_INFO_IS = 0x0B,
  CMD_READ_COUNTER = 0x0C,
  CMD_COUNTER_VALUE_IS = 0x0D,
  CMD_WRITE_COUNTER = 0x0E,
  CMD_INCREMENT_COUNTER = 0x0F,
  CMD_DELETE_OBJECT = 0x10,
  CMD_ENUMERATE_OBJECTS = 0x11,
  CMD_ENUMERATE_OBJECTS_IS = 0x12,
  CMD_OBJECT_COUNT = 0x13,
  CMD_OBJECT_COUNT_IS = 0x14,
  CMD_UNSUPPORTED = 255,
};

/// @brief Enumeration representing property types
SL_ENUM_GENERIC(property_type_t, uint8_t) {
  PROP_MAX_OBJECT_SIZE = 0x01,
  PROP_MAX_WRITE_SIZE = 0x02,
  PROP_UNKNOWN = 0xFF,
};

/*******************************************************************************
 **************************   STRUCTS   ****************************************
 ******************************************************************************/
/// @brief Struct representing CPC NVM3 properties.
struct sli_cpc_nvm3_properties {
  uint16_t max_object_size;
  uint16_t max_write_size;
};

/// @brief Struct representing a header.
typedef __PACKED_STRUCT {
  cmd_t cmd;
  uint16_t len;
  uint32_t unique_id;
  uint8_t transaction_id;
} header_t;

/// @brief Struct representing a get version request from the host.
typedef __PACKED_STRUCT {
  header_t header;
} get_version_t;

/// @brief Struct representing a write data request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  nvm3_ObjectKey_t key;
  uint16_t offset;
  uint8_t last_frag;
  uint8_t data[];
} write_data_command_t;

/// @brief Struct representing a write counter request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  nvm3_ObjectKey_t key;
  uint32_t data;
} write_counter_command_t;

/// @brief Struct representing a increment counter request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  nvm3_ObjectKey_t key;
} increment_counter_command_t;

/// @brief Struct representing a object info request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  nvm3_ObjectKey_t key;
} object_info_command_t;

/// @brief Struct representing a property get request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  property_type_t property_type;
} property_get_command_t;

/// @brief Struct representing delete object request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  nvm3_ObjectKey_t key;
} delete_object_command_t;

/// @brief Struct representing a property value response.
typedef __PACKED_STRUCT {
  header_t header;
  property_type_t property_type;
  uint8_t value[];
} property_value_is_t;

/// @brief Struct representing a read data request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  nvm3_ObjectKey_t key;
  uint16_t max_read_size;
} read_data_command_t;

/// @brief Struct representing a enumerate objects request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  uint16_t max_object_count;
} enumerate_objects_command_t;

/// @brief Struct representing a read counter request from the host.
typedef __PACKED_STRUCT {
  header_t header;
  nvm3_ObjectKey_t key;
} read_counter_command_t;

/// @brief Struct representing a read data response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  uint8_t last_frag;
  uint8_t data[];
} read_data_is_t;

/// @brief Struct representing a get object count response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  uint16_t object_count;
} object_count_is_t;

/// @brief Struct representing a enumerate objects response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  uint8_t last_frag;
  uint8_t data[];
} enumerate_object_is_t;

/// @brief Struct representing a read counter response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  uint32_t data;
} read_counter_is_t;

/// @brief Struct representing a get object info response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  uint8_t object_type;
  uint16_t object_size;
} object_info_is_t;

/// @brief Struct representing a version response from the secondary.
typedef __PACKED_STRUCT {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
} version_t;

/// @brief Struct representing a version response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  version_t version;
} version_is_t;

/// @brief Struct representing an unsupported cmd response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  cmd_t cmd_unsupported;
} cmd_unsupported_is_t;

/// @brief Struct representing a reply buffer.
typedef struct {
  uint8_t buffer[SL_CPC_NVM3_TX_PAYLOAD_MAX_LENGTH];
  uint16_t buffer_len;
} tx_buffer_t;

/// @brief Struct representing a status response from the secondary.
typedef __PACKED_STRUCT {
  header_t header;
  status_is_response_type_t response_type;
  uint32_t value;
} status_is_t;

#define SL_CPC_NVM3_MAX_READ_FRAGMENT_SIZE (SL_CPC_NVM3_TX_PAYLOAD_MAX_LENGTH - sizeof(read_data_is_t))
#define SL_CPC_NVM3_MAX_ENUMERATED_OBJECTS_PER_TRANSACTION (5)
/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
#define THREAD_STACK_SIZE   (SL_CPC_NVM3_TASK_STACK_SIZE * sizeof(void *)) & 0xFFFFFFF8u
__ALIGNED(4) static uint8_t thread_cb[osThreadCbSize];
__ALIGNED(8) static uint8_t thread_stack[THREAD_STACK_SIZE];
__ALIGNED(4) static uint8_t semaphore_cb[osSemaphoreCbSize];

// Thread handle
static osThreadId_t thread_id;

// Endpoint state change signal
static osSemaphoreId_t state_change_signal;
#endif

// Endpoint restart flag
static bool restart_flag = false;

// Endpoint state
static endpoint_state_t state = ENDPOINT_STATE_INIT;

// Endpoint handle
static sl_cpc_endpoint_handle_t ep_nvm3;

// Write buffer
static uint8_t write_buffer[SL_CPC_NVM3_MAXIMUM_WRITE_SIZE];

// CPC NVM3 properties
static const struct sli_cpc_nvm3_properties sli_cpc_nvm3_properties_values = {
  .max_object_size = NVM3_MAX_OBJECT_SIZE,
  .max_write_size = SL_CPC_NVM3_MAXIMUM_WRITE_SIZE,
};

typedef struct {
  size_t object_len;
  nvm3_ObjectKey_t key;
  uint16_t offset;
} read_data_command_context_t;

typedef struct {
  uint16_t offset;
  uint16_t total_write_size;
  uint32_t lock;
} write_data_command_context_t;

static bool enumerate_objects_command_incomplete = false;
static bool read_command_incomplete = false;
static uint16_t rx_buffer_length;
static void *rx_buffer;
static read_data_command_context_t read_data_command_context;
static write_data_command_context_t write_data_command_context;

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void on_write_completed(sl_cpc_user_endpoint_id_t endpoint_id,
                               void *buffer,
                               void *arg,
                               sl_status_t status);

static void on_error(sl_cpc_user_endpoint_id_t endpoint_id,
                     void *buffer,
                     void *arg,
                     sl_status_t status);

static sl_status_t init_endpoint(void);

static void restart_endpoint(void);

static endpoint_state_t wait_state_change(void);

static void signal_state_change(endpoint_state_t new_state);

static void read_packet(tx_buffer_t *tx_buffer);

static void parse_packet(void *rx_buffer,
                         uint16_t rx_buffer_length,
                         tx_buffer_t *tx_buffer);

static void transmit_packet(tx_buffer_t *tx_buffer);

static void on_get_version(void *rx_buffer,
                           tx_buffer_t *tx_buffer);

static void on_unsupported_cmd(uint8_t unsupported_cmd,
                               tx_buffer_t*tx_buffer);

/***************************************************************************//**
 * Write completed
 ******************************************************************************/
static void on_write_completed(sl_cpc_user_endpoint_id_t endpoint_id,
                               void *buffer, void *arg, sl_status_t status)
{
  (void) buffer;
  (void) arg;
  (void) status;

  EFM_ASSERT(endpoint_id == SL_CPC_ENDPOINT_NVM3);
  signal_state_change(ENDPOINT_STATE_WRITE_COMPLETED);
}

/***************************************************************************//**
 * Error callback
 ******************************************************************************/
static void on_error(sl_cpc_user_endpoint_id_t endpoint_id,
                     void *buffer, void *arg, sl_status_t status)
{
  (void) buffer;
  (void) arg;
  (void) status;

  EFM_ASSERT(endpoint_id == SL_CPC_ENDPOINT_NVM3);
  restart_flag = true;
  signal_state_change(ENDPOINT_STATE_RESTART);
}

/***************************************************************************//**
 * Get a status_is reply packet ready.
 ******************************************************************************/
static void status_is(uint32_t status, tx_buffer_t *tx_buffer, status_is_response_type_t response_type)
{
  status_is_t *tx_packet = (status_is_t *) tx_buffer->buffer;

  if (response_type == SL_STATUS) {
    tx_packet->response_type = 0;
  } else {
    tx_packet->response_type = 1;
  }

  tx_packet->header.cmd = CMD_STATUS_IS;
  tx_packet->value = status;

  tx_packet->header.len = (sizeof(*tx_packet) - (sizeof(header_t)));
  tx_buffer->buffer_len = sizeof(*tx_packet);
}

/***************************************************************************//**
 * Initialize endpoint
 ******************************************************************************/
static sl_status_t init_endpoint(void)
{
  sl_status_t status;

  status = sli_cpc_open_service_endpoint(&ep_nvm3, SL_CPC_ENDPOINT_NVM3, 0, 1);
  if (status != SL_STATUS_OK) {
    EFM_ASSERT(false);
    return status;
  }

  status = sl_cpc_set_endpoint_option(&ep_nvm3, SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED,
                                      (void *)on_write_completed);
  if (status != SL_STATUS_OK) {
    sl_cpc_close_endpoint(&ep_nvm3);
    EFM_ASSERT(false);
    return status;
  }

  status = sl_cpc_set_endpoint_option(&ep_nvm3,
                                      SL_CPC_ENDPOINT_ON_ERROR,
                                      (void *)on_error);
  if (status != SL_STATUS_OK) {
    sl_cpc_close_endpoint(&ep_nvm3);
    EFM_ASSERT(false);
    return status;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Wait for state transition.
 ******************************************************************************/
static endpoint_state_t wait_state_change(void)
{
#if defined(SL_CATALOG_KERNEL_PRESENT)
  EFM_ASSERT(osSemaphoreAcquire(state_change_signal, osWaitForever) == osOK);
#endif // SL_CATALOG_KERNEL_PRESENT
  return state;
}

/***************************************************************************//**
 * Signal state transition.
 ******************************************************************************/
static void signal_state_change(endpoint_state_t new_state)
{
  state = new_state;
  #if defined(SL_CATALOG_KERNEL_PRESENT)
  osSemaphoreRelease(state_change_signal);
  #endif // SL_CATALOG_KERNEL_PRESENT
}

/***************************************************************************//**
 * Restart endpoint.
 ******************************************************************************/
static void restart_endpoint(void)
{
  sl_cpc_endpoint_state_t ep_state;

  ep_state = sl_cpc_get_endpoint_state(&ep_nvm3);

  switch (ep_state) {
    case SL_CPC_STATE_CLOSED:
    case SL_CPC_STATE_CLOSING:
      break;
    case SL_CPC_STATE_OPEN:
    case SL_CPC_STATE_CONNECTED:
    case SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE:
    case SL_CPC_STATE_ERROR_SECURITY_INCIDENT:
    case SL_CPC_STATE_ERROR_FAULT:
      sl_cpc_close_endpoint(&ep_nvm3);
      break;
    case SL_CPC_STATE_FREED:
      restart_flag = false;
      if (init_endpoint() == SL_STATUS_OK) {
        signal_state_change(ENDPOINT_STATE_INIT);
      } else {
        restart_flag = true;
        EFM_ASSERT(false);
      }
      break;
    default:
      EFM_ASSERT(false);
      break;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  osDelay(((uint64_t)osKernelGetTickFreq() * CPC_NVM3_OS_DELAY_MS) / 1000);
#endif // SL_CATALOG_KERNEL_PRESENT
}

/***************************************************************************//**
 * Parse CMD_GET_VERSION packet and get a reply packet ready.
 ******************************************************************************/
static void on_get_version(void *rx_buffer,
                           tx_buffer_t *tx_buffer)
{
  get_version_t *rx_packet = (get_version_t *)rx_buffer;
  EFM_ASSERT(rx_packet->header.len == 0);

  version_is_t *tx_packet = (version_is_t *)tx_buffer;

  tx_packet->header.cmd = CMD_VERSION_IS;
  tx_packet->header.len = sizeof(tx_packet->version);
  tx_packet->version.major = CPC_NVM3_API_VERSION_MAJOR;
  tx_packet->version.minor = CPC_NVM3_API_VERSION_MINOR;
  tx_packet->version.patch = CPC_NVM3_API_VERSION_PATCH;
}

/***************************************************************************//**
 * Parse CMD_WRITE_COUNTER packet and get a reply packet ready.
 ******************************************************************************/
static void on_write_counter_command(void *rx_buffer,
                                     tx_buffer_t *tx_buffer)
{
  Ecode_t error_code;

  write_counter_command_t *rx_packet = (write_counter_command_t*)rx_buffer;

  // Write the data to the NVM3 instance
  error_code = nvm3_writeCounter(nvm3_defaultHandle, rx_packet->key, rx_packet->data);
  if (error_code != ECODE_OK) {
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    return;
  }

  status_is(SL_STATUS_OK, tx_buffer, SL_STATUS);
}

/***************************************************************************//**
 * Parse CMD_INCREMENT_COUNTER packet and get a reply packet ready.
 ******************************************************************************/
static void on_increment_counter_command(void *rx_buffer,
                                         tx_buffer_t *tx_buffer)
{
  Ecode_t error_code;
  uint32_t new_value;

  read_counter_is_t *tx_packet = (read_counter_is_t*)tx_buffer->buffer;
  increment_counter_command_t *rx_packet = (increment_counter_command_t*)rx_buffer;

  error_code = nvm3_incrementCounter(nvm3_defaultHandle, rx_packet->key, &new_value);
  if (error_code != ECODE_OK) {
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    return;
  }

  // Prepare the reply
  tx_packet->data = new_value;
  tx_packet->header.cmd = CMD_COUNTER_VALUE_IS;
  tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t));
  tx_buffer->buffer_len = sizeof(*tx_packet);
}

/***************************************************************************//**
 * Parse CMD_GET_OBJECT_INFO packet and get a reply packet ready.
 ******************************************************************************/
static void on_get_object_info(void *rx_buffer,
                               tx_buffer_t *tx_buffer)
{
  Ecode_t error_code;
  size_t object_size;
  uint32_t object_type;

  object_info_is_t *tx_packet = (object_info_is_t*)tx_buffer->buffer;
  object_info_command_t *rx_packet = (object_info_command_t*)rx_buffer;

  error_code = nvm3_getObjectInfo(nvm3_defaultHandle, rx_packet->key, &object_type, &object_size);
  if (error_code != ECODE_OK) {
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    return;
  }

  // Prepare the reply
  tx_packet->object_size = (uint16_t)object_size;
  tx_packet->object_type = (uint8_t)object_type;
  tx_packet->header.cmd = CMD_OBJECT_INFO_IS;
  tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t));
  tx_buffer->buffer_len = sizeof(*tx_packet);
}

/***************************************************************************//**
 * Parse CMD_DELETE_OBJECT packet and get a reply packet ready.
 ******************************************************************************/
static void on_delete_object_command(void *rx_buffer,
                                     tx_buffer_t *tx_buffer)
{
  Ecode_t error_code;

  delete_object_command_t *rx_packet = (delete_object_command_t*)rx_buffer;

  error_code = nvm3_deleteObject(nvm3_defaultHandle, rx_packet->key);
  if (error_code != ECODE_OK) {
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    return;
  }

  status_is(SL_STATUS_OK, tx_buffer, SL_STATUS);
}

/***************************************************************************//**
 * Mark a NVM3 write transcation as complete
 ******************************************************************************/
static void complete_write_transaction(void)
{
  MCU_DECLARE_IRQ_STATE;
  MCU_ENTER_ATOMIC();
  write_data_command_context.offset = 0;
  write_data_command_context.total_write_size = 0;
  write_data_command_context.lock = 0;
  MCU_EXIT_ATOMIC();
}

/***************************************************************************//**
 * This function is called when the write lock has timed out
 ******************************************************************************/
static void on_write_lock_timeout(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  complete_write_transaction();
}

/***************************************************************************//**
 * Parse CMD_WRITE_DATA packet and get a reply packet ready.
 ******************************************************************************/
static void on_write_data_command(void *rx_buffer,
                                  tx_buffer_t *tx_buffer)
{
  static sl_sleeptimer_timer_handle_t write_lock_timer;
  Ecode_t error_code;
  sl_status_t status;
  uint32_t timer_timeout = 0;
  write_data_command_context_t write_context;
  write_data_command_t *rx_packet = (write_data_command_t *)rx_buffer;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  sl_sleeptimer_get_timer_time_remaining(&write_lock_timer, &timer_timeout);
  sl_sleeptimer_stop_timer(&write_lock_timer);
  write_context = write_data_command_context;
  CORE_EXIT_ATOMIC();

  // Check if the write is locked
  if ((write_context.lock > 0 && write_context.lock != rx_packet->header.unique_id)) {
    status = sl_sleeptimer_restart_timer(&write_lock_timer, timer_timeout, on_write_lock_timeout, NULL, 0, 0);
    status_is(SL_STATUS_BUSY, tx_buffer, SL_STATUS);
    return;
  }

  uint16_t write_fragment_size = rx_packet->header.len - (sizeof(write_data_command_t) - (sizeof(header_t)));
  if (rx_packet->offset < write_context.offset || rx_packet->offset + write_fragment_size > sizeof(write_buffer)) {
    status_is(SL_STATUS_FAIL, tx_buffer, SL_STATUS);
    complete_write_transaction();
    //Timer already stopped
    return;
  }

  // Assign the lock to the current unique id
  write_context.lock = rx_packet->header.unique_id;
  memcpy(write_buffer + rx_packet->offset, rx_packet->data, write_fragment_size);
  write_context.offset = rx_packet->offset;
  write_context.total_write_size += write_fragment_size;

  MCU_ATOMIC_SECTION(write_data_command_context = write_context; );

  // Start the lock timeout timer
  timer_timeout = sl_sleeptimer_ms_to_tick(SL_CPC_NVM3_WRITE_LOCK_TIMEOUT_MS);
  status = sl_sleeptimer_restart_timer(&write_lock_timer, timer_timeout, on_write_lock_timeout, NULL, 0, 0);
  EFM_ASSERT(status == SL_STATUS_OK);

  if (rx_packet->last_frag) {
    // Write to the NVM3 instance
    error_code = nvm3_writeData(nvm3_defaultHandle, rx_packet->key, write_buffer, write_context.total_write_size);
    complete_write_transaction();
    sl_sleeptimer_stop_timer(&write_lock_timer);
    if (error_code != ECODE_OK) {
      status_is((uint32_t)error_code, tx_buffer, ECODE);
      return;
    }
  }
  status_is(SL_STATUS_OK, tx_buffer, SL_STATUS);
}

/***************************************************************************//**
 * Parse CMD_PROP_VALUE_GET packet and get a reply packet ready.
 ******************************************************************************/
static void on_prop_value_get(void *rx_buffer,
                              tx_buffer_t *tx_buffer)
{
  property_get_command_t *rx_packet = (property_get_command_t *)rx_buffer;
  property_value_is_t *tx_packet = (property_value_is_t *)tx_buffer->buffer;

  switch (rx_packet->property_type) {
    case PROP_MAX_OBJECT_SIZE:
      tx_packet->property_type = PROP_MAX_OBJECT_SIZE;
      tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t)) + sizeof(sli_cpc_nvm3_properties_values.max_object_size);
      tx_buffer->buffer_len = sizeof(*tx_packet) + sizeof(sli_cpc_nvm3_properties_values.max_object_size);
      memcpy(&tx_buffer->buffer[sizeof(*tx_packet)], &sli_cpc_nvm3_properties_values.max_object_size, sizeof(sli_cpc_nvm3_properties_values.max_object_size));
      break;
    case PROP_MAX_WRITE_SIZE:
      tx_packet->property_type = PROP_MAX_WRITE_SIZE;
      tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t)) + sizeof(sli_cpc_nvm3_properties_values.max_write_size);
      tx_buffer->buffer_len = sizeof(*tx_packet) + sizeof(sli_cpc_nvm3_properties_values.max_write_size);
      memcpy(&tx_buffer->buffer[sizeof(*tx_packet)], &sli_cpc_nvm3_properties_values.max_write_size, sizeof(sli_cpc_nvm3_properties_values.max_write_size));
      break;
    default:
      tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t));
      tx_packet->property_type = PROP_UNKNOWN;
      tx_buffer->buffer_len = sizeof(*tx_packet);
  }

  tx_packet->header.cmd = CMD_PROP_VALUE_IS;
}

/***************************************************************************//**
 * Parse CMD_READ_COUNTER packet and get a reply packet ready.
 ******************************************************************************/
static void on_read_counter_command(void *rx_buffer,
                                    tx_buffer_t *tx_buffer)
{
  size_t object_len;
  uint32_t type;
  Ecode_t error_code;
  read_counter_command_t *rx_packet = (read_counter_command_t*)rx_buffer;
  read_counter_is_t *tx_packet = (read_counter_is_t*)tx_buffer->buffer;
  uint32_t read_value;

  // Make sure the object is a counter
  error_code = nvm3_getObjectInfo(nvm3_defaultHandle, rx_packet->key, &type, &object_len);
  if (error_code != ECODE_OK) {
    // Failed to get object info, reply with error code
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    return;
  }

  // Check for invalid type
  if (type != NVM3_OBJECTTYPE_COUNTER) {
    status_is((uint32_t)ECODE_NVM3_ERR_OBJECT_IS_NOT_A_COUNTER, tx_buffer, ECODE);
    return;
  }

  // Read from NVM3
  error_code = nvm3_readCounter(nvm3_defaultHandle, rx_packet->key, &read_value);
  if (error_code != ECODE_OK) {
    // Read failed, reply with error code
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    return;
  }

  // Prepare the reply
  tx_packet->data = read_value;
  tx_packet->header.cmd = CMD_COUNTER_VALUE_IS;
  tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t));
  tx_buffer->buffer_len = sizeof(*tx_packet);
}

/***************************************************************************//**
 * Enumerate objects command can be multiple steps, this function will run a step and
 * enumerate_objects_command_incomplete will stay true if another step is required.
 ******************************************************************************/
static void enumerate_objects_command_reply_step(tx_buffer_t *tx_buffer, uint16_t max_object_count)
{
  size_t keys_count_written_to_list;
  static size_t object_count_limit = 0;
  static size_t total_objects_submitted = 0;
  static size_t total_objects_count;
  static size_t object_count_to_fetch;
  static size_t last_key_obtained = 0;
  static nvm3_ObjectKey_t keys[SL_CPC_NVM3_MAX_ENUMERATED_OBJECTS_PER_TRANSACTION];

  enumerate_object_is_t *tx_packet = (enumerate_object_is_t*)tx_buffer->buffer;

  // First iteration
  if (enumerate_objects_command_incomplete == false) {
    // Get total amount of objects
    object_count_limit = max_object_count;
    total_objects_submitted = 0;
    last_key_obtained = 0;
    total_objects_count = nvm3_enumObjects(nvm3_defaultHandle, NULL, 0, NVM3_KEY_MIN, NVM3_KEY_MAX);
  }

  // Populate the list
  object_count_to_fetch = object_count_limit - total_objects_submitted;
  object_count_to_fetch = SL_MIN(object_count_to_fetch, total_objects_count);
  object_count_to_fetch = SL_MIN(object_count_to_fetch, (uint16_t)SL_CPC_NVM3_MAX_ENUMERATED_OBJECTS_PER_TRANSACTION);

  EFM_ASSERT(object_count_to_fetch > 0);

  keys_count_written_to_list = nvm3_enumObjects(nvm3_defaultHandle, keys, object_count_to_fetch, NVM3_KEY_MIN + last_key_obtained, NVM3_KEY_MAX);
  total_objects_submitted += keys_count_written_to_list;
  last_key_obtained = keys[keys_count_written_to_list - 1] + 1;

  // Do we need to run this step again ?
  if (total_objects_submitted < total_objects_count && total_objects_submitted != object_count_limit) {
    enumerate_objects_command_incomplete = true;
    tx_packet->last_frag = 0;
  } else {
    enumerate_objects_command_incomplete = false;
    tx_packet->last_frag = 1;
  }

  // Prepare the reply
  tx_packet->header.cmd = CMD_ENUMERATE_OBJECTS_IS;
  tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t)) + (keys_count_written_to_list * sizeof(nvm3_ObjectKey_t));
  tx_buffer->buffer_len = sizeof(*tx_packet) + (keys_count_written_to_list * sizeof(nvm3_ObjectKey_t));
  memcpy(&tx_buffer->buffer[sizeof(*tx_packet)], keys, keys_count_written_to_list * sizeof(nvm3_ObjectKey_t));
}

/***************************************************************************//**
 * Read command can be multiple steps, this function will run a step and
 * read_command_incomplete will stay true if another step is required.
 ******************************************************************************/
static void read_command_reply_step(tx_buffer_t *tx_buffer)
{
  size_t read_size;
  Ecode_t error_code;
  bool is_last_fragment = false;
  __ALIGNED(4) static uint32_t read_buffer[SL_CPC_NVM3_MAX_READ_FRAGMENT_SIZE];

  // Calculate the read size
  if (read_data_command_context.object_len - read_data_command_context.offset > SL_CPC_NVM3_MAX_READ_FRAGMENT_SIZE) {
    read_size = SL_CPC_NVM3_MAX_READ_FRAGMENT_SIZE;
    read_command_incomplete = true;
  } else {
    read_size = read_data_command_context.object_len - read_data_command_context.offset;
    is_last_fragment = true;
    read_command_incomplete = false;
  }

  // Read from NVM3
  error_code = nvm3_readPartialData(nvm3_defaultHandle,
                                    read_data_command_context.key,
                                    read_buffer,
                                    read_data_command_context.offset,
                                    read_size);
  if (error_code != ECODE_OK) {
    // Read failed, reply with error code
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    read_command_incomplete = false;
    return;
  }

  // Update the offset for the next chunk
  if (!is_last_fragment) {
    read_data_command_context.offset += read_size;
  }

  // Prepare the reply
  read_data_is_t *tx_packet = (read_data_is_t*)tx_buffer->buffer;
  tx_packet->last_frag = is_last_fragment ? 1 : 0;
  tx_packet->header.cmd = CMD_READ_DATA_IS;
  tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t)) + read_size;
  tx_buffer->buffer_len = sizeof(*tx_packet) + read_size;
  memcpy(&tx_buffer->buffer[sizeof(*tx_packet)], read_buffer, read_size);
}

/***************************************************************************//**
 * Parse CMD_READ_DATA packet and get a reply packet ready.
 ******************************************************************************/
static void on_read_data_command(void *rx_buffer,
                                 tx_buffer_t *tx_buffer)
{
  Ecode_t error_code;
  size_t object_len;
  uint32_t type;

  read_data_command_t *rx_packet = (read_data_command_t*)rx_buffer;

  // Get the actual object size
  error_code = nvm3_getObjectInfo(nvm3_defaultHandle, rx_packet->key, &type, &object_len);
  if (error_code != ECODE_OK) {
    // Failed to get object info, reply with error code
    status_is((uint32_t)error_code, tx_buffer, ECODE);
    return;
  }

  // Check for object larger than user buffer
  if (object_len > rx_packet->max_read_size) {
    status_is((uint32_t)ECODE_NVM3_ERR_SIZE_TOO_SMALL, tx_buffer, ECODE);
    return;
  }

  // Check for invalid type
  if (type != NVM3_OBJECTTYPE_DATA) {
    status_is((uint32_t)ECODE_NVM3_ERR_OBJECT_IS_NOT_DATA, tx_buffer, ECODE);
    return;
  }

  read_data_command_context.key = rx_packet->key;
  read_data_command_context.object_len = object_len;
  read_data_command_context.offset = 0;
  read_command_reply_step(tx_buffer);
}

/***************************************************************************//**
 * Parse CMD_OBJECT_COUNT packet and get a reply packet ready.
 ******************************************************************************/
static void on_object_count_command(tx_buffer_t *tx_buffer)
{
  uint16_t total_objects_count;
  object_count_is_t *tx_packet = (object_count_is_t*)tx_buffer;

  total_objects_count = nvm3_enumObjects(nvm3_defaultHandle, NULL, 0, NVM3_KEY_MIN, NVM3_KEY_MAX);

  // Prepare the reply
  tx_packet->object_count = total_objects_count;
  tx_packet->header.cmd = CMD_OBJECT_COUNT_IS;
  tx_packet->header.len = (sizeof(*tx_packet) - sizeof(header_t));
  tx_buffer->buffer_len = sizeof(*tx_packet);
}

/***************************************************************************//**
 * Parse CMD_ENUMERATE_OBJECTS packet and get a reply packet ready.
 ******************************************************************************/
static void on_enumerate_objects_command(void *rx_buffer,
                                         tx_buffer_t *tx_buffer)
{
  enumerate_objects_command_t *rx_packet = (enumerate_objects_command_t*)rx_buffer;
  enumerate_objects_command_reply_step(tx_buffer, rx_packet->max_object_count);
}

/***************************************************************************//**
 * Parse unsupported packet and get a reply packet ready.
 ******************************************************************************/
static void on_unsupported_cmd(uint8_t unsupported_cmd, tx_buffer_t * tx_buffer)
{
  cmd_unsupported_is_t *tx_packet = (cmd_unsupported_is_t *)tx_buffer;

  tx_packet->header.cmd = CMD_UNSUPPORTED;
  tx_packet->header.len = sizeof(tx_packet->cmd_unsupported);
  tx_packet->cmd_unsupported = unsupported_cmd;
}

/***************************************************************************//**
 * Validate the length field of a header
 ******************************************************************************/
static sl_status_t validate_len(void *rx_buffer, uint16_t rx_buffer_length)
{
  header_t *header = (header_t*)rx_buffer;
  if (rx_buffer_length < sizeof(header_t) || (header->len != rx_buffer_length - sizeof(header_t))) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Update the TX header content
 ******************************************************************************/
static void update_tx_header(void *rx_buffer, tx_buffer_t *tx_buffer)
{
  header_t *rx_header = (header_t*)rx_buffer;
  header_t *tx_header = (header_t*)tx_buffer->buffer;
  tx_header->transaction_id = rx_header->transaction_id;
  tx_header->unique_id = rx_header->unique_id;
}

/***************************************************************************//**
 * Parse packet and get a reply packet ready.
 ******************************************************************************/
static void parse_packet(void *rx_buffer,
                         uint16_t rx_buffer_length,
                         tx_buffer_t *tx_buffer)
{
  header_t *header = (header_t *)rx_buffer;
  uint8_t cmd = header->cmd;

  // Process the command only if the length is valid
  if (validate_len(rx_buffer, rx_buffer_length) == SL_STATUS_OK) {
    switch (cmd) {
      case CMD_GET_VERSION:
        on_get_version(rx_buffer, tx_buffer);
        break;

      case CMD_WRITE_DATA:
        on_write_data_command(rx_buffer, tx_buffer);
        break;

      case CMD_WRITE_COUNTER:
        on_write_counter_command(rx_buffer, tx_buffer);
        break;

      case CMD_READ_DATA:
        on_read_data_command(rx_buffer, tx_buffer);
        break;

      case CMD_READ_COUNTER:
        on_read_counter_command(rx_buffer, tx_buffer);
        break;

      case CMD_INCREMENT_COUNTER:
        on_increment_counter_command(rx_buffer, tx_buffer);
        break;

      case CMD_PROP_VALUE_GET:
        on_prop_value_get(rx_buffer, tx_buffer);
        break;

      case CMD_GET_OBJECT_INFO:
        on_get_object_info(rx_buffer, tx_buffer);
        break;

      case CMD_DELETE_OBJECT:
        on_delete_object_command(rx_buffer, tx_buffer);
        break;

      case CMD_ENUMERATE_OBJECTS:
        on_enumerate_objects_command(rx_buffer, tx_buffer);
        break;

      case CMD_OBJECT_COUNT:
        on_object_count_command(tx_buffer);
        break;

      default:
        on_unsupported_cmd(cmd, tx_buffer);
        break;
    }

    update_tx_header(rx_buffer, tx_buffer);

    EFM_ASSERT(tx_buffer->buffer_len <= sizeof(tx_buffer->buffer));
    signal_state_change(ENDPOINT_STATE_REPLY_READY);
  }
}

/***************************************************************************//**
 * Read an incoming packet.
 ******************************************************************************/
static void read_packet(tx_buffer_t *tx_buffer)
{
  sl_status_t status;

  status = sl_cpc_read(&ep_nvm3, &rx_buffer, &rx_buffer_length, 0, 0);
  switch (status) {
    case SL_STATUS_OK:
      break;
    case SL_STATUS_EMPTY:
    case SL_STATUS_ALLOCATION_FAILED:
    case SL_STATUS_INVALID_STATE:
      return;

    default:
      EFM_ASSERT(false);
      return;
  }

  parse_packet(rx_buffer, rx_buffer_length, tx_buffer);
  status = sl_cpc_free_rx_buffer(rx_buffer);
  EFM_ASSERT(status == SL_STATUS_OK);
}

/***************************************************************************//**
 * Transmit a packet.
 ******************************************************************************/
static void transmit_packet(tx_buffer_t *tx_buffer)
{
  sl_status_t status;
  uint16_t buffer_len = sizeof(header_t) + ((header_t*)tx_buffer)->len;

  status = sl_cpc_write(&ep_nvm3, tx_buffer, buffer_len, 0, NULL);
  if (status == SL_STATUS_OK) {
    signal_state_change(ENDPOINT_STATE_WRITE_PENDING);
  } else if (status == SL_STATUS_NO_MORE_RESOURCE) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    osDelay(((uint64_t)osKernelGetTickFreq() * CPC_NVM3_OS_DELAY_MS) / 1000);
#endif // SL_CATALOG_KERNEL_PRESENT
    signal_state_change(ENDPOINT_STATE_REPLY_READY);
  }
}

/***************************************************************************//**
 * Task
 ******************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
static void task(void *arg)
{
  (void)arg;

  if (init_endpoint() != SL_STATUS_OK) {
    EFM_ASSERT(false);
    osThreadExit();
  }

  while (true) {
    sl_cpc_nvm3_process_action();
  }
}
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * NVM3 endpoint init
 ******************************************************************************/
sl_status_t sl_cpc_nvm3_init(void)
{
  memset(&write_data_command_context, 0x00, sizeof(write_data_command_context));
  memset(&read_data_command_context, 0x00, sizeof(read_data_command_context));

#if defined(SL_CATALOG_KERNEL_PRESENT)
  osThreadAttr_t task_attr = { 0 };
  osSemaphoreAttr_t semaphore_attr = { 0 };

  semaphore_attr.cb_mem = semaphore_cb;
  semaphore_attr.cb_size = osSemaphoreCbSize;
  state_change_signal = osSemaphoreNew(1, 1, &semaphore_attr);
  if (state_change_signal == 0) {
    EFM_ASSERT(false);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  task_attr.name = "CPC NVM3";
  task_attr.priority = SL_CPC_NVM3_TASK_PRIORITY;
  task_attr.stack_mem = &thread_stack[0];
  task_attr.stack_size = THREAD_STACK_SIZE;
  task_attr.cb_mem = &thread_cb[0];
  task_attr.cb_size = osThreadCbSize;
  thread_id = osThreadNew(&task, NULL, &task_attr);
  EFM_ASSERT(thread_id != NULL);
#else
  init_endpoint();
#endif // SL_CATALOG_KERNEL_PRESENT

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * NVM3 endpoint process action
 ******************************************************************************/
void sl_cpc_nvm3_process_action(void)
{
  static tx_buffer_t tx_buffer;
  endpoint_state_t state;

  if (restart_flag) {
    restart_endpoint();
    return;
  }

  state = wait_state_change();

  if (!restart_flag) {
    switch (state) {
      case ENDPOINT_STATE_RESTART:
        break;
      case ENDPOINT_STATE_INIT:
      case ENDPOINT_STATE_WRITE_COMPLETED:
        if (enumerate_objects_command_incomplete) {
          enumerate_objects_command_reply_step(&tx_buffer, 0);
          signal_state_change(ENDPOINT_STATE_REPLY_READY);
        } else if (read_command_incomplete) {
          read_command_reply_step(&tx_buffer);
          signal_state_change(ENDPOINT_STATE_REPLY_READY);
        } else {
          read_packet(&tx_buffer);
        }
        break;
      case ENDPOINT_STATE_REPLY_READY:
        transmit_packet(&tx_buffer);
        break;
      case ENDPOINT_STATE_WRITE_PENDING:
        break;
      default:
        EFM_ASSERT(false);
    }
  }
}

#if SLI_CPC_NVM3_DEVICE_UNDER_TEST
void sli_cpc_nvm3_restart_endpoint(void)
{
  restart_flag = true;
}
#endif
