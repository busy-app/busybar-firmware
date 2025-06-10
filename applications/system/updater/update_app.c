#include "update_config.h"

#include <toolbox/update_lib/dfu_file.h>
#include <toolbox/update_lib/update_manifest.h>
#include <toolbox/update_lib/common_vals.h>
#include <toolbox/path.h>

#include <furi_hal_flash.h>
#include <furi_hal_power.h>
#include <furi.h>
#include <furi_hal_nvm.h>
#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>

#include "worker/update_task.h"

#define TAG "UpdaterApp"

typedef struct {
    Gui* gui;
    Storage* storage;
    FuriEventLoop* event_loop;

    // UI Elements
    // Back display
    FlexLayout* flex;
    Label *stage_label, *progress_label;

    UpdateTask* update_task;
    FuriString* startup_arg;
} Updater;

static void updater_task_on_progress(
    const char* status,
    const uint8_t stage_pct,
    bool failed,
    void* state) {
    furi_check(state);
    Updater* updater = state;

    with_gui(updater->gui, {
        if(failed) {
            label_set_text_fmt(
                updater->stage_label, "Error: %s", status ? status : "Unknown error");
            label_set_text_fmt(updater->progress_label, "Progress: %d%%", stage_pct);
        } else {
            label_set_text_fmt(updater->stage_label, "%s", status ? status : "Unknown stage");
            label_set_text_fmt(updater->progress_label, "Progress: %d%%", stage_pct);
        }
    });
}

Updater* updater_alloc(const char* arg) {
    Updater* updater = malloc(sizeof(Updater));

    updater->storage = furi_record_open(RECORD_STORAGE);
    updater->startup_arg = furi_string_alloc_set(arg ? arg : "");
    updater->update_task = update_task_alloc();

    updater->gui = furi_record_open(RECORD_GUI);
    updater->event_loop = furi_event_loop_alloc();

    with_gui(updater->gui, {
        GuiLayer* main_layer = gui_get_layer(updater->gui, GuiLayerIdMain);
        updater->flex = flex_layout_alloc(
            gui_layer_get_root_widget(main_layer, GuiDisplayIdBack), FlexLayoutTypeColumn);

        Widget* flex_base = flex_layout_get_base(updater->flex);
        updater->stage_label = label_alloc(flex_base);
        label_set_text(updater->stage_label, "Updater is starting...");

        updater->progress_label = label_alloc(flex_base);
        label_set_text(updater->progress_label, "Progress: 0%%");
    });

    update_task_set_progress_cb(updater->update_task, updater_task_on_progress, updater);
    update_task_start(updater->update_task);

    return updater;
}

void updater_free(Updater* updater) {
    furi_check(updater);

    if(updater->update_task) {
        update_task_set_progress_cb(updater->update_task, NULL, NULL);
        update_task_free(updater->update_task);
    }

    flex_layout_free(updater->flex);
    label_free(updater->stage_label);
    label_free(updater->progress_label);
    furi_event_loop_free(updater->event_loop);
    furi_string_free(updater->startup_arg);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);
    free(updater);
}

int32_t updater_srv(void* arg) {
    Updater* updater = updater_alloc((const char*)arg);

    furi_event_loop_run(updater->event_loop);
    updater_free(updater);
    return 0;
}
