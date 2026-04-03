#include <matter/matter.h>

#include <containers/pipe.h>
#include <cli/args.h>
#include <cli/cli_command.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>

typedef struct {
    Matter* matter;
    CliRegistry* commands;
    CliShell* shell;
    FuriPubSubSubscription* subscription;
    FuriStateSub* switch_state_sub;
} MatterCli;

typedef struct {
    const char* label;
    const char* color;
} MatterCliSwitchStateDesc;

typedef struct {
    const char* arg;
    const char* help;
} MatterCliCertificationTypeDesc;

static const MatterCliSwitchStateDesc switch_state_descs[MatterSwitchStateMax] = {
    [MatterSwitchStateUnknown] =
        {
            .label = "UNKNOWN",
            .color = ANSI_FG_YELLOW,
        },
    [MatterSwitchStateOff] =
        {
            .label = "OFF",
            .color = ANSI_FG_RED,
        },
    [MatterSwitchStateOn] =
        {
            .label = "ON",
            .color = ANSI_FG_GREEN,
        },
};

static const char* const startup_modes[MatterSwitchStartupModeMAX] = {
    "OFF",
    "ON",
    "TOGGLE",
    "LAST",
};

static const MatterCliCertificationTypeDesc certification_type_descs[MatterCertificationTypeMax] = {
    [MatterCertificationTypeProduction] =
        {
            .arg = "production",
            .help = "for end users",
        },
    [MatterCertificationTypeDevelopment] =
        {
            .arg = "development",
            .help = "for in-house development and testing",
        },
    [MatterCertificationTypeProvisional] =
        {
            .arg = "certification",
            .help = "for performing certification testing",
        },
};

static MatterCertificationType matter_cli_get_cert_type_by_arg(const char* arg) {
    MatterCertificationType cert_type;

    for(cert_type = 0; cert_type < MatterCertificationTypeMax; ++cert_type) {
        if(strcmp(certification_type_descs[cert_type].arg, arg) == 0) {
            break;
        }
    }

    return cert_type;
}

static const char* matter_cli_get_arg_by_cert_type(MatterCertificationType cert_type) {
    return (cert_type < MatterCertificationTypeMax) ? certification_type_descs[cert_type].arg :
                                                      "(not set)";
}

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
        for(i = MatterSwitchStateOff; i < MatterSwitchStateMax; i++) {
            if(furi_string_cmpi(arg, switch_state_descs[i].label) == 0) {
                break;
            }
        }

        if(i == MatterSwitchStateMax) {
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

    MatterCommissioningInfo info;
    const MatterStatus status = matter_enable_commissioning(matter_cli->matter, &info);

    if(status == MatterStatusOk) {
        printf("Manual pairing code : %s\r\n", info.manual_code);
        printf("QR code payload     : %s\r\n", info.qr_code);
        printf("Ready to pair for   : %lu seconds\r\n", info.window_duration_s);

    } else {
        printf(ANSI_FG_RED "failed to enable commissioning\r\n" ANSI_RESET);
    }
}

static void matter_cli_cmd_fabrics(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    furi_assert(context);
    MatterCli* matter_cli = context;

    MatterCommissionedFabrics fabrics;
    const MatterStatus status = matter_get_commissioned_fabrics(matter_cli->matter, &fabrics);

    if(status == MatterStatusOk) {
        if(fabrics.count) {
            printf("device is commissioned to %lu fabrics\r\n", fabrics.count);
        } else {
            printf("device is not commissioned to any fabric\r\n");
        }
    } else {
        printf(ANSI_FG_RED "failed to get fabrics\r\n" ANSI_RESET);
    }
}

static void matter_cli_cmd_cd_print_usage(MatterCli* matter_cli) {
    MatterCertificationConfig cert_config;
    matter_get_certification_config(matter_cli->matter, &cert_config);

    printf("configured CD : %s\r\n", matter_cli_get_arg_by_cert_type(cert_config.wanted));
    printf("effective  CD : %s\r\n", matter_cli_get_arg_by_cert_type(cert_config.actual));

    printf("\r\nuse `cd <certificate_name>` to configure new CD\r\n");
    printf("\r\navailable CDs:\r\n");

    for(uint32_t i = 0; i < COUNT_OF(certification_type_descs); ++i) {
        const MatterCliCertificationTypeDesc* desc = &certification_type_descs[i];
        printf("\t%s:\t%s\r\n", desc->arg, desc->help);
    }
}

static void matter_cli_cmd_cd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    furi_assert(context);
    MatterCli* matter_cli = context;

    const MatterCertificationType cert_type =
        matter_cli_get_cert_type_by_arg(furi_string_get_cstr(args));

    if(cert_type < MatterCertificationTypeMax) {
        if(matter_set_certification_config(matter_cli->matter, cert_type) == MatterStatusOk) {
            printf("Done. Please do a manual hardware reset of both chips.\r\n");
        } else {
            printf("Failed to set configuration");
        }

    } else {
        matter_cli_cmd_cd_print_usage(matter_cli);
    }
}

// =========
// Utilities
// =========

static void matter_cli_format_switch_state(FuriString* out, MatterSwitchState switch_state) {
    furi_assert(switch_state < MatterSwitchStateMax);

    const MatterCliSwitchStateDesc* desc = &switch_state_descs[switch_state];
    furi_string_cat_printf(out, "Switch: %s%s%s", desc->color, desc->label, ANSI_RESET);
}

static void matter_cli_print_event(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    const MatterEvent* event = message;
    MatterCli* matter_cli = context;

    FuriString* notification = furi_string_alloc();

    do {
        if(event->type == MatterEventTypeCommissioning) {
            furi_string_set_str(notification, "Commissioning status: ");
            static const char* state_names[MatterCommissioningStatusMAX] = {
                [MatterCommissioningStatusStarted] = "started",
                [MatterCommissioningStatusFailed] = "failed",
                [MatterCommissioningStatusComplete] = "complete",
            };
            furi_string_cat_str(notification, state_names[event->commissioning.status]);
        } else if(event->type == MatterEventTypeFabricCountChanged) {
            furi_string_printf(notification, "Fabrics count: %zu", event->fabric_count);
        }

        cli_shell_notification_print(matter_cli->shell, notification);
    } while(0);

    furi_string_free(notification);
}

static void matter_cli_switch_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    MatterCli* matter_cli = context;
    FuriString* notification_str = furi_string_alloc();

    const MatterSwitchState switch_state = *(MatterSwitchState*)item;
    matter_cli_format_switch_state(notification_str, switch_state);
    cli_shell_notification_print(matter_cli->shell, notification_str);

    furi_string_free(notification_str);
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

    MatterSwitchState switch_state;
    furi_state_get(matter_get_switch_state(matter_cli->matter), &switch_state);
    matter_cli_format_switch_state(formatted_state, switch_state);

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

    FuriState* switch_state = matter_get_switch_state(matter_cli->matter);
    // FIXME: cli_shell_notification_print() cannot be called from the cli thread,
    //        hence furi_state_get_subscribe() hack
    matter_cli->switch_state_sub =
        furi_state_get_subscribe(switch_state, NULL, matter_cli_switch_state_callback, matter_cli);

    return matter_cli;
}

static void matter_cli_free(MatterCli* matter_cli) {
    furi_assert(matter_cli);

    FuriPubSub* pubsub = matter_get_pubsub(matter_cli->matter);
    furi_pubsub_unsubscribe(pubsub, matter_cli->subscription);

    furi_state_unsubscribe(matter_cli->switch_state_sub);

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
