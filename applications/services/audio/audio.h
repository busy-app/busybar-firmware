/**
 * @file audio.h
 *
 * @brief APIs for playing audio (e.g. files from MMC storage).
 *
 */
#pragma once

#include <stdbool.h>
#include <core/pubsub.h>

/** Record key to access the Audio instance. */
#define RECORD_AUDIO "audio"

#ifdef __cplusplus
extern "C" {
#endif

/** Audio opaque type declaration. */
typedef struct Audio Audio;

/**
 * @brief The types of events that can occur.
 */
typedef enum {
    AudioEventVolumeUpdate, /**< Volume has been changed */
    AudioEventPlayEnd, /**< File ended playing */
} AudioEventType;

/**
 * @brief Audio event structure emitted by the PubSub.
 */
typedef struct {
    AudioEventType type; /**< The event type */
} AudioEvent;

/**
 * @brief Get the PubSub instance for events subscription.
 *
 * The received events will be of @ref AudioEvent underlying type.
 *
 * @param[in] instance pointer to the Audio instance
 * @returns pointer to the PubSub instance
 */
FuriPubSub* audio_get_pubsub(Audio* instance);

/**
 * @brief Play an audio file from storage.
 *
 * The file MUST be in the following format:
 * - Header: none
 * - Channels: 1
 * - Rate: 44100 Hz
 * - Bits: 16bit LE
 *
 * If this function is called when a file is already playing, the new file
 * will start playing immediately without waiting for the playback to end.
 *
 * @param[in,out] instance pointer to the Audio instance
 * @param[in] file_name full path to the file location on the storage
 * @returns true if the file could be played, false otherwise
 */
bool audio_play_file(Audio* instance, const char* file_name);

/**
 * @brief Stop playing current audio from file.
 *
 * @param[in,out] instance pointer to the Audio instance
 * @returns true if the audio is stopping, false if not playing
 */
bool audio_stop(Audio* instance);

/**
 * @brief Set the playback volume.
 *
 * The volume MUST be in range 0.0 (mute) to 1.0 (full volume).
 *
 * @param[in,out] instance pointer to the Audio instance
 * @param[in] volume new volume value (see above range)
 *
 */
void audio_set_volume(Audio* instance, float volume);

/**
 * @brief Get the playback volume.
 *
 * The volume is in range 0.0 (mute) to 1.0 (full volume).
 *
 * @param[in] instance pointer to the Audio instance
 * @returns volume value (see above range)
 *
 */
float audio_get_volume(Audio* instance);

#ifdef __cplusplus
}
#endif
