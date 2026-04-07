#pragma once

#include <intercom/intercom.h>
#include <intercom/intercom_frame.h>
#include <furi.h>

typedef enum {
    BleIntercomFrameSourceUnknown,
    BleIntercomFrameSourceSystem,
    BleIntercomFrameSourceService,
} BleIntercomFrameSource;

typedef enum {
    BleIntercomFrameTypeUnknown,
    BleIntercomFrameTypeRequest,
    BleIntercomFrameTypeResponse,
} BleIntercomFrameType;

typedef uint8_t BleCommandCode;

typedef struct FURI_PACKED {
    bool result;
    BleCommandCode command;
    uint16_t service_index;

    uint32_t num;
    BleIntercomFrameSource source;
    BleIntercomFrameType frame_type;
    uint32_t data_size;
} BleIntercomFrameHeader;

#define MAX_BLE_INTERCOM_FRAME_SIZE (INTERCOM_FRAME_DATA_SIZE - sizeof(BleIntercomFrameHeader))

typedef struct FURI_PACKED {
    BleIntercomFrameHeader header;
    uint8_t data[MAX_BLE_INTERCOM_FRAME_SIZE];
} BleIntercomFrameGeneric;

//==========================================================================================================

typedef struct FURI_PACKED {
    uint16_t index;
    uint16_t data_size;
    BleIntercomFrameType frame_type;
    uint32_t seq_num;
} BleCharacteristicDataHeader;

typedef struct FURI_PACKED {
    BleCharacteristicDataHeader header;
    uint8_t data[];
} BleCharacteristicData;

typedef uint32_t BleCharacteristicCountType;

typedef struct FURI_PACKED {
    BleCharacteristicCountType char_count;
    BleCharacteristicData chars_config[];
} BleIntercomServiceData;

//=============================================
