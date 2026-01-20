/**
 * @file img.h
 * Animation file image decoding
 */

#pragma once

#include <anim_file_i.h>

#ifdef __cplusplus
extern "C" {
#endif

// see `anim_file_format.h` for terminology

typedef struct {
    uint8_t* encoded_buffer; //<! may not be present
    uint8_t* packed_buffer; //<! may not be present
    uint8_t* color_buffer; //<! provided by application
} AnimFileImg;

/**
 * @brief Length of Packed buffer
 */
size_t anim_file_img_packed_length(const AnimFileHeader* file_hdr);

void anim_file_img_init(AnimFile* anim, uint8_t* color_buffer);

void anim_file_img_deinit(AnimFile* anim);

/**
 * @returns `NULL` on any kind of error (logged with `ANIM_FILE_DETAILED_ERRORS`)
 */
uint8_t* anim_file_img_encoded_buffer(AnimFile* anim, AnimFileFrameEncoding encoding);

bool anim_file_img_full_decode(AnimFile* anim, const AnimFileFrameHeader* frame_hdr);

#ifdef __cplusplus
}
#endif
