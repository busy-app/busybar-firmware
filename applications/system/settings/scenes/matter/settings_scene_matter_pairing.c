#include "../../settings.h"
#include "../../storage_macros.h"
#include "../settings_scenes.h"
#include "matter_scenes_common.h"

#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <gui/modules/rect.h>
#include <gui/modules/qr_code.h>

#include <matter/matter.h>

typedef struct {
    bool ui_initialized;

    struct {
        Image* info_icon;
        Label* label;
    } front;

    struct {
        Rect* card;

        Image* logo;
        Label* wordmark;

        FlexLayout* man_code_layout;
        Label* man_code_title;
        Label* man_code;

        QRCode* qr_code;
    } back;
} SettingsSceneMatterPairing;

typedef enum {
    SettingsSceneMatterPairingEventSwitchToConnectWifi = SettingsCustomEventSceneEventsStart,
} SettingsSceneMatterPairingEvent;

static void settings_scene_matter_pairing_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneMatterPairing* scene = scene_manager_get_current_scene_data(app->scene_manager);

    scene->ui_initialized = false;

    if(!app->is_wifi_available) {
        settings_send_custom_event(app, SettingsSceneMatterPairingEventSwitchToConnectWifi);
        return;
    }

    FuriString* qr_code = furi_string_alloc();
    FuriString* man_code = furi_string_alloc();

    size_t window_secs = matter_enable_commissioning(app->matter, qr_code, man_code);
    UNUSED(window_secs);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        /* front */ {
            scene->front.info_icon = image_alloc(app->front_scene_window);
            image_set_source(scene->front.info_icon, SETTINGS_IMG_PATH("info_front_7x7.bin"));
            Widget* info_icon_base = image_get_base(scene->front.info_icon);
            widget_set_align(info_icon_base, AlignLeftMid);

            scene->front.label = label_alloc(app->front_scene_window);
            label_set_text(scene->front.label, "Look at back\nscreen");
            Widget* label_base = label_get_base(scene->front.label);
            widget_set_align(label_base, AlignLeftMid);
            widget_set_pos(label_base, 10, 0);
        }

        /* back */ {
            scene->back.card = rect_alloc(app->back_scene_window);
            Widget* card_base = rect_get_base(scene->back.card);
            widget_set_padding(card_base, 4, 6, 6, 6);
            widget_set_size(card_base, 146, 64);

            /* logo */ {
                scene->back.logo = image_alloc(card_base);
                image_set_source(scene->back.logo, SETTINGS_IMG_PATH("matter_back_14x14.bin"));
                Widget* image_base = image_get_base(scene->back.logo);
                widget_set_align(image_base, AlignTopLeft);

                scene->back.wordmark = label_alloc(card_base);
                label_set_text(scene->back.wordmark, "matter");
                label_set_font(scene->back.wordmark, LabelFontMedium);
                label_set_color(scene->back.wordmark, LabelColorBlack);
                Widget* wordmark_base = label_get_base(scene->back.wordmark);
                widget_set_align(wordmark_base, AlignTopLeft);
                widget_set_pos(wordmark_base, 19, 1);
            }

            /* manual code */ {
                scene->back.man_code_layout = flex_layout_alloc(card_base, FlexLayoutTypeColumn);
                flex_layout_set_align(
                    scene->back.man_code_layout,
                    FlexLayoutAlignEnd,
                    FlexLayoutAlignStart,
                    FlexLayoutAlignStart);
                flex_layout_set_spacing(scene->back.man_code_layout, 1);
                Widget* layout_base = flex_layout_get_base(scene->back.man_code_layout);
                widget_set_align(layout_base, AlignBottomLeft);

                scene->back.man_code_title = label_alloc(layout_base);
                label_set_text(scene->back.man_code_title, "Manual code");
                label_set_color(scene->back.man_code_title, LabelColorGrey);

                scene->back.man_code = label_alloc(layout_base);
                label_set_text(scene->back.man_code, furi_string_get_cstr(man_code));
                label_set_color(scene->back.man_code, LabelColorBlack);
                label_set_font(scene->back.man_code, LabelFontNumerals);
            }

            scene->back.qr_code = qr_code_alloc(card_base);
            qr_code_set_size(scene->back.qr_code, 50);
            qr_code_set_data(scene->back.qr_code, furi_string_get_cstr(qr_code));
            Widget* qr_base = qr_code_get_base(scene->back.qr_code);
            widget_set_align(qr_base, AlignRightMid);
        }
    });

    scene->ui_initialized = true;

    furi_string_free(qr_code);
    furi_string_free(man_code);
}

static void settings_scene_matter_pairing_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneMatterPairing* scene = scene_manager_get_current_scene_data(app->scene_manager);

    if(!scene->ui_initialized) return;

    with_gui(app->gui, {
        // front:
        label_free(scene->front.label);
        image_free(scene->front.info_icon);

        // back:
        qr_code_free(scene->back.qr_code);

        label_free(scene->back.man_code);
        label_free(scene->back.man_code_title);
        flex_layout_free(scene->back.man_code_layout);

        label_free(scene->back.wordmark);
        image_free(scene->back.logo);

        rect_free(scene->back.card);
    });
}

static bool settings_scene_matter_pairing_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    SettingsApp* app = context;

    bool consumed = false;

    do {
        if(event->type == SceneManagerEventTypeCustom) {
            consumed = matter_scene_replace_current(app, event->event);
            if(consumed) break;

            if(event->event == SettingsSceneMatterPairingEventSwitchToConnectWifi) {
                scene_manager_replace_current_scene(
                    app->scene_manager, SettingsAppSceneIdConnectWifi);
                consumed = true;
            }
        }
    } while(0);

    return consumed;
}

const Scene settings_scene_matter_pairing = {
    .enter_callback = settings_scene_matter_pairing_on_enter,
    .exit_callback = settings_scene_matter_pairing_on_exit,
    .event_callback = settings_scene_matter_pairing_on_event,
    .data_size = sizeof(SettingsSceneMatterPairing),
};
