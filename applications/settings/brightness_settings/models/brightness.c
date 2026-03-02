#include "brightness.h"
#include <brightness_control/brightness_control.h>
#include <toolbox/float_tools.h>

struct BrightnessModel {
    BrightnessControl* ctrl;
};

BrightnessModel* brightness_model_alloc(void) {
    BrightnessModel* model = malloc(sizeof(BrightnessModel));
    model->ctrl = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
    return model;
}

void brightness_model_free(BrightnessModel* model) {
    furi_assert(model);
    furi_record_close(RECORD_BRIGHTNESS_CONTROL);
    free(model);
}

void brightness_model_set_auto_mode(BrightnessModel* model) {
    furi_assert(model);

    brightness_control_set_auto_brightness(model->ctrl);
}

BrightnessMode brightness_model_get_mode(BrightnessModel* model) {
    furi_assert(model);

    FuriState* fstate = brightness_control_get_state(model->ctrl);
    BrightnessControlState state;
    furi_state_get(fstate, &state);

    return state.mode == BrightnessControlBrightnessModeAuto ? BrightnessModeAuto :
                                                               BrightnessModeManual;
}

void brightness_model_set(BrightnessModel* model, uint8_t brightness) {
    furi_assert(model);

    brightness_control_set_manual_brightness(model->ctrl, brightness);
}

uint8_t brightness_model_get(BrightnessModel* model) {
    furi_assert(model);

    FuriState* fstate = brightness_control_get_state(model->ctrl);
    BrightnessControlState state;
    furi_state_get(fstate, &state);

    return state.brightness_setting;
}

void brightness_model_format(BrightnessModel* model, FuriString* string) {
    furi_assert(model);
    furi_assert(string);

    if(brightness_model_get_mode(model) == BrightnessModeAuto) {
        furi_string_set_str(string, "Auto");
    } else {
        furi_string_printf(string, "%hhu%%", brightness_model_get(model));
    }
}
