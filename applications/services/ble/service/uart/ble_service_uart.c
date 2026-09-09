#include "ble_service_uart.h"

#include "../ble_service_i.h"
#include <furi_hal_info.h>
#include <stdint.h>

#define TAG "BleUart"

// Nordic UART Service UUIDs
// UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
// UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
// UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_SERVICE_UUID \
    {0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}
#define UART_RX_CHAR_UUID \
    {0x6E, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}
#define UART_TX_CHAR_UUID \
    {0x6E, 0x40, 0x00, 0x03, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}
#define UART_CNT_CHAR_UUID \
    {0x6E, 0x40, 0x00, 0x04, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}

// HM-10 Bluetooth UUIDs
// HM10_UART_SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb"
// HM10_UART_TX_CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
// HM10_UART_RX_CHAR_UUID = "0000ffe2-0000-1000-8000-00805f9b34fb"
#define HM10_UART_SERVICE_UUID \
    {0x00, 0x00, 0xFF, 0xE0, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}
#define HM10_UART_TX_CHAR_UUID \
    {0x00, 0x00, 0xFF, 0xE1, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}
#define HM10_UART_RX_CHAR_UUID \
    {0x00, 0x00, 0xFF, 0xE2, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}

#define NORDIC_UART_INITIAL_DATA_SIZE (237) //RSI_DEV_ATT_LEN - ATTRIBUTE_HEADER_SIZE (240-3 = 237)
#define HM10_UART_INITIAL_DATA_SIZE   (237)

static bool ble_service_uart_init(void* object) {
    furi_assert(object);
    BLE_LOG_D("uart_init");
    BleServiceObject* instance = object;
    UNUSED(instance);

    return true;
}

static const BleCharacteristicConfig nordic_uart_service_characteristics[] = {
    {
        .intercom_index = BleUartCharacteristicIndexRx,
        .name = "Uart Rx",
        .initial_data_size = NORDIC_UART_INITIAL_DATA_SIZE,
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_128 = UART_RX_CHAR_UUID},
        .uuid_size = 16,
        .char_properties = BLE_ATT_PROPERTY_WRITE,
#endif
    },
    {
        .intercom_index = BleUartCharacteristicIndexTx,
        .name = "Uart Tx",
        .initial_data_size = NORDIC_UART_INITIAL_DATA_SIZE,
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_128 = UART_TX_CHAR_UUID},
        .uuid_size = 16,
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_INDICATE,
#endif
    },
    {
        .intercom_index = BleUartCharacteristicIndexSession,
        .name = "Uart Cnt",
        .initial_data_size = sizeof(uint32_t),
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_128 = UART_CNT_CHAR_UUID},
        .uuid_size = 16,
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_WRITE,
#endif
    },
};

//==========================================================

const BleServiceConfig ble_service_config_nordic_uart = {
    .name = "Nordic UART",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_128 = UART_SERVICE_UUID},
    .uuid_size = 16,
#endif
    .index = BleServiceIndexNordicUart,
    .char_count = COUNT_OF(nordic_uart_service_characteristics),
    .char_configs = nordic_uart_service_characteristics,
    .init = ble_service_uart_init,
};

//==========================================================
//==========================================================

static const BleCharacteristicConfig hm10_uart_service_characteristics[] = {
    {
        .intercom_index = BleUartCharacteristicIndexRx,
        .name = "HM10 Rx",
        .initial_data_size = HM10_UART_INITIAL_DATA_SIZE,
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_128 = HM10_UART_RX_CHAR_UUID},
        .uuid_size = 16,
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_WRITE,
#endif
    },
    {
        .intercom_index = BleUartCharacteristicIndexTx,
        .name = "HM10 Tx",
        .initial_data_size = HM10_UART_INITIAL_DATA_SIZE,
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_128 = HM10_UART_TX_CHAR_UUID},
        .uuid_size = 16,
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_NOTIFY,
#endif
    },
};

//==========================================================

const BleServiceConfig ble_service_config_hm10_uart = {
    .name = "HM10 UART",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_128 = HM10_UART_SERVICE_UUID},
    .uuid_size = 16,
#endif
    .index = BleServiceIndexHm10Uart,
    .char_count = COUNT_OF(hm10_uart_service_characteristics),
    .char_configs = hm10_uart_service_characteristics,
    .init = ble_service_uart_init,
};
