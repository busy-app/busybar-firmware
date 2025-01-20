#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AudioCommandEnable,
    AudioCommandDisable,
    AudioCommandPlayClick1,
    AudioCommandPlayClick2,
    AudioCommandPlayBusyEnd,
} AudioCommand;

void audio_play(AudioCommand sound);

#ifdef __cplusplus
}
#endif
