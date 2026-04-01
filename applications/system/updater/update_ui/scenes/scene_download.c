#include "../update_ui_i.h"

#include <gui/modules/progress_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>

#include <inttypes.h>

#define BACK_DETAIL_LABEL_TEXT_COLOR ((Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88))

typedef enum {
    UpdateUiDownloadSceneEventUpdateStateChange = UpdateUiEventSceneEventsStart,
} UpdateUiDownloadEvent;

typedef struct {
    FlexLayout* front_layout;
    Label* front_percent_label;
    ProgressBar* front_progress_bar;

    FlexLayout* back_layout;
    Label* back_percent_label;
    ProgressBar* back_progress_bar;
    Label* back_detail_label;

    FuriStateSub* update_state_subscription;
} UpdateUiDownloadScene;

static void update_ui_download_scene_update_state_callback(const void* item, void* context) {
    UNUSED(item);

    UpdateUi* instance = context;

    update_ui_internal_fire_event(instance, UpdateUiDownloadSceneEventUpdateStateChange);
}

static void update_ui_download_scene_on_update_state_change(UpdateUi* instance) {
    UpdateUiDownloadScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateUiSceneIdxDownload);

    UpdaterUpdateState update_state;
    furi_state_get(updater_get_update_state(instance->updater), &update_state);

    if(update_state.status == UpdaterStatusDownloadAbort) {
        furi_event_loop_stop(instance->event_loop);
        return;
    }

    if(update_state.status == UpdaterStatusDownloadFailure) {
        furi_string_set(instance->failure_preset.front_text, "Download failed");
        furi_string_set(instance->failure_preset.back_primary_text, "Download failed");
        furi_string_set(instance->failure_preset.back_detail_text, "Cannot download file");

        scene_manager_replace_current_scene(instance->scene_manager, UpdateUiSceneIdxFailure);
        return;
    }

    if(update_state.action == UpdaterUpdateActionDownload) {
        if(update_state.event == UpdaterUpdateEventActionProgress) {
            size_t received = update_state.as_download.received_size;
            size_t total = update_state.as_download.total_size;

            uint8_t percent = (total > 0) ? 100 * received / total : 0;
            FuriString* percent_text = furi_string_alloc_printf("%" PRIu8 "%%", percent);

            with_gui(instance->gui, {
                label_set_text(scene->front_percent_label, furi_string_get_cstr(percent_text));
                progress_bar_set_value(scene->front_progress_bar, percent);

                label_set_text(scene->back_percent_label, furi_string_get_cstr(percent_text));
                progress_bar_set_value(scene->back_progress_bar, percent);
            });

            furi_string_free(percent_text);
            return;
        }
    } else {
        scene_manager_replace_current_scene(instance->scene_manager, UpdateUiSceneIdxPrepare);
    }
}

static void update_ui_download_scene_on_enter(void* context) {
    UpdateUi* instance = context;
    UpdateUiDownloadScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateUiSceneIdxDownload);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_layout =
            flex_layout_alloc(instance->front_scene_window, FlexLayoutTypeColumn);
        flex_layout_set_spacing(scene->front_layout, 1);

        FlexLayout* front_status_layout =
            flex_layout_alloc(flex_layout_get_base(scene->front_layout), FlexLayoutTypeRow);
        widget_set_height(flex_layout_get_base(front_status_layout), 9); /* font's line height */

        Label* front_status_label = label_alloc(flex_layout_get_base(front_status_layout));
        label_set_text(front_status_label, "Downloading");
        label_set_font(front_status_label, FONT_BUSY_REGULAR_5);
        widget_set_size_content(label_get_base(front_status_label));
        flex_layout_set_child_widget_grow(
            front_status_layout, label_get_base(front_status_label), 1);

        scene->front_percent_label = label_alloc(flex_layout_get_base(front_status_layout));
        label_set_text(scene->front_percent_label, "0%");
        label_set_font(scene->front_percent_label, FONT_BUSY_REGULAR_5);
        widget_set_size_content(label_get_base(scene->front_percent_label));
        flex_layout_set_child_widget_grow(
            front_status_layout, label_get_base(scene->front_percent_label), 0);

        scene->front_progress_bar = progress_bar_alloc(flex_layout_get_base(scene->front_layout));
        widget_set_height(progress_bar_get_base(scene->front_progress_bar), 4);

        /* back layout setup */
        scene->back_layout = flex_layout_alloc(instance->back_scene_window, FlexLayoutTypeColumn);
        flex_layout_set_align(
            scene->back_layout, FlexLayoutAlignEnd, FlexLayoutAlignStart, FlexLayoutAlignCenter);
        widget_set_padding(flex_layout_get_base(scene->back_layout), 2, 2, 0, 0);

        FlexBox* back_status_container = flex_box_alloc(flex_layout_get_base(scene->back_layout));
        flex_box_set_flow(back_status_container, FlexBoxFlowRow);
        flex_box_set_align(back_status_container, FlexBoxAlignStart, FlexBoxAlignEnd);
        flex_box_set_spacing(back_status_container, 4);
        widget_set_margin(flex_box_get_base(back_status_container), 0, 0, 0, 7);

        Image* back_status_image = image_alloc(flex_box_get_base(back_status_container));
        image_set_source(back_status_image, THIS_IMG_PATH("download_back_12x12.image"));
        widget_set_padding(image_get_base(back_status_image), 0, 0, 0, 1);

        Label* back_status_label = label_alloc(flex_box_get_base(back_status_container));
        label_set_text(back_status_label, "Downloading");
        label_set_font(back_status_label, FONT_BUSY_REGULAR_9);

        scene->back_percent_label = label_alloc(flex_box_get_base(back_status_container));
        label_set_text(scene->back_percent_label, "0%");
        label_set_font(scene->back_percent_label, FONT_BUSY_REGULAR_9);
        widget_set_ignore_layout(label_get_base(scene->back_percent_label), true);
        widget_set_align(label_get_base(scene->back_percent_label), AlignRightMid);

        scene->back_progress_bar = progress_bar_alloc(flex_layout_get_base(scene->back_layout));
        widget_set_height(progress_bar_get_base(scene->back_progress_bar), 6);
        widget_set_margin(progress_bar_get_base(scene->back_progress_bar), 0, 0, 0, 14);

        FlexBox* back_detail_container = flex_box_alloc(flex_layout_get_base(scene->back_layout));
        flex_box_set_flow(back_detail_container, FlexBoxFlowRow);
        flex_box_set_align(back_detail_container, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(back_detail_container, 3);
        widget_set_padding(flex_box_get_base(back_detail_container), 0, 0, 0, 8);

        scene->back_detail_label = label_alloc(flex_box_get_base(back_detail_container));
        label_set_text(scene->back_detail_label, "To cancel update press");
        label_set_font(scene->back_detail_label, FONT_BUSY_REGULAR_7);
        label_set_text_color(scene->back_detail_label, BACK_DETAIL_LABEL_TEXT_COLOR);

        Image* back_detail_image = image_alloc(flex_box_get_base(back_detail_container));
        image_set_source(back_detail_image, THIS_IMG_PATH("arrow_back_11x11.image"));
    });

    scene->update_state_subscription = furi_state_subscribe(
        updater_get_update_state(instance->updater),
        update_ui_download_scene_update_state_callback,
        instance);

    update_ui_download_scene_on_update_state_change(instance);
}

static void update_ui_download_scene_on_exit(void* context) {
    UpdateUi* instance = context;
    UpdateUiDownloadScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateUiSceneIdxDownload);

    furi_state_unsubscribe(scene->update_state_subscription);

    with_gui(instance->gui, {
        flex_layout_free(scene->back_layout);
        flex_layout_free(scene->front_layout);
    });
}

static bool update_ui_download_scene_on_event(const SceneManagerEvent* event, void* context) {
    UpdateUi* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case UpdateUiDownloadSceneEventUpdateStateChange:
            update_ui_download_scene_on_update_state_change(instance);
            break;

        default:
            return false;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        updater_abort_download(instance->updater);
    }

    return true;
}

const Scene update_ui_internal_scene_download = {
    .enter_callback = update_ui_download_scene_on_enter,
    .exit_callback = update_ui_download_scene_on_exit,
    .event_callback = update_ui_download_scene_on_event,
    .data_size = sizeof(UpdateUiDownloadScene),
};
