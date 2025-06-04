#include "busy_presets.h"

const BusyTransition busy_transitions[BusyTransitionTypeMax] = {
    [BusyTransitionTypeDefault] =
        {
            .color = COLOR_MAKE_HEX(0x000000),
            .color_mode = TransitionOverlayColorModeNormal,
            .timings =
                {
                    .in_ms = 200,
                    .out_ms = 200,
                },
        },
    [BusyTransitionTypeAutomatic] =
        {
            .mask_path = BUSY_ANIM_PATH("transition_oval_72x16.anim"),
            .mask_mode = TransitionOverlayMaskModeMultiply,
            .timings =
                {
                    .in_ms = 340,
                    .out_ms = 340,
                },
        },
    [BusyTransitionTypeSkip] =
        {
            .mask_path = BUSY_ANIM_PATH("transition_skip_72x16.anim"),
            .mask_mode = TransitionOverlayMaskModeAdd,
            .timings =
                {
                    .in_ms = 250,
                    .out_ms = 250,
                },
        },
    [BusyTransitionTypeSelect] =
        {
            .mask_path = BUSY_ANIM_PATH("transition_select_72x16.anim"),
            .mask_mode = TransitionOverlayMaskModeAdd,
            .timings =
                {
                    .in_ms = 100,
                    .out_ms = 1000,
                },
            .enable_press = true,
        },
    [BusyTransitionTypeWork] =
        {
            .mask_path = BUSY_ANIM_PATH("transition_select_red_72x16.anim"),
            .mask_mode = TransitionOverlayMaskModeAdd,
            .timings =
                {
                    .in_ms = 100,
                    .out_ms = 1000,
                },
            .enable_press = true,
        },
    [BusyTransitionTypeRest] =
        {
            .mask_path = BUSY_ANIM_PATH("transition_select_green_72x16.anim"),
            .mask_mode = TransitionOverlayMaskModeAdd,
            .timings =
                {
                    .in_ms = 100,
                    .out_ms = 1000,
                },
            .enable_press = true,
        },
    [BusyTransitionTypeWorkDone] =
        {
            .mask_path = BUSY_ANIM_PATH("transition_done_red_72x16.anim"),
            .mask_mode = TransitionOverlayMaskModeAdd,
            .timings =
                {
                    .in_ms = 134,
                    .out_ms = 1000,
                },
        },
    [BusyTransitionTypeRestDone] =
        {
            .mask_path = BUSY_ANIM_PATH("transition_done_green_72x16.anim"),
            .mask_mode = TransitionOverlayMaskModeAdd,
            .timings =
                {
                    .in_ms = 134,
                    .out_ms = 1000,
                },
        },
};

const StatusLightsCommand busy_status_lights[BusyStatusLightsTypeMax] = {
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
