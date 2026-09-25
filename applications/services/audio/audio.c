#include "audio_i.h"

#include <json_helper.h>

typedef bool (*AudioApiMessageHandler)(Audio* instance, AudioMessage* api_message);

static bool audio_enable_amplifier(Audio* instance) {
    const bool was_amplifier_enabled = instance->is_amplifier_enabled;

    if(instance->is_amplifier_enabled) {
        if(furi_event_loop_timer_is_running(instance->cooldown_timer)) {
            AUDIO_TRACE("Amplifier disable aborted");
            furi_event_loop_timer_stop(instance->cooldown_timer);
        }

    } else {
        AUDIO_TRACE("Amplifier enabled");
        furi_hal_sai_enable_amplifier();
        instance->is_amplifier_enabled = true;
    }

    return was_amplifier_enabled;
}

static void audio_disable_amplifier(Audio* instance) {
    if(instance->is_amplifier_enabled) {
        if(!furi_event_loop_timer_is_running(instance->cooldown_timer)) {
            AUDIO_TRACE("Amplifier disable scheduled");
            furi_event_loop_timer_start(
                instance->cooldown_timer, furi_ms_to_ticks(AUDIO_AMPLIFIER_COOLDOWN_MS));
        }
    }
}

static void audio_sai_callback(FuriHalSaiEvent event, void* context) {
    furi_assert(context);
    Audio* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, 1UL << event);
}

static void audio_sai_init(Audio* instance) {
    furi_hal_sai_set_buffer(instance->buffer, COUNT_OF(instance->buffer));
    furi_hal_sai_set_callback(audio_sai_callback, instance);
}

static void audio_sai_start(Audio* instance) {
    if(!instance->is_sai_running) {
        AUDIO_TRACE("SAI start");

        furi_hal_sai_start();
        instance->is_sai_running = true;
    }
}

static void audio_sai_stop(Audio* instance) {
    if(instance->is_sai_running) {
        AUDIO_TRACE("SAI stop");

        furi_hal_sai_stop();
        instance->is_sai_running = false;
    }
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

static FURI_ALWAYS_INLINE float audio_get_effective_volume(const Audio* instance) {
    return instance->volume * ((float)instance->fade_counter / AUDIO_FADE_SAMPLES); // NOLINT
}

static FURI_ALWAYS_INLINE void audio_update_fade_counter(Audio* instance) {
    if(instance->fade_direction == AudioFadeDirectionIn) {
        instance->fade_counter += AUDIO_FADE_IN_RATE;
    } else if(instance->fade_direction == AudioFadeDirectionOut) {
        instance->fade_counter -= AUDIO_FADE_OUT_RATE;
    } else {
        furi_crash("Invalid AudioFadeDirection value");
    }

    instance->fade_counter = CLAMP(instance->fade_counter, AUDIO_FADE_SAMPLES, 0);
}

static void audio_adjust_volume(Audio* instance, void* data_ptr, size_t data_size) {
    int16_t* buffer = data_ptr;
    const size_t count = data_size / sizeof(int16_t);

    for(size_t i = 0; i < count; i++) {
        buffer[i] = roundf(buffer[i] * audio_get_effective_volume(instance));
        audio_update_fade_counter(instance);
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
    furi_check(!instance->is_sai_running);

    bool success = false;

    do {
        if(furi_string_empty(instance->queued_file_path)) {
            AUDIO_TRACE("No queued file to play");
            break;
        }

        AUDIO_TRACE("Loading queued file");

        instance->fade_counter = 0;
        instance->fade_direction = AudioFadeDirectionIn;

        const char* path = furi_string_get_cstr(instance->queued_file_path);

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

    furi_string_reset(instance->queued_file_path);

    if(!success) {
        audio_disable_amplifier(instance);
    }

    return success;
}

static void audio_warmup_timer_callback(void* context) {
    furi_assert(context);
    Audio* instance = context;

    AUDIO_TRACE("Amplifier warmup done");
    audio_do_load_queued_file(instance);
}

static void audio_cooldown_timer_callback(void* context) {
    furi_assert(context);

    Audio* instance = context;
    furi_check(instance->is_amplifier_enabled);

    AUDIO_TRACE("Amplifier disabled");
    furi_hal_sai_disable_amplifier();
    instance->is_amplifier_enabled = false;
}

static bool audio_play_file_api_message_handler(Audio* instance, AudioMessage* api_message) {
    bool success = true;

    furi_string_set_str(instance->queued_file_path, api_message->file_name);

    if(instance->is_sai_running) {
        // next file will be played after current one fades out
        instance->fade_direction = AudioFadeDirectionOut;

    } else {
        const bool was_amplifier_enabled = audio_enable_amplifier(instance);

        if(was_amplifier_enabled) {
            if(furi_event_loop_timer_is_running(instance->warmup_timer)) {
                // warmup already in progress, start playing after it completes
            } else {
                // amplifier is warmed up, start playing immediately
                success = audio_do_load_queued_file(instance);
            }
        } else {
            // amplifier disabled, begin warmup and start playing after it
            furi_event_loop_timer_start(
                instance->warmup_timer, furi_ms_to_ticks(AUDIO_AMPLIFIER_WARMUP_MS));
        }
    }

    return success;
}

static bool audio_stop_api_message_handler(Audio* instance, AudioMessage* api_message) {
    UNUSED(api_message);

    bool success = false;

    if(instance->is_sai_running) {
        instance->fade_direction = AudioFadeDirectionOut;
        success = true;

    } else if(furi_event_loop_timer_is_running(instance->warmup_timer)) {
        // SAI never started; cancel the warmup and signal play end immediately
        furi_event_loop_timer_stop(instance->warmup_timer);
        audio_disable_amplifier(instance);

        AudioEvent pub_event = {.type = AudioEventPlayEnd};
        furi_pubsub_publish(instance->event_pubsub, &pub_event);
        success = true;
    }

    furi_string_reset(instance->queued_file_path);
    return success;
}

static bool audio_set_volume_api_message_handler(Audio* instance, AudioMessage* api_message) {
    instance->volume = api_message->set_volume;
    json_config_write_single_number(AUDIO_CONFIG_FILE, "volume", instance->volume);

    AudioEvent pub_event = {.type = AudioEventVolumeUpdate};
    furi_pubsub_publish(instance->event_pubsub, &pub_event);

    return true;
}

static bool audio_get_volume_api_message_handler(Audio* instance, AudioMessage* api_message) {
    furi_assert(api_message->get_volume);
    *api_message->get_volume = instance->volume;

    return true;
}

static const AudioApiMessageHandler audio_api_message_handlers[] = {
    [AudioMessageTypePlayFile] = audio_play_file_api_message_handler,
    [AudioMessageTypeStop] = audio_stop_api_message_handler,
    [AudioMessageTypeSetVolume] = audio_set_volume_api_message_handler,
    [AudioMessageTypeGetVolume] = audio_get_volume_api_message_handler,
};

static_assert(COUNT_OF(audio_api_message_handlers) == AudioMessageTypeMax);

static void audio_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Audio* instance = context;
    furi_assert(object == instance->message_queue);

    AudioMessage api_message;
    while(furi_message_queue_get(instance->message_queue, &api_message, 0) == FuriStatusOk) {
        const AudioMessageType type = api_message.type;
        furi_check(type < AudioMessageTypeMax);

        const bool result = audio_api_message_handlers[type](instance, &api_message);

        if(api_message.result != NULL) {
            *api_message.result = result;
        }

        if(api_message.lock != NULL) {
            api_lock_unlock(api_message.lock);
        }
    }
}

static void audio_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Audio* instance = context;
    AudioBufferIndex buffer_index = events;

    /* event loop may re-arm its notify flag after event bits were already drained */
    if(buffer_index == AudioBufferIndexNone) {
        return;
    }

    bool should_stop = false;

    if(instance->fade_direction == AudioFadeDirectionOut && instance->fade_counter == 0) {
        AUDIO_TRACE("Fade out finished");
        should_stop = true;

    } else {
        if(buffer_index >= AudioBufferIndexBoth) {
            FURI_LOG_W(TAG, "Possible SAI underrun");
        }

        if(!audio_load_file_data(instance, buffer_index)) {
            should_stop = true;
        }
    }

    if(should_stop) {
        audio_sai_stop(instance);
        storage_file_close(instance->file);

        AudioEvent pub_event = {.type = AudioEventPlayEnd};
        furi_pubsub_publish(instance->event_pubsub, &pub_event);

        audio_do_load_queued_file(instance);
    }
}

static void audio_load_settings(Audio* instance) {
    float default_volume = AUDIO_VOLUME_DEFAULT;
    json_config_read_single_number(
        AUDIO_CONFIG_FILE, "volume", &instance->volume, &default_volume);
}

static Audio* audio_alloc(void) {
    Audio* instance = malloc(sizeof(Audio));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(AUDIO_MAX_MESSAGES, sizeof(AudioMessage));
    instance->event_pubsub = furi_pubsub_alloc();
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->file = storage_file_alloc(instance->storage);
    instance->queued_file_path = furi_string_alloc();
    instance->warmup_timer = furi_event_loop_timer_alloc(
        instance->event_loop, audio_warmup_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->cooldown_timer = furi_event_loop_timer_alloc(
        instance->event_loop, audio_cooldown_timer_callback, FuriEventLoopTimerTypeOnce, instance);

    audio_load_settings(instance);
    audio_sai_init(instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        audio_message_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, audio_custom_event_callback, instance);

    furi_record_create(RECORD_AUDIO, instance);

    return instance;
}

int32_t audio_srv(void* p) {
    UNUSED(p);

    Audio* instance = audio_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
