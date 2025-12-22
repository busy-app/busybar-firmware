/**
 * @brief Animation file format
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <storage/storage.h>

typedef struct AnimFile AnimFile;

/**
 * @brief Loads an `AnimFile` from the specified path
 * 
 * @param[in] storage `Storage` service
 * @param[in] path Path to `.anim` file
 * 
 * @returns Allocated and loaded `AnimFile`, or `NULL` on error.
 */
AnimFile* anim_file_alloc(Storage* storage, const char* path);

/**
 * @brief Unloads an `AnimFile`
 * @param[inout] anim `AnimFile` instance
 */
void anim_file_free(AnimFile* anim);

/**
 * @brief Key information about an `AnimFile`
 */
typedef struct {
    size_t width;
    size_t height;
    size_t fps;
} AnimFileInfo;

/**
 * @brief Gets information about an `AnimFile`
 * 
 * @param[in] anim `AnimFile` instance
 * 
 * @returns File information
 */
AnimFileInfo anim_file_info(const AnimFile* anim);

/**
 * @brief Flags related to a just-shown frame
 */
typedef enum {
    AnimFileFrameFlagLast = (1 << 0), //<! The frame is the last one in the section
    AnimFileFrameFlagFinished = (1 << 1), //<! No more sections to play and looping disabled
    AnimFileFrameFlagLooping = (1 << 2), //<! Looping the active section
    AnimFileFrameFlagSwitchToRequested = (1 << 3), //<! Switched to the requested section
    AnimFileFrameFlagError = (1 << 31), //<! Failed to access frame or file contains an error
} AnimFileFrameFlag;

/**
 * @brief Draws the next frame of the animation onto a canvas buffer
 * @note This function should be called with a period specified by `info.fps`
 * 
 * @param[in] anim `AnimFile` instance
 * @param[out] buffer BGR888 buffer of size `info.width * info.height * 3`. The
 *                    contents of the provided buffer must not change in between
 *                    calls to this function.
 * 
 * @returns Flags related to the just-shown frame
 */
AnimFileFrameFlag anim_file_frame(AnimFile* anim, void* buffer);

/**
 * @brief Flags for the `anim_file_set_section_*` family
 */
typedef enum {
    AnimFilePlayFlagNone = 0,
    AnimFilePlayFlagFinishCurrentSection = (1 << 0), //<! Finish playing current section, then switch to requested one
    AnimFilePlayFlagLoop = (1 << 1), //<! Play requested section in a loop
} AnimFilePlayFlag;

/**
 * @brief Sets the current section to be played back, using explicit frame
 *        indices
 * 
 * @param[in] anim `AnimFile` instance
 * @param[in] flags See `AnimFilePlayFlag`
 * @param[in] start Index of the first frame. Inclusive.
 * @param[in] end Index of the last frame. Inclusive, clamped to the highest
 *                frame index.
 * 
 * @returns Whether the operation was successful.
 */
bool anim_file_set_section_manual(AnimFile* anim, AnimFilePlayFlag flags, size_t start, size_t end);

/**
 * @brief Sets the current section to be played back, using a section index
 * 
 * @param[in] anim `AnimFile` instance
 * @param[in] flags See `AnimFilePlayFlag`
 * @param[in] index Index of the section
 * 
 * @returns Whether the operation was successful.
 */
bool anim_file_set_section_indexed(AnimFile* anim, AnimFilePlayFlag flags, size_t index);

/**
 * @brief The index that when provided to `anim_file_set_section_indexed`
 *        specifies the entire animation file
 */
#define ANIM_FILE_WHOLE_SECTION_INDEX 0

/**
 * @brief Sets the current section to be played back, using a section name
 * 
 * @param[in] anim `AnimFile` instance
 * @param[in] flags See `AnimFilePlayFlag`
 * @param[in] name Name of the section
 * 
 * @returns Whether the operation was successful.
 */
bool anim_file_set_section_named(AnimFile* anim, AnimFilePlayFlag flags, const char* name);

/**
 * @brief The name that when provided to `anim_file_set_section_named`
 *        specifies the entire animation file
 */
#define ANIM_FILE_WHOLE_SECTION_NAME "whole"

#ifdef __cplusplus
}
#endif
