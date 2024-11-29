#include "intercom_rpc.h"

#include <furi.h>
#include <rpc/rpc_handlers.h>

#include "intercom.h"

static void
    intercom_session_send_bytes_callback(void* context, const uint8_t* data, size_t data_len) {
    Intercom* intercom = context;
    intercom_tx(intercom, data, data_len, FuriWaitForever);
}

static void intercom_rx_callback(const void* data, size_t data_size, void* context) {
    RpcSession* intercom_session = context;
    rpc_session_feed(intercom_session, data, data_size, FuriWaitForever);
}

void intercom_on_system_start(void* p) {
    UNUSED(p);

    Rpc* rpc = furi_record_open(RECORD_RPC);
    Intercom* intercom = furi_record_open(RECORD_INTERCOM);

    RpcSession* intercom_session =
        rpc_session_open(rpc, RpcOwnerIntercom, rpc_systems, rpc_system_count);
    rpc_session_set_context(intercom_session, intercom);
    rpc_session_set_send_bytes_callback(intercom_session, intercom_session_send_bytes_callback);
    intercom_set_rx_callback(intercom, intercom_rx_callback, intercom_session);

    furi_record_create(RECORD_INTERCOM_RPC, intercom_session);
}
