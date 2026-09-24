#include "audio_i.h"

static void audio_send_message(Audio* instance, const AudioMessage* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) {
        api_lock_wait_unlock_and_free(message->lock);
    }
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

bool audio_stop(Audio* instance) {
    furi_check(instance);

    bool result;

    const AudioMessage msg = {
        .type = AudioMessageTypeStop,
        .lock = api_lock_alloc_locked(),
        .result = &result,
    };

    audio_send_message(instance, &msg);

    return result;
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

FuriPubSub* audio_get_pubsub(Audio* audio) {
    furi_check(audio);
    return audio->event_pubsub;
}
