/**
 * @brief Public API for `AnimFile`
 */

#include "anim_file_i.h"

AnimFile* FURI_WARN_UNUSED anim_file_alloc(Storage* storage, const char* path) {
    furi_check(storage);
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

        size_t frame_count = anim_file_count_display_frames(&header, file);
        if(frame_count == 0) break;

        if(!anim_file_validate_section_0(&header, sections_chunk, frame_count)) break;

        AnimFile anim = {
            .file = file,
            .meta =
                {
                    .info =
                        {
                            .fps = header.fps,
                            .width = header.width,
                            .height = header.height,
                            .frames = frame_count,
                        },
                    .color_format = header.color_format,
                    .section_count = section_count,
                    .sections = sections_chunk,
                    .header = header,
                },
        };

        if(header.max_encoded_length) {
            anim.playback.encoded_buffer = malloc(header.max_encoded_length);
        }
        anim.playback.packed_buffer = malloc(anim_file_packed_length(&header));

        if(!anim_file_set_section_indexed(
               &anim, AnimFilePlayFlagNone, ANIM_FILE_WHOLE_SECTION_INDEX)) {
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
    if(anim->playback.encoded_buffer) free(anim->playback.encoded_buffer);
    free(anim->playback.packed_buffer);
    free(anim->meta.sections);
    storage_file_free(anim->file);
    free(anim);
}

AnimFileInfo anim_file_info(const AnimFile* anim) {
    furi_check(anim);
    return anim->meta.info;
}

AnimFileFrameInfo anim_file_frame(AnimFile* anim, void* buffer) {
    furi_check(anim);
    furi_check(buffer);

    AnimFileFrameInfo info;

    do {
        if(!anim_file_load_current_frame(anim)) {
            info.flags = AnimFileFrameFlagError;
            break;
        }

        if(!anim->playback.did_display_frame) {
            if(!anim_file_decode_frame(anim, buffer)) {
                info.flags = AnimFileFrameFlagError;
                break;
            }
            anim->playback.did_display_frame = true;
        }

        info.flags = anim_file_frame_flags(anim) | anim->playback.forced_flags;
        info.index = anim->playback.disp_frame_idx;

        anim->playback.forced_flags = 0;
    } while(0);

    return info;
}

bool FURI_WARN_UNUSED
    anim_file_set_section_manual(AnimFile* anim, AnimFilePlayFlag flags, size_t start, size_t end) {
    furi_check(anim);
    if(!anim_file_compute_start(anim, flags, start, end)) return false;
    return true;
}

bool FURI_WARN_UNUSED
    anim_file_set_section_indexed(AnimFile* anim, AnimFilePlayFlag flags, size_t index) {
    furi_check(anim);

    const AnimFileHeader* header = &anim->meta.header;
    const uint8_t* sections = anim->meta.sections;
    const AnimFileSection* section = NULL;

    void callback(size_t cur_index, const AnimFileSection* cur_section, void* context) {
        UNUSED(context);
        if(cur_index == index) section = cur_section;
    }

    anim_file_iterate_sections(header, sections, callback, &section);

    if(section) {
        anim_file_set_precomputed_start(anim, flags, section);
        return true;
    }
    return false;
}

bool FURI_WARN_UNUSED
    anim_file_set_section_named(AnimFile* anim, AnimFilePlayFlag flags, const char* name) {
    furi_check(anim);
    furi_check(name);

    const AnimFileHeader* header = &anim->meta.header;
    const uint8_t* sections = anim->meta.sections;
    const AnimFileSection* section = NULL;

    void callback(size_t cur_index, const AnimFileSection* cur_section, void* context) {
        UNUSED(cur_index);
        UNUSED(context);
        if(strcmp(cur_section->name, name) == 0) section = cur_section;
    }

    anim_file_iterate_sections(header, sections, callback, &section);

    if(section) {
        anim_file_set_precomputed_start(anim, flags, section);
        return true;
    }
    return false;
}
