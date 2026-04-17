/***************************************************************************//**
 * @file sl_iostream_ble_spp.c
 * @brief I/O Stream BLE Serial Port Profile
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sl_assert.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_status.h"
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#include "sl_bluetooth.h"
#include "sl_bt_api.h"
#include "gatt_db.h"
#include "sl_bluetooth_advertiser_config.h"
#include "sl_bluetooth_connection_config.h"
#include "sl_iostream_ble_spp.h"
#include "sl_iostream_ble_spp_config.h"

_Static_assert(SL_BT_CONFIG_USER_ADVERTISERS == 1UL,
               "The maximum value for SL_BT_CONFIG_USER_ADVERTISERS should be 1 as only one advertiser is supported.");

_Static_assert(SL_BT_CONFIG_MAX_CONNECTIONS == 1UL,
               "The maximum value for SL_BT_CONFIG_MAX_CONNECTIONS should be 1 as only one connection is supported.");
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// SPP Thread stack size in words
#define SL_IOSTREAM_BLE_SPP_STACK_SIZE_WORD            (128UL)

// SPP Event flag all mask
#define SL_IOSTREAM_BLE_SPP_EVT_ALL_MSK                (0x00FFFFFFUL)

// SPP Event flag FIFO TX mask
#define SL_IOSTREAM_BLE_SPP_EVT_FIFO_TX_MSK            (0x00000001UL << 0UL)

// SPP Event flag mode mask
#define SL_IOSTREAM_BLE_SPP_EVT_MODE_MSK               (0x00000001UL << 1UL)

// SPP Event flag error mask
#define SL_IOSTREAM_BLE_SPP_EVT_ERROR_MSK              (0x00000001UL << 31UL)

// SPP FIFO data structure
typedef struct {
  uint8_t *buff; // Buffer pointer
  uint8_t *head; // Head pointer
  uint8_t *tail; // Tail pointer
  uint32_t size; // Size of the buffer
} spp_fifo_t;
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief I/O Stream SPP write callback function.
 * @details This function is the write function for the SPP I/O stream (output data on a stream).
 *
 * @param[in] context The context of the I/O stream.
 * @param[in] buffer Buffer that contains the data to output.
 * @param[in] buffer_length Data length contained in the buffer.
 *
 * @return SL_STATUS_OK
 *****************************************************************************/
static sl_status_t _iostream_spp_write_cb(void *context,
                                          const void *buffer,
                                          size_t buffer_length);

/**************************************************************************//**
 * @brief I/O Stream SPP read callback function.
 * @details This function is the read function for the SPP I/O stream (get data from a stream)
 *
 * @param[in] context The context of the I/O stream.
 * @param[out] buffer Buffer that contains the data to output.
 * @param[in] buffer_length Data length contained in the buffer.
 * @param[out] bytes_read Data length copied to the buffer.
 *
 * @return SL_STATUS_OK
 *****************************************************************************/
static sl_status_t _iostream_spp_read_cb(void *context,
                                         void *buffer,
                                         size_t buffer_length,
                                         size_t *bytes_read);

/**************************************************************************//**
 * @brief FIFO write.
 * @details This function writes data to the FIFO.
 *
 * @param[in] f The FIFO data structure.
 * @param[in] data The data to write.
 * @param[in] size The size of the data.
 *
 * @return SL_STATUS_OK if successful, error code otherwise
 *****************************************************************************/
static sl_status_t _fifo_write(spp_fifo_t * const f, const uint8_t * const data, const size_t size);

/**************************************************************************//**
 * @brief FIFO read.
 * @details This function reads data from the FIFO.
 *
 * @param[in] f The FIFO data structure.
 * @param[out] data The data read.
 * @param[out] size The size of the data.
 *
 * @return SL_STATUS_OK if successful, error code otherwise
 *****************************************************************************/
static sl_status_t _fifo_read(spp_fifo_t * const f, uint8_t * const data, size_t * const size);

/**************************************************************************//**
 * @brief Task function
 * @details This function is the main task for the SPP
 *
 * @param[in] args Arguments
 *****************************************************************************/
static void _spp_task_fnc(void *args);

/**************************************************************************//**
 * @brief Acquire SPP mutex
 * @details Internal mutex lock
 *****************************************************************************/
__STATIC_INLINE void _spp_mutex_acquire(void);

/**************************************************************************//**
 * @brief Release SPP mutex
 * @details Internal mutex release
 *****************************************************************************/
__STATIC_INLINE void _spp_mutex_release(void);
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// BLE connection handle
static uint8_t _connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;

// SPP FIFO TX buffer (data to be sent)
static uint8_t _spp_tx_buff[SL_IOSTREAM_BLE_SPP_FIFO_TX_SIZE] = { 0U };

// SPP FIFO RX buffer (data received)
static uint8_t _spp_rx_buff[SL_IOSTREAM_BLE_SPP_FIFO_RX_SIZE] = { 0U };

// SPP FIFO TX
static spp_fifo_t _spp_fifo_tx = {
  .buff = _spp_tx_buff,
  .head = _spp_tx_buff,
  .tail = _spp_tx_buff,
  .size = SL_IOSTREAM_BLE_SPP_FIFO_TX_SIZE
};

// SPP FIFO RX
static spp_fifo_t _spp_fifo_rx = {
  .buff = _spp_rx_buff,
  .head = _spp_rx_buff,
  .tail = _spp_rx_buff,
  .size = SL_IOSTREAM_BLE_SPP_FIFO_RX_SIZE
};

// SPP I/O stream
static sl_iostream_t _iostream_spp = {
  .read = _iostream_spp_read_cb,
  .write = _iostream_spp_write_cb,
  .context = NULL
};

// SPP task ID
static osThreadId_t _spp_task = NULL;

// SPP task attributes
static const osThreadAttr_t _spp_task_attr = {
  .name       = "SPP_task",
  .attr_bits  = osThreadDetached,
  .cb_mem     = NULL,
  .cb_size    = 0UL,
  .stack_mem  = NULL,
  .stack_size = (SL_IOSTREAM_BLE_SPP_STACK_SIZE_WORD * sizeof(void *)) & 0xFFFFFFF8U,
  .priority   = osPriorityNormal1,
  .tz_module  = 0UL,
  .reserved   = 0UL
};

// SPP mutex
static osMutexId_t _spp_mtx = NULL;

// SPP mutex attribute
static const osMutexAttr_t _spp_mtx_attr = {
  .name      = "SPP_mtx",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0UL
};

// SPP FIFO event flags
static osEventFlagsId_t _spp_fifo_evt = NULL;

// SPP FIFO event flags attributes
static const osEventFlagsAttr_t _spp_fifo_evt_attr = {
  .name      = "SppFifoEvtFlags",
  .attr_bits = 0UL,
  .cb_mem    = NULL,
  .cb_size   = 0UL
};
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
// SPP I/O stream handle
sl_iostream_t *iostream_spp_handle = &_iostream_spp;

// SPP I/O stream instance info
sl_iostream_instance_info_t iostream_instance_spp_info = {
  .handle = &_iostream_spp,
  .name = "spp",
  .type = SL_IOSTREAM_TYPE_UNDEFINED,
  .periph_id = 0U,
  .init = &sl_iostream_spp_init_cb,
};
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/* I/O Stream SPP init */
void sl_iostream_ble_spp_init(void)
{
  // Create mutex
  _spp_mtx = osMutexNew(&_spp_mtx_attr);
  EFM_ASSERT(_spp_mtx != NULL);

  // Create event flags
  _spp_fifo_evt = osEventFlagsNew(&_spp_fifo_evt_attr);
  EFM_ASSERT(_spp_fifo_evt != NULL);

  // Create task
  _spp_task = osThreadNew(_spp_task_fnc, NULL, &_spp_task_attr);
  EFM_ASSERT(_spp_task != NULL);
}

/* SPP Stream init callback */
sl_status_t sl_iostream_spp_init_cb(void)
{
  // Set default IO stream to SPP
  sl_iostream_set_system_default(&_iostream_spp);

  return SL_STATUS_OK;
}

/* Save BLE connection handle */
void sl_iostream_ble_spp_save_connection_handle(const uint8_t conn)
{
  _spp_mutex_acquire();
  _connection_handle = conn;
  _spp_mutex_release();
}

/* Get BLE connection handle */
uint8_t sl_iostream_ble_spp_get_connection_handle(void)
{
  uint8_t conn = SL_BT_INVALID_CONNECTION_HANDLE;

  _spp_mutex_acquire();
  conn = _connection_handle;
  _spp_mutex_release();

  return conn;
}

/* Save SPP mode */
void sl_iostream_ble_spp_save_mode(const sl_iostream_ble_spp_mode_t mode)
{
  if (mode == SL_IOSTREAM_BLE_SPP_MODE_ON) {
    (void) osEventFlagsSet(_spp_fifo_evt, SL_IOSTREAM_BLE_SPP_EVT_MODE_MSK);
  } else if (mode == SL_IOSTREAM_BLE_SPP_MODE_OFF) {
    (void) osEventFlagsClear(_spp_fifo_evt, SL_IOSTREAM_BLE_SPP_EVT_MODE_MSK);
  }
}

/* Get SPP mode */
sl_status_t sl_iostream_ble_spp_get_mode(sl_iostream_ble_spp_mode_t * const mode)
{
  uint32_t flags = 0UL;

  flags = osEventFlagsGet(_spp_fifo_evt);
  // Event flag error
  if (flags & SL_IOSTREAM_BLE_SPP_EVT_ERROR_MSK) {
    printf("[Failed: BLE SPP mode evt flags (0x%08lX)]\n", flags);
    return SL_STATUS_FAIL;
  }

  *mode = (flags & SL_IOSTREAM_BLE_SPP_EVT_MODE_MSK)
          ? SL_IOSTREAM_BLE_SPP_MODE_ON : SL_IOSTREAM_BLE_SPP_MODE_OFF;

  return SL_STATUS_OK;
}

/* RX FIFO write */
sl_status_t sl_iostream_ble_spp_rx_fifo_write(const uint8_t * const data, const size_t size)
{
  if (data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return _fifo_write(&_spp_fifo_rx, data, size);
}

/* RX FIFO read */
sl_status_t sl_iostream_ble_spp_rx_fifo_read(uint8_t * const data, size_t * const size)
{
  if (data == NULL || size == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return _fifo_read(&_spp_fifo_rx, data, size);
}

/* TX FIFO write */
sl_status_t sl_iostream_ble_spp_tx_fifo_write(const uint8_t *data, const size_t size)
{
  if (data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return _fifo_write(&_spp_fifo_tx, data, size);
}

/* TX FIFO read */
sl_status_t sl_iostream_ble_spp_tx_fifo_read(uint8_t * const data, size_t * const size)
{
  if (data == NULL || size == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return _fifo_read(&_spp_fifo_tx, data, size);
}
/**************************************************************************//**
 * Callback for Bluetooth events
 *
 * @param[in] evt event coming from the Bluetooth stack
 *****************************************************************************/
void sl_iostream_ble_spp_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc = SL_STATUS_FAIL;
  uint16_t max_mtu_out = 0U;
  sl_iostream_ble_spp_mode_t spp_mode = SL_IOSTREAM_BLE_SPP_MODE_OFF;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Set the maximum size (250) of ATT Message Transfer Units (MTU) in the host stack.
      sc = sl_bt_gatt_server_set_max_mtu(250U, &max_mtu_out);
      EFM_ASSERT(sc == SL_STATUS_OK);

      // Set default connection parameters
      sc = sl_bt_connection_set_default_parameters(16U,      // min. connection interval (in unit of 1.25ms)
                                                   40U,      // max. connection interval (in unit of 1.25ms)
                                                   0U,       // slave latency
                                                   500U,     // connection timeout (in unit of 10ms)
                                                   0x0000U,  // min. connection event length (in unit of 0.625ms)
                                                   0xFFFFU); // max. connection event length (in unit of 0.625ms)
      EFM_ASSERT(sc == SL_STATUS_OK);

      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      // Save connection handle
      sl_iostream_ble_spp_save_connection_handle(evt->data.evt_connection_opened.connection);

      break;

    // -------------------------------
    // This event indicates that a MTU exchange has finished.
    case sl_bt_evt_gatt_mtu_exchanged_id:
      // Print exchanged MTU.
      printf("[BLE MTU exchanged: %d]\n", evt->data.evt_gatt_mtu_exchanged.mtu);

      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Save SPP mode
      sl_iostream_ble_spp_save_mode(SL_IOSTREAM_BLE_SPP_MODE_OFF);

      break;

    // -------------------------------
    // This event indicates either that a local Client Characteristic Configuration descriptor
    // has been changed by the remote GATT client, or that a confirmation from the remote GATT
    // client was received upon a successful reception of the indication.
    case sl_bt_evt_gatt_server_characteristic_status_id:
      if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_spp_data)
          && (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_client_config)) {
        // Characteristic client configuration (CCC) for spp_data has been changed
        if (evt->data.evt_gatt_server_characteristic_status.client_config_flags & sl_bt_gatt_server_notification) {
          sl_iostream_ble_spp_save_mode(SL_IOSTREAM_BLE_SPP_MODE_ON);
        } else {
          sl_iostream_ble_spp_save_mode(SL_IOSTREAM_BLE_SPP_MODE_OFF);
        }
      }

      break;

    // -------------------------------
    // This event indicates that the value of an attribute in the local GATT
    // database was changed by a remote GATT client.
    case sl_bt_evt_gatt_server_attribute_value_id:
      if ((evt->data.evt_gatt_server_attribute_value.attribute == gattdb_spp_data)
          && (evt->data.evt_gatt_server_attribute_value.value.len != 0U)
          && (sl_iostream_ble_spp_get_connection_handle() != SL_BT_INVALID_CONNECTION_HANDLE)
          && (sl_iostream_ble_spp_get_mode(&spp_mode) == SL_STATUS_OK)
          && (spp_mode == SL_IOSTREAM_BLE_SPP_MODE_ON)) {
        // Write RX FIFO with the received data from GATT Client
        (void) sl_iostream_ble_spp_rx_fifo_write(evt->data.evt_gatt_server_attribute_value.value.data,
                                                 evt->data.evt_gatt_server_attribute_value.value.len);
      }

      break;

    // -------------------------------
    // Default event handler
    default:
      break;
  }
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/* SPP I/O Stream write callback */
static sl_status_t _iostream_spp_write_cb(void *context,
                                          const void *buffer,
                                          size_t buffer_length)
{
  sl_status_t mode_status = SL_STATUS_FAIL;
  sl_status_t fifo_status = SL_STATUS_OK;
  sl_iostream_ble_spp_mode_t spp_mode = SL_IOSTREAM_BLE_SPP_MODE_OFF;

  (void) context;

  if (buffer == NULL) {
    return SL_STATUS_OK;
  }

  mode_status = sl_iostream_ble_spp_get_mode(&spp_mode);

  // FIFO write only if BLE connection is active and SPP mode is ON
  if ((_connection_handle != SL_BT_INVALID_CONNECTION_HANDLE)
      && (mode_status == SL_STATUS_OK)
      && (spp_mode == SL_IOSTREAM_BLE_SPP_MODE_ON)) {
    fifo_status = _fifo_write(&_spp_fifo_tx, (const uint8_t *)buffer, buffer_length);
    if (fifo_status != SL_STATUS_INVALID_PARAMETER) {
      // Notify SPP thread to consume FIFO
      (void) osEventFlagsSet(_spp_fifo_evt, SL_IOSTREAM_BLE_SPP_EVT_FIFO_TX_MSK);
    }
  }

  return SL_STATUS_OK;
}

/* SPP I/O Stream read callback */
static sl_status_t _iostream_spp_read_cb(void *context,
                                         void *buffer,
                                         size_t buffer_length,
                                         size_t *bytes_read)
{
  sl_status_t status = SL_STATUS_FAIL;
  sl_status_t mode_status = SL_STATUS_FAIL;
  sl_iostream_ble_spp_mode_t spp_mode = SL_IOSTREAM_BLE_SPP_MODE_OFF;

  (void) context;
  (void) buffer_length;

  if (buffer == NULL || bytes_read == NULL) {
    return SL_STATUS_FAIL;
  }

  mode_status = sl_iostream_ble_spp_get_mode(&spp_mode);

  if ((_connection_handle != SL_BT_INVALID_CONNECTION_HANDLE)
      && (mode_status == SL_STATUS_OK)
      && (spp_mode == SL_IOSTREAM_BLE_SPP_MODE_ON)) {
    *bytes_read = 1UL;
    status = _fifo_read(&_spp_fifo_rx, (uint8_t *)buffer, bytes_read); // Read FIFO buffer as a bytestream
  }

  return status;
}

/* SPP FIFO write */
static sl_status_t _fifo_write(spp_fifo_t * const f, const uint8_t * const data, const size_t size)
{
  uint32_t freespace = 0UL;
  size_t s = size;

  if (f == NULL || data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _spp_mutex_acquire();

  freespace = f->head - f->buff; // calculate free space
  if (freespace + s > f->size) {
    s = f->size - freespace; // overflow, trim size
  }

  if (s) {
    memcpy(f->head, data, s); // write FIFO buffer
    f->head += s;
  }

  _spp_mutex_release();

  return SL_STATUS_OK;
}

/* SPP FIFO read */
static sl_status_t _fifo_read(spp_fifo_t * const f, uint8_t * const data, size_t * const size)
{
  sl_status_t ret = SL_STATUS_OK;
  uint32_t available = 0UL;

  if (f == NULL || data == NULL || size == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _spp_mutex_acquire();

  available = f->head - f->tail; // calculate available data
  if (available < *size) {
    *size = available; // underflow, trim size
  }

  if (*size) {
    memcpy(data, f->tail, *size); // read FIFO buffer
    f->tail += *size;
  }

  // if head is at tail (fifo empty), set pointers to default to increase possible write size to the max
  if (f->head == f->tail) {
    f->head = f->tail = f->buff; // reset FIFO buffer pointers
    ret = SL_STATUS_EMPTY;
  }

  _spp_mutex_release();

  return ret;
}

/* Mutex acquire */
__STATIC_INLINE void _spp_mutex_acquire(void)
{
  EFM_ASSERT(osMutexAcquire(_spp_mtx, osWaitForever) == osOK);
}

/* Mutex release */
__STATIC_INLINE void _spp_mutex_release(void)
{
  EFM_ASSERT(osMutexRelease(_spp_mtx) == osOK);
}

/* SPP task function */
static void _spp_task_fnc(void *args)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t flags = 0UL;
  static uint8_t _spp_tx[SL_IOSTREAM_BLE_SPP_MAX_NOTIFICATION_SIZE];
  size_t fifo_read_size = 0UL;
  sl_status_t mode_status = SL_STATUS_FAIL;
  sl_iostream_ble_spp_mode_t spp_mode = SL_IOSTREAM_BLE_SPP_MODE_OFF;

  (void) args;

  // Clear all event flags
  (void) osEventFlagsClear(_spp_fifo_evt, SL_IOSTREAM_BLE_SPP_EVT_ALL_MSK);

  // Reset TX buffer
  memset(_spp_tx, 0U, sizeof(_spp_tx));

  // SPP thread loop
  SL_IOSTREAM_BLE_SPP_THREAD_LOOP {
    flags = osEventFlagsWait(_spp_fifo_evt,
                             SL_IOSTREAM_BLE_SPP_EVT_FIFO_TX_MSK,
                             osFlagsWaitAny,
                             osWaitForever);

    if (flags & SL_IOSTREAM_BLE_SPP_EVT_ERROR_MSK) {
      printf("[Failed: SPP FIFO evt flags (0x%08lX)]\n", flags);
      continue;
    }

    if (flags & SL_IOSTREAM_BLE_SPP_EVT_FIFO_TX_MSK) {
      mode_status = sl_iostream_ble_spp_get_mode(&spp_mode);

      if ((_connection_handle != SL_BT_INVALID_CONNECTION_HANDLE)
          && (mode_status == SL_STATUS_OK)
          && (spp_mode == SL_IOSTREAM_BLE_SPP_MODE_ON)) {
        do {
          // Reset read size to max notification size
          fifo_read_size = SL_IOSTREAM_BLE_SPP_MAX_NOTIFICATION_SIZE;
          // Read SPP FIFO in chunks of ATT_MTU - 3 number of bytes
          status = _fifo_read(&_spp_fifo_tx, _spp_tx, &fifo_read_size);
          // BLE send notification with the data read from the FIFO
          (void) sl_bt_gatt_server_send_notification(_connection_handle,
                                                     gattdb_spp_data,
                                                     (uint8_t)fifo_read_size,
                                                     _spp_tx);
          // Introduce a delay to manage BLE notification rate for large data transfers
          // (Synchronize I/O operations with BLE DMP processing capabilities)
          osDelay(10U);
        } while (status != SL_STATUS_EMPTY); // Continue until FIFO is empty
      }
    }
  }
}
