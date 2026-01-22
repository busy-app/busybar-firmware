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

#include "components/anim_file_img.h"
#include "components/anim_file_load.h"
#include "components/anim_file_seq.h"
#include "components/anim_file_start.h"

struct AnimFile {
    File* file;
    AnimFileMeta meta;

    // Components should not touch other components' state directly.
    AnimFileImg img;
    AnimFileSeq seq;
    AnimFileStart start;
};

#ifdef __cplusplus
}
#endif
