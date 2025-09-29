#include "../settings.h"
#include "../storage_macros.h"

#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <gui/modules/qr_code.h>
#include "../widgets/status_view.h"

#include <matter/matter.h>

#define WIFI_HELP_URL "https://docs.busy.app/bar/basics/connect-wifi"

typedef struct {
    struct {
        StatusView* status_view;
    } front;
    struct {
        FlexLayout* left_column;
        Image* wifi_icon;
        Label* connect_message;
        QRCode* help_url;
    } back;
} SettingsSceneDebugApps;

static void settings_scene_connect_wifi_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        /* front */ {
            scene->front.status_view = status_view_alloc(app->front_scene_window);
            status_view_set_icon(
                scene->front.status_view, SETTINGS_IMG_PATH("wifi_front_7x7.bin"));
            status_view_set_header(
                scene->front.status_view, "Connect to Wi-Fi\nto pair the device");
        }

        /* back */ {
            scene->back.left_column =
                flex_layout_alloc(app->back_scene_window, FlexLayoutTypeColumn);
            flex_layout_set_spacing(scene->back.left_column, 4);
            Widget* left_column_base = flex_layout_get_base(scene->back.left_column);
            widget_set_align(left_column_base, AlignLeftMid);
            widget_set_padding(left_column_base, 5, 0, 0, 0);

            scene->back.wifi_icon = image_alloc(left_column_base);
            image_set_source(scene->back.wifi_icon, SETTINGS_IMG_PATH("wifi_back_12x12.bin"));

            scene->back.connect_message = label_alloc(left_column_base);
            label_set_text(
                scene->back.connect_message, "Connect BUSY Bar\nto Wi-Fi via PC\nor BUSY App");
            label_set_line_spacing(scene->back.connect_message, 2);

            scene->back.help_url = qr_code_alloc(app->back_scene_window);
            Widget* qr_base = qr_code_get_base(scene->back.help_url);
            widget_set_align(qr_base, AlignTopRight);
            widget_set_pos(qr_base, -3, 0);
            qr_code_set_size(scene->back.help_url, 33);
            qr_code_set_data(scene->back.help_url, WIFI_HELP_URL);
        }
    });
}

static void settings_scene_connect_wifi_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        // front:
        status_view_free(scene->front.status_view);
        // back:
        qr_code_free(scene->back.help_url);
        label_free(scene->back.connect_message);
        image_free(scene->back.wifi_icon);
        flex_layout_free(scene->back.left_column);
    });
}

static bool settings_scene_connect_wifi_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    SettingsApp* app = context;

    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

const Scene settings_scene_connect_wifi = {
    .enter_callback = settings_scene_connect_wifi_on_enter,
    .exit_callback = settings_scene_connect_wifi_on_exit,
    .event_callback = settings_scene_connect_wifi_on_event,
    .data_size = sizeof(SettingsSceneDebugApps),
};
