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

static const char* const switch_states[2] = {
    "OFF",
    "ON",
};

static const char* const switch_state_colors[2] = {
    ANSI_FG_RED,
    ANSI_FG_GREEN,
};

static const char* const startup_modes[MatterSwitchStartupModeMAX] = {
    "OFF",
    "ON",
    "TOGGLE",
    "LAST",
};

// ============
// Sub-commands
// ============

static void matter_cli_cmd_switch_print_usage(void) {
    printf("Usage: switch <state>\r\n");
    printf("  state: on|off\r\n");
}

static void matter_cli_cmd_switch(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    furi_assert(context);
    MatterCli* matter_cli = context;

    FuriString* arg = furi_string_alloc();

    do {
        // parse device state
        if(!args_read_string_and_trim(args, arg)) {
            matter_cli_cmd_switch_print_usage();
            break;
        }

        size_t i;
        for(i = 0; i < COUNT_OF(switch_states); i++) {
            if(furi_string_cmpi(arg, switch_states[i]) == 0) {
                break;
            }
        }

        if(i == COUNT_OF(switch_states)) {
            matter_cli_cmd_switch_print_usage();
            break;
        }

        matter_set_switch_state(matter_cli->matter, i);

    } while(0);

    furi_string_free(arg);
}

static void matter_cli_cmd_startup_print_usage(void) {
    printf("Usage: startup <mode>\r\n");
    printf("  mode: off|on|toggle|last\r\n");
}

static void matter_cli_cmd_startup(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    furi_assert(context);
    MatterCli* matter_cli = context;

    FuriString* arg = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, arg)) {
            matter_cli_cmd_startup_print_usage();
            break;
        }

        size_t i;
        for(i = 0; i < COUNT_OF(startup_modes); i++) {
            if(furi_string_cmpi(arg, startup_modes[i]) == 0) {
                break;
            }
        }

        if(i == COUNT_OF(startup_modes)) {
            matter_cli_cmd_startup_print_usage();
            break;
        }

        matter_set_switch_startup_mode(matter_cli->matter, i);

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

static void matter_cli_cmd_comm(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    furi_assert(context);
    MatterCli* matter_cli = context;

    FuriString* qr_code = furi_string_alloc();
    FuriString* man_code = furi_string_alloc();
    size_t window_len = matter_enable_commissioning(matter_cli->matter, qr_code, man_code);

    if(!window_len) {
        printf(ANSI_FG_RED "failed to enable commissioning\r\n" ANSI_RESET);
    } else {
        printf("Manual pairing code : %s\r\n", furi_string_get_cstr(man_code));
        printf("QR code payload     : %s\r\n", furi_string_get_cstr(qr_code));
        printf("Ready to pair for   : %zu seconds\r\n", window_len);
    }

    furi_string_free(qr_code);
    furi_string_free(man_code);
}

static void matter_cli_cmd_fabrics(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    furi_assert(context);
    MatterCli* matter_cli = context;

    if(matter_is_commissioned(matter_cli->matter)) {
        printf("device is commissioned to one or more fabrics\r\n");
    } else {
        printf("device is not commissioned to any fabric\r\n");
    }
}

static void matter_cli_cmd_cd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    furi_assert(context);
    MatterCli* matter_cli = context;

    if(furi_string_empty(args)) {
        const char* configured = matter_get_wanted_cd_selection(matter_cli->matter);
        if(!strlen(configured)) configured = "<not set>";
        printf("configured CD      : %s\r\n", configured);
        printf(
            "de facto active CD : %s\r\n", matter_get_de_facto_cd_selection(matter_cli->matter));

        printf("\r\nuse `cd <certificate_name>` to configure new CD\r\n");

        printf("\r\navailable CDs:\r\n");
        printf("  dev: for in-house development and testing\r\n");
        printf("  certification: for performing certification testing\r\n");
        printf("  production: for end users\r\n");
        return;
    }

    bool success = matter_set_wanted_cd_selection(matter_cli->matter, furi_string_get_cstr(args));
    if(success) {
        printf("Done. Please do a manual hardware reset of both chips.\r\n");
    } else {
        printf("Failed to set configuration");
    }
}

// =========
// Utilities
// =========

static void matter_cli_format_switch_state(MatterCli* matter_cli, FuriString* out, bool state) {
    UNUSED(matter_cli);

    const char* const state_name = switch_states[state];
    const char* const state_color = switch_state_colors[state];

    furi_string_cat_printf(out, "Switch: %s%s%s", state_color, state_name, ANSI_RESET);
}

static void matter_cli_print_event(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    const MatterEvent* event = message;
    MatterCli* matter_cli = context;

    FuriString* notification = furi_string_alloc();

    do {
        if(event->type == MatterEventTypeSwitchState) {
            matter_cli_format_switch_state(matter_cli, notification, event->switch_state.value);

        } else if(event->type == MatterEventTypeCommissioning) {
            furi_string_set_str(notification, "Commissioning status: ");
            static const char* state_names[MatterCommissioningStatusMAX] = {
                [MatterCommissioningStatusStarted] = "started",
                [MatterCommissioningStatusFailed] = "failed",
                [MatterCommissioningStatusComplete] = "complete",
            };
            furi_string_cat_str(notification, state_names[event->commissioning.status]);
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

    FuriString* formatted_state = furi_string_alloc();

    matter_cli_format_switch_state(
        matter_cli, formatted_state, matter_get_switch_state(matter_cli->matter));
    printf("  %s\r\n", furi_string_get_cstr(formatted_state));

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
        "switch",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_switch,
        matter_cli);
    cli_registry_add_command(
        matter_cli->commands,
        "startup",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_startup,
        matter_cli);
    cli_registry_add_command(
        matter_cli->commands,
        "reset",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_reset,
        matter_cli);
    cli_registry_add_command(
        matter_cli->commands,
        "comm",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_comm,
        matter_cli);
    cli_registry_add_command(
        matter_cli->commands,
        "fabrics",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_fabrics,
        matter_cli);
    cli_registry_add_command(
        matter_cli->commands,
        "cd",
        CliCommandFlagParallelSafe | CliCommandFlagUseShellThread,
        matter_cli_cmd_cd,
        matter_cli);

    cli_shell_start(matter_cli->shell);
    cli_shell_join(matter_cli->shell);

    matter_cli_free(matter_cli);
}
