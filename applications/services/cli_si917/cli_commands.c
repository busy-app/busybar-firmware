#include <cli/cli_registry.h>
#include <cli/cli_commands.h>
#include <applications.h>

static void cli_commands_init(CliRegistry* registry) {
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
