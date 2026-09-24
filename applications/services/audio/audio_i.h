#pragma once

#include "audio.h"

#include <furi_hal_sai.h>

#include <storage/storage.h>

#include <toolbox/api_lock.h>

#define TAG "Audio"

#define AUDIO_DEBUG
#ifdef AUDIO_DEBUG
#define AUDIO_TRACE(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define AUDIO_TRACE(...)
#endif // AUDIO_DEBUG

#define AUDIO_MAX_MESSAGES (8)
#define AUDIO_BUFFER_DEPTH (0x1000)

#define AUDIO_VOLUME_MIN     (0.0F)
#define AUDIO_VOLUME_MAX     (1.0F)
#define AUDIO_VOLUME_DEFAULT (AUDIO_VOLUME_MAX)

#define AUDIO_SAMPLE_RATE (44100)

/**
 * Fade envelope:
 *
 * amplitude
 *     ^
 *     |     _____________________________________
 *     |    /                                     \
 *     |   /                                       \
 *     |  /                                         \
 *     | /                                           \
 *     |/                                             \
 * ----+--------------------------------------------------------------> time
 *     | in |            full volume             | out |
 *     |                                               |
 *     | <-- start of playback     end of playback --> |
 */
#define AUDIO_FADE_SAMPLES  (AUDIO_SAMPLE_RATE * 100 / 1000)
#define AUDIO_FADE_IN_RATE  (100)
#define AUDIO_FADE_OUT_RATE (10)

#define AUDIO_PLAY_HOLDOFF furi_ms_to_ticks(100)

#define AUDIO_CONFIG_FILE APP_DATA_PATH("audio.json")

typedef enum {
    AudioBufferIndexNone = 0,
    AudioBufferIndexPing = (1UL << FuriHalSaiEventHalfTransfer),
    AudioBufferIndexPong = (1UL << FuriHalSaiEventTransferComplete),
    AudioBufferIndexBoth = (AudioBufferIndexPing | AudioBufferIndexPong),
} AudioBufferIndex;

typedef enum {
    AudioFadeDirectionIn, //<! Raise volume
    AudioFadeDirectionOut, //<! Lower volume
    AudioFadeDirectionMAX,
} AudioFadeDirection;

typedef enum {
    AudioMessageTypePlayFile,
    AudioMessageTypeStop,
    AudioMessageTypeSetVolume,
    AudioMessageTypeGetVolume,
    AudioMessageTypeMax,
} AudioMessageType;

typedef struct {
    AudioMessageType type;
    FuriApiLock lock;
    bool* result;
    union {
        float* get_volume;
        const char* file_name;
        float set_volume;
    };
} AudioMessage;

struct Audio {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    Storage* storage;
    File* file;
    FuriPubSub* event_pubsub;
    int16_t buffer[AUDIO_BUFFER_DEPTH];
    float volume;

    bool sai_running;
    int32_t fade_timer;
    AudioFadeDirection fade_direction;
    FuriString* queued_file;

    FuriEventLoopTimer* holdoff_timer;
};
