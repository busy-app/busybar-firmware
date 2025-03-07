#include <furi/furi.h>

#include <cli/cli.h>

static void input_cli_command(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    printf("Hu hui\r\n");
}

void input_on_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "input", CliCommandFlagParallelSafe, input_cli_command, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(nfc_cli);
#endif
}
