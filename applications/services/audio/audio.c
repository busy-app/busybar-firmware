#include "audio.h"

#include <furi.h>
#include <furi_hal_sai.h>

static const uint16_t audio_click_1[] = {
#include "click_1.inc"
};

static const uint16_t audio_click_2[] = {
#include "click_2.inc"
};

static const uint16_t audio_busy_end[] = {
#include "busy_end.inc"
};

typedef struct {
    uint16_t* data;
    size_t size;
    size_t position;
    bool loop;
} Audio;

static FuriMessageQueue* audio_message_queue = NULL;
static volatile Audio* audio_current = NULL;

static int16_t audio_callback(void* context) {
    UNUSED(context);

    int16_t sample = 0;

    if(audio_current) {
        if(audio_current->data) {
            sample = audio_current->data[audio_current->position];

            // uint12 to int16 conversion
            sample -= 2048;
            sample <<= 4;
        }

        audio_current->position++;
        if(audio_current->position >= audio_current->size) {
            if(audio_current->loop) {
                audio_current->position = 0;
            } else {
                audio_current = NULL;
            }
        }
    }

    return sample;
}

static void audio_process_message(AudioCommand command) {
    Audio audio = {0};

    switch(command) {
    case AudioCommandEnable:
        furi_hal_sai_start(audio_callback, NULL);
        // furi_hal_dac_pa_enable();
        break;
    case AudioCommandDisable:
        // furi_hal_dac_pa_disable();
        furi_hal_sai_stop();
        break;
    case AudioCommandPlayClick1:
        audio.data = (uint16_t*)audio_click_1;
        audio.size = sizeof(audio_click_1) / sizeof(audio_click_1[0]);
        break;
    case AudioCommandPlayClick2:
        audio.data = (uint16_t*)audio_click_2;
        audio.size = sizeof(audio_click_2) / sizeof(audio_click_2[0]);
        break;
    case AudioCommandPlayBusyEnd:
        audio.data = (uint16_t*)audio_busy_end;
        audio.size = sizeof(audio_busy_end) / sizeof(audio_busy_end[0]);
        break;
    default:
        furi_crash();
        break;
    }

    audio_current = &audio;

    while(audio_current) {
        furi_delay_ms(10);
    }
}

int32_t audio_srv(void* p) {
    UNUSED(p);

    audio_message_queue = furi_message_queue_alloc(8, sizeof(AudioCommand));

    audio_play(AudioCommandEnable);
    audio_play(AudioCommandPlayBusyEnd);

    while(true) {
        AudioCommand command;
        furi_check(
            furi_message_queue_get(audio_message_queue, &command, FuriWaitForever) ==
            FuriStatusOk);
        audio_process_message(command);
    }

    return 0;
}

void audio_play(AudioCommand command) {
    furi_check(
        furi_message_queue_put(audio_message_queue, &command, FuriWaitForever) == FuriStatusOk);
}
