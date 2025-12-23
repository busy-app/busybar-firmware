/**
 * @brief Image decoding functions
 */

#include "anim_file_i.h"

#include <toolbox/rle_encode.h>

size_t anim_file_packed_length(const AnimFileHeader* file_hdr) {
    furi_assert(file_hdr);

    if(file_hdr->color_format == AnimFileColorFormatBgr888) {
        return file_hdr->width * file_hdr->height * 3;
    } else if(file_hdr->color_format == AnimFileColorFormatGray4) {
        return file_hdr->width * file_hdr->height / 2;
    } else {
        furi_crash();
    }
}

AnimFileFrameFlag anim_file_frame_flags(const AnimFile* anim) {
    furi_assert(anim);

    AnimFileFrameFlag flags = 0;

    const AnimFileRange* active = &anim->active_range;
    const AnimFileRange* pending = &anim->pending_range;
    const AnimFilePlayback* playback = &anim->playback;
    
    bool is_last_frame = playback->disp_frame_idx == active->end;
    if(is_last_frame) flags |= AnimFileFrameFlagLast;

    bool finish_cur_section = !!(pending->flags & AnimFilePlayFlagFinishCurrentSection);
    bool switch_to_pend_allowed = (finish_cur_section && is_last_frame) || !finish_cur_section;
    bool is_pending = anim->is_pending_range;
    bool switch_to_pending = false;

    if(is_pending && switch_to_pend_allowed) {
        switch_to_pending = true;
        flags |= AnimFileFrameFlagSwitchToRequested;
    }

    if(is_last_frame && (active->flags & AnimFilePlayFlagLoop) && !is_pending) {
        switch_to_pending = true;
        flags |= AnimFileFrameFlagLooping;
    }

    if(is_last_frame && !switch_to_pending) {
        flags |= AnimFileFrameFlagFinished;
    }

    return flags;
}

void anim_file_load_current_frame(AnimFile* anim) {
    furi_assert(anim);

    AnimFileRange* active = &anim->active_range;
    AnimFileRange* pending = &anim->pending_range;
    AnimFilePlayback* playback = &anim->playback;
    AnimFileFrameHeader* frame_hdr = &playback->frame_hdr;

    AnimFileFrameFlag flags = anim_file_frame_flags(anim);
    bool read_next_frame = false;

    if(flags & (AnimFileFrameFlagSwitchToRequested | AnimFileFrameFlagLooping)) {
        // load frame from start of range
        if(flags & AnimFileFrameFlagLooping) {
            *pending = *active;
        } else {
            anim->is_pending_range = false;
        }
        *active = *pending;
        playback->disp_frame_idx = active->start;
        playback->file_offset = active->start_offset;
        read_next_frame = true;

    } else if(!(flags & AnimFileFrameFlagFinished)) {
        // load next frame in range
        playback->disp_frame_idx++;
        if(--playback->remaining_duration == 0) {
            playback->file_offset += sizeof(*frame_hdr) + frame_hdr->encoded_length;
            read_next_frame = true;
        }
    }

    if(read_next_frame) {
        storage_file_seek(anim->file, playback->file_offset, true);
        storage_file_read(anim->file, frame_hdr, sizeof(*frame_hdr));

        uint8_t* frame_buffer = (frame_hdr->encoding != AnimFileFrameEncodingRaw) ? playback->encoded_buffer : playback->packed_buffer;
        furi_check(frame_buffer);
        storage_file_read(anim->file, frame_buffer, frame_hdr->encoded_length);

        playback->did_display_frame = false;
        playback->remaining_duration = (flags & AnimFileFrameFlagSwitchToRequested) ? active->start_duration_override : frame_hdr->duration;
    }
}

void anim_file_decode_frame(AnimFile* anim, uint8_t* buffer) {
    furi_assert(anim);
    furi_assert(buffer);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    const AnimFileFrameHeader* frame_hdr = &anim->playback.frame_hdr;
    AnimFileInfo info = anim->meta.info;
    const uint8_t* encoded_buf = anim->playback.encoded_buffer;
    uint8_t* packed_buf = anim->playback.packed_buffer;

    AnimFileColorFormat color_fmt = anim->meta.color_format;
    size_t packed_len = anim_file_packed_length(file_hdr);
    size_t blk_size = 0;

    if(color_fmt == AnimFileColorFormatBgr888) {
        blk_size = 3;
    } else if(color_fmt == AnimFileColorFormatGray4) {
        blk_size = 1;
    }

    if(frame_hdr->encoding == AnimFileFrameEncodingRle) {
        furi_check(encoded_buf);
        size_t decoded_sz = 0;
        furi_check(rle_decompress(encoded_buf, frame_hdr->encoded_length, packed_buf, packed_len, blk_size, &decoded_sz));
        furi_check(decoded_sz == packed_len);
    }

    if(color_fmt == AnimFileColorFormatBgr888) {
        memcpy(buffer, packed_buf, packed_len);

    } else if(color_fmt == AnimFileColorFormatGray4) {
        size_t x = 0;
        size_t y = 0;
        size_t dest_idx = 0;
        for(size_t i = 0; i < packed_len; i++) {
            uint8_t left_px = packed_buf[i] & 0xF0;
            uint8_t right_px = packed_buf[i] << 4;
            buffer[dest_idx + 0] = left_px;
            buffer[dest_idx + 1] = left_px;
            buffer[dest_idx + 2] = left_px;
            buffer[dest_idx + 3] = right_px;
            buffer[dest_idx + 4] = right_px;
            buffer[dest_idx + 5] = right_px;

            dest_idx += 6;
            x += 2;
            if(x >= info.width) {
                y++;
                x = 0;
            }
            if(y >= info.height) break;
        }
    }
}
