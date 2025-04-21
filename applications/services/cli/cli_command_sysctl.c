#include "cli_command_sysctl.h"
#include "cli_command_gpio.h"
#include <toolbox/args.h>
#include <furi_hal_nvm.h>
#include <storage/storage.h>

static void cli_command_sysctl_print_usage() {
    printf("Usage:\r\n");
    printf("sysctl <cmd>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdebug - enables or disables some debug commands\r\n");
    printf("\tfactory_reset - reset device configs to default\r\n");
}

static void cli_command_sysctl_debug(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);

    if(furi_string_equal_str(args, "0")) {
        cli_delete_command(cli, "gpio");
        printf("Debug disabled.");
    } else if(furi_string_equal_str(args, "1")) {
        cli_add_command(cli, "gpio", CliCommandFlagParallelSafe, cli_command_gpio, NULL);
        furi_hal_rtc_set_flag(FuriHalRtcFlagDebug);
        printf("Debug enabled.");
    } else {
        cli_print_usage("sysctl debug", "<1|0>", furi_string_get_cstr(args));
    }
}

static void cli_command_sysctl_step_format_emmc() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    printf("Format EMMC...\r\n");
    FS_Error error = storage_sd_format(storage);

    if(error != FSE_OK) {
        printf("Error: %s", storage_error_get_desc(error));
    } else {
        printf("EMMC was successfully formatted.\r\n");
    }

    furi_record_close(RECORD_STORAGE);
}

static void cli_command_sysctl_step_reset_pairing() {
    // printf("Reset WiFi/BLE pairing...\r\n");
    /// TODO: implement when pairing will be present
}

static void cli_command_sysctl_step_wifi_ble_restore_default_config() {
    // printf("Restore default WiFi/BLE settings...\r\n");
    /// TODO: implement after wifi/ble configs will be implemented
}

static void cli_command_sysctl_factroy_reset(Cli* cli) {
    printf("Warning! This will wipe all the data from the device! Are you sure? y/n\r\n");

    while(true) {
        char answer = cli_getc(cli);
        if(answer == 'n' || answer == 'N') {
            printf("\r\nCancelled.");
            break;
        } else if(answer == 'y' || answer == 'Y') {
            printf("Performing factory reset...\r\n");
            cli_command_sysctl_step_format_emmc();
            cli_command_sysctl_step_reset_pairing();
            cli_command_sysctl_step_wifi_ble_restore_default_config();
            furi_hal_rtc_reset_registers();
            printf("Done");
            break;
        }
    }
}

void cli_command_sysctl(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            cli_command_sysctl_print_usage();
            break;
        }

        if(furi_string_equal_str(cmd, "debug")) {
            cli_command_sysctl_debug(cli, args, context);
            break;
        }

        if(furi_string_equal_str(cmd, "factory_reset")) {
            cli_command_sysctl_factroy_reset(cli);
            break;
        }

        cli_command_sysctl_print_usage();
    } while(false);

    furi_string_free(cmd);
}
