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

#include "cli_debug_mode.h"

void cli_command_update_debug_mode(void) {
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
