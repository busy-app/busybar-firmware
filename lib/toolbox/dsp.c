#include "dsp.h"

float dsp_low_pass(float input, float prev_output, float alpha) {
    return (alpha * input) + ((1.0f - alpha) * prev_output);
}
