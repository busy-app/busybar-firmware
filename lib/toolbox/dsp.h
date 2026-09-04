#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

#define DSP_EPSILON 0.001f // 1/4th of the minimum RGB value

typedef struct {
    uint8_t* first_pixel;
    size_t width;
    size_t stride;
    size_t height;
    size_t channels;
} DspImageBuffer;

/**
 * @brief Applies a one pole low-pass filter to the input signal.
 *
 * @param input The current input sample.
 * @param prev_output The previous output sample.
 * @param alpha The smoothing factor (0 < alpha < 1).
 * @return The filtered output sample.
 */
float dsp_low_pass(float input, float prev_output, float alpha);

/**
 * @brief Calculate a 2-dimensional kernel for a sub-pixel translation operation.
 * 
 * The resulting kernel is already normalized. The range of acceptable `x` and
 * `y` values is `+/- (kernel_sz - 1) / 2`. E.g., for a kernel size of 3, the
 * maximum shift is `+/-1.0` pixels on either axis.
 * 
 * To perform the shift operation, call `dsp_2d_kernel_apply`.
 * 
 * @param[in] kernel_sz Size of one axis of the kernel. Can't be less than 3,
 *                      must be odd.
 * @param[out] kernel Result (2-dimensional array of size `kernel_sz` x
 *                    `kernel_sz`)
 * @param[in] x X-axis translation amount
 * @param[in] y Y-axis translation amount
 */
void dsp_2d_kernel_subpixel_translate(
    size_t kernel_sz,
    float kernel[kernel_sz][kernel_sz],
    float x,
    float y);

/**
 * @brief Checks whether the provided kernel is an identity kernel.
 * 
 * An identity kernel has all values set to 0, except the middle one, which is
 * set to 1. When such a kernel is applied, the resulting image stays completely
 * the same.
 * 
 * @param[in] kernel_sz Size of one axis of the kernel. Can't be less than 3,
 *                      must be odd.
 * @param[out] kernel Kernel to analyze (2-dimensional array of size `kernel_sz`
 *                    x `kernel_sz`)
 */
bool dsp_2d_kernel_is_identity(size_t kernel_sz, float kernel[kernel_sz][kernel_sz]);

/**
 * @brief Apply a 2-dimensional kernel to part of an image
 * 
 * @param[in] kernel_sz Size of one axis of the kernel. Can't be less than 3,
 *                      must be odd.
 * @param[in] kernel Normalized kernel (2-dimensional array of size `kernel_sz`
 *                   x `kernel_sz`)
 * @param[in] src Descriptor for source buffer. Must have a black margin around
 *                the image of size `kernel_sz / 2`.
 * @param[in] dst Descriptor for destination buffer
 * @param[in] offs_x X-axis offset of destination window relative to source sheet
 * @param[in] offs_y Y-axis offset of destination window relative to source sheet
 */
void dsp_2d_kernel_apply(
    size_t kernel_sz,
    const float* kernel,
    DspImageBuffer src,
    DspImageBuffer dst,
    int offs_x,
    int offs_y);

#ifdef __cplusplus
}
#endif
