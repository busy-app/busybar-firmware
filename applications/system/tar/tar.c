#include "tar.h"

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/cli_status.h>

#include <toolbox/tar/tar_archive.h>
#include <toolbox/path.h>

#define TAG "Tar"

#define CHECK_STORAGE_EXT_PATH_PREFIX    STORAGE_EXT_PATH_PREFIX "/"
#define CHECK_STORAGE_BACKUP_PATH_PREFIX STORAGE_BACKUP_PATH_PREFIX "/"

typedef bool (*TarCliCommandCallback)(FuriString* path, FuriString* args);

typedef struct {
    const char* command;
    const char* help;
    const TarCliCommandCallback impl;
} TarCliCommand;

static void tar_compress_directory_example(const char* cmd) {
    printf("Example: tar %s /ext/backup.tar /ext/mydir\r\n", cmd);
}

static bool tar_check_params(const char* cmd, FuriString* path, FuriString* args) {
    if(!(strncmp(
             furi_string_get_cstr(path),
             CHECK_STORAGE_EXT_PATH_PREFIX,
             strlen(CHECK_STORAGE_EXT_PATH_PREFIX)) == 0 ||
         strncmp(
             furi_string_get_cstr(path),
             CHECK_STORAGE_BACKUP_PATH_PREFIX,
             strlen(CHECK_STORAGE_BACKUP_PATH_PREFIX)) == 0)) {
        printf(
            ANSI_FG_RED "Error: Destination path must start with %s or %s: %s\r\n" ANSI_RESET,
            STORAGE_EXT_PATH_PREFIX,
            STORAGE_BACKUP_PATH_PREFIX,
            furi_string_get_cstr(path));
        tar_compress_directory_example(cmd);
        return false;
    }

    if(!(strncmp(
             furi_string_get_cstr(args),
             CHECK_STORAGE_EXT_PATH_PREFIX,
             strlen(CHECK_STORAGE_EXT_PATH_PREFIX)) == 0 ||
         strncmp(
             furi_string_get_cstr(args),
             CHECK_STORAGE_BACKUP_PATH_PREFIX,
             strlen(CHECK_STORAGE_BACKUP_PATH_PREFIX)) == 0)) {
        printf(
            ANSI_FG_RED "Error: Source path must start with %s or %s: %s\r\n" ANSI_RESET,
            STORAGE_EXT_PATH_PREFIX,
            STORAGE_BACKUP_PATH_PREFIX,
            furi_string_get_cstr(args));
        tar_compress_directory_example(cmd);
        return false;
    }

    return true;
}

bool tar_compress_directory_cli(FuriString* path, FuriString* args) {
    if(!tar_check_params("c", path, args)) {
        printf(CLI_STATUS_ERROR);
        return false;
    }

    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    TarArchive* archive = tar_archive_alloc(storage);
    do {
        if(!storage_dir_exists(storage, furi_string_get_cstr(args))) {
            printf(
                ANSI_FG_RED "Error: Destination directory does not exist: %s\r\n" ANSI_RESET,
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            break;
        }

        if(!tar_archive_open(archive, furi_string_get_cstr(path), TarOpenModeWrite)) {
            printf(ANSI_FG_RED "Error: Failed to open archive for writing\r\n" ANSI_RESET);
            printf(CLI_STATUS_ERROR);
            break;
        }
        uint32_t start_tick = furi_get_tick();
        printf(
            "Compressing directory %s to %s\r\n",
            furi_string_get_cstr(args),
            furi_string_get_cstr(path));
        success = tar_archive_add_dir(archive, furi_string_get_cstr(args), "");
        tar_archive_finalize(archive);
        uint32_t end_tick = furi_get_tick();
        printf(
            "Compression %s in %lu ms \r\n",
            success ? ANSI_FG_GREEN "success" ANSI_RESET : ANSI_FG_RED "failed" ANSI_RESET,
            (end_tick - start_tick) * furi_kernel_get_tick_frequency() / 1000);
        printf(success ? CLI_STATUS_OK : CLI_STATUS_ERROR);
    } while(false);
    tar_archive_free(archive);
    furi_record_close(RECORD_STORAGE);
    return success;
}

bool tar_extract_files_cli(FuriString* path, FuriString* args) {
    if(!tar_check_params("x", path, args)) {
        printf(CLI_STATUS_ERROR);
        return false;
    }
    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    TarArchive* archive = tar_archive_alloc(storage);
    do {
        if(path_recursive_create_dir(storage, args) != FSE_OK) {
            printf(
                ANSI_FG_RED "Error: Failed to create directory: %s\r\n" ANSI_RESET,
                furi_string_get_cstr(args));
            printf(CLI_STATUS_ERROR);
            break;
        }
        if(!tar_archive_open(archive, furi_string_get_cstr(path), TarOpenModeReadAuto)) {
            printf(ANSI_FG_RED "Error: Failed to open archive for reading\r\n" ANSI_RESET);
            printf(CLI_STATUS_ERROR);
            break;
        }
        uint32_t start_tick = furi_get_tick();
        printf(
            "Extracting archive %s to %s\r\n",
            furi_string_get_cstr(path),
            furi_string_get_cstr(args));
        success = tar_archive_unpack_to(archive, furi_string_get_cstr(args), NULL);
        uint32_t end_tick = furi_get_tick();
        printf(
            "Decompression %s in %lu ms \r\n",
            success ? ANSI_FG_GREEN "success" ANSI_RESET : ANSI_FG_RED "failed" ANSI_RESET,
            (end_tick - start_tick) * furi_kernel_get_tick_frequency() / 1000);
        printf(success ? CLI_STATUS_OK : CLI_STATUS_ERROR);
    } while(false);
    tar_archive_free(archive);
    furi_record_close(RECORD_STORAGE);
    return success;
}

static const TarCliCommand tar_cli_commands[] = {
    {
        "c",
        "Compress directory to tar archive, <path> is the destination path, <args> is the source directory",
        &tar_compress_directory_cli,
    },
    {
        "x",
        "Extract files from tar archive, <path> is the tar file path, <args> is the destination directory",
        &tar_extract_files_cli,
    },
};

static void tar_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("tar <cmd> <path> <args>\r\n");
    printf("The path must start with /bkp or /ext\r\n");
    printf("Cmd list:\r\n");

    for(size_t i = 0; i < COUNT_OF(tar_cli_commands); ++i) {
        const TarCliCommand* command_descr = &tar_cli_commands[i];
        const char* cli_cmd = command_descr->command;
        printf(
            "\t%s%s - %s\r\n", cli_cmd, strlen(cli_cmd) > 8 ? "\t\t" : "\t", command_descr->help);
    }
}

void tar_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    FuriString* cmd;
    FuriString* path;
    cmd = furi_string_alloc();
    path = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            tar_cli_print_usage();
            break;
        }

        if(!args_read_probably_quoted_string_and_trim(args, path)) {
            tar_cli_print_usage();
            break;
        }

        size_t i = 0;
        for(; i < COUNT_OF(tar_cli_commands); ++i) {
            const TarCliCommand* command_descr = &tar_cli_commands[i];
            if(furi_string_cmp_str(cmd, command_descr->command) == 0) {
                command_descr->impl(path, args);
                break;
            }
        }

        if(i == COUNT_OF(tar_cli_commands)) {
            tar_cli_print_usage();
        }
    } while(false);

    furi_string_free(path);
    furi_string_free(cmd);
}
