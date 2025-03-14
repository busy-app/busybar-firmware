#include "desktop_overlay.h"

#include <furi.h>

#define TAG "DesktopOverlay"

#define OVERLAY_ANIM_TIME_MS (100)

struct DesktopOverlay {
    Gui* gui;
    lv_obj_t* dimmer;
    bool show_requested;
};

static void desktop_overlay_anim_callback(void* var, int32_t value) {
    lv_obj_set_style_opa(var, value, LV_PART_MAIN);
}

static void desktop_overlay_start_anim(DesktopOverlay* instance, int32_t end) {
    with_gui(instance->gui, {
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, instance->dimmer);
        lv_anim_set_values(&anim, lv_obj_get_style_opa(instance->dimmer, LV_PART_MAIN), end);
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
        Widget* root = gui_get_root_widget(gui, GuiDisplayIdFront, GuiLayerIdSystem);

        instance->dimmer = lv_obj_create((lv_obj_t*)root);
        lv_obj_set_size(instance->dimmer, widget_get_width(root), widget_get_height(root));
        lv_obj_set_style_bg_color(instance->dimmer, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_opa(instance->dimmer, LV_OPA_TRANSP, LV_PART_MAIN);
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
