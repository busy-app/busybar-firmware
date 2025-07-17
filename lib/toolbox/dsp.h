#pragma once

/**
 * @brief Applies a one pole low-pass filter to the input signal.
 *
 * @param input The current input sample.
 * @param prev_output The previous output sample.
 * @param alpha The smoothing factor (0 < alpha < 1).
 * @return The filtered output sample.
 */
float dsp_low_pass(float input, float prev_output, float alpha);
