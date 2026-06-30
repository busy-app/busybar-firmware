#include "dsp.h"

#define EPSILON 0.001f // 1/4th of the minimum RGB value

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

    memset(kernel, 0, sizeof(float) * kernel_sz * kernel_sz);
    int center = kernel_sz / 2;

    furi_assert(fabsf(x) < (center + EPSILON));
    furi_assert(fabsf(y) < (center + EPSILON));

    int sign(float f) {
        if(f > EPSILON) return +1;
        return -1;
    }

    /**
     * Sign is reversed:
     * To offset an image by 1 pixel downwards, the kernel needs to take one
     * pixel upwards from the source image.
     */
    float* middle = &kernel[center][center];
    float* y_axis = &kernel[center - sign(y)][center];
    float* x_axis = &kernel[center][center - sign(x)];
    float* diagonal = &kernel[center - sign(y)][center - sign(x)];

    /**
     * Start with an "identity" (no-op) kernel:
     * 0.00  0.00  0.00
     * 0.00  1.00  0.00
     * 0.00  0.00  0.00
     */
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
    float kernel[kernel_sz][kernel_sz],
    DspImageBuffer src,
    DspImageBuffer dst,
    int offs_x,
    int offs_y) {
    furi_assert(kernel_sz);
    furi_assert((kernel_sz % 2) == 1);
    furi_assert(kernel);

    furi_assert(dst.width <= src.width);
    furi_assert(dst.height <= src.height);
    furi_assert(dst.channels == src.channels);

    size_t n_chans = src.channels;
    int kernel_mid = kernel_sz / 2;

    // https://en.wikipedia.org/wiki/Kernel_(image_processing)#Convolution

    for(int dst_y = 0; dst_y < (int)dst.height; dst_y++) {
        for(int dst_x = 0; dst_x < (int)dst.width; dst_x++) {
            for(size_t chan = 0; chan < n_chans; chan++) {
                float sum = 0.0f;

                int src_y = dst_y - kernel_mid + offs_y;
                int src_x = dst_x - kernel_mid + offs_x;
                const uint8_t* src_buf =
                    &src.first_pixel[(src_y * src.stride * src.channels) + (src_x * n_chans) + chan];
                size_t src_buf_line_stride = (src.stride * src.channels) - (kernel_sz * n_chans);

                const float* kernel_flat = (float*)kernel;

                for(int k_y = 0; k_y < (int)kernel_sz; k_y++) {
                    for(int k_x = 0; k_x < (int)kernel_sz; k_x++) {
                        bool is_in_bounds = (src_x >= 0) && (src_y >= 0) &&
                                            (src_x < (int)src.width) && (src_y < (int)src.height);

                        float value = is_in_bounds ? *src_buf : 0.0f;
                        sum += value * *kernel_flat;

                        src_x++;
                        src_buf += n_chans;
                        kernel_flat++;
                    }
                    src_y++;
                    src_x -= kernel_sz;
                    src_buf += src_buf_line_stride;
                }

                size_t buf_idx = (dst_y * dst.stride * dst.channels) + (dst_x * n_chans) + chan;
                dst.first_pixel[buf_idx] = CLAMP((int)sum, UINT8_MAX, 0);
            }
        }
    }
}
