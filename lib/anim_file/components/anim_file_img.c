#include <anim_file_i_struct.h>

#include <toolbox/rle_encode.h>
#include <toolbox/dsp.h>

#define ANIM_FILE_BUFFER_MARGIN (ANIM_FILE_IMG_KERNEL_SZ - 1)

void anim_file_img_init(AnimFile* anim, uint8_t* cutout_buffer, size_t width, size_t height) {
    furi_assert(anim);
    furi_assert(cutout_buffer);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileImg* img = &anim->img;

    if((width != anim->meta.info.width) || (height != anim->meta.info.height)) {
        furi_check(anim->options & AnimFileOptionIntermediateInternalBuffer);
    }

    if(!img->buffer_a.data) {
        furi_assert(!img->buffer_b.data);
        furi_assert(!img->buffer_persistent.data);
        size_t pixels = (file_hdr->width + ANIM_FILE_BUFFER_MARGIN) *
                        (file_hdr->height + ANIM_FILE_BUFFER_MARGIN);
        size_t bytes = pixels * ANIM_FILE_OUT_BYTES_PER_PIXEL;

        img->buffer_a = (AnimFileBuffer){
            .data = malloc(bytes),
            .max_bytes = bytes,
            .filled_bytes = 0,
            .content = AnimFileBufferContentUninitialized,
        };
        img->buffer_b = (AnimFileBuffer){
            .data = malloc(bytes),
            .max_bytes = bytes,
            .filled_bytes = 0,
            .content = AnimFileBufferContentUninitialized,
        };
        if(anim->options & AnimFileOptionIntermediateInternalBuffer) {
            img->buffer_persistent = (AnimFileBuffer){
                .data = malloc(bytes),
                .max_bytes = bytes,
                .filled_bytes = 0,
                .content = AnimFileBufferContentUninitialized,
            };
        }
    }

    img->cutout_w = width;
    img->cutout_h = height;
    img->buffer_cutout = (AnimFileBuffer){
        .data = cutout_buffer,
        .max_bytes = width * height * ANIM_FILE_OUT_BYTES_PER_PIXEL,
        .filled_bytes = 0,
        .content = AnimFileBufferContentUninitialized,
    };

    anim_file_img_set_cutout(anim, 0, 0);
}

void anim_file_img_deinit(AnimFile* anim) {
    furi_assert(anim);
    AnimFileImg* img = &anim->img;

    free(img->buffer_a.data);
    free(img->buffer_b.data);
    free(img->buffer_persistent.data);
    img->buffer_a.data = NULL;
    img->buffer_b.data = NULL;
    img->buffer_persistent.data = NULL;
    img->buffer_a.content = AnimFileBufferContentUninitialized;
    img->buffer_b.content = AnimFileBufferContentUninitialized;
    img->buffer_persistent.content = AnimFileBufferContentUninitialized;

    img->buffer_cutout.data = NULL;
    img->buffer_cutout.content = AnimFileBufferContentUninitialized;
}

static AnimFileBuffer* anim_file_img_request_buffer(AnimFile* anim, AnimFileBuffer* previous) {
    furi_assert(anim);
    AnimFileImg* img = &anim->img;

    if(previous == &img->buffer_a) return &img->buffer_b;
    if(previous == &img->buffer_b) return &img->buffer_a;
    if(previous == &img->buffer_persistent) return &img->buffer_a;
    furi_crash();
}

void anim_file_img_set_cutout(AnimFile* anim, float x, float y) {
    furi_assert(anim);
    AnimFileImg* img = &anim->img;

    img->cutout_x = (int)floorf(x);
    img->cutout_y = (int)floorf(y);

    dsp_2d_kernel_subpixel_translate(
        ANIM_FILE_IMG_KERNEL_SZ, img->cutout_kernel, img->cutout_x - x, img->cutout_y - y);
}

AnimFileBuffer* anim_file_img_initial_buffer(AnimFile* anim) {
    furi_assert(anim);
    AnimFileImg* img = &anim->img;
    return &img->buffer_a;
}

static bool anim_file_img_step_decode_qoi(
    AnimFile* anim,
    AnimFileBuffer* destination,
    AnimFileBuffer* source) {
    furi_assert(anim);
    furi_assert(source);
    furi_assert(source->content == AnimFileBufferContentFromFile);
    furi_assert(destination);

    uint32_t hash_lut[64];
    memset(hash_lut, 0, sizeof(hash_lut));
    uint32_t last_px = 0xff'00'00'00;

    size_t max_pixels = destination->max_bytes / ANIM_FILE_OUT_BYTES_PER_PIXEL;
    size_t dest_pixels = 0;
    uint32_t* dest_buf = (uint32_t*)destination->data;

    inline size_t pixel_hash(uint32_t pixel) {
        size_t a = (pixel >> 24) & 0xff;
        size_t r = (pixel >> 16) & 0xff;
        size_t g = (pixel >> 8) & 0xff;
        size_t b = pixel & 0xff;
        return ((r * 3) + (g * 5) + (b * 7) + (a * 11)) % 64;
    }

    inline void output_pixel(uint32_t pixel) {
        hash_lut[pixel_hash(pixel)] = pixel;
        last_px = pixel;
        if(dest_pixels < max_pixels) dest_buf[dest_pixels] = pixel;
        dest_pixels++;
    }

    inline uint8_t wrapping_add8(uint8_t old, int8_t diff) {
        return (uint8_t)(((int)old + (int)diff) & 0xff);
    }

    for(size_t i = 0; i < source->filled_bytes; i++) {
        size_t remaining_bytes = source->filled_bytes - i;
        size_t additional_bytes_used = 0;

        uint8_t tag = source->data[i];
        uint8_t short_tag = source->data[i] & ANIM_FILE_QOI_SHORT_TAG_MASK;
        uint8_t tag_payload = tag & ~ANIM_FILE_QOI_SHORT_TAG_MASK;
        uint8_t* payload = &source->data[i + 1];

        if(tag == ANIM_FILE_QOI_OP_RGB) {
            if(remaining_bytes < 4) {
                ANIM_FILE_ERR("QOI_OP_RGB source overrun");
                return false;
            }
            uint32_t alpha = last_px & 0xff'00'00'00;
            output_pixel(alpha | (payload[0] << 16) | (payload[1] << 8) | payload[2]);
            additional_bytes_used = 3;

        } else if(tag == ANIM_FILE_QOI_OP_RGBA) {
            if(remaining_bytes < 5) {
                ANIM_FILE_ERR("QOI_OP_RGBA source overrun");
                return false;
            }
            output_pixel(
                ((size_t)payload[3] << 24) | ((size_t)payload[0] << 16) |
                ((size_t)payload[1] << 8) | (size_t)payload[2]);
            additional_bytes_used = 4;

        } else if(short_tag == ANIM_FILE_QOI_SHORT_OP_INDEX) {
            output_pixel(hash_lut[tag_payload]);

        } else if(short_tag == ANIM_FILE_QOI_SHORT_OP_DIFF) {
            int8_t dr = (((int)tag_payload >> 4) & 0x3) - 2;
            int8_t dg = (((int)tag_payload >> 2) & 0x3) - 2;
            int8_t db = ((int)tag_payload & 0x3) - 2;
            uint8_t* last_ch = (uint8_t*)&last_px;
            output_pixel(
                (last_ch[3] << 24) | (wrapping_add8(last_ch[2], dr) << 16) |
                (wrapping_add8(last_ch[1], dg) << 8) | wrapping_add8(last_ch[0], db));

        } else if(short_tag == ANIM_FILE_QOI_SHORT_OP_LUMA) {
            if(remaining_bytes < 2) {
                ANIM_FILE_ERR("QOI_OP_LUMA source overrun");
                return false;
            }
            int8_t dg = ((int)tag_payload & 0x3f) - 32;
            int8_t dr = (((int)payload[0] >> 4) & 0xf) - 8 + dg;
            int8_t db = ((int)payload[0] & 0xf) - 8 + dg;
            uint8_t* last_ch = (uint8_t*)&last_px;
            output_pixel(
                (last_ch[3] << 24) | (wrapping_add8(last_ch[2], dr) << 16) |
                (wrapping_add8(last_ch[1], dg) << 8) | wrapping_add8(last_ch[0], db));
            additional_bytes_used = 1;

        } else if(short_tag == ANIM_FILE_QOI_SHORT_OP_RUN) {
            for(size_t j = 0; j < tag_payload + 1U; j++)
                output_pixel(last_px);
        }

        i += additional_bytes_used;
    }

    if(dest_pixels > max_pixels) {
        ANIM_FILE_ERR("QOI destination overrun by %zu px", dest_pixels - max_pixels);
        return false;
    }

    destination->content = AnimFileBufferContentFullColor;
    destination->filled_bytes = dest_pixels * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    return true;
}

static AnimFileBuffer* anim_file_img_step_decode(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source) {
    furi_assert(anim);
    furi_assert(frame_hdr);
    furi_assert(source);
    furi_assert(source->content == AnimFileBufferContentFromFile);

    AnimFilePixelEncoding encoding = anim_file_px_encoding(frame_hdr->joint_encoding);
    if(encoding == AnimFilePixelEncodingRaw) {
        source->content = AnimFileBufferContentDecoded;
        return source;
    }

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileBuffer* destination = anim_file_img_request_buffer(anim, source);

    if(encoding == AnimFilePixelEncodingRle) {
        static const size_t blk_sizes[] = {
            [AnimFileColorFormatBgr888] = 3,
            [AnimFileColorFormatGray4] = 1,
            [AnimFileColorFormatBgra8888] = 4,
        };
        size_t blk_size = blk_sizes[file_hdr->color_format];

        size_t decoded_sz_limit = destination->max_bytes;
        size_t decoded_sz = 0;
        if(!rle_decompress(
               source->data,
               source->filled_bytes,
               destination->data,
               decoded_sz_limit,
               blk_size,
               &decoded_sz)) {
            ANIM_FILE_ERR("RLE compressed pixels too large");
            return NULL;
        }
        destination->filled_bytes = decoded_sz;
        destination->content = AnimFileBufferContentDecoded;

    } else if(encoding == AnimFilePixelEncodingQoiLike) {
        if(!anim_file_img_step_decode_qoi(anim, destination, source)) return NULL;
    }

    return destination;
}

static AnimFileBuffer* anim_file_img_step_unpack(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source) {
    furi_assert(anim);
    furi_assert(frame_hdr);
    furi_assert(source);

    AnimFileColorFormat format = anim->meta.color_format;
    if(format == AnimFileColorFormatBgra8888) {
        source->content = AnimFileBufferContentFullColor;
        return source;
    }

    if(source->content == AnimFileBufferContentFullColor) return source;
    furi_assert(source->content == AnimFileBufferContentDecoded);

    AnimFileBuffer* destination = anim_file_img_request_buffer(anim, source);
    uint8_t* src_data = source->data;
    uint8_t* dest_data = destination->data;

    size_t will_fill_bytes = 0;
    if(format == AnimFileColorFormatGray4) {
        will_fill_bytes = source->filled_bytes * ANIM_FILE_OUT_BYTES_PER_PIXEL * 2;
    } else if(format == AnimFileColorFormatBgr888) {
        will_fill_bytes = source->filled_bytes * ANIM_FILE_OUT_BYTES_PER_PIXEL / 3;
    }

    furi_assert(
        will_fill_bytes <= destination->max_bytes,
        "Incorrectly chosen buffer size in anim_file_img_init");

    if(format == AnimFileColorFormatGray4) {
        for(size_t i = 0; i < source->filled_bytes; i++) {
            uint8_t left_px = src_data[i] & 0xF0;
            uint8_t right_px = src_data[i] << 4;
            dest_data[0] = left_px;
            dest_data[1] = left_px;
            dest_data[2] = left_px;
            dest_data[3] = 255;
            dest_data[4] = right_px;
            dest_data[5] = right_px;
            dest_data[6] = right_px;
            dest_data[7] = 255;
            dest_data += 8;
        }
        destination->filled_bytes = source->filled_bytes * ANIM_FILE_OUT_BYTES_PER_PIXEL * 2;
    } else if(format == AnimFileColorFormatBgr888) {
        for(size_t i = 0; i < source->filled_bytes; i += 3) {
            dest_data[0] = src_data[0];
            dest_data[1] = src_data[1];
            dest_data[2] = src_data[2];
            dest_data[3] = 255;
            dest_data += 4;
            src_data += 3;
        }
        destination->filled_bytes = source->filled_bytes * ANIM_FILE_OUT_BYTES_PER_PIXEL / 3;
    }

    destination->content = AnimFileBufferContentFullColor;
    return destination;
}

static AnimFileBuffer* anim_file_img_step_disperse(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source) {
    furi_assert(anim);
    furi_assert(frame_hdr);
    furi_assert(source);
    furi_assert(source->content == AnimFileBufferContentFullColor);
    AnimFileImg* img = &anim->img;

    bool cutout_requested = anim->options & AnimFileOptionIntermediateInternalBuffer;
    bool margin_needed = cutout_requested;

    AnimFileBuffer* destination = cutout_requested ? &img->buffer_persistent : &img->buffer_cutout;

    size_t half_margin = ANIM_FILE_BUFFER_MARGIN / 2;
    size_t w_with_margin = anim->meta.info.width + ANIM_FILE_BUFFER_MARGIN;
    size_t h_with_margin = anim->meta.info.height + ANIM_FILE_BUFFER_MARGIN;

    size_t effective_w = margin_needed ? w_with_margin : anim->meta.info.width;
    size_t effective_h = margin_needed ? h_with_margin : anim->meta.info.height;

    uint32_t* src_pixel = (uint32_t*)&source->data[0];
    size_t src_pixels_left = source->filled_bytes / ANIM_FILE_OUT_BYTES_PER_PIXEL;
    size_t src_pixels_overrun = 0;

#ifdef ANIM_FILE_SHOW_MASK_INSTEAD_OF_IMAGE
    for(size_t i = 0; i < destination->max_bytes; i++) {
        destination->data[i] = (uint8_t)MAX(0, (int)destination->data[i] - 0x22);
    }
#endif

    void place_pixels(AnimFileMaskPixelRange range, void* context) {
        UNUSED(context);

        if(margin_needed) {
            range.y += half_margin;
            range.x_start += half_margin;
            range.x_end += half_margin;
        }

        size_t start_idx = (range.y * effective_w) + range.x_start;
        uint32_t* dst_start = (uint32_t*)(destination->data + (start_idx * sizeof(uint32_t)));

        size_t wanted_pixel_cnt = range.x_end - range.x_start;
        size_t pixel_cnt = MIN(wanted_pixel_cnt, src_pixels_left);
        if(wanted_pixel_cnt > src_pixels_left)
            src_pixels_overrun += (wanted_pixel_cnt - src_pixels_left);

#ifdef ANIM_FILE_SHOW_MASK_INSTEAD_OF_IMAGE
        memset(dst_start, 0xff, wanted_pixel_cnt * ANIM_FILE_OUT_BYTES_PER_PIXEL);
#else
        memcpy(dst_start, src_pixel, pixel_cnt * ANIM_FILE_OUT_BYTES_PER_PIXEL);
#endif
        src_pixel += pixel_cnt;
        src_pixels_left -= pixel_cnt;
    }

    anim_file_mask_iterate(anim, frame_hdr, place_pixels, NULL);

    size_t allowed_leftover_pixels =
        (anim->meta.header.color_format == AnimFileColorFormatGray4) ? 1 : 0;
    if(src_pixels_left > allowed_leftover_pixels) {
        ANIM_FILE_ERR("Invalid frame: %zu leftover pixels after dispersion", src_pixels_left);
        return NULL;
    }
    if(src_pixels_overrun) {
        ANIM_FILE_ERR("Invalid frame: %zu overrun pixels during dispersion", src_pixels_overrun);
        return NULL;
    }

    destination->content = cutout_requested ? AnimFileBufferContentDispersed :
                                              AnimFileBufferContentCut;
    destination->filled_bytes = effective_w * effective_h * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    return destination;
}

static AnimFileBuffer* anim_file_img_step_cut(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source) {
    furi_assert(anim);
    furi_assert(frame_hdr);
    furi_assert(source);

    if(!(anim->options & AnimFileOptionIntermediateInternalBuffer)) {
        furi_assert(source->content == AnimFileBufferContentCut);
        return source;
    }

    furi_assert(source->content == AnimFileBufferContentDispersed);
    AnimFileImg* img = &anim->img;

    const AnimFileInfo* info = &anim->meta.info;
    AnimFileBuffer* destination = &img->buffer_cutout;

    size_t half_margin = ANIM_FILE_BUFFER_MARGIN / 2;
    size_t w_with_margin = info->width + ANIM_FILE_BUFFER_MARGIN;
    size_t h_with_margin = info->height + ANIM_FILE_BUFFER_MARGIN;

    furi_assert(
        source->filled_bytes == w_with_margin * h_with_margin * ANIM_FILE_OUT_BYTES_PER_PIXEL);

    dsp_2d_kernel_apply(
        ANIM_FILE_IMG_KERNEL_SZ,
        (const float*)img->cutout_kernel,
        (DspImageBuffer){
            .first_pixel = (uint8_t*)&((uint32_t*)source->data)[w_with_margin + half_margin],
            .width = info->width,
            .stride = w_with_margin,
            .height = info->height,
            .channels = ANIM_FILE_OUT_BYTES_PER_PIXEL,
        },
        (DspImageBuffer){
            .first_pixel = destination->data,
            .width = img->cutout_w,
            .stride = img->cutout_w,
            .height = img->cutout_h,
            .channels = ANIM_FILE_OUT_BYTES_PER_PIXEL,
        },
        img->cutout_x,
        img->cutout_y);

    destination->content = AnimFileBufferContentCut;
    destination->filled_bytes = info->width * info->height * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    return destination;
}

typedef AnimFileBuffer* (*AnimFileImgPipelineStepPerform)(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source);

typedef struct {
    const char* name;
    AnimFileImgPipelineStepPerform perform;
} AnimFileImgPipelineStep;

bool anim_file_img_full_decode(AnimFile* anim, const AnimFileFrameHeader* frame_hdr) {
    furi_assert(anim);
    AnimFileImg* img = &anim->img;

    AnimFileImgPipelineStep steps[] = {
        {
            "decode",
            anim_file_img_step_decode,
        },
        {
            "unpack",
            anim_file_img_step_unpack,
        },
        {
            "disperse",
            anim_file_img_step_disperse,
        },
        {
            "cut",
            anim_file_img_step_cut,
        },
    };

    AnimFileBuffer* buffer = anim_file_img_initial_buffer(anim);
#ifdef ANIM_FILE_SHOW_PIPELINE
    FuriString* pipeline = furi_string_alloc();
#endif

    for(size_t i = 0; i < COUNT_OF(steps); i++) {
        const AnimFileImgPipelineStep* step = &steps[i];

#ifdef ANIM_FILE_PROFILE_PERFORMANCE
        profiler_start(anim->profiler, step->name);
#endif

        AnimFileBuffer* input_buffer = buffer;
        buffer = step->perform(anim, frame_hdr, buffer);

#ifdef ANIM_FILE_SHOW_PIPELINE
        if(buffer == input_buffer) {
            furi_string_cat_printf(pipeline, "[-%s-] -> ", step->name);
        } else {
            furi_string_cat_printf(pipeline, "%s -> ", step->name);
        }
#else
        UNUSED(input_buffer);
#endif

#ifdef ANIM_FILE_PROFILE_PERFORMANCE
        profiler_stop(anim->profiler, step->name);
#endif

        if(!buffer) return false;
    }

#ifdef ANIM_FILE_SHOW_PIPELINE
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(pipeline));
    furi_string_free(pipeline);
#endif

    furi_assert(buffer == &img->buffer_cutout);
    furi_assert(buffer->content == AnimFileBufferContentCut);

    return true;
}
