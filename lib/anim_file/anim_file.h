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

#define ANIM_FILE_OUT_BYTES_PER_PIXEL 4

typedef enum {
    AnimFileOptionNone = 0,
    AnimFileOptionIntermediateInternalBuffer =
        (1
         << 0), //<! Allow subpixel movement of resulting images (using @c anim_file_set_offset) and resizing/changing of output buffer (using @c anim_file_set_out_buf more than once)
    AnimFileOptionMAX,
} AnimFileOption;

/**
 * @brief Loads an `AnimFile` from the specified path
 * 
 * @param[in] storage `Storage` service
 * @param[in] path Path to `.anim` file
 * @param[in] options Bitmask of options (see @c AnimFileOption documentation)
 * 
 * @returns Allocated and loaded `AnimFile`, or `NULL` on error.
 */
AnimFile* FURI_WARN_UNUSED
    anim_file_alloc(Storage* storage, const char* path, AnimFileOption options);

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
    size_t frames;
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
    AnimFileFrameFlagNone = 0,
    AnimFileFrameFlagError =
        (1 << 0), //<! Operation failed. Enable `ANIM_FILE_DETAILED_ERRORS` and look in the logs.
    AnimFileFrameFlagLast = (1 << 1), //<! The frame is the last one in the section
    AnimFileFrameFlagFinished = (1 << 2), //<! No more sections to play and looping disabled
    AnimFileFrameFlagLooping = (1 << 3), //<! Looping the active section
    AnimFileFrameFlagSwitchToRequested = (1 << 4), //<! Switched to the requested section
    AnimFileFrameFlagNoChange = (1 << 5), //<! Output buffer hadn't changed
} AnimFileFrameFlag;

/**
 * @brief Complete information about the just-shown frame.
 */
typedef struct {
    AnimFileFrameFlag
        flags; //<! If contains `AnimFileFrameFlagError`, all other fields in this struct are invalid.
    size_t index;
} AnimFileFrameInfo;

/**
 * @brief Sets the output canvas buffer
 * 
 * If the provided buffer is smaller than the animation (as indicated in
 * `info.width` and `info.height`), only the the top-left corner of the
 * animation will be shown. This cutout of the animation can be moved around
 * with `anim_file_set_cutout`.
 * 
 * The buffer will not be touched until the next call to `anim_file_frame`.
 * 
 * If option `IntermediateInternalBuffer` was not set, the contents of the
 * provided buffer must never change. It may be reallocated, but its contents
 * must stay the same.
 * 
 * @param[in] anim `AnimFile` instance
 * @param[in] width Buffer width in pixels
 * @param[in] height Buffer height in pixels
 * @param[out] buffer BGRA8888 buffer of size `width * height * ANIM_FILE_OUT_BYTES_PER_PIXEL`
 */
void anim_file_set_out_buf(AnimFile* anim, size_t width, size_t height, void* buffer);

/**
 * @brief Draws the next frame of the animation onto a canvas buffer
 * 
 * @note This function is advised to be called with a rate specified by
 *       `anim_file_info(anim).fps`
 * 
 * Draws onto the buffer set by `anim_file_set_out_buf`. The contents of that
 * buffer must not change in between calls to this function.
 * 
 * @param[in] anim `AnimFile` instance
 * 
 * @returns Information about the just-shown frame
 */
AnimFileFrameInfo anim_file_frame(AnimFile* anim);

/**
 * @brief Flags for `anim_file_set_section`
 */
typedef enum {
    AnimFilePlayFlagNone = 0,
    AnimFilePlayFlagFinishCurrent =
        (1 << 0), //<! Finish playing current section, then switch to requested one
    AnimFilePlayFlagLoop = (1 << 1), //<! Play requested section in a loop
} AnimFilePlayFlag;

/**
 * @brief Sets the current section to be played back, using a section name
 * 
 * @param[in] anim `AnimFile` instance
 * @param[in] flags See `AnimFilePlayFlag`
 * @param[in] name Name of the section (also see: `ANIM_FILE_DEFAULT_SECTION`)
 * 
 * @returns Whether the operation was successful.
 */
bool FURI_WARN_UNUSED
    anim_file_set_section(AnimFile* anim, AnimFilePlayFlag flags, const char* name);

/**
 * @brief Changes which part of the animation will be in the output buffer
 * 
 * Can only be used (and only makes sense) in case the output buffer is smaller
 * than the animation file.
 * 
 * Supports non-integer (sub-pixel), negative and otherwise out-of-bounds
 * coordinates. Transparent black will be rendered in the out-of-bounds parts.
 * 
 * @note The new values will be applied on the next non-identical frame.
 * 
 * @param[inout] anim `AnimFile` instance
 * @param[in] x X-coordinate of the top-left corner of the cutout
 * @param[in] y Y-coordinate of the top-left corner of the cutout
 */
void anim_file_set_offset(AnimFile* anim, float x, float y);

/**
 * @brief The name that when provided to `anim_file_set_section` specifies the
 *        entire animation file.
 */
#define ANIM_FILE_DEFAULT_SECTION "default"

#ifdef __cplusplus
}
#endif
