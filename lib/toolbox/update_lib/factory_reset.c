#include "factory_reset.h"

#include <ble/ble.h>
#include <matter/matter.h>

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

static void reset_ble_pairing(void) {
#ifndef FW_CFG_recovery
    printf("Resetting BLE pairing...\r\n");

    if(furi_record_exists(RECORD_BLE)) {
        Ble* ble = furi_record_open(RECORD_BLE);
        ble_forget(ble);
        furi_record_close(RECORD_BLE);

        printf("BLE pairing reset done\r\n");
    } else {
        printf("BLE not ready, skipping\r\n");
    }
#endif
}

static void reset_matter_pairing(void) {
#ifndef FW_CFG_recovery
    printf("Resetting Matter pairing...\r\n");

    if(furi_record_exists(RECORD_MATTER)) {
        Matter* matter = furi_record_open(RECORD_MATTER);
        matter_factory_reset(matter);
        furi_record_close(RECORD_MATTER);
        printf("Matter pairing reset done\r\n");
    } else {
        printf("Matter not ready, skipping\r\n");
    }
#endif
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

    reset_ble_pairing();
    reset_matter_pairing();

    // Wifi settings will be reset here because they live on EMMC
    format_emmc_ext();

#ifndef FURI_DEBUG
    furi_hal_nvm_reset();
#endif

    if(shipping_mode) {
        furi_hal_nvm_set_flag(FuriHalNvmFlagRebootIntoShippingMode);
    }

    reset_firmware_to_backup(updater);
}
