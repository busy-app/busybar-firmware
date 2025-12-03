#include "busy_presets.h"

const TransitionOverlayPreset busy_transitions[BusyTransitionTypeMax] = {
    [BusyTransitionTypeDefault] =
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
    [BusyTransitionTypeAutomatic] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeMultiply,
            .timings =
                {
                    .in_ms = 340,
                    .out_ms = 340,
                },
            .mask.file_path = BUSY_ANIM_PATH("transition_oval_72x16.anim"),
        },
    [BusyTransitionTypeSkip] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 250,
                    .out_ms = 250,
                },
            .mask.file_path = BUSY_ANIM_PATH("transition_skip_72x16.anim"),
        },
    [BusyTransitionTypeSelect] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 100,
                    .out_ms = 1000,
                },
            .effect = TransitionOverlayEffectPress,
            .mask.file_path = BUSY_ANIM_PATH("transition_select_72x16.anim"),
        },
    [BusyTransitionTypeWork] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 100,
                    .out_ms = 1000,
                },
            .effect = TransitionOverlayEffectPress,
            .mask.file_path = BUSY_ANIM_PATH("transition_select_red_72x16.anim"),
        },
    [BusyTransitionTypeRest] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 100,
                    .out_ms = 1000,
                },
            .effect = TransitionOverlayEffectPress,
            .mask.file_path = BUSY_ANIM_PATH("transition_select_green_72x16.anim"),
        },
    [BusyTransitionTypeWorkDone] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 134,
                    .out_ms = 1000,
                },
            .mask.file_path = BUSY_ANIM_PATH("transition_done_red_72x16.anim"),
        },
    [BusyTransitionTypeRestDone] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 134,
                    .out_ms = 1000,
                },
            .mask.file_path = BUSY_ANIM_PATH("transition_done_green_72x16.anim"),
        },
};

const BusyStatusLightsPreset busy_status_lights[BusyStatusLightsTypeMax] = {
    [BusyStatusLightsTypeOff] =
        {
            .preset = StatusLightsPresetOff,
        },
    [BusyStatusLightsTypeWork] =
        {
            .preset = StatusLightsPresetStaticColor,
            .color = COLOR_MAKE_RGB(150, 0, 0),
        },
    [BusyStatusLightsTypeRest] =
        {
            .preset = StatusLightsPresetStaticColor,
            .color = COLOR_MAKE_RGB(10, 150, 5),
        },
};

const TimerIndicatorAnimSources busy_indicator_anim_sources = {
    .states =
        {
            [TimerIndicatorStateWork] = BUSY_ANIM_PATH("busy_label_40x14.anim"),
            [TimerIndicatorStateRest] = BUSY_ANIM_PATH("rest_label_40x14.anim"),
            [TimerIndicatorStateWorkBig] = BUSY_ANIM_PATH("busy_label_70x14.anim"),
            [TimerIndicatorStateRestBig] = BUSY_ANIM_PATH("rest_label_70x14.anim"),
        },
    .transitions =
        {
            [TimerIndicatorTransitionOffToSimple] =
                BUSY_ANIM_PATH("busy_label_transition_70x14.anim"),
        },
};
