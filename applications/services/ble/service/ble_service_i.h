#pragma once

#include "ble_service.h"
#include "ble_characteristic.h"
#include "ble_service_command.h"
#include "ble_service_frame.h"

#include <furi.h>

#define BLE_ATT_PROPERTY_READ     0x02
#define BLE_ATT_PROPERTY_WRITE    0x08
#define BLE_ATT_PROPERTY_NOTIFY   0x10
#define BLE_ATT_PROPERTY_INDICATE 0x20

struct BleServiceObject {
    bool ready;
    const BleServiceConfig* config;
    BleCharacteristicObject** chars;

    FuriMessageQueue* message_queue;
    FuriMutex* service_lock;
    IntercomChannel* intercom_ch;

    BleServiceFrame* output_frame;

    FuriString* error;

    void* context;
    uint32_t sequence_num;
#if defined(BSB_MCU_SI917)
    void* service_handler;
    uint16_t handle;
#endif
};

typedef struct {
    BleServiceObject* service;
    size_t data_size;
} BleServiceObjectMessageHeader;

struct BleServiceObjectMessage {
    BleServiceObjectMessageHeader header;
    uint8_t data[];
};

void ble_service_set_error(BleServiceObject* instance, const char* foramt, ...);

bool ble_service_lock(BleServiceObject* instance);
void ble_service_unlock(BleServiceObject* instance);
void ble_service_send_intercom_frame(BleServiceObject* instance);

size_t ble_service_count_characteristics_and_size(
    BleServiceObject* instance,
    bool modified_only,
    BleCharacteristicCountType* characteristics_count);

BleIntercomServiceData* ble_service_create_intercom_service_data_pack(
    BleServiceObject* instance,
    bool modified_only,
    size_t* output_pack_size);

bool ble_service_parse_intercom_service_data(
    BleServiceObject* instance,
    const BleIntercomServiceData* data);

bool ble_service_send_data(
    BleServiceObject* instance,
    BleServiceCommandEnum command,
    BleIntercomFrameType frame_type,
    bool modified_only);

void ble_service_enqueue_run_with_data(
    BleServiceObject* instance,
    size_t data_size,
    const void* data);
