#include <gui.

void power_cli(Cli* cli, FuriString* args, void* context) {
}

void power_on_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "dled", CliCommandFlagParallelSafe, power_cli, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(power_cli);
#endif
}
