#include "../firmware_i.h"

#include <gui/storage_macros.h>
#include <gui/modules/progress_bar.h>
#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>

#include <path.h>
#include <settings_helpers/gui_params.h>

#include <inttypes.h>

typedef enum {
    ThisSceneEventDownloadAborted = ThisEventSceneEventsStart,
} ThisSceneEvent;

typedef struct {
    FlexLayout* front_layout;
    Label* front_percent_label;
    ProgressBar* front_progress_bar;

    FlexLayout* back_layout;
    Label* back_percent_label;
    ProgressBar* back_progress_bar;
    Label* back_extras_label;

    FuriStateSub* update_state_subscription;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxDownload);
}

static void this_prepare_up_to_date_result(ThisInstance* instance) {
    instance->result_preset.front_image_path = THIS_IMG_PATH("error_front_8x8.bin");
    furi_string_set(instance->result_preset.front_text, "Download failed");

    instance->result_preset.back_image_path = THIS_IMG_PATH("error_back_11x11.bin");
    furi_string_set(instance->result_preset.back_primary_text, "Download failed");
    furi_string_set(instance->result_preset.back_auxiliary_text, "Cannot download file");

    instance->result_preset.timeout = 4000;
}

static void this_update_state_callback(const void* item, void* context) {
    const UpdaterUpdateState* update_state = item;
    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    switch(update_state->event) {
    case UpdaterUpdateEventActionProgress:
        if(update_state->action == UpdaterUpdateActionDownload) {
            size_t received = update_state->as_download.received_size;
            size_t total = update_state->as_download.total_size;

            uint8_t percent = (total > 0) ? 100 * received / total : 0;
            FuriString* percent_text = furi_string_alloc_printf("%" PRIu8 "%%", percent);

            with_gui(instance->gui, {
                label_set_text(scene->front_percent_label, furi_string_get_cstr(percent_text));
                progress_bar_set_value(scene->front_progress_bar, percent);

                label_set_text(scene->back_percent_label, furi_string_get_cstr(percent_text));
                progress_bar_set_value(scene->back_progress_bar, percent);
            });
        }
        break;

    case UpdaterUpdateEventActionDone:
        if(update_state->status == UpdaterStatusDownloadAbort) {
            settings_firmware_app_fire_event(instance, ThisSceneEventDownloadAborted);
        } else if(update_state->status != UpdaterStatusOk) {
            this_prepare_up_to_date_result(instance);
            scene_manager_replace_current_scene(instance->scene_manager, ThisSceneIdxResult);
        }
        break;

    default:
        break;
    }
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    FuriString* filename = furi_string_alloc();
    path_extract_filename(instance->update_info.url, filename, true);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_layout =
            flex_layout_alloc(instance->front_scene_window, FlexLayoutTypeColumn);
        flex_layout_set_spacing(scene->front_layout, 2);
        widget_set_padding(flex_layout_get_base(scene->front_layout), 0, 0, 1, 2);

        Widget* front_status_container = widget_alloc(flex_layout_get_base(scene->front_layout));
        widget_set_height_content(front_status_container);

        Label* front_status_label = label_alloc(front_status_container);
        label_set_text(front_status_label, "Downloading");
        widget_set_align(label_get_base(front_status_label), AlignLeftMid);

        scene->front_percent_label = label_alloc(front_status_container);
        label_set_text(scene->front_percent_label, "0%");
        widget_set_align(label_get_base(scene->front_percent_label), AlignRightMid);

        scene->front_progress_bar = progress_bar_alloc(flex_layout_get_base(scene->front_layout));
        widget_set_height(progress_bar_get_base(scene->front_progress_bar), 4);

        /* back layout setup */
        scene->back_layout = flex_layout_alloc(instance->back_scene_window, FlexLayoutTypeColumn);
        widget_set_padding(flex_layout_get_base(scene->back_layout), 2, 2, 10, 0);

        FlexBox* back_status_container = flex_box_alloc(flex_layout_get_base(scene->back_layout));
        flex_box_set_flow(back_status_container, FlexBoxFlowRow);
        flex_box_set_align(back_status_container, FlexBoxAlignStart, FlexBoxAlignEnd);
        flex_box_set_spacing(back_status_container, 4);
        widget_set_padding(flex_box_get_base(back_status_container), 0, 0, 0, 7);

        Image* back_status_image = image_alloc(flex_box_get_base(back_status_container));
        image_set_source(back_status_image, THIS_IMG_PATH("download_12x12.bin"));
        widget_set_padding(image_get_base(back_status_image), 0, 0, 0, 1);

        Label* back_status_label = label_alloc(flex_box_get_base(back_status_container));
        label_set_text(back_status_label, "Downloading");

        scene->back_percent_label = label_alloc(flex_box_get_base(back_status_container));
        label_set_text(scene->back_percent_label, "0%");
        widget_set_ignore_layout(label_get_base(scene->back_percent_label), true);
        widget_set_align(label_get_base(scene->back_percent_label), AlignBottomRight);

        scene->back_progress_bar = progress_bar_alloc(flex_layout_get_base(scene->back_layout));
        widget_set_height(progress_bar_get_base(scene->back_progress_bar), 8);

        scene->back_extras_label = label_alloc(flex_layout_get_base(scene->back_layout));
        label_set_text_color(scene->back_extras_label, (Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88));
        label_set_text_font_size(scene->back_extras_label, LabelFontSizeSmall);
        label_set_text(scene->back_extras_label, furi_string_get_cstr(filename));
        label_set_text_align(scene->back_extras_label, TextAlignCenter);
        widget_set_width_content(label_get_base(scene->back_extras_label));
        widget_set_padding(label_get_base(scene->back_extras_label), 0, 0, 7, 0);
    });

    scene->update_state_subscription = furi_state_subscribe(
        updater_get_update_state(instance->updater), this_update_state_callback, instance);

    updater_install_from_url(
        instance->updater,
        furi_string_get_cstr(instance->update_info.url),
        furi_string_get_cstr(instance->update_info.sha256));
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    furi_state_unsubscribe(scene->update_state_subscription);

    with_gui(instance->gui, {
        flex_layout_free(scene->back_layout);
        flex_layout_free(scene->front_layout);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    ThisInstance* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisSceneEventDownloadAborted:
            if(!scene_manager_previous_scene(instance->scene_manager)) {
                desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_APP_NAME);
            }
            return true;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        updater_abort_download(instance->updater);
        return true;
    }

    return false;
}

const Scene settings_firmware_app_scene_download = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
