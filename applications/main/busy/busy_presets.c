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

const BusySceneTimerIntervalAsset busy_scene_timer_interval_assets[] = {
    [BusySceneTimerIntervalAssetIdBusy] =
        {
            // Scale version
            // .position_start = {-37.0f, 8.f, 0.f},
            // .position_end = {1.0f, 8.f, 0.f},
            // Position version
            .position_start = {-1.5f, 100.0f, 100.0f},
            .position_end = {54.0f, 100.0f, 100.0f},
            .anim_path = BUSY_ANIM_PATH("busy_particles_41x16.anim"),
            .lottie_path = BUSY_ASSETS_PATH("busy_label_progress_lottie_small.json"),
            .image_path = BUSY_IMG_PATH("busy_text_label_41x16.bin"),
            .countdown_main_color = COLOR_MAKE_HEX(0xFF6077),
            .countdown_blink_color = COLOR_MAKE_HEX(0xFFC8C8),
        },
    [BusySceneTimerIntervalAssetIdRest] =
        {
            .position_start = {20.5f, 33.f, 0.f},
            .position_end = {20.5f, 16.f, 0.f},
            .anim_path = BUSY_ANIM_PATH("rest_particles_41x16.anim"),
            .lottie_path = BUSY_ASSETS_PATH("rest_label_progress_lottie_small.json"),
            .image_path = BUSY_IMG_PATH("rest_text_label_41x16.bin"),
            .countdown_main_color = COLOR_MAKE_HEX(0x3EC287),
            .countdown_blink_color = COLOR_MAKE_HEX(0x7BFFCA),
        },
};
