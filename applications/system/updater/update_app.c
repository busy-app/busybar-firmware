#include "worker/update_task.h"

#include <furi_hal_flash.h>
#include <furi_hal_power.h>
#include <furi.h>
#include <furi_hal_nvm.h>

#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>

#include <toolbox/path.h>
#include <toolbox/update_lib/dfu_file.h>
#include <toolbox/update_lib/update_manifest.h>
#include <toolbox/update_lib/common_vals.h>
#include <toolbox/update_lib/update_config.h>

#define TAG "UpdaterApp"

typedef struct {
    Gui* gui;
    Storage* storage;
    FuriEventLoop* event_loop;

    // UI Elements
    // Back display
    FlexLayout* flex;
    Label *stage_label, *progress_label;
    // Front display
    Canvas* canvas;

    UpdateTask* update_task;
    FuriString* startup_arg;
} Updater;

static const Color ColorGreen = {.r = 0, .g = 140, .b = 20};
static const Color ColorRed = {.r = 140, .g = 0, .b = 10};
static const Color ColorWhite = {.r = 255, .g = 255, .b = 255};

static void updater_task_on_progress(
    const char* status,
    const uint8_t stage_pct,
    bool failed,
    void* state) {
    furi_check(state);
    Updater* updater = state;
    FuriString* progress_pct_str = furi_string_alloc_printf("%d%%", stage_pct);

    with_gui(updater->gui, {
        if(failed) {
            label_set_text_fmt(
                updater->stage_label, "Error: %s", status ? status : "Unknown error");
            label_set_text_fmt(updater->progress_label, "Failed at %d%%", stage_pct);
        } else {
            label_set_text_fmt(updater->stage_label, "%s", status ? status : "Unknown stage");
            label_set_text_fmt(updater->progress_label, "Update progress: %d%%", stage_pct);
        }

        Canvas* canvas = updater->canvas;
        canvas_draw_begin(canvas);
        canvas_clear(canvas);
        canvas_set_fill_color(canvas, failed ? ColorRed : ColorGreen);
        canvas_set_line_color(
            canvas, failed ? ColorRed : ColorGreen); // Though likely unused for filled rects
        canvas_set_line_width(canvas, 1);

        uint8_t bar_x = 1;
        uint8_t bar_y = 7;
        uint8_t bar_h = 8;
        uint8_t bar_w = (70 * stage_pct) / 100;
        const uint8_t radius = 1;

        if(bar_w > 0) { // Only draw if width is positive
            if(bar_w < (2 * radius + 1)) { // If bar is too thin for 1px rounding (i.e., width < 3)
                // Draw a sharp-cornered rectangle
                canvas_draw_rect(canvas, bar_x, bar_y, bar_w, bar_h, true);
            } else {
                // Draw the main horizontal part of the rounded rectangle
                // (width will be bar_w - 2*radius, which is bar_w - 2)
                canvas_draw_rect(canvas, bar_x + radius, bar_y, bar_w - 2 * radius, bar_h, true);
                // Draw the main vertical part of the rounded rectangle
                // (height will be bar_h - 2*radius, which is bar_h - 2)
                canvas_draw_rect(canvas, bar_x, bar_y + radius, bar_w, bar_h - 2 * radius, true);
            }
        }

        canvas_set_fill_color(canvas, ColorWhite);
        canvas_draw_text(canvas, 16, 0, furi_string_get_cstr(progress_pct_str));

        canvas_draw_end(canvas);
    });

    furi_string_free(progress_pct_str);
}

Updater* updater_alloc(const char* arg) {
    Updater* instance = malloc(sizeof(Updater));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->startup_arg = furi_string_alloc_set(arg ? arg : "");
    instance->update_task = update_task_alloc();

    instance->gui = furi_record_open(RECORD_GUI);
    instance->event_loop = furi_event_loop_alloc();

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        instance->flex = flex_layout_alloc(
            gui_layer_get_root_widget(main_layer, GuiDisplayIdBack), FlexLayoutTypeColumn);

        Widget* flex_base = flex_layout_get_base(instance->flex);
        instance->stage_label = label_alloc(flex_base);
        label_set_text(instance->stage_label, "Updater is starting...");

        instance->progress_label = label_alloc(flex_base);
        label_set_text(instance->progress_label, "Waiting for update...");

        // Front display
        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);

        instance->canvas = canvas_alloc(root, 72, 16);
    });

    update_task_set_progress_cb(instance->update_task, updater_task_on_progress, instance);
    update_task_start(instance->update_task);

    return instance;
}

void updater_free(Updater* instance) {
    furi_check(instance);

    if(instance->update_task) {
        update_task_set_progress_cb(instance->update_task, NULL, NULL);
        update_task_free(instance->update_task);
    }

    with_gui(instance->gui, {
        flex_layout_free(instance->flex);
        label_free(instance->stage_label);
        label_free(instance->progress_label);
        canvas_free(instance->canvas);
    });

    furi_event_loop_free(instance->event_loop);
    furi_string_free(instance->startup_arg);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);
    free(instance);
}

int32_t updater_srv(void* arg) {
    Updater* updater = updater_alloc((const char*)arg);

    furi_event_loop_run(updater->event_loop);
    updater_free(updater);
    return 0;
}
