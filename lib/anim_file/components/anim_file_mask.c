#include <anim_file_i_struct.h>
#include <toolbox/bit_q.h>

#define SHORT_RUN_BITS  3
#define LONG_RUN_BITS   8
#define LONG_RUN_MARKER ((1 << SHORT_RUN_BITS) - 1)

void anim_file_mask_init(AnimFile* anim) {
    furi_assert(anim);
    AnimFileMask* mask = &anim->mask;
    const AnimFileHeader* header = &anim->meta.header;

    if(header->max_mask_length) {
        size_t size_bytes = ROUND_UP_TO(header->max_mask_length, 8);
        mask->mask_buffer = malloc(size_bytes);
    }
}

void anim_file_mask_deinit(AnimFile* anim) {
    furi_assert(anim);
    AnimFileMask* mask = &anim->mask;
    free(mask->mask_buffer);
}

uint8_t* anim_file_mask_buffer(AnimFile* anim_file) {
    return anim_file->mask.mask_buffer;
}

static void anim_file_mask_iterate_fully_white(
    AnimFile* anim,
    AnimFileMaskPixelRangeCallback callback,
    void* context) {
    furi_assert(anim);
    furi_assert(callback);

    const AnimFileHeader* file_hdr = &anim->meta.header;

    for(size_t y = 0; y < file_hdr->height; y++) {
        AnimFileMaskPixelRange range = {
            .y = y,
            .x_start = 0,
            .x_end = file_hdr->width,
        };
        callback(range, context);
    }
}

static void anim_file_mask_iterate_rle(
    AnimFile* anim,
    AnimFileMaskPixelRangeCallback callback,
    void* context,
    const AnimFileFrameHeader* frame) {
    furi_assert(anim);
    furi_assert(callback);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileMask* mask = &anim->mask;
    AnimFileMaskEncoding encoding = anim_file_mask_encoding(frame->joint_encoding);
    bool first_is_white = encoding == AnimFileMaskEncodingRleFirstWhite;

    furi_assert(mask->mask_buffer);

    const size_t width = file_hdr->width;
    const size_t height = file_hdr->height;

    size_t current_idx = 0;
    bool current_is_white = first_is_white;
    BitQ bit_q;
    bit_q_init(&bit_q, mask->mask_buffer, frame->mask_length);

    while(!bit_q_end(&bit_q)) {
        size_t run_length = bit_q_read(&bit_q, SHORT_RUN_BITS);
        if(run_length == LONG_RUN_MARKER) {
            run_length = bit_q_read(&bit_q, LONG_RUN_BITS);
        }

        if(current_is_white) {
            size_t left_to_notify = run_length;

            while(left_to_notify) {
                size_t y = current_idx / width;
                size_t x = current_idx % width;
                size_t line_length = MIN(left_to_notify, file_hdr->width - x);

                AnimFileMaskPixelRange range = {
                    .y = y,
                    .x_start = x,
                    .x_end = x + line_length,
                };
                if(range.y < height) {
                    furi_assert(range.x_start < width && range.x_end <= width);
                    callback(range, context);
                }

                current_idx += line_length;
                left_to_notify -= line_length;
            }

        } else {
            current_idx += run_length;
        }

        current_is_white = !current_is_white;
    }
}

static void anim_file_mask_iterate_bitmap(
    AnimFile* anim,
    AnimFileMaskPixelRangeCallback callback,
    void* context,
    const AnimFileFrameHeader* frame) {
    furi_assert(anim);
    furi_assert(callback);

    const AnimFileHeader* file_hdr = &anim->meta.header;
    AnimFileMask* mask = &anim->mask;

    furi_assert(mask->mask_buffer);

    const size_t width = file_hdr->width;
    const size_t height = file_hdr->height;

    size_t current_idx = 0;
    BitQ bit_q;
    bit_q_init(&bit_q, mask->mask_buffer, frame->mask_length);

    while(!bit_q_end(&bit_q)) {
        bool pixel = bit_q_read(&bit_q, 1);

        if(pixel) {
            size_t y = current_idx / width;
            size_t x = current_idx % width;

            AnimFileMaskPixelRange range = {
                .y = y,
                .x_start = x,
                .x_end = x + 1,
            };
            if(range.y < height) {
                furi_assert(range.x_start < width && range.x_end <= width);
                callback(range, context);
            }
        }

        current_idx++;
    }
}

void anim_file_mask_iterate(
    AnimFile* anim,
    const AnimFileFrameHeader* frame,
    AnimFileMaskPixelRangeCallback callback,
    void* context) {
    furi_assert(anim);
    furi_assert(frame);
    furi_assert(callback);

    AnimFileMaskEncoding encoding = anim_file_mask_encoding(frame->joint_encoding);

    if(encoding == AnimFileMaskEncodingFullyBlack) {
        // no-op

    } else if(encoding == AnimFileMaskEncodingFullyWhite) {
        anim_file_mask_iterate_fully_white(anim, callback, context);

    } else if(encoding <= AnimFileMaskEncodingRleFirstWhite) {
        anim_file_mask_iterate_rle(anim, callback, context, frame);

    } else if(encoding == AnimFileMaskEncodingBitmap) {
        anim_file_mask_iterate_bitmap(anim, callback, context, frame);
    }
}
