#include "desktop_overlay.h"

#include <furi.h>
#include <lvgl.h>

#include <gui/modules/image.h>
#include <assets/assets_images.h>

#define TAG "DesktopOverlay"

#define OVERLAY_ANIM_TIME_MS (100)

struct DesktopOverlay {
    Gui* gui;
    Widget* dimmer;
    Image* status_bar;
    bool show_requested;
};

static void desktop_overlay_anim_callback(void* var, int32_t value) {
    lv_obj_set_style_bg_opa(var, value, LV_PART_MAIN);
}

static void desktop_overlay_start_anim(DesktopOverlay* instance, int32_t end) {
    with_gui(instance->gui, {
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, instance->dimmer);
        // TODO: Decide on the color and opacity API
        lv_anim_set_values(
            &anim, lv_obj_get_style_bg_opa((lv_obj_t*)instance->dimmer, LV_PART_MAIN), end);
        lv_anim_set_duration(&anim, OVERLAY_ANIM_TIME_MS);
        lv_anim_set_exec_cb(&anim, desktop_overlay_anim_callback);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
        lv_anim_start(&anim);
    });
}

DesktopOverlay* desktop_overlay_alloc(Gui* gui) {
    DesktopOverlay* instance = malloc(sizeof(DesktopOverlay));
    instance->gui = gui;

    with_gui(instance->gui, {
        GuiLayer* system_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);

        Widget* root;

        root = gui_layer_get_root_widget(system_layer, GuiDisplayIdFront);
        instance->dimmer = widget_alloc(root);
        // TODO: Decide on the color and opacity API
        lv_obj_set_style_bg_opa((lv_obj_t*)instance->dimmer, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_color((lv_obj_t*)instance->dimmer, lv_color_black(), LV_PART_MAIN);

        root = gui_layer_get_root_widget(system_layer, GuiDisplayIdBack);
        instance->status_bar = image_alloc(root);
        // TODO: Implement built-in images properly
        image_set_source(instance->status_bar, (const void*)(&I_status_bar_dummy_12x80));
        widget_set_align(image_get_base(instance->status_bar), AlignRightMid);
    });

    return instance;
}

void desktop_overlay_show(DesktopOverlay* instance) {
    desktop_overlay_start_anim(instance, LV_OPA_COVER);
    instance->show_requested = true;
}

void desktop_overlay_hide(DesktopOverlay* instance) {
    desktop_overlay_start_anim(instance, LV_OPA_TRANSP);
    instance->show_requested = false;
}

bool desktop_overlay_show_requested(const DesktopOverlay* instance) {
    return instance->show_requested;
}
