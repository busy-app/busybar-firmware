#include "cli_command_factory_reset.h"
#include <furi_hal_nvm.h>
#include <furi_hal_power.h>

#include <storage/storage.h>
#include <cli/cli_command.h>
#include <cli/args.h>

#include <toolbox/update_lib/update_config.h>
#include <toolbox/update_lib/common_vals.h>

typedef struct {
    bool shipping_mode;
    bool help;
} FactoryResetArgs;

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

static void cli_command_step_format_emmc(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    printf("Formatting EMMC...\r\n");

    FS_Error error = storage_sd_format(storage, STORAGE_EXT_PATH_PREFIX);

    if(error != FSE_OK) {
        printf("EMMC formatting error: %s\r\n", storage_error_get_desc(error));
    } else {
        printf("EMMC was successfully formatted\r\n");
    }

    furi_record_close(RECORD_STORAGE);
}

static void cli_command_step_reset_pairing() {
    // printf("Reset WiFi/BLE pairing...\r\n");
    /// TODO: implement when pairing will be present
}

static void cli_command_step_wifi_ble_restore_default_config() {
    // printf("Restore default WiFi/BLE settings...\r\n");
    /// TODO: implement after wifi/ble configs will be implemented
}

void cli_command_factory_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    FactoryResetArgs _args;
    if(!parse_command_args(args, &_args) || _args.help) {
        print_command_help();
        return;
    }

    printf("Warning! This will wipe all the data from the device! Are you sure? y/n\r\n");

    for(char response; pipe_receive(pipe, &response, sizeof(response)) == sizeof(response);) {
        if(response == 'y' || response == 'Y') {
            printf("Performing factory reset...\r\n");

            cli_command_step_format_emmc();
            cli_command_step_reset_pairing();
            cli_command_step_wifi_ble_restore_default_config();
            furi_hal_nvm_reset();

            if(_args.shipping_mode) {
                furi_hal_nvm_set_flag(FuriHalNvmFlagRebootIntoShippingMode);
            }

            const char* recovery_manifest = BACKUP_PATH("recovery/update.json");
            printf("Setting up recovery installation from: %s...\r\n", recovery_manifest);

            UpdateConfig* state = update_config_alloc();
            Storage* storage = furi_record_open(RECORD_STORAGE);

            do {
                UpdateConfigValidation config_state = update_config_load(state, recovery_manifest);
                if(config_state != UpdateConfigValidationOK) {
                    printf(
                        "Failed to load recovery configuration: %s\r\n",
                        update_config_validation_get_error_str(config_state));
                    printf("Continuing with factory reset anyway...\r\n");
                    break;
                }

                printf("Recovery configuration valid...\r\n");

                if(!update_config_write_pointer_file(storage, recovery_manifest)) {
                    printf("Failed to write manifest path to pointer file\r\n");
                    break;
                }

                printf("Pointer file written successfully...\r\n");

                furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeUpdate);
                printf("Boot mode set to Update. Rebooting...\r\n");
                furi_hal_power_reset();
            } while(false);

            furi_record_close(RECORD_STORAGE);
            update_config_free(state);

            break;
        } else if(response == 'n' || response == 'N') {
            printf("\r\nCancelled");
            break;
        }
    }
}
