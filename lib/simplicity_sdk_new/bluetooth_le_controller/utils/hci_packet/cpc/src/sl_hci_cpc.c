#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sl_component_catalog.h"
#include "sl_hci_cpc.h"
#include "sli_cpc.h"
#include "sl_btctrl_linklayer.h"
#include "sl_hci_common_transport.h"
#include "sl_status.h"

static sl_hci_cpc_state_t cpc_state = SL_HCI_CPC_STATE_DISCONNECTED;
static sl_cpc_endpoint_handle_t endpoint_handle;
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_btctrl_hci_cpc_rtos.h"

int sl_hci_cpc_new_data()
{
  return 1;//CPC-RTOS calls read packet only if semaphore is set
}

void sl_hci_cpc_rx_done()
{
  //NOOP: semaphore is used to block
}
#else
static uint32_t read_notifications = 0;
void sl_btctrl_hci_cpc_rx(uint8_t endpoint_id, void *arg)
{
  (void)endpoint_id;
  (void)arg;
  read_notifications++;
}

int sl_hci_cpc_new_data()
{
  return read_notifications;
}

void sl_hci_cpc_rx_done()
{
  EFM_ASSERT(read_notifications > 0);
  read_notifications--;
}
#endif

void sl_hci_cpc_tx_callback(sl_cpc_user_endpoint_id_t endpoint_id, void *buffer, void *arg, sl_status_t status);

void sl_hci_cpc_init(void)
{
  sl_status_t status;

  status = sli_cpc_init_service_endpoint(&endpoint_handle, SL_CPC_ENDPOINT_BLUETOOTH_RCP, 0);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_cpc_set_endpoint_option(&endpoint_handle, SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED, (void *)sl_hci_cpc_tx_callback);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_cpc_set_endpoint_option(&endpoint_handle, SL_CPC_ENDPOINT_ON_IFRAME_RECEIVE, (void *)sl_btctrl_hci_cpc_rx);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_cpc_set_endpoint_option(&endpoint_handle, SL_CPC_ENDPOINT_ON_CONNECT, (void*)sl_hci_cpc_on_connect);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_cpc_set_endpoint_option(&endpoint_handle, SL_CPC_ENDPOINT_ON_ERROR, (void*)sl_hci_cpc_error);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_cpc_listen_endpoint(&endpoint_handle, SL_CPC_FLAG_NO_BLOCK);
  EFM_ASSERT(status == SL_STATUS_OK);
}

void sl_hci_cpc_tx_callback(sl_cpc_user_endpoint_id_t endpoint_id, void *buffer, void *arg, sl_status_t status)
{
  (void)endpoint_id;
  (void)buffer;
  (void)arg;
  hci_common_transport_transmit_complete(status);
}

void sl_hci_cpc_on_connect(uint8_t endpoint_id, void *arg)
{
  (void)endpoint_id;
  (void)arg;

  // Endpoint is connected. Allow message transmission.
  hci_common_transport_transmit_reconnected();
  cpc_state = SL_HCI_CPC_STATE_CONNECTED;
}

void sl_hci_cpc_error(uint8_t endpoint_id, void *arg)
{
  (void)endpoint_id;
  (void)arg;
  sl_status_t status;

  cpc_state = SL_HCI_CPC_STATE_DISCONNECTED;
  status = sl_cpc_terminate_endpoint(&endpoint_handle, 0);
  EFM_ASSERT(status == SL_STATUS_OK);
}

int sl_hci_cpc_read(uint8_t **read_buf)
{
  sl_status_t status;
  uint16_t len;

  status = sl_cpc_read(&endpoint_handle, (void **) read_buf, &len, 0, 0);
  if (status != SL_STATUS_OK) {
    len = 0;
  }
  return len;
}

sl_status_t sl_hci_cpc_write(uint8_t *data, uint16_t len)
{
  return sl_cpc_write(&endpoint_handle, data, len, 0, NULL);
}

void sl_hci_cpc_free(void *buf)
{
  sl_cpc_free_rx_buffer((void *) buf);
}

bool sl_hci_is_cpc_connected(void)
{
  // Any state other than SL_CPC_STATE_CONNECTED is a disconnected state.
  return (cpc_state == SL_HCI_CPC_STATE_CONNECTED);
}

void sl_hci_reconnect_cpc(void)
{
  switch (cpc_state) {
    case SL_HCI_CPC_STATE_DISCONNECTED:
      sl_cpc_listen_endpoint(&endpoint_handle, SL_CPC_FLAG_NO_BLOCK);
      cpc_state = SL_HCI_CPC_STATE_CONNECTING;
      break;

    case SL_HCI_CPC_STATE_CONNECTING:
    case SL_HCI_CPC_STATE_CONNECTED:
      // Nothing to do
      break;

    default:
      // Invalid state
      EFM_ASSERT(false);
  }
}
