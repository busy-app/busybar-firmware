/**
 * @file anim_file_start.h
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

void anim_file_start_set_precomputed(
    AnimFile* anim,
    AnimFilePlayFlag flags,
    const AnimFileSection* section);

#ifdef __cplusplus
}
#endif
