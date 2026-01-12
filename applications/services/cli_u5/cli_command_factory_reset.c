#include "cli_command_factory_reset.h"
#include "ble/ble.h"

#include <storage/storage.h>
#include <applications/system/updater/updater.h>

#include <furi_hal_nvm.h>
#include <cli/args.h>

typedef struct {
    bool shipping_mode;
    bool help;
} FactoryResetArgs;

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

static bool parse_command_args(FuriString* args, FactoryResetArgs* parsed_args) {
    parsed_args->shipping_mode = false;
    parsed_args->help = false;

    FuriString* arg = furi_string_alloc();

    bool is_success = true;
    while(args_read_string_and_trim(args, arg)) {
        if(furi_string_equal_str(arg, "-s") || furi_string_equal_str(arg, "--shipping-mode")) {
            parsed_args->shipping_mode = true;
        } else if(furi_string_equal_str(arg, "-h") || furi_string_equal_str(arg, "--help")) {
            parsed_args->help = true;
        } else {
            printf("Unknown argument: %s\r\n", furi_string_get_cstr(arg));
            is_success = false;
            break;
        }
    }

    furi_string_free(arg);
    return is_success;
}

static void print_command_help(void) {
    printf("Usage: factory_reset [options]\r\n");
    printf("Options:\r\n");
    printf("  -s, --shipping-mode    Enter shipping mode after performing reset\r\n");
    printf("  -h, --help             Show this help message\r\n");
}

void cli_command_factory_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    FactoryResetArgs _args;
    if(!parse_command_args(args, &_args) || _args.help) {
        print_command_help();
        return;
    }

    Updater* updater = furi_record_open(RECORD_UPDATER);

    UpdaterStatus update_status = updater_session_start(updater);
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

                if(_args.shipping_mode) {
                    furi_hal_nvm_set_flag(FuriHalNvmFlagRebootIntoShippingMode);
                }

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

    updater_session_stop(updater);

    furi_record_close(RECORD_UPDATER);
}
