#include <anim_file_i_struct.h>

static void anim_file_start_set_requested(AnimFile* anim, const AnimFileRange* requested) {
    furi_assert(anim);
    furi_assert(requested);

    bool wait_for_last = requested->flags & AnimFilePlayFlagFinishCurrent;
    bool on_last_frame = anim_file_seq_frame_idx(anim) == anim->start.active.end;

    if(wait_for_last && !on_last_frame) {
        anim->start.pending = *requested;
        anim->start.is_pending = true;
    } else {
        anim->start.active = *requested;
        anim_file_seq_new_active(anim, requested, AnimFileFrameFlagSwitchToRequested);
        anim->start.is_pending = false;
    }
}

bool anim_file_start_last_frame(AnimFile* anim) {
    furi_assert(anim);

    const AnimFileRange* pending = &anim->start.pending;

    if(anim->start.is_pending) {
        anim->start.active = *pending;
        anim_file_seq_new_active(anim, pending, AnimFileFrameFlagSwitchToRequested);
        anim->start.is_pending = false;
        return true;
    } else if(anim->start.active.flags & AnimFilePlayFlagLoop) {
        anim_file_seq_new_active(anim, &anim->start.active, AnimFileFrameFlagLooping);
        return true;
    } else {
        anim_file_seq_new_active(anim, NULL, AnimFileFrameFlagFinished);
        return false;
    }
}

void anim_file_start_set_precomputed(
    AnimFile* anim,
    AnimFilePlayFlag flags,
    const AnimFileSection* section) {
    furi_assert(section);

    const AnimFileRange range = {
        .start = section->start,
        .end = section->end,
        .flags = flags,
        .start_offset = section->frame_offs,
    };
    anim_file_start_set_requested(anim, &range);
}
