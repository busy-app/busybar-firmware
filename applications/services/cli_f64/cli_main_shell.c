#include "cli_main_shell.h"
#include <cli/cli_ansi.h>
#include <version/version.h>

void cli_main_motd(void* context) {
    UNUSED(context);
    printf(ANSI_FLIPPER_BRAND_ORANGE
           "\r\n" ANSI_FG_BR_WHITE "Welcome to BUSY Bar " ANSI_FG_BR_YELLOW "917" ANSI_FG_BR_WHITE " Command Line Interface!\r\n"
           "Read the manual: [insert docs link here]\r\n"
           "Run `help` or `?` to list available commands\r\n"
           "\r\n" ANSI_RESET);

    const Version* firmware_version = version_get();
    if(firmware_version) {
        printf(
            "Firmware version: %s %s (%s%s built on %s)\r\n",
            version_get_gitbranch(firmware_version),
            version_get_version(firmware_version),
            version_get_githash(firmware_version),
            version_get_dirty_flag(firmware_version) ? "-dirty" : "",
            version_get_builddate(firmware_version));
    }
}
