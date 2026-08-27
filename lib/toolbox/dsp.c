#include "dsp.h"

float dsp_low_pass(float input, float prev_output, float alpha) {
    return alpha * prev_output + (1.0f - alpha) * input;
}

void dsp_2d_kernel_subpixel_translate(
    size_t kernel_sz,
    float kernel[kernel_sz][kernel_sz],
    float x,
    float y) {
    furi_assert(kernel_sz >= 3);
    furi_assert((kernel_sz % 2) == 1);
    furi_assert(kernel);

    const int center = kernel_sz / 2;
    const int max_offset = kernel_sz - 1;

    furi_assert(fabsf(x) < (max_offset + DSP_EPSILON));
    furi_assert(fabsf(y) < (max_offset + DSP_EPSILON));

    /**
     * Sign is reversed:
     * To offset an image by 1 pixel downwards, the kernel needs to take one
     * pixel upwards from the source image.
     */
    const int y_offset = (fabsf(y) > DSP_EPSILON) ? -floorf(y) : +1;
    const int x_offset = (fabsf(x) > DSP_EPSILON) ? -floorf(x) : +1;

    float* const middle = &kernel[center][center];
    float* const y_axis = &kernel[center + y_offset][center];
    float* const x_axis = &kernel[center][center + x_offset];
    float* const diagonal = &kernel[center + y_offset][center + x_offset];

    /**
     * Start with an "identity" (no-op) kernel:
     * 0.00  0.00  0.00
     * 0.00  1.00  0.00
     * 0.00  0.00  0.00
     */
    memset(kernel, 0, sizeof(float) * kernel_sz * kernel_sz);
    *middle = 1.0f;

    /**
     * "Spread" the kernel to the right (suppose x = -0.7):
     * 0.00  0.00  0.00
     * 0.00  0.30  0.70
     * 0.00  0.00  0.00
     * Right column := middle column multiplied by 0.7
     * Middle column is multiplied in-place by (1 - 0.7)
     */
    *x_axis = *middle * fabsf(x);
    *middle *= 1 - fabsf(x);

    /**
     * "Spread" the kernel downwards (suppose y = -0.4):
     * 0.00  0.00  0.00
     * 0.00  0.18  0.42
     * 0.00  0.12  0.28
     * Bottom row := middle row multiplied by 0.4
     * Middle row is multiplied in-place by (1 - 0.4)
     */
    *y_axis = *middle * fabsf(y);
    *diagonal = *x_axis * fabsf(y);
    *middle *= 1 - fabsf(y);
    *x_axis *= 1 - fabsf(y);
}

bool dsp_2d_kernel_is_identity(size_t kernel_sz, float kernel[kernel_sz][kernel_sz]) {
    furi_assert(kernel);

    const size_t middle = kernel_sz / 2;

    for(size_t y = 0; y < kernel_sz; y++) {
        for(size_t x = 0; x < kernel_sz; x++) {
            float expected = (x == middle && y == middle) ? 1.0f : 0.0f;
            if(fabsf(expected - kernel[y][x]) > DSP_EPSILON) return false;
        }
    }

    return true;
}

static uint32_t dsp_2d_kernel_iteration(
    const uint32_t* source,
    size_t stride,
    size_t kernel_sz,
    const int32_t* kernel) {
    // temporary unpacked channel
    register uint32_t chan;
    // destination accumulators
    register uint32_t ab = 0, ag = 0, ar = 0, aa = 0;

    register size_t ky = kernel_sz;
    while(ky--) {
        register size_t kx = kernel_sz;
        while(kx--) {
            register uint32_t px = *(source++);
            register uint32_t krnl = *(kernel++);

            __asm__ volatile(
                // 1. extract blue channel; 2. add to corresponding running sum
                "uxtb %[chan], %[px], ror #0\n" // Unsigned Extend Byte
                "smlabb %[ab], %[chan], %[krnl], %[ab]\n" // Signed Multiply-Accumulate Bottom with Bottom halfwords
                // repeat for other channels:
                "uxtb %[chan], %[px], ror #8\n"
                "smlabb %[ag], %[chan], %[krnl], %[ag]\n"
                "uxtb %[chan], %[px], ror #16\n"
                "smlabb %[ar], %[chan], %[krnl], %[ar]\n"
                "uxtb %[chan], %[px], ror #24\n"
                "smlabb %[aa], %[chan], %[krnl], %[aa]\n"
                :
                // both input and output
                [ab] "+&r"(ab),
                [ag] "+&r"(ag),
                [ar] "+&r"(ar),
                [aa] "+&r"(aa),
                // temporary register
                [chan] "=&r"(chan)
                :
                // inputs
                [px] "r"(px),
                [krnl] "r"(krnl));
        }

        source += stride;
    }

    register uint32_t px;

    __asm__ volatile(
        // pack 8-bit values into 32-bit, while diving by 256
        "bfi %[px], %[aa], #16, #16\n" // Bitfield Insert
        "bfi %[px], %[ar], #8, #16\n"
        "bfi %[px], %[ag], #0, #16\n"
        "lsr %[ab], %[ab], #8\n" // Logical Shift Left
        "bfi %[px], %[ab], #0, #8\n"
        :
        // internal temporary
        [px] "=&r"(px)
        :
        // input only
        [ab] "r"(ab),
        [ag] "r"(ag),
        [ar] "r"(ar),
        [aa] "r"(aa));

    return px;
}

void dsp_2d_kernel_apply(
    size_t kernel_sz,
    const float* kernel,
    DspImageBuffer src,
    DspImageBuffer dst,
    int offs_x,
    int offs_y) {
    furi_assert(kernel_sz);
    furi_assert((kernel_sz % 2) == 1);
    furi_assert(kernel);

    furi_assert(dst.width <= src.width);
    furi_assert(dst.height <= src.height);
    furi_assert(dst.channels == 4);
    furi_assert(src.channels == 4);

    const int kernel_mid = kernel_sz / 2;

    int32_t kernel_fixedpoint[kernel_sz * kernel_sz];
    for(size_t i = 0; i < COUNT_OF(kernel_fixedpoint); i++) {
        kernel_fixedpoint[i] = kernel[i] * 256.0f;
    }

    for(size_t y = 0; y < dst.height; y++) {
        uint32_t* line_start = (uint32_t*)dst.first_pixel + (y * dst.stride);
        memset(line_start, 0, dst.width * sizeof(uint32_t));
    }

    /**
     * +--------------+   <-- Source sheet
     * |              |
     * |         +----------------------+   <-- Destination cutout
     * |         |xxxx|                 |
     * |         |xxxx| <-- Safe area   |
     * |         |xxxx|                 |
     * |         |xxxx|                 |
     * +---------|----+                 |
     *           |                      |
     *           +----------------------+
     */

    const int src_x1_in_dst_coordinate_system = -offs_x;
    const int src_x2_in_dst_coordinate_system = src_x1_in_dst_coordinate_system + src.width;
    const int src_y1_in_dst_coordinate_system = -offs_y;
    const int src_y2_in_dst_coordinate_system = src_y1_in_dst_coordinate_system + src.height;

    const int safe_x1 = MAX(src_x1_in_dst_coordinate_system, 0);
    const int safe_x2 = MIN(src_x2_in_dst_coordinate_system, (int)dst.width);
    const int safe_y1 = MAX(src_y1_in_dst_coordinate_system, 0);
    const int safe_y2 = MIN(src_y2_in_dst_coordinate_system, (int)dst.height);

    // https://en.wikipedia.org/wiki/Kernel_(image_processing)#Convolution

    for(int dst_y = safe_y1; dst_y < safe_y2; dst_y++) {
        uint32_t* dst_px = (uint32_t*)dst.first_pixel + ((dst_y * dst.stride) + safe_x1);
        uint32_t* src_px =
            (uint32_t*)src.first_pixel +
            (((offs_y - kernel_mid + dst_y) * src.stride) + (offs_x - kernel_mid) + safe_x1);

        for(int dst_x = safe_x1; dst_x < safe_x2; dst_x++) {
            *(dst_px++) = dsp_2d_kernel_iteration(
                src_px, src.stride - kernel_sz, kernel_sz, kernel_fixedpoint);
            src_px++;
        }
    }
}
