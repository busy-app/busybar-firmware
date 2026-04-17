/***************************************************************************//**
 * @file
 * @brief Bluetooth NCP Transport Layer over CPC source
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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sl_status.h"
#include "sl_common.h"
#include "sl_core.h"
#include "sl_bt_ncp_transport.h"
#include "sli_bt_ncp_transport.h"
#include "sl_bt_ncp_transport_cpc_config.h"
#include "sl_cpc.h"
#include "sli_cpc.h"
#include "app_rta.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_APP_ASSERT_PRESENT
#include "app_assert.h"
#endif // SL_CATALOG_APP_ASSERT_PRESENT

// -----------------------------------------------------------------------------
// Definitions

// Internal state type for transport
typedef struct {
  uint8_t                  tx_buf[SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE];
  app_rta_context_t        ctx;             //< Runtime context
  sl_cpc_endpoint_handle_t endpoint_handle; //< CPC endpoint handle
  bool                     cpc_opened;      //< CPC opened state
  bool                     cpc_connected;   //< CPC connected state
  struct {
    uint8_t                completed;       //< Write completed flag
    sl_status_t            status;          //< Write completed status
    uint16_t               signal;          //< Write signal
  }                        write;           //< Write status
  struct {
    uint16_t               signal;          //< Read signal
  }                        read;            //< Read status
} transport_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

// CPC init
static void cpc_endpoint_init(void);
// CPC connection callback
static void cpc_on_connect(uint8_t endpoint_id, void *arg);
// CPC transmit completed callback
static void cpc_on_tx(sl_cpc_user_endpoint_id_t endpoint_id,
                      void *buffer,
                      void *arg,
                      sl_status_t status);
// CPC receive completed callback
static void cpc_on_rx(uint8_t endpoint_id, void *arg);
// CPC error callback
static void cpc_on_error(uint8_t endpoint_id, void *arg);
// Error callback
static void on_runtime_error(app_rta_error_t error, sl_status_t result);

// -----------------------------------------------------------------------------
// Private variables

// Internal state
static transport_t transport;

// -----------------------------------------------------------------------------
// Public functions (API implementation)

void sl_bt_ncp_transport_transmit(uint32_t len, const uint8_t *data)
{
  sl_status_t sc;

  if (len > SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_TX_OVERFLOW,
                                 SL_STATUS_WOULD_OVERFLOW);
    return;
  }

  // Acquire guard
  sc = app_rta_acquire(transport.ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }
  // Store data in TX buffer until the write completed callback is called
  transport.write.signal = len;
  memcpy((void *)transport.tx_buf, (void *)data, (size_t)len);

  // Only send msg if primary device has connected to the endpoint.
  if (transport.cpc_connected) {
    (void)sl_cpc_write(&transport.endpoint_handle,
                       transport.tx_buf,
                       transport.write.signal,
                       0,
                       (void *)cpc_on_tx);
    transport.write.signal = 0;
  }
  (void)app_rta_proceed(transport.ctx);
  // Release guard
  (void)app_rta_release(transport.ctx);
}

void sl_bt_ncp_transport_receive(void)
{
  // Not needed in CPC communication
}

// -----------------------------------------------------------------------------
// Internal API implementation

void sli_bt_ncp_transport_init(void)
{
  sl_status_t sc;
  app_rta_config_t config = {
    .requirement.runtime = true,
    .requirement.guard   = true,
    .requirement.signal  = false,
    .requirement.queue   = false,
    .step                = sli_bt_ncp_transport_step,
    .priority            = SL_BT_NCP_TRANSPORT_CPC_CONFIG_TASK_PRIO,
    .stack_size          = SL_BT_NCP_TRANSPORT_CPC_CONFIG_TASK_STACK,
    .error               = on_runtime_error,
    .wait_for_guard      = SL_BT_NCP_TRANSPORT_CPC_CONFIG_WAIT_FOR_GUARD,
  };
  sc = app_rta_create_context(&config, &transport.ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
  }
}

void sli_bt_ncp_transport_rta_ready(void)
{
  cpc_endpoint_init();
  (void)app_rta_proceed(transport.ctx);
}

// Step function (process action)
void sli_bt_ncp_transport_step(void)
{
  sl_status_t sc;

  sc = app_rta_acquire(transport.ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }

  // Check if the endpoint is open. If not, we need to reopen it,
  // but first we need to check if it has been freed by CPC
  if (!transport.cpc_opened) {
    if (sl_cpc_get_endpoint_state(&transport.endpoint_handle) == SL_CPC_STATE_FREED) {
      cpc_endpoint_init();
    } else {
      (void)app_rta_proceed(transport.ctx);
      (void)app_rta_release(transport.ctx);
      return;
    }
  }

  // If tx buffer is not empty, and primary device has connected, transmit it
  if (transport.write.signal > 0) {
    if (transport.cpc_connected) {
      (void)sl_cpc_write(&transport.endpoint_handle, transport.tx_buf, transport.write.signal, 0, NULL);
      transport.write.signal = 0;
    }
    (void)app_rta_proceed(transport.ctx);
  }

  if (transport.read.signal > 0) {
    sl_status_t status;
    uint8_t *data;
    uint16_t data_length;
    status = sl_cpc_read(&transport.endpoint_handle, (void **)&data, &data_length, 0, 0);
    if (status == SL_STATUS_OK) {
      // Send data to upper layers
      sl_bt_ncp_transport_on_receive(status, data_length, data);
      sl_cpc_free_rx_buffer(data);
    }
    transport.read.signal--;
    (void)app_rta_proceed(transport.ctx);
  }

  if (transport.write.completed > 0) {
    if (transport.cpc_connected) {
      memset(transport.tx_buf, 0, sizeof(transport.tx_buf));
      sl_bt_ncp_transport_on_transmit(transport.write.completed);
      transport.write.completed = SL_STATUS_FAIL;
      transport.write.completed--;
      (void)app_rta_proceed(transport.ctx);
    }
  }

  // Release guard
  (void)app_rta_release(transport.ctx);
}

// -----------------------------------------------------------------------------
// Weak implementation of callbacks

SL_WEAK void sl_bt_ncp_transport_on_transmit(sl_status_t status)
{
  (void)status;
}

SL_WEAK void sl_bt_ncp_transport_on_receive(sl_status_t status,
                                            uint32_t    len,
                                            uint8_t     *data)
{
  (void)status;
  (void)data;
  (void)len;
}

SL_WEAK void sl_bt_ncp_transport_on_error(sl_bt_ncp_transport_error_t error,
                                          sl_status_t                 status)
{
  (void)error;
  (void)status;
  #ifdef SL_CATALOG_APP_ASSERT_PRESENT
  app_assert_status_f(status,
                      "blutetooth_ncp_transport: Error %u occurred.",
                      error);
  #endif // SL_CATALOG_APP_ASSERT_PRESENT
}

// -----------------------------------------------------------------------------
// Private functions

// CPC init
static void cpc_endpoint_init(void)
{
  sl_status_t status;

  // Clear TX buffer
  memset(transport.tx_buf, 0, sizeof(transport.tx_buf));

  status = sli_cpc_open_service_endpoint(&transport.endpoint_handle,
                                         SL_CPC_ENDPOINT_BLUETOOTH, 0, 1);
  if (status != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_INIT, status);
  }
  status = sl_cpc_set_endpoint_option(&transport.endpoint_handle,
                                      SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED,
                                      (void *)cpc_on_tx);
  if (status != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_INIT, status);
  }
  status = sl_cpc_set_endpoint_option(&transport.endpoint_handle,
                                      SL_CPC_ENDPOINT_ON_IFRAME_RECEIVE,
                                      (void *)cpc_on_rx);
  if (status != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_INIT, status);
  }
  status = sl_cpc_set_endpoint_option(&transport.endpoint_handle,
                                      SL_CPC_ENDPOINT_ON_ERROR,
                                      (void*)cpc_on_error);
  if (status != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_INIT, status);
  }
  status = sl_cpc_set_endpoint_option(&transport.endpoint_handle,
                                      SL_CPC_ENDPOINT_ON_CONNECT,
                                      (void*)cpc_on_connect);
  if (status != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_INIT, status);
  }
  transport.cpc_opened = true;
}

// CPC connection callback
static void cpc_on_connect(uint8_t endpoint_id, void *arg)
{
  (void)endpoint_id;
  (void)arg;
  sl_status_t sc;
  // Acquire guard
  sc = app_rta_acquire(transport.ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }
  transport.cpc_connected = true;
  (void)app_rta_proceed(transport.ctx);
  // Release guard
  (void)app_rta_release(transport.ctx);
}

// CPC transmit completed callback
static void cpc_on_tx(sl_cpc_user_endpoint_id_t endpoint_id,
                      void *buffer,
                      void *arg,
                      sl_status_t status)
{
  (void)(endpoint_id);
  (void)(buffer);
  (void)(arg);
  sl_status_t sc;
  // Acquire guard
  sc = app_rta_acquire(transport.ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }
  CORE_ATOMIC_SECTION(
    transport.write.completed = status;
    transport.write.completed++;
    )
    (void) app_rta_proceed(transport.ctx);
  // Release guard
  (void)app_rta_release(transport.ctx);
}

// CPC receive completed callback
static void cpc_on_rx(uint8_t endpoint_id, void *arg)
{
  (void)endpoint_id;
  (void)arg;
  sl_status_t sc;
  // Acquire guard
  sc = app_rta_acquire(transport.ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }
  transport.read.signal++;
  (void)app_rta_proceed(transport.ctx);
  // Release guard
  (void)app_rta_release(transport.ctx);
}

// CPC error callback
static void cpc_on_error(uint8_t endpoint_id, void *arg)
{
  (void)endpoint_id;
  (void)arg;
  sl_status_t sc;
  // Acquire guard
  sc = app_rta_acquire(transport.ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }
  uint8_t state = sl_cpc_get_endpoint_state(&transport.endpoint_handle);
  if (state == SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE) {
    sl_status_t status = sl_cpc_close_endpoint(&transport.endpoint_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    if (status != SL_STATUS_OK) {
      sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_CLOSE, sc);
      (void)app_rta_release(transport.ctx);
      return;
    }
    transport.cpc_opened = false;
    transport.cpc_connected = false;
  }
  (void)app_rta_proceed(transport.ctx);
  // Release guard
  (void)app_rta_release(transport.ctx);
}

// Error callback
static void on_runtime_error(app_rta_error_t error, sl_status_t result)
{
  (void)error;
  sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, result);
}
