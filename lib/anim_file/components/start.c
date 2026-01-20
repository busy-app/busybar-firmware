#include <anim_file_i.h>

static void anim_file_start_set_requested(AnimFile* anim, const AnimFileRange* requested) {
    furi_assert(anim);
    furi_assert(requested);

    bool wait_for_last = requested->flags & AnimFilePlayFlagFinishCurrentSection;
    bool on_last_frame = anim_file_seq_disp_frame_idx(anim) == anim->start.active.end;

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

bool anim_file_start_compute(
    AnimFile* anim,
    AnimFilePlayFlag flags,
    size_t start_frame,
    size_t end_frame) {
    furi_assert(anim);

    const AnimFileHeader* header = &anim->meta.header;
    size_t start = sizeof(*header) + header->sections_chunk_length;
    if(!storage_file_seek(anim->file, start, true)) return false;

    size_t frame_offset = start;
    size_t size = header->frames_chunk_length;
    size_t file_frame_idx = 0;
    size_t disp_frame_idx = 0;
    AnimFileFrameHeader frame_hdr;

    while(size) {
        if(storage_file_read(anim->file, &frame_hdr, sizeof(frame_hdr)) != sizeof(frame_hdr))
            return false;

        size_t first_disp_frame = disp_frame_idx;
        size_t last_disp_frame = disp_frame_idx + frame_hdr.duration - 1;
        if((start_frame >= first_disp_frame) && (start_frame <= last_disp_frame)) {
            break;
        }

        size_t advance = sizeof(frame_hdr) + frame_hdr.encoded_length;
        frame_offset += advance;
        size -= advance;
        if(!storage_file_seek(anim->file, frame_hdr.encoded_length, false)) return false;
        file_frame_idx++;
        disp_frame_idx += frame_hdr.duration;
    }

    const AnimFileRange range = {
        .start = start_frame,
        .end = end_frame,
        .flags = flags,
        .start_offset = frame_offset,
        .start_duration_override = frame_hdr.duration - (start_frame - disp_frame_idx),
    };
    anim_file_start_set_requested(anim, &range);

    return true;
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
        .start_duration_override = section->duration_override,
    };
    anim_file_start_set_requested(anim, &range);
}
