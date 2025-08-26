#include "ble_service_config.h"
#include "device_info/ble_service_device_info.h"
#include "battery/ble_service_battery.h"
#include "uart/ble_service_uart.h"

const BleServiceDescriptor* service_config[] = {
    [BleServiceIndexDeviceInfo] = &ble_service_config_device_info,
    [BleServiceIndexBattery] = &ble_service_config_battery,
    [BleServiceIndexUart] = &ble_service_config_nordic_uart,
    [BleServiceIndexSilabsUart] = &ble_service_config_hm10_uart,
};
