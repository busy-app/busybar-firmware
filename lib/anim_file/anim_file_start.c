/**
 * @brief Computation and loading of precomputed start parameters
 */

#include "anim_file_i.h"

bool anim_file_compute_start(AnimFile* anim, AnimFilePlayFlag flags, size_t start_frame, size_t end_frame) {
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
        if(storage_file_read(anim->file, &frame_hdr, sizeof(frame_hdr)) != sizeof(frame_hdr)) return false;

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

    anim->pending_range = (AnimFileRange){
        .start = start_frame,
        .end = end_frame,
        .flags = flags,
        .start_offset = frame_offset,
        .start_duration_override = frame_hdr.duration - (start_frame - disp_frame_idx),
    };
    anim->is_pending_range = true;

    return true;
}

void anim_file_set_precomputed_start(AnimFile* anim, AnimFilePlayFlag flags, const AnimFileSection* section) {
    furi_assert(section);

    anim->pending_range = (AnimFileRange){
        .start = section->start,
        .end = section->end,
        .flags = flags,
        .start_offset = section->frame_offs,
        .start_duration_override = section->duration_override,
    };
    anim->is_pending_range = true;
}
