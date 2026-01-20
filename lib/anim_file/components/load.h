/**
 * @file img.h
 * Animation file initial loading
 */

#pragma once

#include <anim_file_i.h>

#ifdef __cplusplus
extern "C" {
#endif

bool anim_file_load_header(AnimFileHeader* header, File* file);

/**
 * @returns Heap-allocated buffer or NULL on failure
 */
uint8_t* anim_file_load_sections(const AnimFileHeader* header, File* file);

bool anim_file_load_iterate_sections(
    const AnimFileHeader* header,
    const uint8_t* sections,
    AnimFileSectionCallback callback,
    void* context);

bool anim_file_load_validate_section_0(const AnimFileHeader* header, const uint8_t* buffer);

#ifdef __cplusplus
}
#endif
