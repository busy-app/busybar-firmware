#include "ble_service_config.h"
#include "gap_and_gatt/generic_access.h"
#include "gap_and_gatt/generic_attribute.h"
#include "device_info/ble_service_device_info.h"
#include "battery/ble_service_battery.h"
#include "uart/ble_service_uart.h"

const BleServiceConfig* service_config[BleServiceIndexCount] = {
    [BleServiceIndexGenericAccess] = &ble_service_generic_access,
    [BleServiceIndexGenericAttribute] = &ble_service_generic_attribute,
    [BleServiceIndexDeviceInfo] = &ble_service_config_device_info,
    [BleServiceIndexBattery] = &ble_service_config_battery,
    [BleServiceIndexNordicUart] = &ble_service_config_nordic_uart,
    [BleServiceIndexHm10Uart] = &ble_service_config_hm10_uart,
};
