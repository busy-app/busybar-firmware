#include "cli_command_gpio.h"
#include "cli_command_otp.h"

#include <furi_hal.h>
#include <furi_hal_nvm.h>
#include <cli/cli_commands.h>
#include <cli/cli_registry.h>
#include <applications.h>

#include "cli_debug_mode.h"

void cli_command_update_debug_mode(void) {
    CliRegistry* registry = furi_record_open(RECORD_CLI);

    // Check if debug is enabled
    if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        // Re-register debug commands
        cli_registry_add_command(registry, "gpio", CliCommandFlagDefault, cli_command_gpio, NULL);
        cli_registry_add_command(registry, "otp", CliCommandFlagDefault, cli_command_otp, NULL);
    } else {
        // Remove debug commands
        cli_registry_delete_command(registry, "gpio");
        cli_registry_delete_command(registry, "otp");
    }

    furi_record_close(RECORD_CLI);
}

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

    cli_command_update_debug_mode();
}
