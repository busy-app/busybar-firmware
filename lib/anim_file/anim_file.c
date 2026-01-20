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
        if(!anim_file_load_header(&header, file)) break;

        sections_chunk = anim_file_load_sections(&header, file);
        if(!sections_chunk) break;

        if(!anim_file_load_validate_section_0(&header, sections_chunk)) break;

        AnimFile anim = {
            .file = file,
            .meta =
                {
                    .info =
                        {
                            .fps = header.fps,
                            .width = header.width,
                            .height = header.height,
                            .frames = header.display_frame_count,
                            .out_buffer_size =
                                header.width * header.height * ANIM_FILE_OUT_BYTES_PER_PIXEL,
                        },
                    .color_format = header.color_format,
                    .sections = sections_chunk,
                    .header = header,
                },
        };

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
    anim_file_img_deinit(anim);
    storage_file_free(anim->file);
    if(anim->meta.sections) free(anim->meta.sections);
    free(anim);
}

AnimFileInfo anim_file_info(const AnimFile* anim) {
    furi_check(anim);
    return anim->meta.info;
}

void anim_file_set_out_buf(AnimFile* anim, void* buffer) {
    furi_check(anim);
    furi_check(buffer);
    anim_file_img_init(anim, buffer);
}

AnimFileFrameInfo anim_file_frame(AnimFile* anim) {
    furi_check(anim);

    AnimFileFrameInfo info;
    info.index = anim_file_seq_disp_frame_idx(anim);
    info.flags = anim_file_seq_load_current_frame(anim);
    return info;
}

bool FURI_WARN_UNUSED
    anim_file_set_section_manual(AnimFile* anim, AnimFilePlayFlag flags, size_t start, size_t end) {
    furi_check(anim);
    if(!anim_file_start_compute(anim, flags, start, end)) return false;
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

    if(!anim_file_load_iterate_sections(header, sections, callback, &section)) return false;

    if(section) {
        anim_file_start_set_precomputed(anim, flags, section);
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

    if(!anim_file_load_iterate_sections(header, sections, callback, &section)) return false;

    if(section) {
        anim_file_start_set_precomputed(anim, flags, section);
        return true;
    }
    return false;
}

bool FURI_WARN_UNUSED
    anim_file_set_section(AnimFile* anim, const AnimFileSectionSelector* selector) {
    furi_check(anim);
    furi_check(selector);

    switch(selector->type) {
    case AnimFileSectionSelectorManual:
        return anim_file_set_section_manual(
            anim, selector->flags, selector->manual.start, selector->manual.end);
    case AnimFileSectionSelectorIndexed:
        return anim_file_set_section_indexed(anim, selector->flags, selector->index);
    case AnimFileSectionSelectorNamed:
        return anim_file_set_section_named(anim, selector->flags, selector->name);
    }

    furi_crash("unreachable");
}

const AnimFileSectionSelector anim_file_whole_selector = {
    .type = AnimFileSectionSelectorIndexed,
    .index = ANIM_FILE_WHOLE_SECTION_INDEX,
};
