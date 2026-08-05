#include <anim_file_i.h>

void anim_file_seq_new_active(AnimFile* anim, const AnimFileRange* range, AnimFileFrameFlag flags) {
    furi_assert(anim);

    AnimFileSeq* seq = &anim->seq;

    if(range) {
        seq->disp_frame_idx = range->start;
        seq->last_disp_frame = range->end;
        seq->requested_file_frame = range->start_offset;
        seq->remaining_duration = range->start_duration_override;
    }
    seq->flags |= flags;
}

size_t anim_file_seq_disp_frame_idx(AnimFile* anim) {
    furi_assert(anim);
    return anim->seq.disp_frame_idx;
}

AnimFileFrameFlag anim_file_seq_load_current_frame(AnimFile* anim) {
    furi_assert(anim);

    AnimFileSeq* seq = &anim->seq;
    AnimFileFrameHeader* frame_hdr = &seq->frame_hdr;

    bool only_frame = anim->meta.info.frames == 1;

    if((seq->loaded_file_frame == seq->requested_file_frame) && !only_frame) {
        if(!seq->remaining_duration) return AnimFileFrameFlagNoChange;

        if(seq->disp_frame_idx == seq->last_disp_frame) {
            if(!anim_file_start_last_frame(anim)) {
                AnimFileFrameFlag flags = seq->flags;

                seq->flags = AnimFileFrameFlagNone;
                seq->remaining_duration = 0;

                return flags;
            }
        }

        seq->disp_frame_idx++;
        if(--seq->remaining_duration > 0) return AnimFileFrameFlagNoChange;

        seq->requested_file_frame += sizeof(*frame_hdr) + frame_hdr->encoded_length;
    }

    if(!storage_file_seek(anim->file, seq->requested_file_frame, true)) {
        ANIM_FILE_ERR("Failed to seek frame header");
        return AnimFileFrameFlagError;
    }

    size_t to_read = sizeof(*frame_hdr);
    if(storage_file_read(anim->file, frame_hdr, to_read) != to_read) {
        ANIM_FILE_ERR("Failed to read frame header");
        return AnimFileFrameFlagError;
    }

    uint8_t* frame_destination = anim_file_img_encoded_buffer(anim, frame_hdr->encoding);
    if(frame_hdr->encoding != AnimFileFrameEncodingRaw) {
        if(frame_hdr->encoded_length > anim->meta.header.max_encoded_length) {
            ANIM_FILE_ERR("Invalid file header: frame.encoded_length > max_encoded_length");
            return AnimFileFrameFlagError;
        }
    }
    if(!frame_hdr->duration) {
        ANIM_FILE_ERR("Invalid frame header: duration = 0");
        return AnimFileFrameFlagError;
    }

    to_read = frame_hdr->encoded_length;
    if(storage_file_read(anim->file, frame_destination, frame_hdr->encoded_length) != to_read) {
        ANIM_FILE_ERR("Invalid frame encoded_length");
        return AnimFileFrameFlagError;
    }

    seq->loaded_file_frame = seq->requested_file_frame;

    bool duration_was_overridden_externally = !!seq->remaining_duration;
    if(!duration_was_overridden_externally) seq->remaining_duration = frame_hdr->duration;

    if(!anim_file_img_full_decode(anim, frame_hdr)) {
        return AnimFileFrameFlagError;
    }

    AnimFileFrameFlag flags = AnimFileFrameFlagNone;

    if(seq->disp_frame_idx == seq->last_disp_frame) {
        flags |= AnimFileFrameFlagLast;
        anim_file_start_last_frame(anim);
    }

    flags |= seq->flags;
    seq->flags = AnimFileFrameFlagNone;

    return flags;
}

void anim_file_seq_redraw_current_frame(AnimFile* anim) {
    furi_assert(anim);

    AnimFileSeq* seq = &anim->seq;

    if(!seq->loaded_file_frame) return;

    if(!storage_file_seek(anim->file, seq->loaded_file_frame + sizeof(AnimFileFrameHeader), true)) {
        ANIM_FILE_ERR("Failed to seek frame");
        return;
    }

    const AnimFileFrameHeader* frame_hdr = &seq->frame_hdr;
    uint8_t* frame_destination = anim_file_img_encoded_buffer(anim, frame_hdr->encoding);
    size_t to_read = frame_hdr->encoded_length;
    if(storage_file_read(anim->file, frame_destination, frame_hdr->encoded_length) != to_read) {
        ANIM_FILE_ERR("Failed to read frame");
        return;
    }

    anim_file_img_full_decode(anim, frame_hdr);
}
