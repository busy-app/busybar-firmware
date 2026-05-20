#include "audio.h"

#include <furi_hal_sai.h>

#include <furi.h>
#include <api_lock.h>

#include <storage/storage.h>
#include <json_helper.h>

#define TAG "Audio"

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
    AudioMessageTypeEnable,
    AudioMessageTypeDisable,
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

    bool play_holdoff_running;
    FuriEventLoopTimer* play_holdoff;
    size_t enable_holders;
};

static void audio_sai_start(Audio* instance) {
    furi_assert(instance);
    if(instance->sai_running) return;
    FURI_LOG_T(TAG, "sai start");
    furi_hal_sai_start();
    instance->sai_running = true;
}

static void audio_sai_stop(Audio* instance) {
    furi_assert(instance);
    if(!instance->sai_running) return;
    FURI_LOG_T(TAG, "sai stop");
    furi_hal_sai_stop();
    instance->sai_running = false;
}

static void audio_sai_callback(FuriHalSaiEvent event, void* context) {
    furi_assert(context);
    Audio* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, 1UL << event);
}

static bool audio_open_file(Audio* instance, const char* file_name) {
    bool success = false;

    do {
        if(storage_file_is_open(instance->file)) {
            if(!storage_file_close(instance->file)) {
                break;
            }
        }

        if(!storage_file_open(instance->file, file_name, FSAM_READ, FSOM_OPEN_EXISTING)) {
            storage_file_close(instance->file);
            break;
        }

        success = true;
    } while(false);

    return success;
}

static void audio_adjust_volume(Audio* instance, void* data_ptr, size_t data_size) {
    int16_t* buffer = data_ptr;
    const size_t count = data_size / sizeof(int16_t);

    for(size_t i = 0; i < count; i++) {
        float sample_vol = 1.0f;

        if(instance->volume < AUDIO_VOLUME_MAX) {
            sample_vol *= instance->volume;
        }

        sample_vol *= (float)instance->fade_timer / (float)AUDIO_FADE_SAMPLES;

        buffer[i] = roundf(buffer[i] * sample_vol);

        if(instance->fade_direction == AudioFadeDirectionIn) {
            instance->fade_timer += AUDIO_FADE_IN_RATE;
        } else if(instance->fade_direction == AudioFadeDirectionOut) {
            instance->fade_timer -= AUDIO_FADE_OUT_RATE;
        }

        if(instance->fade_timer >= AUDIO_FADE_SAMPLES) instance->fade_timer = AUDIO_FADE_SAMPLES;
        if(instance->fade_timer < 0) instance->fade_timer = 0;
    }
}

static bool audio_load_file_data(Audio* instance, AudioBufferIndex fill_type) {
    bool success = false;

    void* data_ptr;
    size_t data_size;

    if(fill_type == AudioBufferIndexBoth) {
        data_ptr = instance->buffer;
        data_size = AUDIO_BUFFER_DEPTH * sizeof(int16_t);

    } else {
        data_size = (AUDIO_BUFFER_DEPTH / 2) * sizeof(int16_t);

        if(fill_type == AudioBufferIndexPing) {
            data_ptr = instance->buffer;
        } else if(fill_type == AudioBufferIndexPong) {
            data_ptr = &instance->buffer[AUDIO_BUFFER_DEPTH / 2];
        } else {
            furi_crash("Invalid fill type");
        }
    }

    const size_t read_data_size = storage_file_read(instance->file, data_ptr, data_size);

    if(read_data_size > 0) {
        audio_adjust_volume(instance, data_ptr, read_data_size);

        if(read_data_size < data_size) {
            memset(data_ptr + read_data_size, 0, data_size - read_data_size);
        }

        success = true;
    }

    return success;
}

static bool audio_do_load_queued_file(Audio* instance) {
    FURI_LOG_D(TAG, "loading queued file");
    bool success = false;

    do {
        audio_sai_stop(instance);

        instance->fade_timer = 0;
        instance->fade_direction = AudioFadeDirectionIn;

        const char* path = furi_string_get_cstr(instance->queued_file);
        if(!strlen(path)) break;

        if(!audio_open_file(instance, path)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", path);
            break;
        }
        if(!audio_load_file_data(instance, AudioBufferIndexBoth)) {
            FURI_LOG_E(TAG, "Failed to load file data: %s", path);
            break;
        }

        audio_sai_start(instance);
        success = true;
    } while(false);

    furi_string_reset(instance->queued_file);
    if(!success && !instance->enable_holders) furi_hal_sai_disable_amplifier();

    return success;
}

static void audio_play_holdoff_finished(void* context) {
    furi_assert(context);
    Audio* instance = context;

    FURI_LOG_T(TAG, "holdoff fired");
    audio_do_load_queued_file(instance);

    instance->play_holdoff_running = false;
}

static void audio_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Audio* instance = context;
    furi_assert(object == instance->message_queue);

    AudioMessage msg;
    furi_check(furi_message_queue_get(instance->message_queue, &msg, 0) == FuriStatusOk);

    bool result = false;

    if(msg.type == AudioMessageTypePlayFile) {
        furi_string_set_str(instance->queued_file, msg.file_name);

        if(instance->enable_holders == 0) {
            furi_crash("Call audio_enable() before audio_play_file()");
        }

        if(instance->sai_running) {
            instance->fade_direction = AudioFadeDirectionOut;
            // next file will be played after current one fades out
            result = true;
        } else if(instance->play_holdoff_running) {
            // file will be played after holdoff fires
            result = true;
        } else {
            result = audio_do_load_queued_file(instance);
        }

    } else if(msg.type == AudioMessageTypeStop) {
        if(instance->sai_running) {
            instance->fade_direction = AudioFadeDirectionOut;
        }
        furi_string_reset(instance->queued_file);
        result = true;

    } else if(msg.type == AudioMessageTypeSetVolume) {
        instance->volume = msg.set_volume;

        json_config_write_single_number(AUDIO_CONFIG_FILE, "volume", instance->volume);

        AudioEvent pub_event = {.type = AudioEventVolumeUpdate};
        furi_pubsub_publish(instance->event_pubsub, &pub_event);
        result = true;

    } else if(msg.type == AudioMessageTypeGetVolume) {
        furi_assert(msg.get_volume);
        memcpy(msg.get_volume, &(instance->volume), sizeof(instance->volume));
        result = true;

    } else if(msg.type == AudioMessageTypeEnable) {
        instance->enable_holders++;
        if(instance->enable_holders == 1) {
            furi_hal_sai_enable_amplifier();
            furi_event_loop_timer_start(instance->play_holdoff, AUDIO_PLAY_HOLDOFF);
            instance->play_holdoff_running = true;
        }
        result = true;

    } else if(msg.type == AudioMessageTypeDisable) {
        instance->enable_holders--;
        if(instance->sai_running || instance->play_holdoff_running) {
            // will be disabled in SAI callback when the file finishes
        } else {
            furi_hal_sai_disable_amplifier();
        }
        result = true;

    } else {
        furi_crash("Invalid message type");
    }

    if(msg.result) {
        *msg.result = result;
    }
    if(msg.lock) {
        api_lock_unlock(msg.lock);
    }
}

static void audio_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Audio* instance = context;
    AudioBufferIndex buffer_index = events;

    bool should_stop = false;

    if(instance->fade_direction == AudioFadeDirectionOut && instance->fade_timer == 0) {
        FURI_LOG_D(TAG, "fade out finished");
        should_stop = true;

    } else {
        if(buffer_index >= AudioBufferIndexBoth) FURI_LOG_W(TAG, "Possible SAI underrun");

        if(!audio_load_file_data(instance, buffer_index)) {
            should_stop = true;
        }
    }

    if(should_stop) {
        audio_sai_stop(instance);
        storage_file_close(instance->file);

        audio_do_load_queued_file(instance);
    }
}

static void audio_send_message(Audio* instance, const AudioMessage* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) {
        api_lock_wait_unlock_and_free(message->lock);
    }
}

static Audio* audio_alloc(void) {
    Audio* instance = malloc(sizeof(Audio));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(AUDIO_MAX_MESSAGES, sizeof(AudioMessage));
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->file = storage_file_alloc(instance->storage);

    float default_volume = AUDIO_VOLUME_DEFAULT;
    json_config_read_single_number(
        AUDIO_CONFIG_FILE, "volume", &instance->volume, &default_volume);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        audio_message_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, audio_custom_event_callback, instance);

    furi_hal_sai_set_buffer(instance->buffer, COUNT_OF(instance->buffer));
    furi_hal_sai_set_callback(audio_sai_callback, instance);

    instance->event_pubsub = furi_pubsub_alloc();

    instance->queued_file = furi_string_alloc();

    instance->play_holdoff = furi_event_loop_timer_alloc(
        instance->event_loop, audio_play_holdoff_finished, FuriEventLoopTimerTypeOnce, instance);

    furi_record_create(RECORD_AUDIO, instance);

    return instance;
}

bool audio_play_file(Audio* instance, const char* file_name) {
    furi_check(instance);
    furi_check(file_name);

    bool result;

    const AudioMessage msg = {
        .type = AudioMessageTypePlayFile,
        .lock = api_lock_alloc_locked(),
        .result = &result,
        .file_name = file_name,
    };

    audio_send_message(instance, &msg);

    return result;
}

void audio_stop(Audio* instance) {
    furi_check(instance);

    const AudioMessage msg = {
        .type = AudioMessageTypeStop,
    };

    audio_send_message(instance, &msg);
}

void audio_set_volume(Audio* instance, float volume) {
    furi_check(instance);
    furi_check(volume >= AUDIO_VOLUME_MIN && volume <= AUDIO_VOLUME_MAX);

    volume = roundf(volume * 100.f) / 100.f;

    const AudioMessage msg = {
        .type = AudioMessageTypeSetVolume,
        .set_volume = volume,
    };

    audio_send_message(instance, &msg);
}

float audio_get_volume(Audio* instance) {
    furi_check(instance);

    float volume;
    AudioMessage msg = {
        .type = AudioMessageTypeGetVolume,
        .get_volume = &volume,
        .lock = api_lock_alloc_locked(),
    };

    audio_send_message(instance, &msg);

    return volume;
}

void audio_enable(Audio* instance) {
    furi_check(instance);

    AudioMessage msg = {
        .type = AudioMessageTypeEnable,
        .lock = api_lock_alloc_locked(),
    };

    audio_send_message(instance, &msg);
}

void audio_disable(Audio* instance) {
    furi_check(instance);

    AudioMessage msg = {
        .type = AudioMessageTypeDisable,
        .lock = api_lock_alloc_locked(),
    };

    audio_send_message(instance, &msg);
}

int32_t audio_srv(void* p) {
    UNUSED(p);

    Audio* instance = audio_alloc();

    furi_event_loop_run(instance->event_loop);

    return 0;
}

FuriPubSub* audio_get_pubsub(Audio* audio) {
    furi_check(audio);
    return audio->event_pubsub;
}
