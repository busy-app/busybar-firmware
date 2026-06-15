#include "brightness.h"
#include <brightness_control/brightness_control.h>
#include <toolbox/float_tools.h>

struct BrightnessModel {
    BrightnessControl* ctrl;
    BrightnessModelBrightnessChangedCallback callback;
    void* callback_context;
    FuriStateSub* state_sub;
};

static void state_callback(const void* item, void* context);

BrightnessModel* brightness_model_alloc(void) {
    BrightnessModel* model = malloc(sizeof(BrightnessModel));
    model->ctrl = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
    model->callback = NULL;
    model->callback_context = NULL;
    model->state_sub = NULL;
    return model;
}

void brightness_model_free(BrightnessModel* model) {
    furi_assert(model);
    if(model->state_sub) {
        furi_state_unsubscribe(model->state_sub);
    }
    furi_record_close(RECORD_BRIGHTNESS_CONTROL);
    free(model);
}

void brightness_model_set_callback(
    BrightnessModel* model,
    BrightnessModelBrightnessChangedCallback callback,
    void* context) {
    model->callback = callback;
    model->callback_context = context;
    if(model->state_sub) {
        furi_state_unsubscribe(model->state_sub);
    }
    if(callback) {
        FuriState* fstate = brightness_control_get_state(model->ctrl);
        model->state_sub = furi_state_subscribe(fstate, state_callback, model);
    } else {
        model->state_sub = NULL;
    }
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

static void state_callback(const void* item, void* context) {
    const BrightnessControlState* state = item;
    BrightnessModel* model = context;

    BrightnessMode mode = state->mode == BrightnessControlBrightnessModeAuto ?
                              BrightnessModeAuto :
                              BrightnessModeManual;

    model->callback(mode, state->brightness_setting, model->callback_context);
}
