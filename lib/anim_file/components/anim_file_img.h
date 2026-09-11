/**
 * @file anim_file_img.h
 * Animation file image decoding
 */

#pragma once

#include <anim_file_i.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANIM_FILE_IMG_KERNEL_SZ 3

typedef enum {
    AnimFileBufferContentUninitialized,
    AnimFileBufferContentFromFile,
    AnimFileBufferContentDecoded,
    AnimFileBufferContentFullColor,
    AnimFileBufferContentDispersed,
    AnimFileBufferContentCut,
} AnimFileBufferContent;

typedef struct {
    uint8_t* data;
    size_t max_bytes;
    size_t filled_bytes;
    AnimFileBufferContent content;
} AnimFileBuffer;

typedef struct {
    AnimFileBuffer buffer_a;
    AnimFileBuffer buffer_b;
    AnimFileBuffer buffer_persistent;
    AnimFileBuffer buffer_cutout; //<! provided by application

    size_t cutout_w;
    size_t cutout_h;
    int cutout_x;
    int cutout_y;
    float cutout_kernel[ANIM_FILE_IMG_KERNEL_SZ][ANIM_FILE_IMG_KERNEL_SZ];
} AnimFileImg;

void anim_file_img_init(AnimFile* anim, uint8_t* cutout_buffer, size_t width, size_t height);

void anim_file_img_deinit(AnimFile* anim);

/**
 * @returns `NULL` on any kind of error (logged with `ANIM_FILE_DETAILED_ERRORS`)
 */
AnimFileBuffer* anim_file_img_initial_buffer(AnimFile* anim);

void anim_file_img_set_cutout(AnimFile* anim, float x, float y);

bool anim_file_img_full_decode(AnimFile* anim, const AnimFileFrameHeader* frame_hdr);

#ifdef __cplusplus
}
#endif
