#include "cli_command_audio.h"

#include <furi/furi.h>

#include <audio/audio.h>
#include <storage/storage.h>
#include <toolbox/args.h>

typedef enum {
    CliAudioCommandStart,
    CliAudioCommandStop,
} CliAudioCommand;

static void audio_cli_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("audio start <path>\r\n");
    printf("audio stop\r\n");
}

void cli_command_audio(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);

    FuriString* str_tmp = furi_string_alloc();
    CliAudioCommand cmd = CliAudioCommandStart;
    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool parsed = false;
    do {
        if(!args_read_string_and_trim(args, str_tmp)) break;
        if(furi_string_cmp_str(str_tmp, "start") == 0) {
            cmd = CliAudioCommandStart;
        } else if(furi_string_cmp_str(str_tmp, "stop") == 0) {
            cmd = CliAudioCommandStop;
        } else {
            printf("Invalid command %s\r\n", furi_string_get_cstr(str_tmp));
            audio_cli_command_print_usage();
            break;
        }

        if(cmd == CliAudioCommandStart) {
            if(!args_read_string_and_trim(args, str_tmp)) break;
            if(!storage_file_exists(storage, furi_string_get_cstr(str_tmp))) {
                printf("File %s does not exist\r\n", furi_string_get_cstr(str_tmp));
                break;
            }
        }
        parsed = true;
    } while(false);

    if(!parsed) {
        return;
    }

    Audio* audio = furi_record_open(RECORD_AUDIO);
    if(cmd == CliAudioCommandStart) {
        if(!audio_play_file(audio, furi_string_get_cstr(str_tmp))) {
            printf("Failed to play file %s\r\n", furi_string_get_cstr(str_tmp));
        }
    } else if(cmd == CliAudioCommandStop) {
        audio_stop(audio);
    }

    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(str_tmp);
}
