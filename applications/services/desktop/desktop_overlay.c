#include "desktop_overlay.h"
#include "storage_macros.h"

#include <furi.h>
#include <lvgl.h>

#include <gui/modules/anim_play.h>

#define TAG "DesktopOverlay"

#define FADE_OUT_ANIM_TIME_MS 100

typedef struct {
    uint32_t begin;
    uint32_t end;
} DesctopOverlayFrameRange;

struct DesktopOverlay {
    Gui* gui;
    Widget* fade_out_widget;
    AnimPlay* mask_anim;
    bool show_requested;
};

#define MASK_L_TO_R "left_to_right"
#define MASK_R_TO_L "right_to_left"

static void
    desktop_overlay_mask_frame(AnimPlay* anim, const AnimFileFrameInfo* frame, void* context) {
    UNUSED(context);
    if(frame->flags & FuriFlagError) return;

    if(frame->flags & AnimFileFrameFlagFinished) {
        widget_set_visible(anim_play_get_base(anim), false);
    }
}

static void desktop_overlay_fade_out_anim_exec_callback(void* var, int32_t value) {
    lv_obj_set_style_bg_opa(var, value, LV_PART_MAIN);
}

DesktopOverlay* desktop_overlay_alloc(Gui* gui) {
    DesktopOverlay* instance = malloc(sizeof(DesktopOverlay));
    instance->gui = gui;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        Widget* root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);

        instance->fade_out_widget = widget_alloc(root);
        widget_set_visible(instance->fade_out_widget, false);
        lv_obj_set_style_bg_color(
            (lv_obj_t*)instance->fade_out_widget, lv_color_black(), LV_PART_MAIN);

        instance->mask_anim = anim_play_alloc(root);
        widget_set_visible(anim_play_get_base(instance->mask_anim), false);
        widget_set_blend_mode(anim_play_get_base(instance->mask_anim), WidgetBlendModeMultiply);
        anim_play_set_source(
            instance->mask_anim, DESKTOP_ANIM_PATH("hosizontal_mask_transition_72x16.anim"));
        anim_play_set_frame_callback(instance->mask_anim, desktop_overlay_mask_frame, instance);
        anim_play_pause(instance->mask_anim);
    });

    instance->show_requested = false;

    return instance;
}

void desktop_overlay_show(DesktopOverlay* instance, DesktopOverlayTransitionType type) {
    furi_check(type < DesktopOverlayTransitionTypesCount);

    if(type != DesktopOverlayTransitionTypeNone) {
        with_gui(instance->gui, {
            widget_set_visible(instance->fade_out_widget, true);

            lv_anim_t fade_out_anim;
            lv_anim_init(&fade_out_anim);
            lv_anim_set_user_data(&fade_out_anim, instance);
            lv_anim_set_var(&fade_out_anim, instance->fade_out_widget);
            lv_anim_set_values(&fade_out_anim, LV_OPA_TRANSP, LV_OPA_COVER);
            lv_anim_set_duration(&fade_out_anim, FADE_OUT_ANIM_TIME_MS);
            lv_anim_set_path_cb(&fade_out_anim, lv_anim_path_ease_in_out);
            lv_anim_set_exec_cb(&fade_out_anim, desktop_overlay_fade_out_anim_exec_callback);
            lv_anim_start(&fade_out_anim);
        });
    }

    instance->show_requested = true;
}

void desktop_overlay_hide(DesktopOverlay* instance, DesktopOverlayTransitionType type) {
    furi_check(type < DesktopOverlayTransitionTypesCount);

    with_gui(instance->gui, {
        widget_set_visible(instance->fade_out_widget, false);

        if(type != DesktopOverlayTransitionTypeNone) {
            const char* section = (type == DesktopOverlayTransitionTypeUp) ? MASK_R_TO_L :
                                                                             MASK_L_TO_R;
            widget_set_visible(anim_play_get_base(instance->mask_anim), true);

            AnimFile* file = anim_play_get_file(instance->mask_anim);
            if(file) {
                if(anim_file_set_section(file, AnimFilePlayFlagNone, section)) {
                    anim_play_start(instance->mask_anim);
                }
            }
        }
    });

    instance->show_requested = false;
}

bool desktop_overlay_show_requested(const DesktopOverlay* instance) {
    return instance->show_requested;
}
