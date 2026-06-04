#pragma once

#include "ble_worker.h"

#include "ble_security.h"
#include "ble_advertise.h"
#include "event/ble_incoming_nwp_event_processor.h"
#include "../service/ble_service_i.h"

#include "../util/ble_canary.h"

#include "_nwp_callbacks/ble_nwp_core_callbacks.h"

#include <m-dict.h>

#define BLE_WORKER_ATTR_HEADER_SIZE 3

typedef struct {
    ///TODO: for now this is ok, for future maybe it is worth to make each characteristic
    /// know its own service.
    BleServiceObject* service;
    uint16_t char_index;
} BleServiceEntry;

DICT_DEF2(BleServiceEntryDict, uint16_t, M_DEFAULT_OPLIST, BleServiceEntry, M_POD_OPLIST)

typedef enum {
    BleWorkerStateIdle,
    BleWorkerStateAdvertising,
    BleWorkerStateConnected,
    BleWorkerStateError,
} BleWorkerState;

struct BleWorker {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    BleIncomingNwpEventProcessor* event_proc;
    BleTransmitter* transport;
    BleDevice* device;
    //----------------------------------------------------

    FuriSemaphore* receive_sem;
    FuriTimer* retry_phy_timer;
    uint8_t pairing_info_available;
    uint16_t rx_pending_handle;
    ///TODO: this can be removed
    bool connected;

    BleWorkerState state;
    uint16_t max_payload_size;
    uint8_t device_found;
    uint8_t conn_params_updated;
    uint8_t remote_name[31];
    uint8_t remote_addr_type;
    uint8_t remote_dev_str_addr[18];
    uint8_t remote_dev_bd_addr[6];

    uint8_t str_remote_address[18];
    uint8_t remote_dev_address[6];

    rsi_ble_event_phy_update_t app_phy_update_complete;
    rsi_ble_event_data_length_update_t data_length_update;
    rsi_ble_event_conn_update_t event_conn_update_complete;

    rsi_ble_event_remote_features_t remote_dev_feature;

    rsi_ble_event_mtu_t app_ble_mtu_event;

    BleSecurityData* security_data;

    BleServiceEntryDict_t service_dict;
    BleConnectionStateChanged on_connection_changed_cb;
    void* on_connection_changed_ctx;
};

bool ble_worker_start_advertising(
    bool advertise_to_paired_only,
    const rsi_bt_event_le_security_keys_t* key,
    const BleAdvertiseContext* advertise);

bool ble_worker_stop_advertising();

int32_t ble_worker_write_response(uint8_t* dev_addr, uint8_t type);
