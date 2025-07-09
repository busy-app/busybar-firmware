#include "audio.h"

#include <furi_hal_sai.h>

#include <furi.h>
#include <api_lock.h>

#include <storage/storage.h>

#define TAG "Audio"

#define AUDIO_MAX_MESSAGES (8)
#define AUDIO_BUFFER_DEPTH (0x1000)

#define AUDIO_VOLUME_MIN     (0.0F)
#define AUDIO_VOLUME_MAX     (1.0F)
#define AUDIO_VOLUME_DEFAULT (AUDIO_VOLUME_MAX)

typedef enum {
    AudioBufferIndexPing = (1UL << FuriHalSaiEventHalfTransfer),
    AudioBufferIndexPong = (1UL << FuriHalSaiEventTransferComplete),
    AudioBufferIndexBoth = (AudioBufferIndexPing | AudioBufferIndexPong),
} AudioBufferIndex;

typedef enum {
    AudioMessageTypePlayFile,
    AudioMessageTypeStop,
    AudioMessageTypeSetVolume,
} AudioMessageType;

typedef struct {
    AudioMessageType type;
    FuriApiLock lock;
    bool* result;
    union {
        const char* file_name;
        float volume;
    };
} AudioMessage;

struct Audio {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    Storage* storage;
    File* file;
    int16_t buffer[AUDIO_BUFFER_DEPTH];
    float volume;
    bool should_stop;
};

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
    if(instance->volume < AUDIO_VOLUME_MAX) {
        int16_t* buffer = data_ptr;
        const uint32_t count = data_size / sizeof(int16_t);

        for(uint32_t i = 0; i < count; ++i) {
            buffer[i] = roundf(buffer[i] * instance->volume);
        }
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

static bool audio_handle_play_file(Audio* instance, const AudioMessage* msg) {
    bool success = false;

    do {
        furi_hal_sai_stop();

        instance->should_stop = false;

        if(!audio_open_file(instance, msg->file_name)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", msg->file_name);
            break;
        }
        if(!audio_load_file_data(instance, AudioBufferIndexBoth)) {
            FURI_LOG_E(TAG, "Failed to load file data: %s", msg->file_name);
            break;
        }

        furi_hal_sai_start();

        success = true;

    } while(false);

    return success;
}

static void audio_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Audio* instance = context;
    furi_assert(object = instance->message_queue);

    AudioMessage msg;
    furi_check(furi_message_queue_get(instance->message_queue, &msg, 0) == FuriStatusOk);

    bool result = false;

    if(msg.type == AudioMessageTypePlayFile) {
        result = audio_handle_play_file(instance, &msg);
    } else if(msg.type == AudioMessageTypeStop) {
        instance->should_stop = true;
    } else if(msg.type == AudioMessageTypeSetVolume) {
        instance->volume = msg.volume;
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

    if(!instance->should_stop) {
        furi_check(events < AudioBufferIndexBoth, "Possible SAI underrun");

        if(!audio_load_file_data(instance, events)) {
            instance->should_stop = true;
        }

    } else {
        furi_hal_sai_stop();
        storage_file_close(instance->file);
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
    instance->volume = AUDIO_VOLUME_DEFAULT;

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

    // TODO: Create record only when MMC has been mounted
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

    const AudioMessage msg = {
        .type = AudioMessageTypeSetVolume,
        .volume = volume,
    };

    audio_send_message(instance, &msg);
}

float audio_get_volume(Audio* instance) {
    furi_check(instance);
    return instance->volume;
}

int32_t audio_srv(void* p) {
    UNUSED(p);

    Audio* instance = audio_alloc();

    furi_event_loop_run(instance->event_loop);

    return 0;
}
