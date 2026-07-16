#include <anim_file_i.h>

#include <toolbox/rle_encode.h>
#include <toolbox/dsp.h>

size_t anim_file_img_packed_length(const AnimFileHeader* file_hdr) {
    furi_assert(file_hdr);

    if(file_hdr->color_format == AnimFileColorFormatBgr888) {
        return file_hdr->width * file_hdr->height * 3;
    } else if(file_hdr->color_format == AnimFileColorFormatGray4) {
        return file_hdr->width * file_hdr->height / 2;
    } else if(file_hdr->color_format == AnimFileColorFormatBgra8888) {
        return file_hdr->width * file_hdr->height * 4;
    } else {
        furi_crash();
    }
}

void anim_file_img_init(
    AnimFile* anim,
    uint8_t* cutout_buffer,
    size_t width,
    size_t height,
    bool force_sheet_buffer) {
    furi_assert(anim);
    furi_assert(cutout_buffer);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileImg* img = &anim->img;

    if(!img->encoded_buffer && file_hdr->max_encoded_length) {
        img->encoded_buffer = malloc(file_hdr->max_encoded_length);
    }

    bool sheet_buffer_needed = (width < file_hdr->width) || (height < file_hdr->height) ||
                               force_sheet_buffer;
    if(sheet_buffer_needed && !img->sheet_buffer) {
        size_t margin = ANIM_FILE_IMG_KERNEL_SZ - 1;
        size_t sheet_buf_size = (file_hdr->width + margin) * (file_hdr->height + margin) *
                                ANIM_FILE_OUT_BYTES_PER_PIXEL;
        img->sheet_buffer = malloc(sheet_buf_size);
    }
    if(!sheet_buffer_needed && img->sheet_buffer) {
        free(img->sheet_buffer);
        img->sheet_buffer = NULL;
    }

    bool packed_buffer_needed = (file_hdr->color_format != AnimFileColorFormatBgra8888) ||
                                sheet_buffer_needed;
    if(packed_buffer_needed && !img->packed_buffer) {
        img->packed_buffer = malloc(anim_file_img_packed_length(file_hdr));
    }
    if(!packed_buffer_needed && img->packed_buffer) {
        free(img->packed_buffer);
        img->packed_buffer = NULL;
    }

    img->cutout_buffer = cutout_buffer;
    img->cutout_w = width;
    img->cutout_h = height;
}

void anim_file_img_deinit(AnimFile* anim) {
    furi_assert(anim);

    AnimFileImg* img = &anim->img;

    if(img->sheet_buffer) free(img->sheet_buffer);
    if(img->packed_buffer) free(img->packed_buffer);
    if(img->encoded_buffer) free(img->encoded_buffer);

    img->cutout_buffer = NULL;
    img->sheet_buffer = NULL;
    img->packed_buffer = NULL;
    img->encoded_buffer = NULL;
}

static uint8_t* anim_file_img_sheet_buffer(AnimFile* anim) {
    furi_assert(anim);

    AnimFileImg* img = &anim->img;

    if(img->sheet_buffer) {
        return img->sheet_buffer;
    }

    return img->cutout_buffer;
}

static uint8_t* anim_file_img_packed_buffer(AnimFile* anim, size_t* size) {
    furi_assert(anim);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileImg* img = &anim->img;

    if((file_hdr->color_format != AnimFileColorFormatBgra8888) || img->sheet_buffer) {
        if(size) *size = anim_file_img_packed_length(file_hdr);
        furi_assert(img->packed_buffer);
        return img->packed_buffer;
    }

    if(size) *size = file_hdr->width * file_hdr->height * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    return anim_file_img_sheet_buffer(anim);
}

uint8_t* anim_file_img_encoded_buffer(AnimFile* anim, AnimFileFrameEncoding encoding) {
    furi_assert(anim);
    furi_assert(encoding < AnimFileFrameEncodingMAX);

    AnimFileImg* img = &anim->img;

    if(encoding != AnimFileFrameEncodingRaw) {
        if(anim->meta.header.max_encoded_length == 0) {
            ANIM_FILE_ERR(
                "Invalid file header: frame_hdr.encoding is non-Raw but max_encoded_length is 0");
            return NULL;
        }
        furi_assert(img->encoded_buffer);
        return img->encoded_buffer;
    }

    return anim_file_img_packed_buffer(anim, NULL);
}

static bool anim_file_img_decode(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    const uint8_t* source,
    uint8_t* dest,
    size_t dest_len) {
    furi_assert(anim);
    furi_assert(frame_hdr);
    furi_assert(source);
    furi_assert(dest);

    furi_assert(frame_hdr->encoding != AnimFileFrameEncodingRaw);
    const AnimFileHeader* file_hdr = &anim->meta.header;

    if(frame_hdr->encoding == AnimFileFrameEncodingRle) {
        size_t blk_size = 0;
        if(file_hdr->color_format == AnimFileColorFormatBgr888) {
            blk_size = 3;
        } else if(file_hdr->color_format == AnimFileColorFormatGray4) {
            blk_size = 1;
        } else if(file_hdr->color_format == AnimFileColorFormatBgra8888) {
            blk_size = 4;
        }

        size_t decoded_sz = 0;

        if(!rle_decompress(
               source, frame_hdr->encoded_length, dest, dest_len, blk_size, &decoded_sz)) {
            ANIM_FILE_ERR("RLE compressed data too large");
            return false;
        }

        if(decoded_sz != dest_len) {
            ANIM_FILE_ERR("RLE compressed data too short");
            return false;
        }
    }

    return true;
}

static bool
    anim_file_img_unpack(AnimFile* anim, const uint8_t* source, size_t src_len, uint8_t* dest) {
    furi_assert(anim);
    furi_assert(source);
    furi_assert(dest);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileImg* img = &anim->img;

    if(file_hdr->color_format == AnimFileColorFormatGray4) {
        for(size_t i = 0; i < src_len; i++) {
            uint8_t left_px = source[i] & 0xF0;
            uint8_t right_px = source[i] << 4;
            dest[0] = left_px;
            dest[1] = left_px;
            dest[2] = left_px;
            dest[3] = 255;
            dest[4] = right_px;
            dest[5] = right_px;
            dest[6] = right_px;
            dest[7] = 255;

            dest += 8;
        }

    } else if(file_hdr->color_format == AnimFileColorFormatBgr888) {
        for(size_t i = 0; i < src_len; i += 3) {
            dest[0] = source[0];
            dest[1] = source[1];
            dest[2] = source[2];
            dest[3] = 255;
            dest += 4;
            source += 3;
        }

    } else if(file_hdr->color_format == AnimFileColorFormatBgra8888) {
        furi_assert(img->sheet_buffer);
    }

    if(img->sheet_buffer) {
        const size_t margin = ANIM_FILE_IMG_KERNEL_SZ / 2;
        const size_t w_with_margin = file_hdr->width + (2 * margin);
        const size_t h_with_margin = file_hdr->height + (2 * margin);

        memset(dest, 0, w_with_margin * h_with_margin * sizeof(uint32_t));

        uint32_t* dst_start = (uint32_t*)dest + (w_with_margin + margin);
        uint32_t* src_start = (uint32_t*)source;
        for(size_t y = 0; y < file_hdr->height; y++) {
            size_t line_size = file_hdr->width * sizeof(uint32_t);
            memcpy(dst_start + (y * w_with_margin), src_start + (y * file_hdr->width), line_size);
        }
    }

    return true;
}

static bool anim_file_img_cut_part(AnimFile* anim, const uint8_t* source, uint8_t* dest) {
    furi_assert(anim);
    furi_assert(source);
    furi_assert(dest);

    const AnimFileInfo* info = &anim->meta.info;
    AnimFileImg* img = &anim->img;

    const size_t margin = ANIM_FILE_IMG_KERNEL_SZ / 2;
    const size_t w_with_margin = info->width + (2 * margin);

    dsp_2d_kernel_apply(
        ANIM_FILE_IMG_KERNEL_SZ,
        (const float*)img->cutout_kernel,
        (DspImageBuffer){
            .first_pixel = (uint8_t*)((uint32_t*)source + ((w_with_margin) + margin)),
            .width = info->width,
            .stride = w_with_margin,
            .height = info->height,
            .channels = ANIM_FILE_OUT_BYTES_PER_PIXEL,
        },
        (DspImageBuffer){
            .first_pixel = dest,
            .width = img->cutout_w,
            .stride = img->cutout_w,
            .height = img->cutout_h,
            .channels = ANIM_FILE_OUT_BYTES_PER_PIXEL,
        },
        img->cutout_x,
        img->cutout_y);

    return true;
}

void anim_file_img_set_cutout(AnimFile* anim, float x, float y) {
    furi_assert(anim);

    AnimFileImg* img = &anim->img;

    img->cutout_x = (int)floorf(x);
    img->cutout_y = (int)floorf(y);

    dsp_2d_kernel_subpixel_translate(
        ANIM_FILE_IMG_KERNEL_SZ, img->cutout_kernel, img->cutout_x - x, img->cutout_y - y);

    bool nonzero_offset = (fabsf(x) > DSP_EPSILON) || (fabsf(y) > DSP_EPSILON);
    bool sheet_buffer_present = !!img->sheet_buffer;

    if(nonzero_offset && !sheet_buffer_present) {
        // force-allocate the sheet buffer
        anim_file_img_init(anim, img->cutout_buffer, img->cutout_w, img->cutout_h, true);
    }
}

bool anim_file_img_full_decode(AnimFile* anim, const AnimFileFrameHeader* frame_hdr) {
    furi_assert(anim);

    AnimFileImg* img = &anim->img;
    furi_check(img->cutout_buffer);

    const uint8_t* encoded_source = anim_file_img_encoded_buffer(anim, frame_hdr->encoding);
    size_t decoded_dst_size;
    uint8_t* decoded_dst = anim_file_img_packed_buffer(anim, &decoded_dst_size);
    if(encoded_source != decoded_dst) {
        if(!anim_file_img_decode(anim, frame_hdr, encoded_source, decoded_dst, decoded_dst_size))
            return false;
    }

    const uint8_t* packed_source = decoded_dst;
    uint8_t* unpacked_dst = anim_file_img_sheet_buffer(anim);
    if(packed_source != unpacked_dst) {
        if(!anim_file_img_unpack(anim, packed_source, decoded_dst_size, unpacked_dst))
            return false;
    }

    const uint8_t* sheet_source = unpacked_dst;
    uint8_t* cutout_dst = img->cutout_buffer;
    if(sheet_source != cutout_dst) {
        if(!anim_file_img_cut_part(anim, sheet_source, cutout_dst)) return false;
    }

    return true;
}
