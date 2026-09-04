/**
 * @file anim_file_mask.h
 * Animation file mask parsing
 */

#pragma once

#include <anim_file_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t* mask_buffer;
} AnimFileMask;

uint8_t* anim_file_mask_buffer(AnimFile* anim_file);

void anim_file_mask_init(AnimFile* anim);

void anim_file_mask_deinit(AnimFile* anim);

typedef struct {
    size_t y;
    size_t x_start; //<! Inclusive
    size_t x_end; //<! Exclusive
} AnimFileMaskPixelRange;

typedef void (*AnimFileMaskPixelRangeCallback)(AnimFileMaskPixelRange range, void* context);

void anim_file_mask_iterate(
    AnimFile* anim,
    const AnimFileFrameHeader* frame,
    AnimFileMaskPixelRangeCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
