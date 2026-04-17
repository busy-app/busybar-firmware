/***************************************************************************//**
 * @file
 * @brief Bluetooth Network Co-Processor (NCP) Host Communication Interface
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdbool.h>
#include "sl_core.h"
#include "sl_bt_ncp_host.h"
#include "sl_bt_ncp_transport.h"
#include "sli_bt_ncp_transport.h"
#include "sl_ncp_host_com.h"
#include "app_assert.h"
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_WAKE_LOCK_PRESENT)
#include "sl_wake_lock.h"
#endif // SL_CATALOG_WAKE_LOCK_PRESENT

// Reception buffer
typedef struct {
  uint16_t len;
  uint8_t buf[SL_NCP_HOST_COM_BUF_SIZE];
  bool available;
} buf_t;

static volatile bool write_completed = false;
static buf_t buf = { 0 };

extern void sli_bt_ncp_transport_usart_cancel_receive(void);

/******************************************************************************
 * NCP host communication initialization.
 *****************************************************************************/
void sl_ncp_host_com_init(void)
{
  buf.len = 0;
  // Register communication interface functions in adaptation layer
  sl_status_t sc = sl_bt_api_initialize_nonblock(sl_ncp_host_com_write,
                                                 sl_ncp_host_com_read,
                                                 sl_ncp_host_com_peek);
  app_assert(sc == SL_STATUS_OK,
             "[E: 0x%04x] Failed to init Bluetooth NCP\n",
             (int)sc);
}

/******************************************************************************
 * Transmit function
 *
 * Transmits len bytes of data from adaptation layer through transport layer.
 *
 * @param[out] len Message length
 * @param[out] data Message data
 *
 * @note After transmit the reception is automatically started.
 *****************************************************************************/
void sl_ncp_host_com_write(uint32_t len, uint8_t *data)
{
  write_completed = false;
  #if defined(SL_CATALOG_WAKE_LOCK_PRESENT)
  // Wake up other controller
  sl_wake_lock_set_remote_req();
  #endif // SL_CATALOG_WAKE_LOCK_PRESENT
  sl_bt_ncp_transport_transmit(len, data);
  while (!write_completed) {
    sli_bt_ncp_transport_step();
  }
  // Start to receive the response as soon as the transmit is completed
  sli_bt_ncp_transport_usart_cancel_receive();
  // Start to receive the response as soon as the transmit is completed
  sl_bt_ncp_transport_receive();
  // Execute receive request
  sli_bt_ncp_transport_step();
}

/******************************************************************************
 * Receive function
 *
 * Copies received data from transport layer to adaptation layer
 *
 * @param[out] len Message length
 * @param[out] data Message data
 *
 * @return Received message length
 *****************************************************************************/
int32_t sl_ncp_host_com_read(uint32_t len, uint8_t *data)
{
  (void)data;
  // Handle receive
  sli_bt_ncp_transport_step();
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // Check if there is data in the buffer from transport layer
  if (len <= buf.len) {
    // Copy data to adaptation layer
    memcpy((void *)data, (void *)buf.buf, (size_t)len);
    buf.len -= len;
    memmove((void *)buf.buf, (void *)&buf.buf[len], buf.len);
  } else {
    len = -1;
  }
  CORE_EXIT_ATOMIC();
  return len;
}

/******************************************************************************
 * Gives back already received message length.
 *
 * This function checks if data arrived from transport layer. This way the calls
 * can be non blocking.
 *
 * @param[out] len Message length
 * @param[out] data Message data
 *
 * @return Buffer length
 *****************************************************************************/
int32_t sl_ncp_host_com_peek(void)
{
  // Handle receive
  sli_bt_ncp_transport_step();
  return buf.len;
}

/******************************************************************************
 * Transmit completed callback
 *
 * Called after transmission is finished.
 *
 * @param[in] status Status of the transmission
 *****************************************************************************/
void sl_bt_ncp_transport_on_transmit(sl_status_t status)
{
  (void)status;

  #if defined(SL_CATALOG_WAKE_LOCK_PRESENT)
  // Signal other controller that it can go to sleep
  sl_wake_lock_clear_remote_req();
  #endif // SL_CATALOG_WAKE_LOCK_PRESENT

  write_completed = true;
}

/******************************************************************************
 * Receive completed callback
 *
 * Called after reception is finished. Puts the message to the reception
 * buffer.
 *
 * @param[in] status Status of the reception
 * @param[in] len Received message length
 * @param[in] data Data received
 *****************************************************************************/
void sl_bt_ncp_transport_on_receive(sl_status_t status,
                                    uint32_t len,
                                    uint8_t *data)
{
  (void)status;
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // command fits into command buffer; otherwise discard it
  if (len <= (sizeof(buf.buf) - buf.len)) {
    memcpy((void *)&buf.buf[buf.len], (void *)data, (size_t)len);
    buf.len += len;
  }
  CORE_EXIT_ATOMIC();
}

bool sl_ncp_host_is_ok_to_sleep(void)
{
  if (buf.len != 0) {
    return false;
  } else {
    return true;
  }
}
