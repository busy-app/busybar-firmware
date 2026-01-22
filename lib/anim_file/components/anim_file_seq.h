/**
 * @file anim_file_seq.h
 * Animation file image sequencing
 */

#pragma once

#include <anim_file_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t requested_file_frame;
    size_t loaded_file_frame;
    AnimFileFrameHeader frame_hdr;

    size_t disp_frame_idx;
    size_t last_disp_frame;
    size_t remaining_duration;

    AnimFileFrameFlag flags;
} AnimFileSeq;

void anim_file_seq_new_active(AnimFile* anim, const AnimFileRange* range, AnimFileFrameFlag flags);

size_t anim_file_seq_disp_frame_idx(AnimFile* anim);

AnimFileFrameFlag anim_file_seq_load_current_frame(AnimFile* anim);

#ifdef __cplusplus
}
#endif
