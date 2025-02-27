/**
 * @file audio.h
 *
 * @brief APIs for playing audio (e.g. files from MMC storage).
 *
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Record key to access the Adio instance. */
#define RECORD_AUDIO "audio"

#ifdef __cplusplus
extern "C" {
#endif

/** Audio opaque type declaration. */
typedef struct Audio Audio;

/**
 * @brief Play an audio file from storage.
 *
 * The file MUST be in the following format:
 * - Heder: none
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
 * @brief Set the playback volume.
 *
 * The volume MUST be in range 0.0 (mute) to 1.0 (full volume).
 *
 * @param[in,out] instance pointer to the Audio instance
 * @param[in] volume new volume value (see above range)
 *
 */
void audio_set_volume(Audio* instance, float volume);

#ifdef __cplusplus
}
#endif
