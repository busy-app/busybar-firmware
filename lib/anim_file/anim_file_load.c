/**
 * @brief Initial file loading and validation
 */

#include "anim_file_i.h"

bool anim_file_read_header(AnimFileHeader* header, File* file) {
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
    if(storage_file_size(file) !=
       sizeof(*header) + header->sections_chunk_length + header->frames_chunk_length) {
        ANIM_FILE_ERR("Invalid size");
        return false;
    }

    return true;
}

bool anim_file_read_sections(const AnimFileHeader* header, uint8_t** buffer, File* file) {
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

void anim_file_iterate_sections(
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

        callback(index, section, context);

        pos += sizeof(AnimFileSection) + strlen(section->name) + 1;
        index++;
    }
}

size_t anim_file_count_sections(const AnimFileHeader* header, const uint8_t* sections) {
    furi_assert(header);
    furi_assert(sections);

    size_t section_cnt;

    void callback(size_t index, const AnimFileSection* section, void* context) {
        UNUSED(index);
        UNUSED(section);
        UNUSED(context);
        section_cnt++;
    }

    anim_file_iterate_sections(header, sections, callback, &section_cnt);

    if(section_cnt == 0) ANIM_FILE_ERR("Empty Sections chunk");
    return section_cnt;
}

size_t anim_file_count_display_frames(const AnimFileHeader* header, File* file) {
    furi_assert(header);
    furi_assert(file);

    size_t start = sizeof(*header) + header->sections_chunk_length;
    if(!storage_file_seek(file, start, true)) {
        ANIM_FILE_ERR("Failed to seek Frames chunk");
        return 0;
    }

    size_t size = header->frames_chunk_length;
    size_t disp_frame_cnt = 0;

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
        if(frame_hdr.duration == 0) {
            ANIM_FILE_ERR("Invalid frame duration");
            return 0;
        }

        disp_frame_cnt += frame_hdr.duration;

        size -= sizeof(frame_hdr) + frame_hdr.encoded_length;
        if(!storage_file_seek(file, frame_hdr.encoded_length, false)) {
            ANIM_FILE_ERR("Failed to seek next frame");
            return 0;
        }
    }

    if(disp_frame_cnt == 0) ANIM_FILE_ERR("Empty Frames chunk");
    return disp_frame_cnt;
}

bool anim_file_validate_section_0(
    const AnimFileHeader* header,
    const uint8_t* buffer,
    size_t frame_count) {
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
