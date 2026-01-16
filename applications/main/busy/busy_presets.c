#include "busy_presets.h"
#include "storage_macros.h"

#define FRAMES_TO_MS(x) ((x) * 1000 / 60)

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
                    .out_ms = 500,
                },
            .mask.file_path = BUSY_ANIM_PATH("transition_done_busy_72x16.anim"),
        },
    [BusyTransitionTypeRestDone] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 134,
                    .out_ms = 500,
                },
            .mask.file_path = BUSY_ANIM_PATH("transition_done_rest_72x16.anim"),
        },
    [BusyTransitionTypeEnding] =
        {
            .type = TransitionOverlayTypeMask,
            .blend_mode = TransitionOverlayBlendModeAdd,
            .timings =
                {
                    .in_ms = 125,
                    .out_ms = 125,
                },
            .mask.file_path = BUSY_ANIM_PATH("transition_flash_72x16.anim"),
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

const TimerIndicatorPreset busy_timer_indicator_presets[BusyTimerIndicatorTypeMax] = {
    [BusyTimerIndicatorTypeWork] =
        {
            .background_config =
                {
                    .anim_path = BUSY_ANIM_PATH("particles_busy_41x16.anim"),
                },
            .progress_config =
                {
                    .lottie_path = BUSY_LOTTIE_PATH("progress_busy_41x16.json"),
                    .direction = TimerIndicatorProgressDirectionHorizontal,
                    .end_offset_px = 38,
                },
            .foreground_config =
                {
                    .image_path = BUSY_IMG_PATH("indicator_busy_41x16.bin"),
                },
        },
    [BusyTimerIndicatorTypeRest] =
        {
            .background_config =
                {
                    .anim_path = BUSY_ANIM_PATH("particles_rest_41x16.anim"),
                },
            .progress_config =
                {
                    .lottie_path = BUSY_LOTTIE_PATH("progress_rest_41x16.json"),
                    .direction = TimerIndicatorProgressDirectionVertical,
                    .end_offset_px = 22,
                },
            .foreground_config =
                {
                    .image_path = BUSY_IMG_PATH("indicator_rest_41x16.bin"),
                },
        },
    [BusyTimerIndicatorTypeWorkBig] =
        {
            .background_config =
                {
                    .anim_path = BUSY_ANIM_PATH("indicator_busy_72x16.anim"),
                },
        },
    [BusyTimerIndicatorTypeRestBig] = {},
};

const TimerIndicatorTransition
    busy_timer_indicator_transitions[BusyTimerIndicatorTransitionTypeMax] = {
        [BusyTimerIndicatorTransitionTypeInfToSimple] =
            {
                .anim_path = BUSY_ANIM_PATH("indicator_busy_transition_72x16.anim"),
                .duration_ms = FRAMES_TO_MS(40),
                .start_width_px = 70,
                .end_width_px = 39,
            },
};

const TimerLabelPreset busy_timer_label_presets[BusyTimerLabelTypeMax] = {
    [BusyTimerLabelTypeWork] =
        {
            .countdown_colors =
                {
                    .base = COLOR_MAKE_HEX(0xFF6077),
                    .blink = COLOR_MAKE_HEX(0x8B2939),
                },
        },
    [BusyTimerLabelTypeRest] =
        {
            .countdown_colors =
                {
                    .base = COLOR_MAKE_HEX(0x3EC287),
                    .blink = COLOR_MAKE_HEX(0x1D6344),
                },
        },
};

const BusyAppGlobalPreset busy_app_global_presets[BusyAppGlobalPresetIdMax] = {
    [BusyAppGlobalPresetIdBusy] =
        {
            .header_img_path = BUSY_IMG_PATH("header_busy_41x16.bin"),
            .start_anim_path = BUSY_ANIM_PATH("start_logo_busy_41x16.anim"),
            .settings_profile_id = BusySettingsProfileIdBusy,
        },

    [BusyAppGlobalPresetIdCustom] =
        {
            .header_img_path = BUSY_IMG_PATH("header_custom_41x16.bin"),
            .start_anim_path = BUSY_ANIM_PATH("start_logo_custom_41x16.anim"),
            .settings_profile_id = BusySettingsProfileIdCustom,
        },
};
