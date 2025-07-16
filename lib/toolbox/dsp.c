#include "dsp.h"

float dsp_low_pass(float input, float prev_output, float alpha) {
    return alpha * prev_output + (1.0f - alpha) * input;
}
