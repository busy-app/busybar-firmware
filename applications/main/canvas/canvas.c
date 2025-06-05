#include <furi.h>

#include <audio/audio.h>
#include <storage/storage.h>
#include <gui/gui.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>
#include <m-dict.h>
#include <toolbox/m_cstr_dup.h>
#include "canvas.h"

typedef struct {
    enum {
        CanvasAppEventExit,
        CanvasAppEventSetTimeout,
    } type;
    char* element_id;
    uint32_t timeout_value;
} CanvasAppQueueEvent;

typedef struct {
    CanvasApp* canvas;
    char* id;
} CanvasWidgetTimeoutContext;

typedef struct {
    FuriEventLoopTimer* timeout_timer;
    CanvasWidgetTimeoutContext* timeout_context;
    CanvasElementType type;
    union {
        Image* image;
        Label* text;
    };
} CanvasWidget;

DICT_DEF2(CanvasWidgetsDict, const char*, M_CSTR_DUP_OPLIST, CanvasWidget, M_POD_OPLIST);

struct CanvasApp {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriMutex* widget_list_mutex;
    Gui* gui;
    CanvasWidgetsDict_t widgets;
    FuriString* app_id;
};

static bool canvas_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    CanvasApp* canvas = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            CanvasAppQueueEvent evt = {.type = CanvasAppEventExit};
            furi_check(
                furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) ==
                FuriStatusOk);
            consumed = true;
        }
    }

    return consumed;
}

static void canvas_element_timeout(void* context) {
    furi_assert(context);
    CanvasApp* canvas = ((CanvasWidgetTimeoutContext*)context)->canvas;

    furi_mutex_acquire(canvas->widget_list_mutex, FuriWaitForever);
    char* id = ((CanvasWidgetTimeoutContext*)context)->id;

    CanvasWidget* widget = CanvasWidgetsDict_get(canvas->widgets, id);
    furi_assert(widget);

    furi_event_loop_timer_free(widget->timeout_timer);

    with_gui(canvas->gui, {
        if(widget->type == CanvasElementTypeImage) {
            furi_assert(widget->image);
            image_free(widget->image);
        } else if(widget->type == CanvasElementTypeText) {
            furi_assert(widget->text);
            label_free(widget->text);
        }
    });

    CanvasWidgetsDict_erase(canvas->widgets, id);

    free(id);
    free(context);

    bool no_more_widgets = CanvasWidgetsDict_empty_p(canvas->widgets);

    furi_mutex_release(canvas->widget_list_mutex);

    if(no_more_widgets) {
        furi_event_loop_stop(canvas->event_loop);
    }
}

static void canvas_app_queue_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    CanvasApp* canvas = context;
    furi_check(object == canvas->event_queue);

    CanvasAppQueueEvent event;
    furi_check(furi_message_queue_get(canvas->event_queue, &event, 0) == FuriStatusOk);

    if(event.type == CanvasAppEventExit) {
        furi_event_loop_stop(canvas->event_loop);
    } else if(event.type == CanvasAppEventSetTimeout) {
        furi_assert(event.element_id);
        furi_mutex_acquire(canvas->widget_list_mutex, FuriWaitForever);
        CanvasWidget* widget = CanvasWidgetsDict_get(canvas->widgets, event.element_id);
        furi_assert(widget);

        if(event.timeout_value > 0) {
            if(!widget->timeout_timer) {
                widget->timeout_timer = furi_event_loop_timer_alloc(
                    canvas->event_loop,
                    canvas_element_timeout,
                    FuriEventLoopTimerTypeOnce,
                    widget->timeout_context);
            }
            furi_event_loop_timer_start(widget->timeout_timer, event.timeout_value * 1000);
        } else if((widget->timeout_timer) && (event.timeout_value == 0)) {
            furi_event_loop_timer_free(widget->timeout_timer);
            widget->timeout_timer = NULL;
        }
        furi_mutex_release(canvas->widget_list_mutex);
    }
}

static CanvasApp* canvas_app_alloc() {
    CanvasApp* canvas = malloc(sizeof(CanvasApp));
    canvas->event_loop = furi_event_loop_alloc();
    canvas->event_queue = furi_message_queue_alloc(16, sizeof(CanvasAppQueueEvent));
    furi_event_loop_subscribe_message_queue(
        canvas->event_loop,
        canvas->event_queue,
        FuriEventLoopEventIn,
        canvas_app_queue_event_callback,
        canvas);
    canvas->widget_list_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    canvas->gui = furi_record_open(RECORD_GUI);
    CanvasWidgetsDict_init(canvas->widgets);
    canvas->app_id = furi_string_alloc();

    with_gui(canvas->gui, {
        GuiLayer* main_layer = gui_get_layer(canvas->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, canvas_app_input_callback, canvas);
    });

    return canvas;
}

static void canvas_app_free(CanvasApp* canvas) {
    with_gui(canvas->gui, {
        GuiLayer* main_layer = gui_get_layer(canvas->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, canvas_app_input_callback);

        CanvasWidgetsDict_it_t it;
        for(CanvasWidgetsDict_it(it, canvas->widgets); !CanvasWidgetsDict_end_p(it);
            CanvasWidgetsDict_next(it)) {
            const CanvasWidgetsDict_itref_t* itref = CanvasWidgetsDict_cref(it);
            const CanvasWidget* widget = &itref->value;

            if(widget->timeout_timer) {
                furi_event_loop_timer_free(widget->timeout_timer);
            }
            if(widget->timeout_context) {
                free(widget->timeout_context->id);
                free(widget->timeout_context);
            }

            if(widget->type == CanvasElementTypeImage) {
                image_free(widget->image);
            } else if(widget->type == CanvasElementTypeText) {
                label_free(widget->text);
            }
        }
    });

    furi_record_close(RECORD_GUI);

    CanvasWidgetsDict_clear(canvas->widgets);
    furi_string_free(canvas->app_id);

    furi_event_loop_unsubscribe(canvas->event_loop, canvas->event_queue);
    furi_message_queue_free(canvas->event_queue);
    furi_event_loop_free(canvas->event_loop);
    furi_mutex_free(canvas->widget_list_mutex);
    free(canvas);
}

int32_t canvas_app(void* arg) {
    UNUSED(arg);
    CanvasApp* canvas = canvas_app_alloc();
    furi_record_create(RECORD_CANVAS, canvas);
    furi_event_loop_run(canvas->event_loop);
    furi_record_destroy(RECORD_CANVAS);
    canvas_app_free(canvas);

    return 0;
}

static bool canvas_element_update(CanvasApp* canvas, const CanvasElement* element) {
    CanvasWidget* widget_old = CanvasWidgetsDict_get(canvas->widgets, element->element_id);
    CanvasWidget widget = {0};
    if(widget_old) {
        if(widget_old->type != element->type) {
            return false;
        }
        memcpy(&widget, widget_old, sizeof(CanvasWidget));
    }

    with_gui(canvas->gui, {
        widget.type = element->type;
        GuiLayer* gui_layer = gui_get_layer(canvas->gui, GuiLayerIdMain);
        Widget* root = gui_layer_get_root_widget(gui_layer, element->display);
        if(widget.type == CanvasElementTypeImage) {
            if(!widget.image) {
                widget.image = image_alloc(root);
            }
            image_set_source(widget.image, furi_string_get_cstr(element->image.file_path));
            widget_set_pos(image_get_base(widget.image), element->x, element->y);
        } else if(widget.type == CanvasElementTypeText) {
            if(!widget.text) {
                widget.text = label_alloc(root);
            }
            label_set_text(widget.text, element->text.text_str);
            widget_set_pos(label_get_base(widget.text), element->x, element->y);
        }
    });

    if((element->timeout > 0) || (widget.timeout_timer)) {
        if(!widget.timeout_context) {
            widget.timeout_context = malloc(sizeof(CanvasWidgetTimeoutContext));
            widget.timeout_context->id = strdup(element->element_id);
            widget.timeout_context->canvas = canvas;
        }
    }

    CanvasWidgetsDict_set_at(canvas->widgets, element->element_id, widget);

    if((element->timeout > 0) || (widget.timeout_timer)) {
        CanvasAppQueueEvent evt = {
            .type = CanvasAppEventSetTimeout,
            .element_id = widget.timeout_context->id,
            .timeout_value = element->timeout};
        furi_check(
            furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);
    }
    return true;
}

bool canvas_show_elements(CanvasApp* canvas, char* app_id, CanvasElementsArray_t elements) {
    furi_assert(canvas);
    furi_assert(app_id);

    UNUSED(elements);

    if(furi_string_empty(canvas->app_id)) {
        furi_string_set(canvas->app_id, app_id);
    } else if(furi_string_cmp(canvas->app_id, app_id) != 0) {
        return false;
    }

    bool success = true;
    furi_mutex_acquire(canvas->widget_list_mutex, FuriWaitForever);
    CanvasElementsArray_it_t it;
    for(CanvasElementsArray_it(it, elements); !CanvasElementsArray_end_p(it);
        CanvasElementsArray_next(it)) {
        const CanvasElement* item = CanvasElementsArray_cref(it);
        if(!canvas_element_update(canvas, item)) {
            success = false;
            break;
        }
    }
    furi_mutex_release(canvas->widget_list_mutex);

    return success;
}
