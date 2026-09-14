#include <anim_file_i_struct.h>

bool anim_file_load_header(AnimFileHeader* header, File* file) {
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
    if(!header->section_count) {
        ANIM_FILE_ERR("Invalid section count");
        return false;
    }
    if(!header->frame_count) {
        ANIM_FILE_ERR("Invalid display frame count");
        return false;
    }
    if(storage_file_size(file) !=
       sizeof(*header) + header->sections_chunk_length + header->frames_chunk_length) {
        ANIM_FILE_ERR("Invalid size");
        return false;
    }

    return true;
}

uint8_t* anim_file_load_sections(const AnimFileHeader* header, File* file) {
    furi_assert(header);
    furi_assert(file);

    size_t chunk_len = header->sections_chunk_length;
    uint8_t* buffer = malloc(chunk_len);
    bool success = false;

    do {
        if(!storage_file_seek(file, sizeof(*header), true)) {
            ANIM_FILE_ERR("Failed to seek Sections chunk");
            break;
        }

        if(storage_file_read(file, buffer, chunk_len) != chunk_len) {
            ANIM_FILE_ERR("Incomplete Sections chunk");
            break;
        }
        if(buffer[chunk_len - 1] != 0x00) {
            ANIM_FILE_ERR("Invalid Sections chunk termination");
            break;
        }

        success = true;
    } while(0);

    if(!success) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

bool anim_file_load_iterate_sections(
    const AnimFileHeader* header,
    const uint8_t* sections,
    AnimFileSectionCallback callback,
    void* context) {
    furi_assert(header);
    furi_assert(sections);
    furi_assert(callback);

    size_t size = header->sections_chunk_length;
    size_t index = 0;
    size_t pos = 0;

    while(pos < size) {
        if((size - pos) < (sizeof(AnimFileSection) + 1)) break;
        const AnimFileSection* section = (const AnimFileSection*)(sections + pos);

        if(index >= header->section_count) {
            ANIM_FILE_ERR("Actual section count exceeds header.section_count");
            return false;
        }

        callback(index, section, context);

        pos += sizeof(AnimFileSection) + strlen(section->name) + 1;
        index++;
    }

    return true;
}

bool anim_file_load_validate_section_0(const AnimFileHeader* header, const uint8_t* buffer) {
    furi_assert(header);
    furi_assert(buffer);

    const AnimFileSection* section = (const AnimFileSection*)buffer;

    if(strcmp(section->name, ANIM_FILE_DEFAULT_SECTION) != 0) {
        ANIM_FILE_ERR("Invalid section 0 name");
        return false;
    }
    if((section->start != 0) || (section->end != header->frame_count - 1)) {
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
