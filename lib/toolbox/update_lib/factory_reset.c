#include "factory_reset.h"

#include "ble/ble.h"

#include <furi_hal_nvm.h>

#include <storage/storage.h>
#include <applications/system/updater/updater.h>

static void format_emmc_ext(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    printf("Formatting EMMC...\r\n");

    FS_Error fs_status = storage_sd_format(storage, STORAGE_EXT_PATH_PREFIX);

    if(fs_status != FSE_OK) {
        printf("EMMC formatting error: %s", storage_error_get_desc(fs_status));
    } else {
        printf("EMMC was successfully formatted\r\n");
    }

    furi_record_close(RECORD_STORAGE);
}

static void wifi_ble_reset_pairing(void) {
#ifdef SRV_BLE
    printf("Resetting BLE pairing...\r\n");

    Ble* ble = furi_record_open(RECORD_BLE);
    ble_forget(ble);
    furi_record_close(RECORD_BLE);

    printf("BLE pairing was successfully reset\r\n");
#endif // SRV_BLE
}

static void wifi_ble_restore_default_config(void) {
#ifdef SRV_BLE
// printf("Restore default WiFi/BLE settings...\r\n");
/// TODO: implement after wifi/ble configs will be implemented
#endif // SRV_BLE
}

static void reset_firmware_to_backup(Updater* updater) {
    printf("Resetting firmware to factory default...\r\n");

    do {
        UpdaterStatus installation_prepare_status =
            updater_installation_prepare(updater, BACKUP_PATH("recovery/update.json"), true);
        if(installation_prepare_status != UpdaterStatusOk) {
            printf(
                "Factory reset prepare install failed: %s\r\n",
                updater_get_status_string(installation_prepare_status));
            break;
        }

        printf("Preparation for the installation is complete, device will reboot...\r\n");

        updater_installation_apply(updater, false);
    } while(false);
}

void factory_reset_perform(Updater* updater, bool shipping_mode) {
    printf("Performing factory reset...\r\n");

    format_emmc_ext();
    wifi_ble_reset_pairing();
    wifi_ble_restore_default_config();

#ifndef FURI_DEBUG
    furi_hal_nvm_reset();
#endif

    if(shipping_mode) {
        furi_hal_nvm_set_flag(FuriHalNvmFlagRebootIntoShippingMode);
    }

    reset_firmware_to_backup(updater);
}
