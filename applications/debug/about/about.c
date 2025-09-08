#include <furi.h>

#include <version.h>

#include <gui/gui.h>
#include <gui/modules/label.h>

typedef enum {
    AboutAppCustomEventExit = 1UL << 0,
} AboutAppCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    Gui* gui;
    Widget* windows[GuiDisplayIdMax];
} AboutApp;

static bool about_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    AboutApp* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, AboutAppCustomEventExit);
            consumed = true;
        }
    }

    return consumed;
}

static void about_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    AboutApp* instance = context;

    if(events & AboutAppCustomEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

static AboutApp* about_alloc(void) {
    AboutApp* instance = malloc(sizeof(AboutApp));
    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, about_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, about_input_callback, instance);

        Widget* root;

        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            root = gui_layer_get_root_widget(main_layer, id);

            Widget* window = widget_alloc(root);
            widget_set_scrollbar_mode(window, WidgetScrollBarModeAuto);
            widget_set_size(window, widget_get_width(root), widget_get_height(root));

            Label* label = label_alloc(window);
            label_set_line_spacing(label, 2);
            label_set_text_fmt(
                label,
                "Firmware version:\n %s\n"
                "Git branch:\n %s\n"
                "Git commit:\n %s\n"
                "Build date:\n %s",
                version_get_version(NULL),
                version_get_gitbranch(NULL),
                version_get_githash(NULL),
                version_get_builddate(NULL));

            instance->windows[id] = window;
        }
    });

    return instance;
}

static void about_free(AboutApp* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, about_input_callback);

        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            widget_free(instance->windows[id]);
        }
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t about_app(void* arg) {
    UNUSED(arg);

    AboutApp* instance = about_alloc();
    furi_event_loop_run(instance->event_loop);
    about_free(instance);

    return 0;
}
