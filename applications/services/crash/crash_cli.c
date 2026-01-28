
#include <cli/cli_command.h>

void crash_cli_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    printf("No regrets.");
    furi_crash("Crash CLI command invoked");
}
