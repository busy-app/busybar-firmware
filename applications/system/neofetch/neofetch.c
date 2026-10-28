#include <furi.h>

#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/cli_status.h>
#include <cli/cli_command.h>

#include <furi_hal_version.h>
#include <version/version.h>
#include <storage/storage.h>
#include <power/power_service/power.h>

#include <FreeRTOS.h>
#include <FreeRTOS-Kernel/include/task.h>

void neofetch_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

#define NEOFETCH_COLOR ANSI_FLIPPER_BRAND_ORANGE
    static const char* const neofetch_logo[] = {
        "    ____    _____________    ____",
        "  _L____J____I_________I____L____J_",
        " / " ANSI_FG_BR_RED " ___ _   _ _____   __          " NEOFETCH_COLOR "\\",
        "|  " ANSI_FG_BR_RED "| _ ) | | / __\\ \\ / /            " NEOFETCH_COLOR "|",
        "|  " ANSI_FG_BR_RED "| _ \\ |_| \\__ \\\\ V /             " NEOFETCH_COLOR "|",
        "|  " ANSI_FG_BR_RED "|___/\\___/|___/ |_|              " NEOFETCH_COLOR "|",
        " \\_________________________________/",
    };

    // Determine logo parameters
    size_t logo_height = COUNT_OF(neofetch_logo), logo_width = 0;
    for(size_t i = 0; i < logo_height; i++)
        logo_width = MAX(logo_width, cli_ansi_printable_strlen(neofetch_logo[i]));
    logo_width += 4; // space between logo and info

    // Format hostname delimiter
    const size_t size_of_hostname = 4 + strlen(furi_hal_version_get_name_ptr());
    char delimiter[64];
    memset(delimiter, '-', size_of_hostname);
    delimiter[size_of_hostname] = '\0';

    // Get heap info
    size_t heap_total = memmgr_get_total_heap();
    size_t heap_used = heap_total - memmgr_get_free_heap();
    uint16_t heap_percent = (100 * heap_used) / heap_total;

    // Get storage info
    Storage* storage = furi_record_open(RECORD_STORAGE);
    uint64_t ext_total, ext_free, ext_used, ext_percent;
    bool is_read_only;
    storage_common_fs_info(storage, "/ext", &ext_total, &ext_free, &is_read_only);
    ext_used = ext_total - ext_free;
    ext_percent = (100 * ext_used) / ext_total;
    ext_used /= 1024 * 1024;
    ext_total /= 1024 * 1024;
    furi_record_close(RECORD_STORAGE);

    // Get battery info
    uint16_t charge_percent = 0;
    const char* charge_state = NULL;
    Power* power = furi_record_open(RECORD_POWER);

    PowerInfo power_info;
    power_get_info(power, &power_info);
    charge_percent = power_info.charge;
    if(power_info.is_full_charged) {
        charge_state = "charged";
    } else {
        if(power_info.is_charging) {
            charge_state = "charging";
        } else {
            charge_state = "discharging";
        }
    }

    furi_record_close(RECORD_POWER);

    // Get misc info
    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    const Version* version = version_get();

    // Print ASCII art with info
    const size_t info_height = 20;
    for(size_t i = 0; i < MAX(logo_height, info_height); i++) {
        const char* logo_line = (i < logo_height) ? neofetch_logo[i] : "";
        printf(NEOFETCH_COLOR "%s", logo_line);
        printf(NEOFETCH_COLOR "%-*s", logo_width - cli_ansi_printable_strlen(logo_line), "");
        switch(i) {
        case 0: // you@<hostname>
            printf("you" ANSI_RESET "@" NEOFETCH_COLOR "%s", furi_hal_version_get_model_name());
            break;
        case 1: // delimiter
            printf(ANSI_RESET "%s", delimiter);
            break;
        case 2: // OS: FURI <edition> <branch> <version> <commit>
            printf(
                "OS" ANSI_RESET ": FURI %s %s %s %s",
                version_get_version(version),
                version_get_gitbranch(version),
                version_get_version(version),
                version_get_githash(version));
            break;
        case 3: // Host: <model> <hostname>
            printf(
                "Host" ANSI_RESET ": %s %s",
                furi_hal_version_get_model_name(),
                furi_hal_version_get_name_ptr());
            break;
        case 4: // Kernel: FreeRTOS <maj>.<min>.<build>
            printf(
                "Kernel" ANSI_RESET ": FreeRTOS %d.%d.%d",
                tskKERNEL_VERSION_MAJOR,
                tskKERNEL_VERSION_MINOR,
                tskKERNEL_VERSION_BUILD);
            break;
        case 5: // Uptime: ?h?m?s
            printf(
                "Uptime" ANSI_RESET ": %luh%lum%lus",
                uptime / 60 / 60,
                uptime / 60 % 60,
                uptime % 60);
            break;
        case 6: // Display 1: ICND2153AP*9+ICN2012*9+SN74HC595 72x16 @ 32 bpp in 6.3"
            printf("Display 1" ANSI_RESET
                   ": ICND2153AP*9+ICN2012*9+SN74HC595 72x16 @ 24 bpp in 6.3\"");
            break;
        case 7: // Display 2: SSD1320 160x80 @ 4bpp
            printf("Display 2" ANSI_RESET ": SSD1320 160x80 @ 4bpp");
            break;
        case 8: // DE: GuiSrv
            printf("DE" ANSI_RESET ": GuiSrv");
            break;
        case 9: // WM: lvgl
            printf("WM" ANSI_RESET ": lvgl");
            break;
        case 10: // Shell: CliSrv
            printf("Shell" ANSI_RESET ": CliShell");
            break;
        case 11: // CPU 1: STM32U595RIT6 Arm Cortex-M33 @ 160 MHz
            printf("CPU 1" ANSI_RESET ": STM32U595RIT6 Arm Cortex-M33 @ 160 MHz");
            break;
        case 12: // CPU 2: SiWG917M111BGTBA Arm Cortex-M4 @ 180 MHz
            printf("CPU 2" ANSI_RESET ": SiWG917M111MGTBA Arm Cortex-M4 @ 180 MHz");
            break;
        case 13: // CPU 3: SiWG917M111BGTBA ThreadArch @ 160 MHz
            printf("CPU 3" ANSI_RESET ": SiWG917M111MGTBA ThreadArch @ 160 MHz");
            break;
        case 14: // Memory (CPU 1): <used> / <total> KiB (??%)
            printf(
                "Memory (CPU 1)" ANSI_RESET ": %zu / %zu KiB (%hu%%)",
                heap_used / 1024,
                heap_total / 1024,
                heap_percent);
            break;
        case 15: // Disk (/ext): <used> / <total> MiB (??%)
            printf(
                "Disk (/ext)" ANSI_RESET ": %llu / %llu MiB (%llu%%)",
                ext_used,
                ext_total,
                ext_percent);
            break;
        case 16: // Battery: ??% (<state>)
            printf("Battery" ANSI_RESET ": %hu%% (%s)" ANSI_RESET, charge_percent, charge_state);
            break;
        case 17: // empty space
            break;
        case 18: // Colors (line 1)
            for(size_t j = 30; j <= 37; j++)
                printf("\e[%dm███", j);
            break;
        case 19: // Colors (line 2)
            for(size_t j = 90; j <= 97; j++)
                printf("\e[%dm███", j);
            break;
        default:
            break;
        }
        printf("\r\n");
    }
    printf(ANSI_RESET);
#undef NEOFETCH_COLOR
}
