#include <core/thread.h>
#include <core/thread_list.h>
#include <furi_hal.h>
#include <task_control_block.h>
#include <time.h>
#include <cli/args.h>
#include <cli/cli_registry.h>
#include <cli/cli_ansi.h>
#include <cli/cli_commands.h>
#include <applications.h>
#include <device_info/device_info.h>

static void
    cli_command_device_info_callback(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-30s: %s\r\n", key, value);
}

void cli_command_device_info(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    DeviceInfo* dev_info = furi_record_open(RECORD_DEVICE_INFO);
    device_info_query(dev_info, cli_command_device_info_callback, '_', NULL);
    furi_record_close(RECORD_DEVICE_INFO);
}

static void cli_commands_init(CliRegistry* registry) {
    cli_registry_add_command(
        registry, "device_info", CliCommandFlagParallelSafe, cli_command_device_info, NULL);

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
}
