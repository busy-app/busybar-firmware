/**
 * @file anim_file_img.h
 * Animation file image decoding
 */

#pragma once

#include <anim_file_i.h>

#ifdef __cplusplus
extern "C" {
#endif

// see `anim_file_format.h` for terminology

#define ANIM_FILE_IMG_KERNEL_SZ 3

typedef struct {
    uint8_t* encoded_buffer; //<! may not be present
    uint8_t* packed_buffer; //<! may not be present
    uint8_t* sheet_buffer; //<! may not be present
    uint8_t* cutout_buffer; //<! provided by application
    size_t cutout_w;
    size_t cutout_h;
    int cutout_x;
    int cutout_y;
    float cutout_kernel[ANIM_FILE_IMG_KERNEL_SZ][ANIM_FILE_IMG_KERNEL_SZ];
} AnimFileImg;

/**
 * @brief Length of Packed buffer
 */
size_t anim_file_img_packed_length(const AnimFileHeader* file_hdr);

void anim_file_img_init(
    AnimFile* anim,
    uint8_t* cutout_buffer,
    size_t width,
    size_t height,
    bool force_sheet_buffer);

void anim_file_img_deinit(AnimFile* anim);

/**
 * @returns `NULL` on any kind of error (logged with `ANIM_FILE_DETAILED_ERRORS`)
 */
uint8_t* anim_file_img_encoded_buffer(AnimFile* anim, AnimFileFrameEncoding encoding);

void anim_file_img_set_cutout(AnimFile* anim, float x, float y);

bool anim_file_img_full_decode(AnimFile* anim, const AnimFileFrameHeader* frame_hdr);

#ifdef __cplusplus
}
#endif
