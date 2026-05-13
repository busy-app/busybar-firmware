#include "sysctl.h"

#include <containers/pipe.h>
#include <furi_hal_nvm.h>
#include <cli/args.h>
#include <cli/cli_command.h>
#include <cli/cli_commands.h>
#include <storage/storage.h>
#include <storage/storage_backup.h>
#include <cli_u5/cli_debug_mode.h>

/* ---------- sysctl variable handlers ---------- */

static bool sysctl_exec_debug(PipeSide* pipe, const char* arg) {
    UNUSED(pipe);
    if(strcmp(arg, "0") == 0) {
        furi_hal_nvm_reset_flag(FuriHalNvmFlagDebug);
        printf("Debug disabled.");
    } else if(strcmp(arg, "1") == 0) {
        furi_hal_nvm_set_flag(FuriHalNvmFlagDebug);
        printf("Debug enabled.");
    } else {
        return false;
    }
    cli_command_update_debug_mode();
    return true;
}

static bool sysctl_exec_storage_bkp_unlock(PipeSide* pipe, const char* arg) {
    UNUSED(pipe);
    bool unlock;
    if(strcmp(arg, "1") == 0) {
        unlock = true;
    } else if(strcmp(arg, "0") == 0) {
        unlock = false;
    } else {
        return false;
    }
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_backup_set_readonly(storage, !unlock);
    furi_record_close(RECORD_STORAGE);
    printf("Backup storage %s.", unlock ? "unlocked" : "locked");
    return true;
}

#ifdef SRV_CLI_SOCKET
static bool sysctl_exec_cli_wifi_enabled(PipeSide* pipe, const char* arg) {
    UNUSED(pipe);
    if(strcmp(arg, "0") == 0) {
        sysctl_set_cli_wifi_enabled(false);
        printf("CLI over WiFi disabled.");
    } else if(strcmp(arg, "1") == 0) {
        sysctl_set_cli_wifi_enabled(true);
        printf("CLI over WiFi enabled.");
    } else {
        return false;
    }
    return true;
}
#endif // SRV_CLI_SOCKET

static bool sysctl_exec_websrv_accesslog_level(PipeSide* pipe, const char* arg) {
    UNUSED(pipe);
    char* end;
    long val = strtol(arg, &end, 10);
    if(end == arg || *end != '\0' || val < 0 || val > 3) return false;
    sysctl_set_websrv_accesslog_level((int)val);
    printf("Web server access log level set to %ld.", val);
    return true;
}

/* ---------- sysctl descriptor table ---------- */

static bool sysctl_visible_always(void) {
    return true;
}

static bool sysctl_visible_debug(void) {
    return furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug);
}

typedef struct {
    const char* name;
    const char* arg_spec;
    const char* description;
    bool (*is_visible)(void);
    bool (*execute)(PipeSide*, const char*);
} SysctlCmd;

static const SysctlCmd sysctl_cmds[] = {
    {"debug", "<1|0>", "enable/disable debug mode", sysctl_visible_always, sysctl_exec_debug},
#ifdef SRV_CLI_SOCKET
    {"cli_wifi_enabled",
     "<1|0>",
     "enable/disable CLI access over WiFi",
     sysctl_visible_always,
     sysctl_exec_cli_wifi_enabled},
#endif
    {"websrv_accesslog_level",
     "<0|1|2|3>",
     "web server access log verbosity (0-3)",
     sysctl_visible_always,
     sysctl_exec_websrv_accesslog_level},
    {"storage_bkp_unlock",
     "<1|0>",
     "lock/unlock backup storage (debug only)",
     sysctl_visible_debug,
     sysctl_exec_storage_bkp_unlock},
};

static void cli_command_sysctl_print_usage(void) {
    printf("Usage:\r\nsysctl <cmd>\r\nCmd list:\r\n");
    for(size_t i = 0; i < COUNT_OF(sysctl_cmds); i++) {
        const SysctlCmd* c = &sysctl_cmds[i];
        if(c->is_visible()) {
            printf("\t%s - %s\r\n", c->name, c->description);
        }
    }
}

void cli_command_sysctl(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd = furi_string_alloc();
    bool handled = false;

    if(args_read_string_and_trim(args, cmd)) {
        const char* cmd_str = furi_string_get_cstr(cmd);
        const char* arg_str = furi_string_get_cstr(args);
        for(size_t i = 0; i < COUNT_OF(sysctl_cmds); i++) {
            const SysctlCmd* c = &sysctl_cmds[i];
            if(strcmp(cmd_str, c->name) == 0 && c->is_visible()) {
                if(!c->execute(pipe, arg_str)) {
                    cli_print_usage(c->name, c->arg_spec, arg_str);
                }
                handled = true;
                break;
            }
        }
    }

    if(!handled) {
        cli_command_sysctl_print_usage();
    }

    furi_string_free(cmd);
}
