#include <anim_file_i_struct.h>

/**
 * @brief Loads and renders the frame at the specified file offset.
 * Fills buffers with data as required, then asks the `img` component to render it.
 */
static AnimFileFrameFlag anim_file_seq_render_frame(AnimFile* anim, size_t file_offset) {
    furi_assert(anim);

    AnimFileSeq* seq = &anim->seq;
    AnimFileFrameHeader* frame_hdr = &seq->frame_hdr;

    if(!storage_file_seek(anim->file, file_offset, true)) {
        ANIM_FILE_ERR("Failed to seek frame header");
        return AnimFileFrameFlagError;
    }

    size_t to_read = sizeof(*frame_hdr);
    if(storage_file_read(anim->file, frame_hdr, to_read) != to_read) {
        ANIM_FILE_ERR("Failed to read frame header");
        return AnimFileFrameFlagError;
    }

    AnimFileMaskEncoding mask_encoding = anim_file_mask_encoding(frame_hdr->joint_encoding);
    AnimFilePixelEncoding px_encoding = anim_file_px_encoding(frame_hdr->joint_encoding);
    bool should_have_mask_data = mask_encoding >= AnimFileMaskEncodingRleFirstBlack;

    if(mask_encoding >= AnimFileMaskEncodingMAX) {
        ANIM_FILE_ERR("Invalid frame header: invalid frame.mask_encoding");
        return AnimFileFrameFlagError;
    }
    if(px_encoding >= AnimFilePixelEncodingMAX) {
        ANIM_FILE_ERR("Invalid frame header: invalid frame.pixel_encoding");
        return AnimFileFrameFlagError;
    }
    if(should_have_mask_data) {
        if(frame_hdr->mask_length > anim->meta.header.max_mask_length) {
            ANIM_FILE_ERR("Invalid file header: frame.mask_length > file.max_mask_length");
            return AnimFileFrameFlagError;
        }
    } else {
        if(frame_hdr->mask_length) {
            ANIM_FILE_ERR(
                "Invalid file header: frame.mask_length != 0 with frame.mask_encoding == Fully{Black,White}");
            return AnimFileFrameFlagError;
        }
    }
    if(px_encoding != AnimFilePixelEncodingRaw) {
        if(frame_hdr->pixel_length > anim->meta.header.max_pixel_length) {
            ANIM_FILE_ERR("Invalid file header: frame.pixel_length > file.max_pixel_length");
            return AnimFileFrameFlagError;
        }
    }

    uint8_t* mask_destination = anim_file_mask_buffer(anim);
    AnimFileBuffer* pixel_buffer = anim_file_img_initial_buffer(anim);

    to_read = ROUND_UP_TO(frame_hdr->mask_length, 8);
    if(storage_file_read(anim->file, mask_destination, to_read) != to_read) {
        ANIM_FILE_ERR("Invalid frame header: frame.mask_length_bytes lies outside of file");
        return AnimFileFrameFlagError;
    }

    to_read = frame_hdr->pixel_length;
    if(to_read > pixel_buffer->max_bytes) {
        ANIM_FILE_ERR(
            "Invalid frame header: frame.pixel_length larger than raw unencoded pixel data");
        return AnimFileFrameFlagError;
    }
    if(storage_file_read(anim->file, pixel_buffer->data, to_read) != to_read) {
        ANIM_FILE_ERR("Invalid frame header: frame.pixel_length lies outside of file");
        return AnimFileFrameFlagError;
    }

    pixel_buffer->content = AnimFileBufferContentFromFile;
    pixel_buffer->filled_bytes = to_read;
    seq->loaded_frame_offset = file_offset;

#ifdef ANIM_FILE_PROFILE_PERFORMANCE
    profiler_start(anim->profiler, "full_decode");
#endif

    if(!anim_file_img_full_decode(anim, frame_hdr)) return AnimFileFrameFlagError;

#ifdef ANIM_FILE_PROFILE_PERFORMANCE
    profiler_stop(anim->profiler, "full_decode");
#endif

    return AnimFileFrameFlagNone;
}

void anim_file_seq_new_active(AnimFile* anim, const AnimFileRange* range, AnimFileFrameFlag flags) {
    furi_assert(anim);

    AnimFileSeq* seq = &anim->seq;

    if(range) {
        seq->frame_idx = range->start;
        seq->last_frame_idx = range->end;
        seq->requested_frame_offset = range->start_offset;
    }
    seq->external_flags |= flags;
}

size_t anim_file_seq_frame_idx(AnimFile* anim) {
    furi_assert(anim);
    return anim->seq.frame_idx;
}

AnimFileFrameFlag anim_file_seq_draw_requested_and_go_to_next(AnimFile* anim) {
    furi_assert(anim);

    AnimFileSeq* seq = &anim->seq;
    AnimFileFrameHeader* frame_hdr = &seq->frame_hdr;

    if(seq->loaded_frame_offset == seq->requested_frame_offset) {
        bool on_last_frame_in_range = seq->frame_idx == seq->last_frame_idx;

        if(on_last_frame_in_range) {
            bool should_continue = anim_file_start_last_frame(anim);

            if(!should_continue) {
                AnimFileFrameFlag previous_flags = seq->external_flags;
                seq->external_flags = AnimFileFrameFlagNone;
                return previous_flags;
            }

        } else {
            seq->frame_idx++;

            size_t following_header =
                frame_hdr->pixel_length + ROUND_UP_TO(frame_hdr->mask_length, 8);
            seq->requested_frame_offset += sizeof(*frame_hdr) + following_header;
        }
    }

    AnimFileFrameFlag flags = anim_file_seq_render_frame(anim, seq->requested_frame_offset);
    if(flags & AnimFileFrameFlagError) return flags;

    if(seq->frame_idx == seq->last_frame_idx) flags |= AnimFileFrameFlagLast;

    flags |= seq->external_flags;
    seq->external_flags = AnimFileFrameFlagNone;

    return flags;
}

void anim_file_seq_redraw_current_frame(AnimFile* anim) {
    furi_assert(anim);

    AnimFileSeq* seq = &anim->seq;

    if(!seq->loaded_frame_offset) return;
    anim_file_seq_render_frame(anim, seq->loaded_frame_offset);
}
