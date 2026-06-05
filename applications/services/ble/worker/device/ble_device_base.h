#pragma once

#include <furi.h>

#define BLE_DEVICE_ADDRESS_LEN (6)

typedef struct BleDeviceBase BleDeviceBase;

typedef enum {
    BleDeviceRoleUnknown,
    BleDeviceRoleRemote,
    BleDeviceRoleCentral,

    BleDeviceRoleCount,
} BleDeviceRole;

typedef enum {
    BleDeviceAddressTypeUnknown,
    BleDeviceAddressTypeOrigin,
    BleDeviceAddressTypeResolvable,

    BleDeviceAddressTypeCount,
} BleDeviceAddressType;

typedef enum {
    //page 0
    BleDeviceFeaturesLEEncryption = 0,
    BleDeviceFeaturesConnectionParametersRequestProcedure = 1,
    BleDeviceFeaturesExtendedRejectIndication = 2,
    BleDeviceFeaturesPeripheralInitiatedFeaturesExchange = 3,
    BleDeviceFeaturesLEPing = 4,
    BleDeviceFeaturesLEDataPacketLengthExtension = 5,
    BleDeviceFeaturesLLPrivacy = 6,
    BleDeviceFeaturesExtendedScanningFilterPolicies = 7,

    //page 1
    BleDeviceFeaturesLE2MPhy = 8,
    BleDeviceFeaturesStableModulationIndexTransmitter = 9,
    BleDeviceFeaturesStableModulationIndexReceiver = 10,
    BleDeviceFeaturesLECodedPhy = 11,
    BleDeviceFeaturesLEExtendedAdvertising = 12,
    BleDeviceFeaturesLEPeriodicAdvertising = 13,
    BleDeviceFeaturesChannelSelectionAlgorithm2 = 14,
    BleDeviceFeaturesLEPowerClass1 = 15,
} BleDeviceFeatures;

BleDeviceBase* ble_device_base_alloc(BleDeviceRole role);
void ble_device_base_free(BleDeviceBase* instance);

const uint8_t* ble_device_base_get_address(BleDeviceBase* instance, BleDeviceAddressType type);

void ble_device_base_set_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    const uint8_t* const addr);

void ble_device_base_format_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    FuriString* output);

void ble_device_base_set_features(BleDeviceBase* instance, const uint8_t* features);
bool ble_device_base_is_feature_supported(BleDeviceBase* instance, BleDeviceFeatures feature);
