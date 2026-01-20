#include <anim_file_i.h>

#include <toolbox/rle_encode.h>

size_t anim_file_img_packed_length(const AnimFileHeader* file_hdr) {
    furi_assert(file_hdr);

    if(file_hdr->color_format == AnimFileColorFormatRgb888) {
        return file_hdr->width * file_hdr->height * 3;
    } else if(file_hdr->color_format == AnimFileColorFormatGray4) {
        return file_hdr->width * file_hdr->height / 2;
    } else {
        furi_crash();
    }
}

void anim_file_img_init(AnimFile* anim, uint8_t* color_buffer) {
    furi_assert(anim);
    furi_assert(color_buffer);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileImg* img = &anim->img;

    furi_check(!img->encoded_buffer);
    furi_check(!img->packed_buffer);
    furi_check(!img->color_buffer);

    if(file_hdr->max_encoded_length) {
        img->encoded_buffer = malloc(file_hdr->max_encoded_length);
    }
    if(file_hdr->color_format != AnimFileColorFormatRgb888) {
        img->packed_buffer = malloc(anim_file_img_packed_length(file_hdr));
    }
    img->color_buffer = color_buffer;
}

void anim_file_img_deinit(AnimFile* anim) {
    furi_assert(anim);

    AnimFileImg* img = &anim->img;

    if(img->encoded_buffer) free(img->encoded_buffer);
    if(img->packed_buffer) free(img->packed_buffer);
}

static uint8_t* anim_file_img_packed_buffer(AnimFile* anim, size_t* size) {
    furi_assert(anim);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileImg* img = &anim->img;

    if(file_hdr->color_format != AnimFileColorFormatRgb888) {
        if(size) *size = anim_file_img_packed_length(file_hdr);
        return img->packed_buffer;
    }

    if(size) *size = file_hdr->width * file_hdr->height * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    return img->color_buffer;
}

uint8_t* anim_file_img_encoded_buffer(AnimFile* anim, AnimFileFrameEncoding encoding) {
    furi_assert(anim);
    furi_assert(encoding < AnimFileFrameEncodingMAX);

    AnimFileImg* img = &anim->img;

    if(encoding != AnimFileFrameEncodingRaw) {
        if(anim->meta.header.max_encoded_length == 0) {
            ANIM_FILE_ERR(
                "Invalid frile header: frame_hdr.encoding is non-Raw but max_encoded_length is 0");
            return NULL;
        }
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
        if(file_hdr->color_format == AnimFileColorFormatRgb888) {
            blk_size = 3;
        } else if(file_hdr->color_format == AnimFileColorFormatGray4) {
            blk_size = 1;
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

    const AnimFileInfo* info = &anim->meta.info;
    const AnimFileHeader* file_hdr = &anim->meta.header;
    furi_assert(file_hdr->color_format != AnimFileColorFormatRgb888);

    if(file_hdr->color_format == AnimFileColorFormatGray4) {
        size_t x = 0;
        size_t y = 0;
        size_t dest_idx = 0;
        for(size_t i = 0; i < src_len; i++) {
            uint8_t left_px = source[i] & 0xF0;
            uint8_t right_px = source[i] << 4;
            *(dest++) = left_px;
            *(dest++) = left_px;
            *(dest++) = left_px;
            *(dest++) = right_px;
            *(dest++) = right_px;
            *(dest++) = right_px;

            dest_idx += 6;
            x += 2;
            if(x >= info->width) {
                y++;
                x = 0;
            }
            if(y >= info->height) break;
        }
    }

    return true;
}

bool anim_file_img_full_decode(AnimFile* anim, const AnimFileFrameHeader* frame_hdr) {
    furi_assert(anim);

    AnimFileImg* img = &anim->img;
    furi_check(img->color_buffer);

    const uint8_t* encoded_source = anim_file_img_encoded_buffer(anim, frame_hdr->encoding);
    size_t decoded_dst_size;
    uint8_t* decoded_dst = anim_file_img_packed_buffer(anim, &decoded_dst_size);
    if(encoded_source != decoded_dst) {
        if(!anim_file_img_decode(anim, frame_hdr, encoded_source, decoded_dst, decoded_dst_size))
            return false;
    }

    const uint8_t* packed_source = decoded_dst;
    uint8_t* unpacked_dst = img->color_buffer;
    if(packed_source != unpacked_dst) {
        if(!anim_file_img_unpack(anim, encoded_source, decoded_dst_size, decoded_dst))
            return false;
    }

    return true;
}
