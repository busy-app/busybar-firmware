#include "cli_command_sl_cli.h"
#include <cli_intercom/cli_intercom.h>
#include <containers/pipe_util.h>

#define TEMP_PIPE_SZ 128U
#define CLI_PROMPT   "\r\n917>: "

#ifdef SRV_INTERCOM
FURI_CHECK_RETURN bool
    cli_command_sl_cli_send_command_get_response(PipeSide* pipe, const char* sl_cmd) {
    if(!furi_record_exists(RECORD_CLI_INTERCOM)) return false;

    PipeSideBundle temp_bundle = pipe_alloc(TEMP_PIPE_SZ, 1);
    PipeSide* temp_own_pipe = temp_bundle.alices_side;
    PipeSide* temp_shell_pipe = temp_bundle.bobs_side;

    CliIntercom* cli_intercom = furi_record_open(RECORD_CLI_INTERCOM);

    bool is_success = false;

    do {
        if(cli_intercom_spawn(cli_intercom, temp_shell_pipe, true) != CliIntercomSpawnStatusOk) {
            pipe_free(temp_shell_pipe);
            break;
        }

        if(!pipe_copy_until(temp_own_pipe, NULL, CLI_PROMPT)) break;
        if(pipe_send(temp_own_pipe, sl_cmd, strlen(sl_cmd)) != strlen(sl_cmd)) break;
        if(pipe_send(temp_own_pipe, "\r", 1) != 1) break;
        if(!pipe_copy_until(temp_own_pipe, NULL, sl_cmd)) break;
        if(!pipe_copy_until(temp_own_pipe, NULL, "\r\n")) break;
        if(!pipe_copy_until(temp_own_pipe, pipe, CLI_PROMPT)) break;

        is_success = true;
    } while(false);

    pipe_free(temp_own_pipe);
    cli_intercom_join(cli_intercom);

    furi_record_close(RECORD_CLI_INTERCOM);

    return is_success;
}

void cli_command_sl_cli(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliIntercom* cli_intercom = furi_record_open(RECORD_CLI_INTERCOM);
    furi_check(cli_intercom_spawn(cli_intercom, pipe, false) == CliIntercomSpawnStatusOk);
    cli_intercom_join(cli_intercom);
    furi_record_close(RECORD_CLI_INTERCOM);
}

void cli_command_sl_echo(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);
}
#endif // SRV_INTERCOM
