#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_AUDIO "audio"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Audio Audio;

bool audio_play_file(Audio* instance, const char* file_name);

#ifdef __cplusplus
}
#endif
