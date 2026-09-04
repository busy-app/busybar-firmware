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
    size_t requested_frame_offset;
    size_t loaded_frame_offset;
    AnimFileFrameHeader frame_hdr;

    size_t frame_idx;
    size_t last_frame_idx;

    AnimFileFrameFlag external_flags;
} AnimFileSeq;

void anim_file_seq_new_active(AnimFile* anim, const AnimFileRange* range, AnimFileFrameFlag flags);

size_t anim_file_seq_frame_idx(AnimFile* anim);

AnimFileFrameFlag anim_file_seq_draw_requested_and_go_to_next(AnimFile* anim);

void anim_file_seq_redraw_current_frame(AnimFile* anim);

#ifdef __cplusplus
}
#endif
