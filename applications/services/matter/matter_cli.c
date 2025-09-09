#include <matter/matter.h>

#include <containers/pipe.h>
#include <cli/args.h>
#include <cli/cli_command.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>

typedef struct {
    MatterSrv* matter;
    CliRegistry* commands;
    CliShell* shell;
    FuriPubSubSubscription* subscription;
} MatterCli;

// ============
// Sub-commands
// ============

static void matter_cli_cmd_set_print_usage(void) {
    printf("Usage: set <device> <state>\r\n");
    printf("  device: switch1\r\n");
    printf("  state: on|off\r\n");
}

static void matter_cli_cmd_set(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    furi_assert(context);
    MatterCli* matter_cli = context;

    static const char* const device_names[MatterVirtualDeviceMAX] = {
        [MatterVirtualDeviceSwitch1] = "switch1",
        [MatterVirtualDeviceSwitch2] = "switch2",
    };
    static const char* const switch_states[2] = {
        "off",
        "on",
    };

    MatterVirtualDeviceState state;

    FuriString* arg = furi_string_alloc();

    do {
        // parse device name

        if(!args_read_string_and_trim(args, arg)) {
            matter_cli_cmd_set_print_usage();
            break;
        }

        for(MatterVirtualDevice device = 0; device < MatterVirtualDeviceMAX; device++) {
            if(furi_string_cmpi_str(arg, device_names[device]) == 0) {
                state.device = device;
                break;
            }
        }

        // parse device state

        if(!args_read_string_and_trim(args, arg)) {
            matter_cli_cmd_set_print_usage();
            break;
        }
        bool did_parse_state = false;

        switch(state.device) {
        case MatterVirtualDeviceSwitch1:
        case MatterVirtualDeviceSwitch2: {
            for(size_t i = 0; i < COUNT_OF(switch_states); i++) {
                if(furi_string_cmpi_str(arg, switch_states[i]) == 0) {
                    state.bool_val = (bool)i;
                    did_parse_state = true;
                    break;
                }
            }
            break;
        }

        case MatterVirtualDeviceMAX:
            furi_crash();
        }

        if(!did_parse_state) {
            matter_cli_cmd_set_print_usage();
            break;
        }

        matter_set_state(matter_cli->matter, state);
    } while(0);

    furi_string_free(arg);
}

static void matter_cli_cmd_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    furi_assert(context);
    MatterCli* matter_cli = context;

    matter_factory_reset(matter_cli->matter);

    printf("Done. Please do a manual hardware reset of both chips.\r\n");
}

// =========
// Utilities
// =========

static void matter_cli_format_device_state(
    MatterCli* matter_cli,
    FuriString* out,
    const MatterVirtualDeviceState* state) {
    UNUSED(matter_cli);
    static const char* const device_names[MatterVirtualDeviceMAX] = {
        [MatterVirtualDeviceSwitch1] = "Switch 1",
        [MatterVirtualDeviceSwitch2] = "Switch 2",
    };
    static const char* const switch_states[2] = {
        ANSI_FG_RED "off" ANSI_RESET,
        ANSI_FG_GREEN "on" ANSI_RESET,
    };

    furi_string_reset(out);
    furi_string_cat_str(out, device_names[state->device]);
    furi_string_cat_str(out, ": ");

    switch(state->device) {
    case MatterVirtualDeviceSwitch1:
    case MatterVirtualDeviceSwitch2:
        furi_string_cat_str(out, switch_states[state->bool_val]);
        break;

    case MatterVirtualDeviceMAX:
        furi_crash();
    }
}

static void matter_cli_print_event(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    const MatterEvent* event = message;
    MatterCli* matter_cli = context;

    FuriString* notification = furi_string_alloc();

    do {
        if(event->type == MatterEventTypeStateUpdate) {
            furi_string_printf(notification, "State update: ");
            FuriString* state = furi_string_alloc();
            matter_cli_format_device_state(matter_cli, state, &event->update.new_state);
            furi_string_cat(notification, state);
            furi_string_free(state);
        }

        cli_shell_notification_print(matter_cli->shell, notification);
    } while(0);

    furi_string_free(notification);
}

// =============
// Command setup
// =============

static void matter_cli_motd(void* context) {
    furi_assert(context);
    MatterCli* matter_cli = context;

    printf("\r\n");
    printf(ANSI_FG_BLACK ANSI_BG_BR_WHITE "   ↓  Matter  " ANSI_RESET "\r\n");
    printf(ANSI_FG_BLACK ANSI_BG_BR_WHITE "  ↗ ↖ CLI     " ANSI_RESET "\r\n");
    printf("\r\n");

    printf("Virtual device state:\r\n");
    FuriString* formatted_state = furi_string_alloc();

    for(MatterVirtualDevice device = 0; device < MatterVirtualDeviceMAX; device++) {
        MatterVirtualDeviceState state = matter_get_state(matter_cli->matter, device);
        matter_cli_format_device_state(matter_cli, formatted_state, &state);
        printf("  %s\r\n", furi_string_get_cstr(formatted_state));
    }

    furi_string_free(formatted_state);
}

static MatterCli* matter_cli_alloc(PipeSide* pipe) {
    MatterCli* matter_cli = malloc(sizeof(MatterCli));

    matter_cli->matter = furi_record_open(RECORD_MATTER);
    matter_cli->commands = cli_registry_alloc();
    matter_cli->shell =
        cli_shell_alloc(matter_cli_motd, matter_cli, pipe, matter_cli->commands, NULL);
    cli_shell_set_prompt(matter_cli->shell, "matter");

    FuriPubSub* pubsub = matter_get_pubsub(matter_cli->matter);
    matter_cli->subscription = furi_pubsub_subscribe(pubsub, matter_cli_print_event, matter_cli);

    return matter_cli;
}

static void matter_cli_free(MatterCli* matter_cli) {
    furi_assert(matter_cli);

    FuriPubSub* pubsub = matter_get_pubsub(matter_cli->matter);
    furi_pubsub_unsubscribe(pubsub, matter_cli->subscription);

    cli_shell_free(matter_cli->shell);
    cli_registry_free(matter_cli->commands);
    furi_record_close(RECORD_MATTER);
}

void matter_cli_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    MatterCli* matter_cli = matter_cli_alloc(pipe);

    cli_registry_add_command(
        matter_cli->commands,
        "set",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_set,
        matter_cli);
    cli_registry_add_command(
        matter_cli->commands,
        "reset",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_reset,
        matter_cli);

    cli_shell_start(matter_cli->shell);
    cli_shell_join(matter_cli->shell);

    matter_cli_free(matter_cli);
}
