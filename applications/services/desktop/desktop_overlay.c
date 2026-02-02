#include "desktop_overlay.h"
#include "storage_macros.h"

#include <furi.h>
#include <lvgl.h>

#include <gui/modules/anim_image.h>

#define TAG "DesktopOverlay"

#define FADE_OUT_ANIM_TIME_MS 100

typedef struct {
    uint32_t begin;
    uint32_t end;
} DesctopOverlayFrameRange;

struct DesktopOverlay {
    Gui* gui;
    Widget* fade_out_widget;
    AnimImage* mask_anim_image;
    bool show_requested;
};

static const DesctopOverlayFrameRange mask_anim_frame_ranges[] = {
    [DesktopOverlayTransitionTypeUp] = {.begin = 10, .end = 19},
    [DesktopOverlayTransitionTypeDown] = {.begin = 0, .end = 9},
};

static void desktop_overlay_mask_anim_image_completed_callback(AnimImage* anim, void* context) {
    UNUSED(context);
    widget_set_visible(anim_image_get_base(anim), false);
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

        instance->mask_anim_image = anim_image_alloc(root);
        widget_set_visible(anim_image_get_base(instance->mask_anim_image), false);
        widget_set_blend_mode(
            anim_image_get_base(instance->mask_anim_image), WidgetBlendModeMultiply);
        anim_image_set_source(
            instance->mask_anim_image, DESKTOP_ANIM_PATH("hosizontal_mask_transition_72x16.anim"));
        anim_image_set_completed_callback(
            instance->mask_anim_image,
            desktop_overlay_mask_anim_image_completed_callback,
            instance);
        anim_image_stop(instance->mask_anim_image);
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
            const DesctopOverlayFrameRange* range = &mask_anim_frame_ranges[type];

            widget_set_visible(anim_image_get_base(instance->mask_anim_image), true);
            anim_image_set_range(
                instance->mask_anim_image, range->begin, range->end, false, false);
        }
    });

    instance->show_requested = false;
}

bool desktop_overlay_show_requested(const DesktopOverlay* instance) {
    return instance->show_requested;
}
