#include "anim_file.h"
#include "anim_file_format.h"

#define TAG "AnimFile"

#define ANIM_FILE_DETAILED_ERRORS

#ifdef ANIM_FILE_DETAILED_ERRORS
#define ANIM_FILE_ERR(...) FURI_LOG_E(TAG, __VA_ARGS__)
#else
#define ANIM_FILE_ERR(...) FURI_LOF_E(TAG, "Load error")
#endif

// =====
// Types
// =====

/**
 * @brief Information about loaded file
 */
typedef struct {
    AnimFileHeader header;
    AnimFileInfo info; //<! Publicly available information
    size_t section_count;
    uint8_t* sections;
    AnimFileColorFormat color_format;
} AnimFileMeta;

typedef struct {
    size_t start;
    size_t end;
    size_t start_offset;
    size_t start_duration_override;
    AnimFilePlayFlag flags;
} AnimFileRange;

/**
 * @brief Playback state
 */
typedef struct {
    AnimFileFrameHeader frame_hdr;
    uint8_t* encoded_frame_buffer;

    size_t file_offset;
    size_t disp_frame_idx;
    size_t remaining_duration;

    bool did_display_frame;
} AnimFilePlayback;

struct AnimFile {
    File* file;
    AnimFileMeta meta;
    AnimFilePlayback playback;

    AnimFileRange active_range;
    AnimFileRange pending_range;
    bool is_pending_range;
};

// =========================
// File parsing & validation
// =========================

static bool anim_file_read_header(AnimFileHeader* header, File* file) {
    furi_assert(header);
    furi_assert(file);

    if(!storage_file_seek(file, 0, true)) {
        ANIM_FILE_ERR("Failed to seek header");
        return false;
    }
    if(storage_file_read(file, header, sizeof(*header)) != sizeof(*header)) {
        ANIM_FILE_ERR("Incomplete header");
        return false;
    }
    if(memcmp(header->signature, ANIM_FILE_HEADER_SIGNATURE, sizeof(header->signature)) != 0) {
        ANIM_FILE_ERR("Invalid signature");
        return false;
    }
    if(header->color_format >= AnimFileColorFormatMAX) {
        ANIM_FILE_ERR("Invalid color format");
        return false;
    }
    if(header->flags >= AnimFileFlagMAX) {
        ANIM_FILE_ERR("Invalid flags");
        return false;
    }
    if(storage_file_size(file) != sizeof(*header) + header->sections_chunk_length + header->frames_chunk_length) {
        ANIM_FILE_ERR("Invalid size");
        return false;
    }

    return true;
}

static bool anim_file_read_sections(const AnimFileHeader* header, uint8_t** buffer, File* file) {
    furi_assert(header);
    furi_assert(file);

    if(!storage_file_seek(file, sizeof(*header), true)) {
        ANIM_FILE_ERR("Failed to seek Sections chunk");
        return false;
    }

    size_t chunk_len = header->sections_chunk_length;
    *buffer = malloc(chunk_len);

    if(storage_file_read(file, *buffer, chunk_len) != chunk_len) {
        ANIM_FILE_ERR("Incomplete Sections chunk");
        return false;
    }
    if((*buffer)[chunk_len - 1] != 0x00) {
        ANIM_FILE_ERR("Invalid Sections chunk termination");
        return false;
    }

    return true;
}

static size_t anim_file_count_sections(const AnimFileHeader* header, const uint8_t* buffer) {
    furi_assert(header);
    furi_assert(buffer);

    size_t size = header->sections_chunk_length;
    size_t section_cnt = 0;
    size_t pos = 0;

    while(pos < size) {
        if((size - pos) < (sizeof(AnimFileSection) + 1)) break;

        const AnimFileSection* section = (const AnimFileSection*)(buffer + pos);
        pos += sizeof(AnimFileSection) + strlen(section->name) + 1;
        section_cnt++;
    }

    if(section_cnt == 0) ANIM_FILE_ERR("Empty Sections chunk");
    return section_cnt;
}

static size_t anim_file_count_frames(const AnimFileHeader* header, File* file, size_t* disp_frames, size_t* max_encoded_len) {
    furi_assert(header);
    furi_assert(file);

    size_t start = sizeof(*header) + header->sections_chunk_length;
    if(!storage_file_seek(file, start, true)) {
        ANIM_FILE_ERR("Failed to seek Frames chunk");
        return 0;
    }

    size_t size = header->frames_chunk_length;
    size_t frame_cnt = 0;

    while(size) {
        AnimFileFrameHeader frame_hdr;
        if(storage_file_read(file, &frame_hdr, sizeof(frame_hdr)) != sizeof(frame_hdr)) {
            ANIM_FILE_ERR("Incomplete Frames chunk");
            return 0;
        }

        if(frame_hdr.encoding >= AnimFileFrameEncodingMAX) {
            ANIM_FILE_ERR("Invalid frame encoding");
            return 0;
        }

        frame_cnt++;
        (*disp_frames) += frame_hdr.duration;
        *max_encoded_len = MAX(*max_encoded_len, frame_hdr.encoded_length);

        size -= sizeof(frame_hdr) + frame_hdr.encoded_length;
        if(!storage_file_seek(file, frame_hdr.encoded_length, false)) {
            ANIM_FILE_ERR("Failed to seek next frame");
            return 0;
        }
    }

    if(frame_cnt == 0) ANIM_FILE_ERR("Empty Frames chunk");
    return frame_cnt;
}

static bool anim_file_validate_section_0(const AnimFileHeader* header, const uint8_t* buffer, size_t frame_count) {
    furi_assert(header);
    furi_assert(buffer);

    const AnimFileSection* section = (const AnimFileSection*)buffer;

    if(strcmp(section->name, ANIM_FILE_WHOLE_SECTION_NAME) != 0) {
        ANIM_FILE_ERR("Invalid section 0 name");
        return false;
    }
    if((section->start != 0) || (section->end != frame_count - 1)) {
        ANIM_FILE_ERR("Invalid section 0 range");
        return false;
    }

    size_t expected_offs = sizeof(*header) + header->sections_chunk_length;
    if(section->frame_offs != expected_offs) {
        ANIM_FILE_ERR("Invalid section 0 precomputed start info");
        return false;
    }

    return true;
}

// ===============================
// Computation of start parameters
// ===============================

static bool anim_file_compute_start(AnimFile* anim, AnimFilePlayFlag flags, size_t start_frame, size_t end_frame) {
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

static void anim_file_set_precomputed_start(AnimFile* anim, AnimFilePlayFlag flags, const AnimFileSection* section) {
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

// ================
// Playback helpers
// ================

static AnimFileFrameFlag anim_file_frame_flags(const AnimFile* anim) {
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

static void anim_file_load_current_frame(AnimFile* anim) {
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
        storage_file_read(anim->file, playback->encoded_frame_buffer, frame_hdr->encoded_length);
        playback->did_display_frame = false;
        playback->remaining_duration = (flags & AnimFileFrameFlagSwitchToRequested) ? active->start_duration_override : frame_hdr->duration;
    }
}

void anim_file_decode_frame(AnimFile* anim, uint8_t* buffer) {
    furi_assert(anim);
    furi_assert(buffer);

    const AnimFileFrameHeader* header = &anim->playback.frame_hdr;
    AnimFileInfo info = anim->meta.info;
    AnimFileColorFormat color_fmt = anim->meta.color_format;
    size_t canvas_sz = info.width * info.height * 3;

    // TODO: RLE
    furi_check(header->encoding == AnimFileFrameEncodingRaw);
    uint8_t* packed_color = anim->playback.encoded_frame_buffer;

    if(color_fmt == AnimFileColorFormatBgr888) {
        memcpy(buffer, packed_color, canvas_sz);

    } else if(color_fmt == AnimFileColorFormatGray4) {
        size_t packed_color_sz = info.width * info.height / 2;

        size_t x = 0;
        size_t y = 0;
        size_t dest_idx = 0;
        for(size_t i = 0; i < packed_color_sz; i++) {
            uint8_t left_px = packed_color[i] & 0xF0;
            uint8_t right_px = packed_color[i] << 4;
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

// ==========
// Public API
// ==========

AnimFile* anim_file_alloc(Storage* storage, const char* path) {
    furi_check(path);

    AnimFile* result = NULL;
    File* file = storage_file_alloc(storage);
    uint8_t* sections_chunk = NULL;

    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            ANIM_FILE_ERR("Failed to open file: %s", path);
            break;
        }

        AnimFileHeader header;
        if(!anim_file_read_header(&header, file)) break;

        if(!anim_file_read_sections(&header, &sections_chunk, file)) break;

        size_t section_count = anim_file_count_sections(&header, sections_chunk);
        if(section_count == 0) break;

        size_t max_encoded_len = 0;
        size_t disp_frame_count = 0;
        size_t frame_count = anim_file_count_frames(&header, file, &disp_frame_count, &max_encoded_len);
        if(frame_count == 0) break;

        if(!anim_file_validate_section_0(&header, sections_chunk, disp_frame_count)) break;

        AnimFile anim = {
            .file = file,
            .meta = {
                .info = {
                    .fps = header.fps,
                    .width = header.width,
                    .height = header.height,
                },
                .color_format = header.color_format,
                .section_count = section_count,
                .sections = sections_chunk,
                .header = header,
            },
        };

        anim.playback.encoded_frame_buffer = malloc(max_encoded_len);
        if(!anim_file_set_section_indexed(&anim, AnimFilePlayFlagNone, ANIM_FILE_WHOLE_SECTION_INDEX)) {
            ANIM_FILE_ERR("Failed to set section 0");
            break;
        }

        result = malloc(sizeof(anim));
        *result = anim;
    } while(0);

    if(!result) {
        storage_file_free(file);
        if(sections_chunk) free(sections_chunk);
    }

    return result;
}

void anim_file_free(AnimFile* anim) {
    furi_check(anim);
    free(anim->playback.encoded_frame_buffer);
    free(anim->meta.sections);
    storage_file_free(anim->file);
    free(anim);
}

AnimFileInfo anim_file_info(const AnimFile* anim) {
    furi_check(anim);
    return anim->meta.info;
}

AnimFileFrameFlag anim_file_frame(AnimFile* anim, void* buffer) {
    furi_check(anim);

    anim_file_load_current_frame(anim);
    if(!anim->playback.did_display_frame) {
        anim_file_decode_frame(anim, buffer);
    }

    return anim_file_frame_flags(anim);
}

bool anim_file_set_section_manual(AnimFile* anim, AnimFilePlayFlag flags, size_t start, size_t end) {
    furi_check(anim);
    if(!anim_file_compute_start(anim, flags, start, end)) return false;
    return true;
}

bool anim_file_set_section_indexed(AnimFile* anim, AnimFilePlayFlag flags, size_t index) {
    furi_check(anim);

    const AnimFileHeader* header = &anim->meta.header;
    const uint8_t* sections = anim->meta.sections;
    size_t size = header->sections_chunk_length;
    size_t section_idx = 0;
    size_t pos = 0;

    while(pos < size) {
        if((size - pos) < (sizeof(AnimFileSection) + 1)) break;
        const AnimFileSection* section = (const AnimFileSection*)(sections + pos);

        if(section_idx == index) {
            anim_file_set_precomputed_start(anim, flags, section);
            return true;
        }
        
        pos += sizeof(AnimFileSection) + strlen(section->name) + 1;
        section_idx++;
    }

    return false;
}

bool anim_file_set_section_named(AnimFile* anim, AnimFilePlayFlag flags, const char* name) {
    furi_check(anim);

    const AnimFileHeader* header = &anim->meta.header;
    const uint8_t* sections = anim->meta.sections;
    size_t size = header->sections_chunk_length;
    size_t pos = 0;

    while(pos < size) {
        if((size - pos) < (sizeof(AnimFileSection) + 1)) break;
        const AnimFileSection* section = (const AnimFileSection*)(sections + pos);

        if(strcmp(section->name, name) == 0) {
            anim_file_set_precomputed_start(anim, flags, section);
            return true;
        }
        
        pos += sizeof(AnimFileSection) + strlen(section->name) + 1;
    }

    return false;
}
