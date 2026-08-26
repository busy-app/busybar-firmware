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

// Debugging flags
#define ANIM_FILE_DETAILED_ERRORS
// #define ANIM_FILE_SHOW_MASK_INSTEAD_OF_IMAGE

#ifdef ANIM_FILE_DETAILED_ERRORS
#define ANIM_FILE_ERR(...) FURI_LOG_E(TAG, __VA_ARGS__)
#else
#define ANIM_FILE_ERR(...) FURI_LOG_E(TAG, "Error! Enable ANIM_FILE_DETAILED_ERRORS for detail")
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
    AnimFilePlayFlag flags;
} AnimFileRange;

#include "components/anim_file_img.h"
#include "components/anim_file_load.h"
#include "components/anim_file_seq.h"
#include "components/anim_file_start.h"
#include "components/anim_file_mask.h"

// TODO: move to `anim_file_mask.h`. doesn't compile though for some reason.
typedef struct {
    uint8_t* mask_buffer;
} AnimFileMask;

struct AnimFile {
    File* file;
    AnimFileMeta meta;

    // Components should not touch other components' state directly.
    AnimFileImg img;
    AnimFileSeq seq;
    AnimFileStart start;
    AnimFileMask mask;
};

#ifdef __cplusplus
}
#endif
