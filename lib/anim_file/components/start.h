/**
 * @file img.h
 * Animation file start parameters management
 */

#pragma once

#include <anim_file_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    AnimFileRange active;
    AnimFileRange pending;
    bool is_pending;
} AnimFileStart;

/**
 * @returns whether the animation engine should continue
 */
bool anim_file_start_last_frame(AnimFile* anim);

bool anim_file_start_compute(
    AnimFile* anim,
    AnimFilePlayFlag flags,
    size_t start_frame,
    size_t end_frame);

void anim_file_start_set_precomputed(
    AnimFile* anim,
    AnimFilePlayFlag flags,
    const AnimFileSection* section);

#ifdef __cplusplus
}
#endif
