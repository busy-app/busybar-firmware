#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/mirror_card.h>
#include <loader/loader.h>
#include <m-dict.h>
#include <toolbox/m_cstr_dup.h>
#include <toolbox/api_lock.h>
#include <furi_hal_rtc.h>
#include "canvas_i.h"
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <light_sensor/light_sensor.h>
#include <low_power/low_power.h>

#define CANVAS_DEFERRED_TIMEOUT_MS 1500U

typedef struct {
    enum {
        CanvasSrvEventUpdate,
        CanvasSrvEventClear,
        CanvasSrvEventExit,
        CanvasSrvEventGetAppId,
        CanvasSrvEventReevaluatePriority,
    } type;
    FuriApiLock lock;
    CanvasResult* result;
    char* app_id;
    size_t priority;
    size_t loader_priority;
    FuriString* string;
    CanvasDrawCallback callback;
    void* callback_ctx;
    union {
        CanvasElementsArray_t elements;
        const char* const* element_ids;
    };
} CanvasSrvQueueEvent;

DICT_DEF2(CanvasWidgetsDict, const char*, M_CSTR_DUP_OPLIST, CanvasWidget, M_POD_OPLIST);

static void canvas_screen_open(CanvasSrv* canvas);
static void canvas_screen_close(CanvasSrv* canvas);
static void canvas_loader_pubsub_callback(const void* message, void* context);

struct CanvasSrv {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    Gui* gui;
    Widget* background[GuiDisplayIdMax];
    Widget* display[GuiDisplayIdMax];
    CanvasWidgetsDict_t widgets;
    MirrorCard* display_mirror;
    Loader* loader;
    FuriPubSubSubscription* loader_subscription;
    char* app_id;
    size_t priority;
    struct {
        bool pending;
        char* app_id;
        size_t priority;
        CanvasElementsArray_t elements;
        CanvasDrawCallback callback;
        void* callback_ctx;
    } deferred;
    FuriEventLoopTimer* deferred_timer;
    LowPower* low_power;
};

static void canvas_check_back_screen_empty(CanvasSrv* canvas) {
    if(!canvas->gui) return;
    bool back_empty = true;
    CanvasWidgetsDict_it_t it;
    for(CanvasWidgetsDict_it(it, canvas->widgets); !CanvasWidgetsDict_end_p(it);
        CanvasWidgetsDict_next(it)) {
        CanvasWidgetsDict_itref_t* itref = CanvasWidgetsDict_ref(it);
        CanvasWidget* widget = &itref->value;
        if(widget->display == GuiDisplayIdBack) {
            back_empty = false;
            break;
        }
    }
    with_gui(canvas->gui, {
        widget_set_visible(mirror_card_get_base(canvas->display_mirror), back_empty);
    });
}

static void canvas_element_timeout(void* context) {
    furi_assert(context);
    CanvasSrv* canvas = ((CanvasWidgetTimeoutContext*)context)->canvas;

    char* id = ((CanvasWidgetTimeoutContext*)context)->id;

    CanvasWidget* widget = CanvasWidgetsDict_get(canvas->widgets, id);
    furi_assert(widget);

    furi_event_loop_timer_free(widget->timeout_timer);

    with_gui(canvas->gui, { canvas_widget_delete(widget); });

    CanvasWidgetsDict_erase(canvas->widgets, id);
    free(id);
    free(context);
    context = NULL;

    canvas_check_back_screen_empty(canvas);

    bool no_more_widgets = CanvasWidgetsDict_empty_p(canvas->widgets);

    if(no_more_widgets) {
        canvas_screen_close(canvas);
    }
}

static void canvas_element_destroy(CanvasSrv* canvas, CanvasWidget* widget) {
    furi_assert(canvas);
    furi_assert(widget);

    if(widget->timeout_timer) {
        furi_event_loop_timer_free(widget->timeout_timer);
    }
    if(widget->timeout_context) {
        free(widget->timeout_context->id);
        free(widget->timeout_context);
    }

    with_gui(canvas->gui, { canvas_widget_delete(widget); });
}

static CanvasResult
    canvas_element_destroy_multi(CanvasSrv* canvas, const char* const* element_ids) {
    if(element_ids) {
        const char* const* iterator = element_ids;
        const char* element_id = NULL;
        while((element_id = *(iterator++))) {
            bool exists = !!CanvasWidgetsDict_cget(canvas->widgets, element_id);
            if(!exists) return CanvasResultNonexistentElementId;
        }

        iterator = element_ids;
        element_id = NULL;
        while((element_id = *(iterator++))) {
            CanvasWidget* widget = CanvasWidgetsDict_get(canvas->widgets, element_id);
            if(!widget) continue;
            canvas_element_destroy(canvas, widget);
            CanvasWidgetsDict_erase(canvas->widgets, element_id);
        }

    } else {
        CanvasWidgetsDict_it_t it;
        for(CanvasWidgetsDict_it(it, canvas->widgets); !CanvasWidgetsDict_end_p(it);
            CanvasWidgetsDict_next(it)) {
            CanvasWidgetsDict_itref_t* itref = CanvasWidgetsDict_ref(it);
            CanvasWidget* widget = &itref->value;
            canvas_element_destroy(canvas, widget);
        }
        CanvasWidgetsDict_reset(canvas->widgets);
    }

    return CanvasResultOk;
}

static bool canvas_srv_check_elements_visible(CanvasElementsArray_t elements) {
    size_t elements_visible = 0;
    CanvasElementsArray_it_t it;
    for(CanvasElementsArray_it(it, elements); !CanvasElementsArray_end_p(it);
        CanvasElementsArray_next(it)) {
        const CanvasElement* item = CanvasElementsArray_cref(it);
        if(item->display_until > 0) {
            time_t current_stamp = furi_hal_rtc_get_timestamp();
            if(MAX(0, item->display_until - current_stamp) == 0) {
                continue;
            }
        }
        elements_visible++;
    }
    return elements_visible > 0;
}

static CanvasResult canvas_srv_clear(CanvasSrv* canvas, const char* const* element_ids) {
    furi_assert(canvas);

    CanvasResult result = CanvasResultOk;

    if((result = canvas_element_destroy_multi(canvas, element_ids)) != CanvasResultOk)
        return result;

    canvas_check_back_screen_empty(canvas);

    if(CanvasWidgetsDict_empty_p(canvas->widgets)) {
        canvas_screen_close(canvas);
    }

    return result;
}

static bool canvas_element_update(CanvasSrv* canvas, const CanvasElement* element) {
    CanvasWidget* widget_old = CanvasWidgetsDict_get(canvas->widgets, element->id);
    CanvasWidget widget = {0};
    if(widget_old) {
        if((widget_old->type != element->type) || (widget_old->display != element->display)) {
            return false;
        }
        memcpy(&widget, widget_old, sizeof(CanvasWidget));
    }

    widget.z_index = element->z_index;

    int32_t effective_timeout = -1;
    if(element->timeout > 0) {
        furi_check(element->display_until == 0);
        effective_timeout = element->timeout;
    } else if(element->display_until > 0) {
        furi_check(element->timeout == 0);
        time_t current_stamp = furi_hal_rtc_get_timestamp();
        effective_timeout = MAX(0, element->display_until - current_stamp);
    }

    if(effective_timeout == 0) {
        if(widget_old) {
            canvas_element_destroy(canvas, widget_old);
            CanvasWidgetsDict_erase(canvas->widgets, element->id);
            return true;
        }
    } else {
        with_gui(canvas->gui, {
            widget.type = element->type;
            widget.display = element->display;
            furi_assert(element->display < GuiDisplayIdMax);
            Widget* root = canvas->display[element->display];
            canvas_widget_update(&widget, root, element);
        });

        if((effective_timeout > 0) || (widget.timeout_timer)) {
            if(!widget.timeout_context) {
                widget.timeout_context = malloc(sizeof(CanvasWidgetTimeoutContext));
                widget.timeout_context->id = strdup(element->id);
                widget.timeout_context->canvas = canvas;
            }
        }

        if(effective_timeout > 0) {
            if(!widget.timeout_timer) {
                widget.timeout_timer = furi_event_loop_timer_alloc(
                    canvas->event_loop,
                    canvas_element_timeout,
                    FuriEventLoopTimerTypeOnce,
                    widget.timeout_context);
            }
            furi_event_loop_timer_start(widget.timeout_timer, effective_timeout * 1000);
        } else if((widget.timeout_timer) && (effective_timeout == -1)) {
            furi_event_loop_timer_free(widget.timeout_timer);
            widget.timeout_timer = NULL;
        }

        CanvasWidgetsDict_set_at(canvas->widgets, element->id, widget);
    }

    return true;
}

static void canvas_reorder_elements(CanvasSrv* canvas) {
    furi_assert(canvas);

    CanvasWidget* widgets[CanvasWidgetsDict_size(canvas->widgets)];

    size_t i = 0;
    CanvasWidgetsDict_it_t it;
    for(CanvasWidgetsDict_it(it, canvas->widgets); !CanvasWidgetsDict_end_p(it);
        CanvasWidgetsDict_next(it)) {
        CanvasWidgetsDict_itref_t* item = CanvasWidgetsDict_ref(it);
        widgets[i++] = &item->value;
    }

    int compare_z_index(const void* untyped_a, const void* untyped_b) {
        CanvasWidget* const* a = untyped_a;
        CanvasWidget* const* b = untyped_b;
        return (*a)->z_index - (*b)->z_index;
    }
    qsort(widgets, COUNT_OF(widgets), sizeof(widgets[0]), compare_z_index);

    with_gui(canvas->gui, {
        for(size_t i = 0; i < COUNT_OF(widgets); i++) {
            canvas_widget_to_front(widgets[i]);
        }
    });
}

static CanvasResult canvas_update_all(CanvasSrv* canvas, CanvasElementsArray_t elements) {
    CanvasResult result = CanvasResultOk;

    CanvasElementsArray_it_t it;
    for(CanvasElementsArray_it(it, elements); !CanvasElementsArray_end_p(it);
        CanvasElementsArray_next(it)) {
        const CanvasElement* item = CanvasElementsArray_cref(it);
        if(!canvas_element_update(canvas, item)) {
            result = CanvasResultBadParameters;
            break;
        }
        if(CanvasWidgetsDict_size(canvas->widgets) > CANVAS_MAX_ELEMENTS) {
            result = CanvasResultTooManyElements;
            break;
        }
    }

    canvas_reorder_elements(canvas);
    canvas_check_back_screen_empty(canvas);

    if(CanvasWidgetsDict_empty_p(canvas->widgets)) {
        canvas_screen_close(canvas);
    }
    return result;
}

static bool canvas_draw_rejected(CanvasSrv* canvas, const char* app_id, size_t priority) {
    size_t loader_prio = loader_get_priority(canvas->loader);
    size_t current_priority =
        (canvas->gui == NULL || loader_prio > canvas->priority) ? loader_prio : canvas->priority;
    bool same_app = (canvas->gui != NULL) && canvas->app_id &&
                    (strcmp(app_id, canvas->app_id) == 0);
    return (canvas->gui == NULL || same_app) ? (priority < current_priority) :
                                               (priority <= current_priority);
}

static CanvasResult canvas_srv_do_draw(
    CanvasSrv* canvas,
    const char* app_id,
    size_t priority,
    CanvasElementsArray_t elements) {
    if(canvas->gui == NULL) {
        if(!canvas_srv_check_elements_visible(elements)) return CanvasResultEmptyScreen;
        canvas_screen_open(canvas);
    }
    if(canvas->app_id) {
        if(strcmp(app_id, canvas->app_id) != 0) {
            canvas_element_destroy_multi(canvas, NULL);
            free(canvas->app_id);
            canvas->app_id = strdup(app_id);
        }
    } else {
        canvas->app_id = strdup(app_id);
    }
    canvas->priority = priority;
    return canvas_update_all(canvas, elements);
}

static void canvas_deferred_drop(CanvasSrv* canvas) {
    canvas->deferred.pending = false;
    free(canvas->deferred.app_id);
    canvas->deferred.app_id = NULL;
    CanvasElementsArray_clear(canvas->deferred.elements);
}

static void canvas_deferred_complete(CanvasSrv* canvas, CanvasResult result) {
    furi_event_loop_timer_stop(canvas->deferred_timer);
    CanvasDrawCallback callback = canvas->deferred.callback;
    void* callback_ctx = canvas->deferred.callback_ctx;
    canvas_deferred_drop(canvas);
    callback(result, callback_ctx);
}

static void canvas_deferred_try(CanvasSrv* canvas) {
    if(!canvas->deferred.pending) return;
    if(canvas_draw_rejected(canvas, canvas->deferred.app_id, canvas->deferred.priority)) return;

    CanvasResult result = canvas_srv_do_draw(
        canvas, canvas->deferred.app_id, canvas->deferred.priority, canvas->deferred.elements);
    canvas_deferred_complete(canvas, result);
}

static void canvas_deferred_timeout(void* context) {
    canvas_deferred_complete(context, CanvasResultLowPriority);
}

static void canvas_srv_queue_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    CanvasSrv* canvas = context;
    furi_check(object == canvas->event_queue);

    CanvasSrvQueueEvent event;
    furi_check(furi_message_queue_get(canvas->event_queue, &event, 0) == FuriStatusOk);

    CanvasResult res = CanvasResultOk;

    if(event.type == CanvasSrvEventUpdate) {
        bool rejected = canvas_draw_rejected(canvas, event.app_id, event.priority);

        if(rejected && event.callback) {
            if(canvas->deferred.pending) {
                canvas_deferred_complete(canvas, CanvasResultLowPriority);
            }

            canvas->deferred.pending = true;
            canvas->deferred.app_id = strdup(event.app_id);
            canvas->deferred.priority = event.priority;
            CanvasElementsArray_init_set(canvas->deferred.elements, event.elements);
            canvas->deferred.callback = event.callback;
            canvas->deferred.callback_ctx = event.callback_ctx;
            furi_event_loop_timer_start(canvas->deferred_timer, CANVAS_DEFERRED_TIMEOUT_MS);
        } else {
            if(!rejected) {
                res = canvas_srv_do_draw(canvas, event.app_id, event.priority, event.elements);
            } else {
                res = CanvasResultLowPriority;
            }

            if(event.callback) {
                event.callback(res, event.callback_ctx);
                event.callback = NULL;
            }
        }

        CanvasElementsArray_clear(event.elements);

    } else if(event.type == CanvasSrvEventClear) {
        if(canvas->gui == NULL) {
            res = event.element_ids ? CanvasResultNonexistentElementId : CanvasResultOk;
        } else if(event.app_id && canvas->app_id) {
            bool id_match = (strcmp(event.app_id, canvas->app_id) == 0);
            if(id_match) {
                res = canvas_srv_clear(canvas, event.element_ids);
            } else {
                res = CanvasResultWrongAppId;
            }
        } else {
            res = canvas_srv_clear(canvas, event.element_ids);
        }
    } else if(event.type == CanvasSrvEventExit) {
        if(canvas->gui) canvas_srv_clear(canvas, NULL);
        res = CanvasResultOk;

    } else if(event.type == CanvasSrvEventReevaluatePriority) {
        if(canvas->gui != NULL && canvas->priority < event.loader_priority) {
            canvas_srv_clear(canvas, NULL);
        }
        canvas_deferred_try(canvas);
        res = CanvasResultOk;

    } else if(event.type == CanvasSrvEventGetAppId) {
        if(canvas->app_id) {
            furi_string_set_str(event.string, canvas->app_id);
        } else {
            furi_string_reset(event.string);
        }
        res = CanvasResultOk;
    }

    if(event.app_id) {
        free(event.app_id);
        event.app_id = NULL;
    }

    if(event.result) *event.result = res;
    if(event.lock) api_lock_unlock(event.lock);
}

static bool canvas_srv_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    CanvasSrv* canvas = context;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyBack:
        case InputKeyBusy:
        case InputKeyCustom:
        case InputKeyOff:
        case InputKeyApps:
        case InputKeySettings: {
            CanvasSrvQueueEvent evt = {.type = CanvasSrvEventExit};
            // avoid blocking input thread holding gui lock
            furi_message_queue_put(canvas->event_queue, &evt, 0);
            break;
        }
        default:
            break;
        }
    }

    // Consume all input events
    return true;
}

static void canvas_screen_open(CanvasSrv* canvas) {
    canvas->gui = furi_record_open(RECORD_GUI);
    with_gui(canvas->gui, {
        GuiLayer* input_layer = gui_get_layer(canvas->gui, GuiLayerIdSystem);
        gui_layer_add_input_callback(input_layer, canvas_srv_input_callback, canvas);

        GuiLayer* draw_layer = gui_get_layer(canvas->gui, GuiLayerIdTop);
        Color background = COLOR_MAKE_HEXA(0x000000FF);
        for(GuiDisplayId i = 0; i < GuiDisplayIdMax; i++) {
            // Get a size from main layer not to cover status bar on back display
            Widget* main_layer_root =
                gui_layer_get_root_widget(gui_get_layer(canvas->gui, GuiLayerIdMain), i);
            size_t root_w = widget_get_width(main_layer_root);
            size_t root_h = widget_get_height(main_layer_root);

            Widget* root = gui_layer_get_root_widget(draw_layer, i);
            canvas->background[i] = widget_alloc(root);
            widget_set_background_color(canvas->background[i], background);
            widget_set_pos(canvas->background[i], 0, 0);
            widget_set_padding(canvas->background[i], 0, 0, 0, 0);
            widget_set_margin(canvas->background[i], 0, 0, 0, 0);
            widget_set_ignore_layout(canvas->background[i], true);
            widget_set_size(canvas->background[i], root_w, root_h);
            widget_set_max_size(canvas->background[i], root_w, root_h);

            canvas->display[i] = widget_alloc(root);
            widget_set_pos(canvas->display[i], 0, 0);
            widget_set_padding(canvas->display[i], 0, 0, 0, 0);
            widget_set_margin(canvas->display[i], 0, 0, 0, 0);
            widget_set_ignore_layout(canvas->display[i], true);

            widget_set_size(canvas->display[i], root_w, root_h);
            widget_set_max_size(canvas->display[i], root_w, root_h);
        }
        canvas->display_mirror = mirror_card_alloc(canvas->display[GuiDisplayIdBack]);
        mirror_card_set_header_text(canvas->display_mirror, "ACTIVE");
        mirror_card_set_show_footer(canvas->display_mirror, false);

        Widget* mirror_base = mirror_card_get_base(canvas->display_mirror);
        widget_set_align(mirror_base, AlignCenter);
        widget_set_margin(mirror_base, 0, 0, 2, 2);
        widget_set_visible(mirror_base, false);
    });

    low_power_lock(canvas->low_power);
}

static void canvas_screen_close(CanvasSrv* canvas) {
    if(!canvas->gui) return;
    with_gui(canvas->gui, {
        GuiLayer* input_layer = gui_get_layer(canvas->gui, GuiLayerIdSystem);
        gui_layer_remove_input_callback(input_layer, canvas_srv_input_callback);

        mirror_card_free(canvas->display_mirror);
        canvas->display_mirror = NULL;
        for(GuiDisplayId i = 0; i < GuiDisplayIdMax; i++) {
            widget_free(canvas->display[i]);
            canvas->display[i] = NULL;
            widget_free(canvas->background[i]);
            canvas->background[i] = NULL;
        }
    });
    furi_record_close(RECORD_GUI);
    canvas->gui = NULL;
    canvas->priority = 0;
    low_power_unlock(canvas->low_power);
}

static void canvas_loader_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    const LoaderEvent* event = message;
    if(event->type != LoaderEventTypePriorityChanged) return;

    CanvasSrv* canvas = context;
    CanvasSrvQueueEvent evt = {
        .type = CanvasSrvEventReevaluatePriority,
        .loader_priority = event->priority,
    };
    furi_message_queue_put(canvas->event_queue, &evt, 0);
}

static CanvasSrv* canvas_srv_alloc() {
    CanvasSrv* canvas = malloc(sizeof(CanvasSrv));
    canvas->event_loop = furi_event_loop_alloc();
    canvas->event_queue = furi_message_queue_alloc(8, sizeof(CanvasSrvQueueEvent));
    furi_event_loop_subscribe_message_queue(
        canvas->event_loop,
        canvas->event_queue,
        FuriEventLoopEventIn,
        canvas_srv_queue_event_callback,
        canvas);

    CanvasWidgetsDict_init(canvas->widgets);

    canvas->loader = furi_record_open(RECORD_LOADER);
    canvas->priority = 0;

    canvas->low_power = furi_record_open(RECORD_LOW_POWER);

    canvas->loader_subscription = furi_pubsub_subscribe(
        loader_get_pubsub(canvas->loader), canvas_loader_pubsub_callback, canvas);

    canvas->deferred.pending = false;
    canvas->deferred.app_id = NULL;
    CanvasElementsArray_init(canvas->deferred.elements);
    canvas->deferred_timer = furi_event_loop_timer_alloc(
        canvas->event_loop, canvas_deferred_timeout, FuriEventLoopTimerTypeOnce, canvas);

    return canvas;
}

int32_t canvas_service_start(void* arg) {
    UNUSED(arg);
    CanvasSrv* canvas = canvas_srv_alloc();
    furi_record_create(RECORD_CANVAS, canvas);

    furi_event_loop_run(canvas->event_loop);
    return 0;
}

CanvasResult canvas_show_elements(
    CanvasSrv* canvas,
    const char* app_id,
    size_t priority,
    CanvasElementsArray_t elements) {
    furi_assert(canvas);
    furi_assert(app_id);

    CanvasResult res = CanvasResultOk;

    CanvasSrvQueueEvent evt = {
        .lock = api_lock_alloc_locked(),
        .type = CanvasSrvEventUpdate,
        .app_id = strdup(app_id),
        .priority = priority,
        .result = &res,
    };

    CanvasElementsArray_init_set(evt.elements, elements);
    furi_check(furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(evt.lock);
    return res;
}

void canvas_show_elements_async(
    CanvasSrv* canvas,
    const char* app_id,
    size_t priority,
    CanvasElementsArray_t elements,
    CanvasDrawCallback callback,
    void* callback_ctx) {
    furi_assert(canvas);
    furi_assert(app_id);
    furi_assert(callback);

    CanvasSrvQueueEvent evt = {
        .type = CanvasSrvEventUpdate,
        .app_id = strdup(app_id),
        .priority = priority,
        .callback = callback,
        .callback_ctx = callback_ctx,
    };
    CanvasElementsArray_init_set(evt.elements, elements);
    furi_check(furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);
}

CanvasResult
    canvas_delete_elements(CanvasSrv* canvas, const char* app_id, const char* const* element_ids) {
    furi_check(canvas);

    CanvasResult res = CanvasResultOk;

    CanvasSrvQueueEvent evt = {
        .lock = api_lock_alloc_locked(),
        .type = CanvasSrvEventClear,
        .app_id = app_id ? strdup(app_id) : NULL,
        .element_ids = element_ids,
        .result = &res,
    };
    furi_check(furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(evt.lock);
    return res;
}

CanvasResult canvas_get_app_id(CanvasSrv* canvas, FuriString* string) {
    furi_check(canvas);
    furi_check(string);

    CanvasResult res = CanvasResultOk;

    CanvasSrvQueueEvent evt = {
        .lock = api_lock_alloc_locked(),
        .type = CanvasSrvEventGetAppId,
        .string = string,
        .result = &res,
    };
    furi_check(furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(evt.lock);
    return res;
}
