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
        uint32_t* dst_px = (uint32_t*)dst.first_pixel + (dst_y * dst.stride);
        uint32_t* src_px = (uint32_t*)src.first_pixel +
                           (((offs_y - kernel_mid + dst_y) * src.stride) + (offs_x - kernel_mid));

        for(int dst_x = safe_x1; dst_x < safe_x2; dst_x++) {
            register const int32_t* kernel_ptr = (const int32_t*)kernel_fixedpoint;
            // temporary unpacked channels
            register uint32_t b, g, r, a;
            // destination accumulators
            register uint32_t ab = 0, ag = 0, ar = 0, aa = 0;

            for(size_t ky = 0; ky < kernel_sz; ky++) {
                for(size_t kx = 0; kx < kernel_sz; kx++) {
                    register uint32_t px, krnl;
                    __asm__ volatile(
                        // load pixel and increment pointer
                        "ldmia %[src_px]!, {%[px]}\n" // Load Multiple, Increment After
                        // unpack 4 channels into 4 registers
                        "uxtb %[b], %[px], ror #0\n" // Unsigned Extend Byte
                        "uxtb %[g], %[px], ror #8\n"
                        "uxtb %[r], %[px], ror #16\n"
                        "uxtb %[a], %[px], ror #24\n"
                        // load kernel and increment pointer
                        "ldmia %[kernel_ptr]!, {%[krnl]}\n"
                        // multiply kernel and source pixel, add to destination pixel running sum
                        "smlabb %[ab], %[b], %[krnl], %[ab]\n" // Signed Multiply-Accumulate Bottom with Bottom halfwords
                        "smlabb %[ag], %[g], %[krnl], %[ag]\n"
                        "smlabb %[ar], %[r], %[krnl], %[ar]\n"
                        "smlabb %[aa], %[a], %[krnl], %[aa]\n"
                        :
                        // temporaries used in assembly snippet, marked as outputs to ask the compiler to allocate registers for us
                        [b] "=r"(b),
                        [g] "=r"(g),
                        [r] "=r"(r),
                        [a] "=r"(a),
                        // both input and output
                        [ab] "+r"(ab),
                        [ag] "+r"(ag),
                        [ar] "+r"(ar),
                        [aa] "+r"(aa),
                        // pointers which will be changed
                        [src_px] "+r"(src_px),
                        [kernel_ptr] "+r"(kernel_ptr),
                        // also internal temporaries
                        [px] "=r"(px),
                        [krnl] "=r"(krnl));
                }

                src_px += src.stride - kernel_sz;
            }

            register uint32_t px;
            __asm__ volatile(
                // pack 8-bit values into 32-bit, while diving by 256
                "bfi %[px], %[aa], #16, #16\n" // Bitfield Insert
                "bfi %[px], %[ar], #8, #16\n"
                "bfi %[px], %[ag], #0, #16\n"
                "lsr %[ab], %[ab], #8\n" // Logical Shift Left
                "bfi %[px], %[ab], #0, #8\n"
                // store and increment
                "stmia %[dst_px]!, {%[px]}\n" // Store Multiple, Increment After
                :
                // input only, marked as input-output to force the compiler to allocate a different register for the other ones;
                // normally it assumes that we do all reads before we do all writes, and allocates the same register for, say "ab" and "px", which we don't want
                [ab] "+r"(ab),
                [ag] "+r"(ag),
                [ar] "+r"(ar),
                [aa] "+r"(aa),
                // pointer which will be changed
                [dst_px] "+r"(dst_px),
                // internal temporary
                [px] "=r"(px));

            src_px -= kernel_sz * src.stride - 1;
        }
    }
}
