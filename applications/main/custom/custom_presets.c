#include "custom_presets.h"

const TransitionOverlayPreset custom_transitions[CustomTransitionTypeMax] = {
    [CustomTransitionTypeDefault] =
        {
            .type = TransitionOverlayTypeColor,
            .blend_mode = TransitionOverlayBlendModeNormal,
            .timings =
                {
                    .in_ms = 200,
                    .out_ms = 200,
                },
            .mask.color = COLOR_MAKE_HEX(0x000000),
        },
    [CustomTransitionTypeSelect] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 100,
                    .out_ms = 1000,
                },
            .effect = TransitionOverlayEffectPress,
            .mask.file_path = CUSTOM_ANIM_PATH("transition_select_72x16.anim"),
        },
};

const CustomStatusLightsPreset custom_status_lights[CustomStatusLightsTypeMax] = {
    [CustomStatusLightsTypeOff] =
        {
            .preset = StatusLightsPresetOff,
        },
    [CustomStatusLightsTypeWork] =
        {
            .preset = StatusLightsPresetStaticColor,
            .color = COLOR_MAKE_RGB(150, 0, 0),
        },
};
