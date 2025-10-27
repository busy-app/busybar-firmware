#include "ble_security.h"

#include "../ble_common.h"
#include "nvm/nvm.h"

#define TAG "BleSecurity"

static void ble_security_format_array(FuriString* buf, const uint8_t* data, size_t data_size) {
    for(uint8_t i = 0; i < data_size; i++) {
        furi_string_cat_printf(buf, "%02X ", data[i]);
    }
}

static void ble_security_format_item(
    FuriString* buf,
    const char* header,
    const void* item,
    const size_t size) {
    furi_string_cat_printf(buf, header);
    ble_security_format_array(buf, item, size);
}

void ble_sercurity_format_rpa_data(
    FuriString* output,
    const rsi_bt_event_le_security_keys_t* security) {
    furi_assert(security);
    furi_assert(output);

    furi_string_printf(output, "Dev addr type: %02X", security->dev_addr_type);
    ble_security_format_item(output, "\r\nDev addr: ", security->dev_addr, RSI_DEV_ADDR_LEN);

    furi_string_cat_printf(output, "\r\nIdentity addr type: %02X", security->Identity_addr_type);
    ble_security_format_item(
        output, "\r\nIdentity addr: ", security->Identity_addr, RSI_DEV_ADDR_LEN);

    ble_security_format_item(output, "\r\nLocal IRK: ", security->local_irk, 16);
    ble_security_format_item(output, "\r\nRemote IRK: ", security->remote_irk, 16);

    ble_security_format_item(output, "\r\nRemote Rand: ", security->remote_rand, 16);
    ble_security_format_item(output, "\r\nRemote LTK: ", security->remote_ltk, 16);
}

void ble_security_print_encryption_data(
    FuriString* output,
    const rsi_bt_event_encryption_enabled_t* encryption) {
    furi_assert(output);
    furi_assert(encryption);

    furi_string_printf(
        output,
        "Encryption: %02X, sc: %02X addr type: %02X",
        encryption->enabled,
        encryption->sc_enable,
        encryption->dev_addr_type);

    ble_security_format_item(output, "\r\nDev addr: ", encryption->dev_addr, RSI_DEV_ADDR_LEN);
    ble_security_format_item(output, "\r\nLocal LTK: ", encryption->localltk, 16);

    furi_string_cat_printf(output, "\r\nLocal EDIV: %04X", encryption->localediv);
    ble_security_format_item(output, "\r\nLocal Rand: ", encryption->localrand, 8);
}

bool ble_security_load_data(BleSecurityData* security) {
    furi_assert(security);

    Nvm* nvm = furi_record_open(RECORD_NVM);

    size_t length = 0;
    bool result = false;
    do {
        if(!nvm_exists(nvm, NvmKeyBlePairingData, &length)) {
            BLE_LOG_W("LTK key not exist");
            break;
        }

        const size_t struct_size = sizeof(BleSecurityData);
        if(length != struct_size) {
            BLE_LOG_W("Wrong LTK key size %d != %d", length, struct_size);
            break;
        }

        BLE_LOG_I("Read LTK key struct of %d bytes", length);
        if(!nvm_read(nvm, NvmKeyBlePairingData, security, length)) {
            BLE_LOG_W("Failed to read LTK key");
            break;
        }

        BLE_LOG_I("LTK read done");
        result = true;
    } while(false);

    furi_record_close(RECORD_NVM);
    return result;
}

bool ble_security_save_data(const BleSecurityData* const security) {
    furi_assert(security);
    Nvm* nvm = furi_record_open(RECORD_NVM);
    bool result = nvm_write(nvm, NvmKeyBlePairingData, security, sizeof(BleSecurityData));
    furi_record_close(RECORD_NVM);
    return result;
}

bool ble_security_delete_data() {
    Nvm* nvm = furi_record_open(RECORD_NVM);
    bool result = nvm_delete(nvm, NvmKeyBlePairingData);
    furi_record_close(RECORD_NVM);

    return result;
}
// bool ble_security_init_rpa(rsi_bt_event_le_security_keys_t* rpa_keys) {
//     sl_status_t status = rsi_ble_set_local_irk_value(rpa_keys->local_irk);
//     if(status != RSI_SUCCESS) {
//         BLE_LOG_W("rsi_ble_set_local_irk_value failed: 0x%08lx", status);
//         break;
//     }
// }

bool ble_security_rpa_enable(rsi_bt_event_le_security_keys_t* rpa_keys) {
    bool result = false;
    do {
        uint8_t resp = 0;
        sl_status_t status = rsi_ble_get_resolving_list_size(&resp);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("Failed to get resolving list size: 0x%08lx", status);
            break;
        }
        BLE_LOG_I("Resolving list size: %d", resp);

        status = rsi_ble_resolvlist(
            RSI_BLE_ADD_TO_RESOLVE_LIST,
            rpa_keys->Identity_addr_type,
            rpa_keys->Identity_addr,
            rpa_keys->remote_irk,
            rpa_keys->local_irk);

        if(status != RSI_SUCCESS) {
            BLE_LOG_W("rsi_ble_resolvlist status: 0x%08lx", status);
            break;
        }

        //set address resolution enable and resolvable private address timeout
        status = rsi_ble_set_addr_resolution_enable(1, 10);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("line %d -> status: 0x%lx", __LINE__, status);
            break;
        }

        //set privacy mode
        status = rsi_ble_set_privacy_mode(
            rpa_keys->Identity_addr_type, rpa_keys->Identity_addr, RSI_BLE_DEVICE_PRIVACY_MODE);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("line %d -> status: 0x%lx", __LINE__, status);
            break;
        }

        result = true;
    } while(false);
    return result;
}

bool ble_security_rpa_disable() {
    bool result = false;
    do {
        sl_status_t status = rsi_ble_set_addr_resolution_enable(0, 10);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("line %d -> status: 0x%lx", __LINE__, status);
            break;
        }

        rsi_ble_resolvlist_t dummy = {0};
        status = rsi_ble_resolvlist(
            RSI_BLE_CLEAR_RESOLVE_LIST,
            dummy.remote_dev_addr_type,
            dummy.remote_dev_addr,
            dummy.peer_irk,
            dummy.local_irk);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("Unable to clear resolvlist %08lX", status);
            break;
        }
        BLE_LOG_I("RPA disabled");
        result = true;
    } while(false);
    return result;
}
