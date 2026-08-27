#include <anim_file_i.h>

#include <toolbox/rle_encode.h>
#include <toolbox/dsp.h>

#define ANIM_FILE_BUFFER_MARGIN (ANIM_FILE_IMG_KERNEL_SZ - 1)

void anim_file_img_init(AnimFile* anim, uint8_t* cutout_buffer, size_t width, size_t height) {
    furi_assert(anim);
    furi_assert(cutout_buffer);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileImg* img = &anim->img;

    if(!img->buffer_a.data) {
        furi_assert(!img->buffer_b.data);
        size_t pixels = (file_hdr->width + ANIM_FILE_BUFFER_MARGIN) *
                        (file_hdr->height + ANIM_FILE_BUFFER_MARGIN);
        size_t bytes = pixels * ANIM_FILE_OUT_BYTES_PER_PIXEL;

        img->buffer_a = (AnimFileBuffer){
            .data = malloc(bytes),
            .max_size = bytes,
            .content = AnimFileBufferContentUninitialized,
            .filled_size = 0,
        };
        img->buffer_b = (AnimFileBuffer){
            .data = malloc(bytes),
            .max_size = bytes,
            .content = AnimFileBufferContentUninitialized,
            .filled_size = 0,
        };
        img->buffer_persistent = (AnimFileBuffer){
            .data = malloc(bytes),
            .max_size = bytes,
            .content = AnimFileBufferContentUninitialized,
            .filled_size = 0,
        };
    }

    img->cutout_w = width;
    img->cutout_h = height;
    img->buffer_cutout = (AnimFileBuffer){
        .data = cutout_buffer,
        .max_size = width * height * ANIM_FILE_OUT_BYTES_PER_PIXEL,
        .content = AnimFileBufferContentUninitialized,
        .filled_size = 0,
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

        size_t decoded_sz_limit = destination->max_size;
        size_t decoded_sz = 0;
        if(!rle_decompress(
               source->data,
               source->filled_size,
               destination->data,
               decoded_sz_limit,
               blk_size,
               &decoded_sz)) {
            ANIM_FILE_ERR("RLE compressed pixels too large");
            return NULL;
        }
        destination->filled_size = decoded_sz;
    }

    destination->content = AnimFileBufferContentDecoded;
    return destination;
}

static AnimFileBuffer* anim_file_img_step_unpack(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source) {
    furi_assert(anim);
    furi_assert(frame_hdr);
    furi_assert(source);
    furi_assert(source->content == AnimFileBufferContentDecoded);

    AnimFileColorFormat format = anim->meta.color_format;
    if(format == AnimFileColorFormatBgra8888) {
        source->content = AnimFileBufferContentFullColor;
        return source;
    }

    AnimFileBuffer* destination = anim_file_img_request_buffer(anim, source);
    uint8_t* src_data = source->data;
    uint8_t* dest_data = destination->data;

    if(format == AnimFileColorFormatGray4) {
        for(size_t i = 0; i < source->filled_size; i++) {
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
        destination->filled_size = source->filled_size * 2;
    } else if(format == AnimFileColorFormatBgr888) {
        for(size_t i = 0; i < source->filled_size; i += 3) {
            dest_data[0] = src_data[0];
            dest_data[1] = src_data[1];
            dest_data[2] = src_data[2];
            dest_data[3] = 255;
            dest_data += 4;
            src_data += 3;
        }
        destination->filled_size = source->filled_size * ANIM_FILE_OUT_BYTES_PER_PIXEL / 3;
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

    AnimFileBuffer* destination = &img->buffer_persistent;

    size_t half_margin = ANIM_FILE_BUFFER_MARGIN / 2;
    size_t w_with_margin = anim->meta.info.width + ANIM_FILE_BUFFER_MARGIN;
    size_t h_with_margin = anim->meta.info.height + ANIM_FILE_BUFFER_MARGIN;

    uint32_t* src_pixel = (uint32_t*)&source->data[0];
    size_t src_pixels_left = source->filled_size / ANIM_FILE_OUT_BYTES_PER_PIXEL;

#ifdef ANIM_FILE_SHOW_MASK_INSTEAD_OF_IMAGE
    for(size_t i = 0; i < destination->max_size; i++) {
        destination->data[i] = (uint8_t)MAX(0, (int)destination->data[i] - 0x22);
    }
#endif

    void place_pixels(AnimFileMaskPixelRange range, void* context) {
        UNUSED(context);

        range.y += half_margin;
        range.x_start += half_margin;
        range.x_end += half_margin;

        uint32_t* dst_start =
            (uint32_t*)destination->data + ((range.y * w_with_margin) + range.x_start);
        size_t pixel_cnt = MIN(range.x_end - range.x_start, src_pixels_left);
#ifdef ANIM_FILE_SHOW_MASK_INSTEAD_OF_IMAGE
        memset(dst_start, 0xff, pixel_cnt * ANIM_FILE_OUT_BYTES_PER_PIXEL);
#else
        memcpy(dst_start, src_pixel, pixel_cnt * ANIM_FILE_OUT_BYTES_PER_PIXEL);
#endif
        src_pixel += pixel_cnt;
        src_pixels_left -= pixel_cnt;
    }

    anim_file_mask_iterate(anim, frame_hdr, place_pixels, NULL);

    if(src_pixels_left) {
        ANIM_FILE_ERR("Invalid frame: %zu leftover pixels after dispersion", src_pixels_left);
        return NULL;
    }

    destination->content = AnimFileBufferContentDispersed;
    destination->filled_size = w_with_margin * h_with_margin * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    return destination;
}

static AnimFileBuffer* anim_file_img_step_cut(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source) {
    furi_assert(anim);
    furi_assert(frame_hdr);
    furi_assert(source);
    furi_assert(source->content == AnimFileBufferContentDispersed);
    AnimFileImg* img = &anim->img;

    const AnimFileInfo* info = &anim->meta.info;
    AnimFileBuffer* destination = &img->buffer_cutout;

    size_t half_margin = ANIM_FILE_BUFFER_MARGIN / 2;
    size_t w_with_margin = info->width + ANIM_FILE_BUFFER_MARGIN;
    size_t h_with_margin = info->height + ANIM_FILE_BUFFER_MARGIN;

    furi_assert(
        source->filled_size == w_with_margin * h_with_margin * ANIM_FILE_OUT_BYTES_PER_PIXEL);

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
    destination->filled_size = info->width * info->height * ANIM_FILE_OUT_BYTES_PER_PIXEL;
    return destination;
}

typedef AnimFileBuffer* (*AnimFileImgPipelineStep)(
    AnimFile* anim,
    const AnimFileFrameHeader* frame_hdr,
    AnimFileBuffer* source);

bool anim_file_img_full_decode(AnimFile* anim, const AnimFileFrameHeader* frame_hdr) {
    furi_assert(anim);
    AnimFileImg* img = &anim->img;

    AnimFileImgPipelineStep steps[] = {
        anim_file_img_step_decode,
        anim_file_img_step_unpack,
        anim_file_img_step_disperse,
        anim_file_img_step_cut,
    };

    AnimFileBuffer* buffer = anim_file_img_initial_buffer(anim);

    for(size_t i = 0; i < COUNT_OF(steps); i++) {
        buffer = steps[i](anim, frame_hdr, buffer);
        if(!buffer) return false;
    }

    furi_assert(buffer == &img->buffer_cutout);
    furi_assert(buffer->content == AnimFileBufferContentCut);

    return true;
}
