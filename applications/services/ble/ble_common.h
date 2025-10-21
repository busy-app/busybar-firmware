#pragma once

#include "ble_state.h"
#include "service/ble_service_index.h"

#include <furi.h>
#include <intercom/intercom.h>

// #define BLE_DEBUG

#ifdef BLE_DEBUG
#define BLE_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define BLE_LOG_D(...)
#endif

#define BLE_LOG_I(...) FURI_LOG_I(TAG, __VA_ARGS__)
#define BLE_LOG_W(...) FURI_LOG_W(TAG, __VA_ARGS__)

typedef enum {
    BleIntercomFrameTypeRequest,
    BleIntercomFrameTypeResponse,
    BleIntercomFrameTypeNotification,
} BleIntercomFrameType;

//==========================================================================================================
typedef enum {
    BleCommandInit,
    BleCommandEnable,
    BleCommandDisable,
    BleCommandGetState,
    //-------------------------------------
    BleCommandServiceInit,
    BleCommandServiceRun,
    BleCommandServiceUpdate,
    //-------------------------------------
    BleCommandServiceProcessFrame,
} BleCommand;

typedef struct {
    BleCommand command;
    uint16_t service_index;
    uint16_t char_index;
} BleServiceCommand;
//==========================================================================================================

typedef struct /*FURI_PACKED*/ {
    BleIntercomFrameType frame_type;
    BleCommand command;
    BleServiceIndex service_index;
    size_t data_size;
} BleIntercomFrameHeader;

#define MAX_BLE_INTERCOM_FRAME_SIZE (512U - sizeof(BleIntercomFrameHeader))

typedef struct {
    BleIntercomFrameHeader header;
    uint8_t data[MAX_BLE_INTERCOM_FRAME_SIZE];
} BleIntercomFrameGeneric;

typedef struct {
    BleIntercomFrameHeader header;
    BleServiceState state;
} BleIntercomFrameStatus;

typedef struct {
    uint8_t index;
    uint8_t data_size;
} BleCharacteristicDataHeader;

typedef struct /*FURI_PACKED*/ {
    BleCharacteristicDataHeader header;
    uint8_t data[];
} BleCharacteristicData;

typedef uint8_t BleCharacteristicCountType;

typedef struct {
    BleCharacteristicCountType char_count;
    BleCharacteristicData chars_config[];
} BleIntercomServiceData;

typedef struct /*FURI_PACKED*/ {
    BleIntercomFrameHeader header;
    BleIntercomServiceData service_init;
} BleIntercomFrameServiceConfig;

//=============================================
