#include "cli_commands.h"
#include "cli_command_gpio.h"
#include "cli_command_display.h"
#include "cli_command_status_lights.h"
#include "cli_command_light_sensor.h"
#include "cli_command_audio.h"
#include "cli_command_sl_cli.h"
#include "cli_command_factory_reset.h"

#include <core/thread.h>
#include <core/thread_list.h>
#include <furi_hal.h>
#include <furi_hal_nvm.h>
#include <task_control_block.h>
#include <time.h>
#include <loader/loader.h>
#include <cli/args.h>
#include <furi_hal_info.h>
#include <intercom/intercom.h>
#include <cli/cli_registry.h>
#include <cli/cli_ansi.h>
#include <firmware_applications_f20/applications.h>
#include <storage/storage_backup.h>

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
    } else {
        // Remove debug commands
        cli_registry_delete_command(registry, "gpio");
        // cli_registry_delete_command(registry, "sl_echo");
        cli_registry_delete_command(registry, "factory_reset");
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

static void cli_command_sysctl_bkp_unlock(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(furi_string_equal_str(args, "0")) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        storage_backup_readonly(storage, true);
        furi_record_close(RECORD_STORAGE);
        printf("Backup storage locked.");
    } else if(furi_string_equal_str(args, "1")) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        storage_backup_readonly(storage, false);
        furi_record_close(RECORD_STORAGE);
        printf("Backup storage unlocked.");
    } else {
        cli_print_usage("sysctl bkp_unlock", "<1|0>", furi_string_get_cstr(args));
    }

    cli_command_update_debug_mode();
}

static void cli_command_sysctl_print_usage() {
    printf("Usage:\r\n");
    printf("sysctl <cmd>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdebug - enables or disables debug mode\r\n");

    if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        printf("\tbkp_unlock - locks or unlocks backup storage\r\n");
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

        if(furi_string_cmp_str(cmd, "bkp_unlock") == 0) {
            cli_command_sysctl_bkp_unlock(pipe, args, context);
            break;
        }
        cli_command_sysctl_print_usage();
    } while(false);

    furi_string_free(cmd);
}

static void cli_command_uptime(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);
    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    printf(
        "Uptime: %02lud %02luh %02lum %02lus",
        uptime / 60 / 60 / 24,
        uptime / 60 / 60,
        uptime / 60 % 60,
        uptime % 60);
}

static void cli_command_log_tx_callback(const uint8_t* buffer, size_t size, void* context) {
    PipeSide* pipe = context;
    pipe_send(pipe, buffer, size);
}

static bool cli_command_log_level_set_from_string(FuriString* level) {
    FuriLogLevel log_level;
    if(furi_log_level_from_string(furi_string_get_cstr(level), &log_level)) {
        furi_log_set_level(log_level);
        return true;
    } else {
        printf("<log> — start logging using the current level from the system settings\r\n");
        printf("<log error> — only critical errors and other important messages\r\n");
        printf("<log warn> — non-critical errors and warnings including <log error>\r\n");
        printf("<log info> — non-critical information including <log warn>\r\n");
        printf("<log default> — the default system log level (equivalent to <log info>)\r\n");
        printf(
            "<log debug> — debug information including <log info> (may impact system performance)\r\n");
        printf(
            "<log trace> — system traces including <log debug> (may impact system performance)\r\n");
    }
    return false;
}

static void cli_command_log(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    FuriLogLevel previous_level = furi_log_get_level();
    bool restore_log_level = false;

    if(furi_string_size(args) > 0) {
        if(!cli_command_log_level_set_from_string(args)) {
            return;
        }
        restore_log_level = true;
    }

    const char* current_level;
    furi_log_level_to_string(furi_log_get_level(), &current_level);
    printf("Current log level: %s\r\n", current_level);

    FuriLogHandler log_handler = {
        .callback = cli_command_log_tx_callback,
        .context = pipe,
    };

    furi_log_add_handler(log_handler);

    printf("Use <log ?> to list available log levels\r\n");
    printf("Press CTRL+C to stop...\r\n");
    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_delay_ms(100);
    }

    furi_log_remove_handler(log_handler);

    if(restore_log_level) {
        // There will be strange behaviour if log level is set from settings while log command is running
        furi_log_set_level(previous_level);
    }
}

static void cli_command_top(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    int interval = 1000;
    args_read_int_and_trim(args, &interval);

    FuriThreadList* thread_list = furi_thread_list_alloc();
    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        uint32_t tick = furi_get_tick();
        furi_thread_enumerate(thread_list);

        if(interval) printf(ANSI_CURSOR_POS("1", "1"));

        uint32_t uptime = tick / furi_kernel_get_tick_frequency();
        printf(
            "Threads: %zu, ISR Time: %0.2f%%, Uptime: %luh%lum%lus" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            furi_thread_list_size(thread_list),
            (double)furi_thread_list_get_isr_time(thread_list),
            uptime / 60 / 60,
            uptime / 60 % 60,
            uptime % 60);

        printf(
            "Heap: total %zu, free %zu, minimum %zu, max block %zu" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            memmgr_get_total_heap(),
            memmgr_get_free_heap(),
            memmgr_get_minimum_free_heap(),
            memmgr_heap_get_max_free_block());

        printf(
            "%-17s %-20s %-10s %5s %12s %6s %10s %7s %5s" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            "AppID",
            "Name",
            "State",
            "Prio",
            "Stack start",
            "Stack",
            "Stack Min",
            "Heap",
            "%CPU");

        for(size_t i = 0; i < furi_thread_list_size(thread_list); i++) {
            const FuriThreadListItem* item = furi_thread_list_get_at(thread_list, i);
            printf(
                "%-17s %-20s %-10s %5d   0x%08lx %6lu %10lu %7zu %5.1f" ANSI_ERASE_LINE(
                    ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
                item->app_id,
                item->name,
                item->state,
                item->priority,
                item->stack_address,
                item->stack_size,
                item->stack_min_free,
                item->heap,
                (double)item->cpu);
        }

        printf(ANSI_ERASE_DISPLAY(ANSI_ERASE_FROM_CURSOR_TO_END));
        fflush(stdout);

        if(interval > 0) {
            furi_delay_ms(interval);
        } else {
            break;
        }
    }
    furi_thread_list_free(thread_list);
}

static void cli_command_free(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    printf("Free heap size: %zu\r\n", memmgr_get_free_heap());
    printf("Total heap size: %zu\r\n", memmgr_get_total_heap());
    printf("Minimum heap size: %zu\r\n", memmgr_get_minimum_free_heap());
    printf("Maximum heap block: %zu\r\n", memmgr_heap_get_max_free_block());

    printf("Pool free: %zu\r\n", memmgr_pool_get_free());
    printf("Maximum pool block: %zu\r\n", memmgr_pool_get_max_block());
}

static void cli_command_free_blocks(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    memmgr_heap_printf_free_blocks();
}

static void cli_command_echo(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    printf("%s\r\n", furi_string_get_cstr(args));
}

static void
    cli_command_device_info_callback(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-30s: %s\r\n", key, value);
}

static void cli_command_device_info(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    furi_hal_info_get(cli_command_device_info_callback, '_', NULL);
    cli_command_sl_cli_send_command_get_response(pipe, "device_info");
}

static void cli_commands_init(CliRegistry* registry) {
    cli_registry_add_command(
        registry, "device_info", CliCommandFlagParallelSafe, cli_command_device_info, NULL);
    cli_registry_add_command(
        registry, "uptime", CliCommandFlagParallelSafe, cli_command_uptime, NULL);
    cli_registry_add_command(registry, "log", CliCommandFlagParallelSafe, cli_command_log, NULL);
    cli_registry_add_command(registry, "top", CliCommandFlagParallelSafe, cli_command_top, NULL);
    cli_registry_add_command(registry, "free", CliCommandFlagParallelSafe, cli_command_free, NULL);
    cli_registry_add_command(
        registry, "free_blocks", CliCommandFlagParallelSafe, cli_command_free_blocks, NULL);

    cli_registry_add_command(
        registry, "sysctl", CliCommandFlagParallelSafe, cli_command_sysctl, NULL);

    cli_registry_add_command(registry, "echo", CliCommandFlagDefault, cli_command_echo, NULL);
    cli_registry_add_command(
        registry, "display", CliCommandFlagParallelSafe, cli_command_display, NULL);
    cli_registry_add_command(
        registry, "status_lights", CliCommandFlagParallelSafe, cli_command_status_lights, NULL);
    cli_registry_add_command(
        registry, "light_sensor", CliCommandFlagParallelSafe, cli_command_light_sensor, NULL);
    cli_registry_add_command(
        registry, "audio", CliCommandFlagParallelSafe, cli_command_audio, NULL);
    cli_registry_add_command(
        registry, "sl_cli", CliCommandFlagParallelSafe, cli_command_sl_cli, NULL);

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
