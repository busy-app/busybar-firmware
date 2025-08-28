#pragma once

#include <furi.h>

#define BLE_LOCAL_NAME                "Busybar"
#define BLE_ADVERTISE_PACKET_MAX_SIZE (32)

typedef struct FURI_PACKED {
    uint8_t length;
    uint8_t type;
} BleAdvertiseHeader;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    uint8_t data;
} BleAdvertiseByteData;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    uint16_t data;
} BleAdvertiseWordData;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    char data[sizeof(BLE_LOCAL_NAME)];
} BleAdvertiseLocalName;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    uint16_t data;
} BleAdvertiseServiceClassUUID;

typedef struct FURI_PACKED {
    BleAdvertiseByteData flags;
    BleAdvertiseWordData appearance;
    BleAdvertiseLocalName local_name;
    BleAdvertiseWordData manufacturer;
    BleAdvertiseServiceClassUUID service_class;
} BleAdvertiseConfig;

extern const BleAdvertiseConfig advertise_config;
