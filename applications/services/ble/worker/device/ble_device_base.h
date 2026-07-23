/**
 * @file ble_device_base.h
 * @brief Common logic for remote and bsb devices
 */
#pragma once

#include <furi.h>

/**
 * @brief Device address length
 */
#define BLE_DEVICE_ADDRESS_LEN (6)

/**
 * @brief Opaque BleDeviceBase type declaration.
 */
typedef struct BleDeviceBase BleDeviceBase;

/**
 * @brief Enumeration of device roles
 *
 * This roles match with devices roles in ble spec
 * Currently BSB is a remote and phone is a central
 */
typedef enum {
    BleDeviceRoleUnknown, /**< Stub to avoid zero value */
    BleDeviceRoleRemote, /**< Remote device, can only advertise itself but cannot establish any connections */
    BleDeviceRoleCentral, /**< Central device, controls connection establishing and all parameters */

    BleDeviceRoleCount, /**< Roles count */
} BleDeviceRole;

/**
 * @brief Enumeration of device address types
 *
 * Currently only origin is used because this enum 
 * added for future logic implementation
 */
typedef enum {
    BleDeviceAddressTypeUnknown, /**< Stub to avoid zero value */
    BleDeviceAddressTypeOrigin, /**< Public address provided to the device during manufacturing */
    BleDeviceAddressTypeResolvable, /**< Random address generated using rpa */

    BleDeviceAddressTypeCount, /**< Address count */
} BleDeviceAddressType;

/**
 * @brief Features bits enumeration from ble spec
 *
 * In spec there is much more feature bits, but they are 
 * not used in case of this device, so this enum is limited
 */
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

/**
 * @brief Create base device instance with desired role
 *
 * @param[in] role of a new device
 * @return pointer to device instance
 */
BleDeviceBase* ble_device_base_alloc(BleDeviceRole role);

/**
 * @brief Free base device instance
 * @param[in] instance of base device
 */
void ble_device_base_free(BleDeviceBase* instance);

/**
 * @brief Get address of desired type from device
 *
 * If device does not have address it will return array of zeros
 * @param[in] instance of base device
 * @param[in] type desired address type
 * @return pointer to an array with address
 */
const uint8_t* ble_device_base_get_address(BleDeviceBase* instance, BleDeviceAddressType type);

/**
 * @brief Set address and type
 * @param[in] instance of base device
 * @param[in] type address type
 * @param[in] addr array with address bytes must be of @ref BLE_DEVICE_ADDRESS_LEN
 */
void ble_device_base_set_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    const uint8_t* const addr);

/**
 * @brief Converts address to string
 * @param[in] instance of base device
 * @param[in] type address type
 * @param[out] output pointer to string where result will be placed
 */
void ble_device_base_format_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    FuriString* output);

/**
 * @brief Store device features received from nwp internally
 * @param[in] instance of base device
 * @param[in] features array received from nwp during connection establishing
 */
void ble_device_base_set_features(BleDeviceBase* instance, const uint8_t* features);

/**
 * @brief Checks if mentioned feature is supported by device or not
 * @param[in] instance of base device
 * @param[in] feature to be tested
 * @returns true if mentioned feature is supported, otherwise false
 */
bool ble_device_base_is_feature_supported(BleDeviceBase* instance, BleDeviceFeatures feature);
