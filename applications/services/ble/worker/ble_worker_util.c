#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "../ble_log.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#define TAG "BleUtil"

#define BLE_TAKE_NEXT_HANDLE_ERROR_MAX (3)

typedef struct FURI_PACKED {
    uint8_t properties;
    uint16_t value_handle;
    uint8_t uuid[];
} BleCharacteristicInfo;

typedef enum {
    BleItemTypeService,
    BleItemTypeCharacteristic,
    BleItemTypeCharacteristicDescriptor,
    BleItemTypeValue,
    BleItemTypeUnknown,
} BleItemType;

static inline void
    ble_data_cat_printf_forward(const uint8_t* data, const uint8_t length, FuriString* output) {
    for(int32_t i = 0; i < length; i++) {
        furi_string_cat_printf(output, "%02X", data[i]);
    }
}

static inline void
    ble_data_cat_printf_reverse(const uint8_t* data, const uint8_t length, FuriString* output) {
    for(int32_t i = length - 1; i >= 0; i--) {
        furi_string_cat_printf(output, "%02X", data[i]);
    }
}

static void ble_data_cat_printf(
    FuriString* output,
    const uint8_t* data,
    const uint8_t length,
    const char* header,
    bool reverse) {
    if(header) furi_string_cat_printf(output, "%s", header);

    if(!reverse)
        ble_data_cat_printf_forward(data, length, output);
    else
        ble_data_cat_printf_reverse(data, length, output);
}

void ble_print_service_hierarchy(void) {
    FuriString* str = furi_string_alloc();

    uint16_t handle = 0x0001;
    uint8_t error_cnt = 0;
    BleItemType expected_type = BleItemTypeService;
    bool descriptor_present = false;
    BLE_LOG_I("=========BLE service hierarchy===========");
    while(true) {
        bool skip_increment = false;
        rsi_ble_resp_local_att_value_t value;
        int32_t res = rsi_ble_get_local_att_value(handle, &value);

        if(error_cnt > BLE_TAKE_NEXT_HANDLE_ERROR_MAX) {
            BLE_LOG_I("Exit");
            break;
        }

        if(res != RSI_SUCCESS || value.handle == 0) {
            BLE_LOG_D("Take next handle, res: %08lX", res);
            handle++;
            error_cnt++;
            continue;
        }

        if((expected_type == BleItemTypeService) && (value.data_len != 2) &&
           (value.data_len != 16) && (value.data_len != 32)) {
            expected_type = BleItemTypeCharacteristic;
        }

        furi_string_cat_printf(str, "Handle: 0x%04X ", value.handle);
        if(expected_type == BleItemTypeService) {
            ble_data_cat_printf(str, value.data, value.data_len, "\e[36mService: \e[0m", true);
            expected_type = BleItemTypeCharacteristic;
        } else if(expected_type == BleItemTypeCharacteristic) {
            BleCharacteristicInfo* char_descr = (BleCharacteristicInfo*)value.data;
            uint8_t uuid_len = value.data_len - 3;

            ble_data_cat_printf(str, char_descr->uuid, uuid_len, "Char: ", true);
            furi_string_cat_printf(
                str,
                " Props: 0x%02X Value_handle: 0x%04X",
                char_descr->properties,
                char_descr->value_handle);

            descriptor_present =
                ((char_descr->properties & RSI_BLE_ATT_PROPERTY_INDICATE) ||
                 (char_descr->properties & RSI_BLE_ATT_PROPERTY_NOTIFY));

            expected_type = BleItemTypeValue;

            if(char_descr->value_handle != 0) {
                handle = char_descr->value_handle;
                skip_increment = true;
            }
        } else if(expected_type == BleItemTypeValue) {
            ble_data_cat_printf(str, value.data, value.data_len, "Data: ", false);
            expected_type = descriptor_present ? BleItemTypeCharacteristicDescriptor :
                                                 BleItemTypeService;
        } else if(expected_type == BleItemTypeCharacteristicDescriptor) {
            ble_data_cat_printf(str, value.data, value.data_len, "Descriptor: ", false);
            furi_string_cat_printf(str, "\n");

            expected_type = BleItemTypeService;
            descriptor_present = false;
        }

        if(!skip_increment) handle++;

        BLE_LOG_I("%s", furi_string_get_cstr(str));
        furi_string_reset(str);
    }
    furi_string_free(str);
}

static bool ble_compare_uuid(const uuid_t* uuid_1, const uuid_t* uuid_2) {
    furi_assert(uuid_1);
    furi_assert(uuid_2);

    bool result = false;
    do {
        if(uuid_1->size != uuid_2->size) break;

        if(uuid_1->size == 2) {
            result = (uuid_1->val.val16 == uuid_2->val.val16);
            break;
        }

        if(uuid_1->size == 16) {
            result = (memcmp(&uuid_1->val.val128, &uuid_2->val.val128, uuid_1->size) == 0);
            break;
        }

    } while(false);

    return result;
}

bool ble_find_characteristic_value_handle_by_uuid(
    const uuid_t* uuid,
    uint16_t last_handle,
    uint16_t* output_handle) {
    uint16_t handle = 0x0001;
    BleItemType expected_type = BleItemTypeService;
    bool found = false;
    while(true) {
        uuid_t buf = {0};
        bool skip_increment = false;
        rsi_ble_resp_local_att_value_t value;
        int32_t res = rsi_ble_get_local_att_value(handle, &value);

        if(handle > last_handle) {
            BLE_LOG_I("Exit");
            break;
        }

        if(res != RSI_SUCCESS || value.handle == 0) {
            BLE_LOG_D("Take next handle");
            handle++;
            continue;
        }

        if((expected_type == BleItemTypeService) && (value.data_len != 2) &&
           (value.data_len != 16) && (value.data_len != 32)) {
            expected_type = BleItemTypeCharacteristic;
        }

        if(expected_type == BleItemTypeService) {
            buf.size = value.data_len;
            memcpy(&buf.val, value.data, buf.size);
            found = ble_compare_uuid(uuid, &buf);

            expected_type = BleItemTypeCharacteristic;
        } else if(expected_type == BleItemTypeCharacteristic) {
            BleCharacteristicInfo* char_descr = (BleCharacteristicInfo*)value.data;

            buf.size = value.data_len - 3;
            memcpy(&buf.val, char_descr->uuid, buf.size);
            found = ble_compare_uuid(uuid, &buf);
            if(found) {
                *output_handle = char_descr->value_handle;
                break;
            }

            if((char_descr->properties & RSI_BLE_ATT_PROPERTY_INDICATE) ||
               (char_descr->properties & RSI_BLE_ATT_PROPERTY_NOTIFY)) {
                expected_type = BleItemTypeCharacteristicDescriptor;
            } else {
                expected_type = BleItemTypeValue;
            }

            if(char_descr->value_handle != 0) {
                handle = char_descr->value_handle;
                skip_increment = true;
            }

        } else if(expected_type == BleItemTypeCharacteristicDescriptor) {
            expected_type = BleItemTypeValue;

        } else if(expected_type == BleItemTypeValue) {
            expected_type = BleItemTypeService;
        }

        if(!skip_increment) handle++;
    }

    return found;
}

static inline void
    ble_worker_util_format_payload(FuriString* output, const uint8_t* data, size_t send_size) {
    furi_string_reset(output);
    for(size_t i = 0; i < send_size; i++) {
        char sym = data[i];
        if(sym == '\n')
            furi_string_cat_printf(output, "\\n");
        else if(sym == '\r')
            furi_string_cat_printf(output, "\\r");
        else if(sym == '\0')
            furi_string_cat_printf(output, "\\0");
        else
            furi_string_cat_printf(output, "%c", sym);
    }
}

void ble_worker_util_log_payload(
    const uint16_t handle,
    const uint8_t chunk_num,
    const uint8_t* data,
    const size_t data_size) {
    FuriString* str = furi_string_alloc();
    ble_worker_util_format_payload(str, data, data_size);
    BLE_LOG_I("H[%04X] C[%d] S[%d]: %s", handle, chunk_num, data_size, furi_string_get_cstr(str));
    furi_string_free(str);
}
