#include "cli_command_factory_reset.h"

#include <furi_hal_nvm.h>
#include <storage/storage.h>
#include <applications/system/updater/update.h>

static void format_emmc_ext(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    printf("Format EMMC...\r\n");

    FS_Error fs_status = storage_sd_format(storage, STORAGE_EXT_PATH_PREFIX);

    if(fs_status != FSE_OK) {
        printf("Error: %s", storage_error_get_desc(fs_status));
    } else {
        printf("EMMC was successfully formatted.\r\n");
    }

    furi_record_close(RECORD_STORAGE);
}

static void wifi_ble_reset_pairing(void) {
    // printf("Reset WiFi/BLE pairing...\r\n");
    /// TODO: implement when pairing will be present
}

static void wifi_ble_restore_default_config(void) {
    // printf("Restore default WiFi/BLE settings...\r\n");
    /// TODO: implement after wifi/ble configs will be implemented
}

void cli_command_factory_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

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

            updater_prepare_install(BACKUP_PATH("recovery/update.json"));
            updater_reboot_install();

            break;
        } else if(response == 'n' || response == 'N') {
            printf("\r\nCancelled.");
            break;
        }
    }
}
