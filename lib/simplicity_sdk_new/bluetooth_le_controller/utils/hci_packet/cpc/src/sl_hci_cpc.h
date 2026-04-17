#ifndef SL_HCI_CPC_H
#define SL_HCI_CPC_H

#include "sl_common.h"
#include "sl_status.h"

// Enum of CPC states
typedef enum {
  SL_HCI_CPC_STATE_DISCONNECTED = 0,
  SL_HCI_CPC_STATE_CONNECTING   = 1,   // sl_cpc_listen_endpoint() called, SL_CPC_ENDPOINT_ON_CONNECT callback not yet invoked
  SL_HCI_CPC_STATE_CONNECTED    = 2,   // SL_CPC_ENDPOINT_ON_CONNECT callback invoked
} sl_hci_cpc_state_t;

void sl_hci_cpc_init(void);
int sl_hci_cpc_read(uint8_t **read_buf);
void sl_hci_cpc_free(void *buf);
void sl_hci_cpc_rx_done();
sl_status_t sl_hci_cpc_write(uint8_t *data, uint16_t len);
int sl_hci_cpc_new_data();
void sl_hci_cpc_on_connect(uint8_t endpoint_id, void *arg);
void sl_hci_cpc_error(uint8_t endpoint_id, void *arg);
bool sl_hci_is_cpc_connected(void);
void sl_hci_reconnect_cpc(void);

#endif // SL_HCI_CPC_H
