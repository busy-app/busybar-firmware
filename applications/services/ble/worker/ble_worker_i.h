#pragma once

#include "ble_worker.h"

#include "device/ble_device.h"
#include "event_processor/ble_incoming_nwp_event_processor.h"
#include "../service/ble_service_i.h"

#include "../util/ble_canary.h"

#include "_nwp_callbacks/ble_nwp_core_callbacks.h"

struct BleWorker {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    BleIncomingNwpEventProcessor* event_proc;
    BleTransmitter* transport;
    BleDevice* device;
    //----------------------------------------------------

    FuriTimer* retry_phy_timer;
    uint8_t pairing_info_available;
    ///TODO: this can be removed

    uint8_t remote_dev_address[6];

    rsi_ble_event_phy_update_t app_phy_update_complete;
    rsi_ble_event_data_length_update_t data_length_update;
    rsi_ble_event_conn_update_t event_conn_update_complete;

    BleSecurityData* security_data;

    BleConnectionStateChanged on_connection_changed_cb;
    void* on_connection_changed_ctx;
};

bool ble_worker_start_advertising(
    bool advertise_to_paired_only,
    const rsi_bt_event_le_security_keys_t* key,
    const BleAdvertiseContext* advertise);

bool ble_worker_stop_advertising();

int32_t ble_worker_write_response(uint8_t* dev_addr, uint8_t type);
