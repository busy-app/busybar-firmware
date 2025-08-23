#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/label.h>
#include <furi_hal_rtc.h>

#define TAG                      "Clock"
#define CLOCK_INTERVAL_UPDATE_MS (500) // 500 milliseconds

typedef enum {
    ClockCustomEventExit = 1UL << 0,
} ClockCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    Gui* gui;
    Label* labels[GuiDisplayIdMax];
} Clock;

static bool clock_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Clock* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, ClockCustomEventExit);
            consumed = true;
        }
    }

    return consumed;
}

static void clock_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Clock* instance = context;

    if(events & ClockCustomEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

char* clock_get_time_string(Clock* instance) {
    furi_assert(instance);
    UNUSED(instance);
    DateTime date_time;
    furi_hal_rtc_get_datetime(&date_time);

    static char time_string[32];
    snprintf(
        time_string,
        sizeof(time_string),
        "   %02d:%02d:%02d\n%02d-%02d-%04d",
        date_time.hour,
        date_time.minute,
        date_time.second,
        date_time.day,
        date_time.month,
        date_time.year);
    return time_string;
}

static void clock_timer_callback(void* context) {
    Clock* instance = context;

    // Update the labels with the current time
    for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
        Label* label = instance->labels[id];
        if(label) {
            label_set_text(label, clock_get_time_string(instance));
        }
    }
}

static Clock* clock_alloc(void) {
    Clock* instance = malloc(sizeof(Clock));
    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, clock_timer_callback, FuriEventLoopTimerTypePeriodic, instance);

    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, clock_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, clock_input_callback, instance);

        Widget* root;

        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            root = gui_layer_get_root_widget(main_layer, id);

            Label* label = label_alloc(root);
            label_set_text(label, clock_get_time_string(instance));
            widget_set_align(label_get_base(label), AlignCenter);

            instance->labels[id] = label;
        }

        furi_event_loop_timer_start(instance->timer, CLOCK_INTERVAL_UPDATE_MS);
    });

    return instance;
}

static void clock_free(Clock* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, clock_input_callback);
        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            label_free(instance->labels[id]);
        }
    });

    furi_record_close(RECORD_GUI);
    furi_event_loop_timer_stop(instance->timer);
    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t clock_app(void* p) {
    UNUSED(p);
    Clock* instance = clock_alloc();
    furi_event_loop_run(instance->event_loop);
    clock_free(instance);

    return 0;
}
