/**
 * @brief Internal functions of `AnimFile`
 */

#pragma once

#include "anim_file.h"
#include "anim_file_format.h"

#ifdef __cplusplus
extern "C" {
#endif

// =========
// Debugging
// =========

#define TAG "AnimFile"

#define ANIM_FILE_DETAILED_ERRORS

#ifdef ANIM_FILE_DETAILED_ERRORS
#define ANIM_FILE_ERR(...) FURI_LOG_E(TAG, __VA_ARGS__)
#else
#define ANIM_FILE_ERR(...) FURI_LOF_E(TAG, "Load error")
#endif

// =====
// Types
// =====

typedef void (
    *AnimFileSectionCallback)(size_t index, const AnimFileSection* section, void* context);

/**
 * @brief Information about loaded file
 */
typedef struct {
    AnimFileHeader header;
    AnimFileInfo info; //<! Publicly available information
    size_t section_count;
    uint8_t* sections;
    AnimFileColorFormat color_format;
} AnimFileMeta;

typedef struct {
    size_t start;
    size_t end;
    size_t start_offset;
    size_t start_duration_override;
    AnimFilePlayFlag flags;
} AnimFileRange;

/**
 * @brief Playback state
 */
typedef struct {
    AnimFileFrameHeader frame_hdr;
    uint8_t* encoded_buffer; // may not be present
    uint8_t* packed_buffer;

    size_t file_offset;
    size_t disp_frame_idx;
    size_t remaining_duration;

    bool did_load_frame;
    bool did_display_frame;
    AnimFileFlag forced_flags;
} AnimFilePlayback;

struct AnimFile {
    File* file;
    AnimFileMeta meta;
    AnimFilePlayback playback;

    AnimFileRange active_range;
    AnimFileRange pending_range;
    bool is_pending_range;
};

// ================
// anim_file_load.c
// ================

bool anim_file_read_header(AnimFileHeader* header, File* file);

bool anim_file_read_sections(const AnimFileHeader* header, uint8_t** buffer, File* file);

void anim_file_iterate_sections(
    const AnimFileHeader* header,
    const uint8_t* sections,
    AnimFileSectionCallback callback,
    void* context);

size_t anim_file_count_sections(const AnimFileHeader* header, const uint8_t* sections);

size_t anim_file_count_display_frames(const AnimFileHeader* header, File* file);

bool anim_file_validate_section_0(
    const AnimFileHeader* header,
    const uint8_t* buffer,
    size_t frame_count);

// =================
// anim_file_start.c
// =================

bool anim_file_compute_start(
    AnimFile* anim,
    AnimFilePlayFlag flags,
    size_t start_frame,
    size_t end_frame);

void anim_file_set_precomputed_start(
    AnimFile* anim,
    AnimFilePlayFlag flags,
    const AnimFileSection* section);

// ===============
// anim_file_img.c
// ===============

size_t anim_file_packed_length(const AnimFileHeader* file_hdr);

AnimFileFrameFlag anim_file_frame_flags(const AnimFile* anim);

bool anim_file_set_new_active(AnimFile* anim);

bool anim_file_load_current_frame(AnimFile* anim);

bool anim_file_decode_frame(AnimFile* anim, uint8_t* buffer);

#ifdef __cplusplus
}
#endif
