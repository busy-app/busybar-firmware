/**
 * @brief Internal functions of `AnimFile`
 */

#pragma once

#include "anim_file.h"
#include "anim_file_format.h"
#include <toolbox/profiler.h>

#ifdef __cplusplus
extern "C" {
#endif

// =========
// Debugging
// =========

#define TAG "AnimFile"

// Debugging flags
// #define ANIM_FILE_DETAILED_ERRORS
// #define ANIM_FILE_SHOW_MASK_INSTEAD_OF_IMAGE
// #define ANIM_FILE_PROFILE_PERFORMANCE
// #define ANIM_FILE_SHOW_PIPELINE

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

#ifdef __cplusplus
}
#endif
