/**
 * @brief Public API for `AnimFile`
 */

#include "anim_file_i_struct.h"
#include <toolbox/dsp.h>

AnimFile* FURI_WARN_UNUSED
    anim_file_alloc(Storage* storage, const char* path, AnimFileOption options) {
    furi_check(storage);
    furi_check(path);
    furi_check(options < AnimFileOptionMAX);

    AnimFile* result = NULL;
    File* file = storage_file_alloc(storage);
    uint8_t* sections_chunk = NULL;

    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            ANIM_FILE_ERR("Failed to open file: %s", path);
            break;
        }

        AnimFileHeader header;
        if(!anim_file_load_header(&header, file)) break;

        sections_chunk = anim_file_load_sections(&header, file);
        if(!sections_chunk) break;

        if(!anim_file_load_validate_section_0(&header, sections_chunk)) break;

        AnimFile anim = {
            .file = file,
            .options = options,
            .meta =
                {
                    .info =
                        {
                            .fps = header.fps,
                            .width = header.width,
                            .height = header.height,
                            .frames = header.frame_count,
                        },
                    .color_format = header.color_format,
                    .sections = sections_chunk,
                    .header = header,
                },
        };

        if(!anim_file_set_section(&anim, AnimFilePlayFlagNone, ANIM_FILE_DEFAULT_SECTION)) {
            ANIM_FILE_ERR("Failed to set section 0");
            break;
        }

        result = malloc(sizeof(anim));
        *result = anim;

        anim_file_mask_init(result);
    } while(0);

    if(!result) {
        storage_file_free(file);
        if(sections_chunk) free(sections_chunk);
    }

    return result;
}

void anim_file_free(AnimFile* anim) {
    furi_check(anim);
    anim_file_img_deinit(anim);
    anim_file_mask_deinit(anim);
    storage_file_free(anim->file);
    free(anim->meta.sections);
    free(anim);
}

AnimFileInfo anim_file_info(const AnimFile* anim) {
    furi_check(anim);
    return anim->meta.info;
}

void anim_file_set_out_buf(AnimFile* anim, size_t width, size_t height, void* buffer) {
    furi_check(anim);
    furi_check(buffer);
    FURI_LOG_D(TAG, "%p, w=%zu h=%zu buf=%p", anim, width, height, buffer);
    anim_file_img_init(anim, buffer, width, height);
}

AnimFileFrameInfo anim_file_frame(AnimFile* anim) {
    furi_check(anim);

#ifdef ANIM_FILE_PROFILE_PERFORMANCE
    anim->profiler = profiler_alloc(TAG);
    profiler_start(anim->profiler, "frame");
#endif

    AnimFileFrameInfo info;
    info.index = anim_file_seq_frame_idx(anim);
    info.flags = anim_file_seq_draw_requested_and_go_to_next(anim);

#ifdef ANIM_FILE_PROFILE_PERFORMANCE
    profiler_stop(anim->profiler, "frame");
    profiler_dump(anim->profiler);
    profiler_free(anim->profiler);
#endif

    return info;
}

bool FURI_WARN_UNUSED
    anim_file_set_section(AnimFile* anim, AnimFilePlayFlag flags, const char* name) {
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

    if(!anim_file_load_iterate_sections(header, sections, callback, &section)) return false;

    if(section) {
        anim_file_start_set_precomputed(anim, flags, section);
        return true;
    }
    return false;
}

void anim_file_set_offset(AnimFile* anim, float x, float y) {
    furi_check(anim);

    if((fabsf(x) > DSP_EPSILON) || (fabsf(y) > DSP_EPSILON)) {
        furi_check(anim->options & AnimFileOptionIntermediateInternalBuffer);
    }

    anim_file_img_set_cutout(anim, -x, -y);
    anim_file_seq_redraw_current_frame(anim);
}
