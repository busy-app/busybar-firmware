#include "cli_command_gpio.h"
#include "cli_command_display.h"
#include "cli_command_light_sensor.h"
#include "cli_command_audio.h"
#include "cli_command_sl_cli.h"
#include "cli_command_factory_reset.h"
#include "cli_command_otp.h"
#include "cli_command_rtc.h"

#include <core/thread.h>
#include <core/thread_list.h>
#include <furi_hal.h>
#include <furi_hal_nvm.h>
#include <task_control_block.h>
#include <time.h>
#include <loader/loader.h>
#include <cli/args.h>
#include <cli/cli_commands.h>
#include <furi_hal_info.h>
#include <intercom/intercom.h>
#include <cli/cli_registry.h>
#include <cli/cli_ansi.h>
#include <applications.h>
#include <storage/storage_backup.h>
#include <device_name/device_name.h>
#include <sl_info/sl_info.h>

#ifdef SRV_CLI_SOCKET
#include <cli_socket/cli_socket.h>
#include <cli_socket/settings/sysctl_settings.h>
#endif

static void cli_command_update_debug_mode(void) {
    CliRegistry* registry = furi_record_open(RECORD_CLI);

    // Check if debug is enabled
    if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        // Re-register debug commands
        cli_registry_add_command(
            registry, "gpio", CliCommandFlagParallelSafe, cli_command_gpio, NULL);
        // cli_registry_add_command(registry, "sl_echo", CliCommandFlagParallelSafe, cli_command_sl_echo, NULL);
        cli_registry_add_command(
            registry, "factory_reset", CliCommandFlagParallelSafe, cli_command_factory_reset, NULL);
        cli_registry_add_command(
            registry, "otp", CliCommandFlagParallelSafe, cli_command_otp, NULL);
    } else {
        // Remove debug commands
        cli_registry_delete_command(registry, "gpio");
        // cli_registry_delete_command(registry, "sl_echo");
        cli_registry_delete_command(registry, "factory_reset");
        cli_registry_delete_command(registry, "otp");
    }

    furi_record_close(RECORD_CLI);
}

static void cli_command_sysctl_debug(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(furi_string_equal_str(args, "0")) {
        furi_hal_nvm_reset_flag(FuriHalNvmFlagDebug);
        printf("Debug disabled.");
    } else if(furi_string_equal_str(args, "1")) {
        furi_hal_nvm_set_flag(FuriHalNvmFlagDebug);
        printf("Debug enabled.");
    } else {
        cli_print_usage("sysctl debug", "<1|0>", furi_string_get_cstr(args));
    }

    cli_command_update_debug_mode();
}

static void
    cli_command_sysctl_storage_bkp_unlock(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(furi_string_equal_str(args, "0")) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        storage_backup_set_readonly(storage, true);
        furi_record_close(RECORD_STORAGE);
        printf("Backup storage locked.");
    } else if(furi_string_equal_str(args, "1")) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        storage_backup_set_readonly(storage, false);
        furi_record_close(RECORD_STORAGE);
        printf("Backup storage unlocked.");
    } else {
        cli_print_usage("sysctl storage_bkp_unlock", "<1|0>", furi_string_get_cstr(args));
    }
}

#ifdef SRV_CLI_SOCKET
static void cli_command_sysctl_cli_wifi_enabled(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    SysctlSettings settings;
    sysctl_settings_load(&settings);

    if(furi_string_equal_str(args, "0")) {
        settings.cli_wifi_enabled = false;
        sysctl_settings_save(&settings);
        cli_socket_set_wifi_enabled(false);
        printf("CLI over WiFi disabled.");
    } else if(furi_string_equal_str(args, "1")) {
        settings.cli_wifi_enabled = true;
        sysctl_settings_save(&settings);
        cli_socket_set_wifi_enabled(true);
        printf("CLI over WiFi enabled.");
    } else {
        cli_print_usage("sysctl cli_wifi_enabled", "<1|0>", furi_string_get_cstr(args));
    }
}
#endif // SRV_CLI_SOCKET

static void cli_command_sysctl_print_usage() {
    printf("Usage:\r\n");
    printf("sysctl <cmd>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdebug - enables or disables debug mode\r\n");
#ifdef SRV_CLI_SOCKET
    printf("\tcli_wifi_enabled - enables or disables CLI access over WiFi\r\n");
#endif

    if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        printf("\tstorage_bkp_unlock - locks or unlocks backup storage\r\n");
    }
}

static void cli_command_sysctl(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            cli_command_sysctl_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "debug") == 0) {
            cli_command_sysctl_debug(pipe, args, context);
            break;
        }

#ifdef SRV_CLI_SOCKET
        if(furi_string_cmp_str(cmd, "cli_wifi_enabled") == 0) {
            cli_command_sysctl_cli_wifi_enabled(pipe, args, context);
            break;
        }
#endif

        if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
            if(furi_string_cmp_str(cmd, "storage_bkp_unlock") == 0) {
                cli_command_sysctl_storage_bkp_unlock(pipe, args, context);
                break;
            }
        }

        cli_command_sysctl_print_usage();
    } while(false);

    furi_string_free(cmd);
}
static void
    cli_command_device_info_callback(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-30s: %s\r\n", key, value);
}

static void cli_command_device_info_print_name() {
    FuriString* name = furi_string_alloc();
#ifdef SRV_NAME
    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    device_name_get(device_name, name);
    furi_record_close(RECORD_DEVICE_NAME);
#else // SRV_NAME
    furi_string_set_str(name, "BUSY Bar");
#endif // SRV_NAME
    cli_command_device_info_callback("name", furi_string_get_cstr(name), false, NULL);
    furi_string_free(name);
}

static void cli_command_device_info(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    cli_command_device_info_print_name();
    furi_hal_info_get(cli_command_device_info_callback, '_', NULL);
#ifdef SRV_SL_INFO
    const SlInfo* sl_info = furi_record_open(RECORD_SL_INFO);
    const SlInfoStatus sl_info_status =
        sl_info_get_values(sl_info, cli_command_device_info_callback, NULL);
    furi_record_close(RECORD_SL_INFO);
#else // SRV_SL_INFO
    const SlInfoStatus sl_info_status = SlInfoStatusNotReady;
#endif // SRV_SL_INFO
    printf(
        "%-30s: %s\r\n",
        "sl_intercom_status",
        (sl_info_status == SlInfoStatusOk) ? "ok" : "error");
}

static void cli_commands_init(CliRegistry* registry) {
    cli_registry_add_command(
        registry, "device_info", CliCommandFlagParallelSafe, cli_command_device_info, NULL);

    cli_registry_add_command(
        registry, "sysctl", CliCommandFlagParallelSafe, cli_command_sysctl, NULL);

    cli_registry_add_command(
        registry, "display", CliCommandFlagParallelSafe, cli_command_display, NULL);
    cli_registry_add_command(
        registry, "light_sensor", CliCommandFlagParallelSafe, cli_command_light_sensor, NULL);
#ifdef SRV_AUDIO
    cli_registry_add_command(
        registry, "audio", CliCommandFlagParallelSafe, cli_command_audio, NULL);
#endif // SRV_AUDIO

#ifdef SRV_INTERCOM
    cli_registry_add_command(
        registry,
        "sl_cli",
        CliCommandFlagParallelSafe | CliCommandFlagExclusive,
        cli_command_sl_cli,
        NULL);
#endif // SRV_INTERCOM

#ifdef SRV_TIME
    cli_registry_add_command(
        registry, "date", CliCommandFlagParallelSafe, cli_command_rtc_date, NULL);
    cli_registry_add_command(
        registry, "timezone", CliCommandFlagParallelSafe, cli_command_rtc_timezone, NULL);
#endif // SRV_TIME

    // commands from `.fam`s
    for(size_t i = 0; i < FLIPPER_CLI_COMMANDS_COUNT; i++) {
        const FlipperInternalCommandApplication* command = &FLIPPER_CLI_COMMANDS[i];
        cli_registry_add_command_ex(
            registry, command->name, command->flags, command->callback, NULL, command->stack_size);
    }
}

void cli_on_system_start(void) {
    CliRegistry* registry = cli_registry_alloc();
    cli_commands_init(registry);
    furi_record_create(RECORD_CLI, registry);

    cli_command_update_debug_mode();
}
