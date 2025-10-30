#include "cli_command_factory_reset.h"
#include "power/power_service/power.h"
#include "ble/ble.h"

#include <furi_hal_nvm.h>
#include <storage/storage.h>

static void cli_command_step_format_emmc() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    printf("Format EMMC...\r\n");
    FS_Error error = storage_sd_format(storage, STORAGE_EXT_PATH_PREFIX);

    if(error != FSE_OK) {
        printf("Error: %s", storage_error_get_desc(error));
    } else {
        printf("EMMC was successfully formatted.\r\n");
    }

    furi_record_close(RECORD_STORAGE);
}

static void cli_command_step_reset_pairing() {
    printf("Reset BLE pairing...\r\n");
    Ble* ble = furi_record_open(RECORD_BLE);
    ble_forget(ble);
    furi_record_close(RECORD_BLE);
}

static void cli_command_step_wifi_ble_restore_default_config() {
    // printf("Restore default WiFi/BLE settings...\r\n");
    /// TODO: implement after wifi/ble configs will be implemented
}

void cli_command_factory_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);
    printf("Warning! This will wipe all the data from the device! Are you sure? y/n\r\n");

    while(true) {
        char answer;
        if(pipe_receive(pipe, &answer, sizeof(answer)) != sizeof(answer)) break;
        if(answer == 'n' || answer == 'N') {
            printf("\r\nCancelled.");
            break;
        } else if(answer == 'y' || answer == 'Y') {
            printf("Performing factory reset...\r\n");
            cli_command_step_format_emmc();
            cli_command_step_reset_pairing();
            cli_command_step_wifi_ble_restore_default_config();
            furi_hal_nvm_reset();
            printf("Done\r\nRebooting...\r\n");
            furi_delay_ms(100);
            Power* pwr = furi_record_open(RECORD_POWER);
            power_reboot(pwr, PowerRebootNormal);
            furi_record_close(RECORD_POWER);
            break;
        }
    }
}
