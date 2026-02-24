#include "cli_command_factory_reset.h"
#include "power/power_service/power.h"

#include <furi_hal_nvm.h>
#include <storage/storage.h>
#include <cli/args.h>

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

            if(_args.shipping_mode) {
                furi_hal_nvm_set_flag(FuriHalNvmFlagRebootIntoShippingMode);
            }

            printf("Done\r\nRebooting...\r\n");
            furi_delay_ms(100);
            Power* pwr = furi_record_open(RECORD_POWER);
            power_reboot(pwr, PowerRebootNormal);
            furi_record_close(RECORD_POWER);
            break;
        }
    }
}
