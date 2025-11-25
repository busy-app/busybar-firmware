#include "cli_command_factory_reset.h"
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
    printf("Resetting BLE pairing...\r\n");

    Ble* ble = furi_record_open(RECORD_BLE);
    ble_forget(ble);
    furi_record_close(RECORD_BLE);

    printf("BLE pairing was successfully reset\r\n");
}

static void wifi_ble_restore_default_config(void) {
    // printf("Restore default WiFi/BLE settings...\r\n");
    /// TODO: implement after wifi/ble configs will be implemented
}

static void reset_firmware_to_backup(Updater* updater) {
    printf("Resetting firmware to factory default...\r\n");

    do {
        UpdaterStatus prepare_install_status =
            updater_prepare_install(updater, BACKUP_PATH("recovery/update.json"), true);
        if(prepare_install_status != UpdaterStatusOk) {
            printf(
                "Factory reset prepare install failed: %s\r\n",
                updater_get_status_string(prepare_install_status));
            break;
        }

        printf("Preparation for the installation is complete, device will reboot...\r\n");

        updater_reboot_install(updater, false);
    } while(false);
}

void cli_command_factory_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    Updater* updater = furi_record_open(RECORD_UPDATER);

    UpdaterStatus update_status = updater_start_update(updater);
    if(update_status == UpdaterStatusOk) {
        printf("Warning! This will wipe all the data from the device! Are you sure? y/n\r\n");

        for(char response; pipe_receive(pipe, &response, sizeof(response)) == sizeof(response);) {
            if(response == 'y' || response == 'Y') {
                printf("Performing factory reset...\r\n");

                format_emmc_ext();
                wifi_ble_reset_pairing();
                wifi_ble_restore_default_config();

#ifndef FURI_DEBUG
                furi_hal_nvm_reset();
#endif

                reset_firmware_to_backup(updater);

                break;
            } else if(response == 'n' || response == 'N') {
                printf("\r\nCancelled");
                break;
            }
        }
    } else {
        printf("Factory reset is not allowed: %s\r\n", updater_get_status_string(update_status));
    }

    updater_stop_update(updater);

    furi_record_close(RECORD_UPDATER);
}
