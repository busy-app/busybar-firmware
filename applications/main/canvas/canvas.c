#include <furi.h>

#include <audio/audio.h>
#include <storage/storage.h>
#include <gui/gui.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>
#include <gui/modules/countdown.h>
#include <m-dict.h>
#include <toolbox/m_cstr_dup.h>
#include <furi_hal_rtc.h>
#include "canvas.h"
#include "widgets/front_display_mirror.h"

typedef struct {
    enum {
        CanvasAppEventSetTimeout,
        CanvasAppEventClearApp,
    } type;
    union {
        struct {
            char* element_id;
            uint32_t timeout_value;
        };
        char* app_id;
    };
} CanvasAppQueueEvent;

typedef struct {
    CanvasApp* canvas;
    char* id;
} CanvasWidgetTimeoutContext;

typedef struct {
    FuriEventLoopTimer* timeout_timer;
    CanvasWidgetTimeoutContext* timeout_context;
    CanvasElementType type;
    GuiDisplayId display;
    union {
        Image* image;
        Label* text;
        Countdown* countdown;
    };
} CanvasWidget;

DICT_DEF2(CanvasWidgetsDict, const char*, M_CSTR_DUP_OPLIST, CanvasWidget, M_POD_OPLIST);

struct CanvasApp {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriMutex* widget_list_mutex;
    Gui* gui;
    CanvasWidgetsDict_t widgets;
    DisplayMirror* display_mirror;
};

static bool canvas_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    CanvasApp* canvas = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_stop(canvas->event_loop);
        }
    }

    return consumed;
}

static void canvas_check_back_screen_empty(CanvasApp* canvas) {
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
        widget_set_visible(display_mirror_get_base(canvas->display_mirror), back_empty);
    });
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
        } else if(widget->type == CanvasElementTypeCountdown) {
            furi_assert(widget->countdown);
            countdown_free(widget->countdown);
        }
    });

    CanvasWidgetsDict_erase(canvas->widgets, id);
    free(id);
    free(context);
    context = NULL;

    canvas_check_back_screen_empty(canvas);

    bool no_more_widgets = CanvasWidgetsDict_empty_p(canvas->widgets);

    furi_mutex_release(canvas->widget_list_mutex);

    if(no_more_widgets) {
        furi_event_loop_stop(canvas->event_loop);
    }
}

static void canvas_widget_destroy(CanvasApp* canvas, CanvasWidget* widget) {
    furi_assert(canvas);
    furi_assert(widget);

    if(widget->timeout_timer) {
        furi_event_loop_timer_free(widget->timeout_timer);
    }
    if(widget->timeout_context) {
        free(widget->timeout_context->id);
        free(widget->timeout_context);
    }

    with_gui(canvas->gui, {
        if(widget->type == CanvasElementTypeImage) {
            image_free(widget->image);
        } else if(widget->type == CanvasElementTypeText) {
            label_free(widget->text);
        } else if(widget->type == CanvasElementTypeCountdown) {
            countdown_free(widget->countdown);
        }
    });
}

static void canvas_widget_destroy_all(CanvasApp* canvas) {
    CanvasWidgetsDict_it_t it;
    for(CanvasWidgetsDict_it(it, canvas->widgets); !CanvasWidgetsDict_end_p(it);
        CanvasWidgetsDict_next(it)) {
        CanvasWidgetsDict_itref_t* itref = CanvasWidgetsDict_ref(it);
        CanvasWidget* widget = &itref->value;
        canvas_widget_destroy(canvas, widget);
    }
}

static void canvas_app_clear_app_id(CanvasApp* canvas, const char* app_id) {
    furi_assert(canvas);
    furi_check(furi_mutex_acquire(canvas->widget_list_mutex, FuriWaitForever) == FuriStatusOk);

    if(app_id) {
        char needle_prefix[strlen(app_id) + 2];
        strcpy(needle_prefix, app_id);
        strcat(needle_prefix, ".");

        CanvasWidgetsDict_t new_dict;
        CanvasWidgetsDict_init(new_dict);

        CanvasWidgetsDict_it_t it;
        for(CanvasWidgetsDict_it(it, canvas->widgets); !CanvasWidgetsDict_end_p(it);
            CanvasWidgetsDict_next(it)) {
            CanvasWidgetsDict_itref_t* item = CanvasWidgetsDict_ref(it);
            if(strncmp(item->key, needle_prefix, strlen(needle_prefix)) != 0) {
                CanvasWidgetsDict_set_at(new_dict, item->key, item->value);
            } else {
                canvas_widget_destroy(canvas, &item->value);
            }
        }

        CanvasWidgetsDict_move(canvas->widgets, new_dict);

    } else {
        canvas_widget_destroy_all(canvas);
        CanvasWidgetsDict_reset(canvas->widgets);
    }

    canvas_check_back_screen_empty(canvas);

    if(CanvasWidgetsDict_empty_p(canvas->widgets)) {
        furi_event_loop_stop(canvas->event_loop);
    }

    furi_check(furi_mutex_release(canvas->widget_list_mutex) == FuriStatusOk);
}

static void canvas_app_queue_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    CanvasApp* canvas = context;
    furi_check(object == canvas->event_queue);

    CanvasAppQueueEvent event;
    furi_check(furi_message_queue_get(canvas->event_queue, &event, 0) == FuriStatusOk);

    if(event.type == CanvasAppEventSetTimeout) {
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
        free(event.element_id);

    } else if(event.type == CanvasAppEventClearApp) {
        canvas_app_clear_app_id(canvas, event.app_id);
        if(event.app_id) free(event.app_id);
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

    with_gui(canvas->gui, {
        GuiLayer* main_layer = gui_get_layer(canvas->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, canvas_app_input_callback, canvas);
        Widget* back_root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        canvas->display_mirror = display_mirror_alloc(back_root);
    });

    return canvas;
}

static void canvas_app_free(CanvasApp* canvas) {
    canvas_widget_destroy_all(canvas);
    with_gui(canvas->gui, {
        GuiLayer* main_layer = gui_get_layer(canvas->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, canvas_app_input_callback);
        display_mirror_free(canvas->display_mirror);
    });

    furi_record_close(RECORD_GUI);

    CanvasWidgetsDict_clear(canvas->widgets);

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
    furi_check(furi_record_destroy(RECORD_CANVAS));
    canvas_app_free(canvas);

    return 0;
}

static Widget* canvas_element_update_specific(
    CanvasWidget* widget,
    Widget* root,
    const CanvasElement* element) {
    furi_assert(widget);
    furi_assert(root);
    furi_assert(element);

    if(widget->type == CanvasElementTypeImage) {
        if(!widget->image) {
            widget->image = image_alloc(root);
        }
        image_set_source(widget->image, furi_string_get_cstr(element->image.file_path));
        return image_get_base(widget->image);

    } else if(widget->type == CanvasElementTypeText) {
        if(!widget->text) {
            widget->text = label_alloc(root);
        }
        label_set_text(widget->text, element->text.text_str);
        label_set_font(widget->text, element->text.font);
        label_set_text_color(widget->text, element->text.color);

        Widget* base = label_get_base(widget->text);
        if(element->text.width) {
            widget_set_width(base, element->text.width);
        } else {
            widget_set_width_content(base);
        }
        if(element->text.scroll_rate_cpm) {
            uint32_t scroll_dur =
                label_calculate_scroll_duration(widget->text, element->text.scroll_rate_cpm);
            label_set_long_content_mode(widget->text, LabelLongContentModeScroll, scroll_dur);
        } else {
            label_set_long_content_mode(widget->text, LabelLongContentModeClip, 0);
        }
        return base;

    } else if(widget->type == CanvasElementTypeCountdown) {
        if(!widget->countdown) {
            widget->countdown = countdown_alloc(root);
        }
        countdown_set_text_color(widget->countdown, element->countdown.color);
        countdown_begin(
            widget->countdown,
            element->countdown.timestamp,
            element->countdown.direction,
            element->countdown.hours);
        return countdown_get_base(widget->countdown);

    } else {
        furi_crash();
    }
}

/**
 * LVGL applies `pos_x` and `pos_y` relative to the anchor point selected by
 * `Align`. We want alignment to behave like in Flipper Zero: the anchor point
 * is always relative to the top left of the screen, and the object is then
 * aligned relative to this anchor point.
 */
static void canvas_element_reanchor(Widget* root, Align align, int32_t* x, int32_t* y) {
    furi_assert(root);
    furi_assert(x);
    furi_assert(y);

    int32_t disp_width = widget_get_width(root);
    int32_t disp_height = widget_get_height(root);
    AlignBitmask align_bm = widget_align_to_bitmask(align);

    int32_t lvgl_anchor_x;
    if(align_bm & AlignBitmaskLeft) lvgl_anchor_x = 0;
    if(align_bm & AlignBitmaskHorCenter) lvgl_anchor_x = disp_width / 2;
    if(align_bm & AlignBitmaskRight) lvgl_anchor_x = disp_width;

    int32_t lvgl_anchor_y;
    if(align_bm & AlignBitmaskTop) lvgl_anchor_y = 0;
    if(align_bm & AlignBitmaskVerCenter) lvgl_anchor_y = disp_height / 2;
    if(align_bm & AlignBitmaskBottom) lvgl_anchor_y = disp_height;

    *x -= lvgl_anchor_x;
    *y -= lvgl_anchor_y;
}

/**
 * Slight vertical nudge for perceptually better aligned text at low resolution
 */
static int32_t canvas_text_nudge_y(GuiFont font, Align align) {
    AlignBitmask align_bm = widget_align_to_bitmask(align);
    if(font == GuiFontBf4x5) {
        if(align_bm & AlignBitmaskBottom) return 0;
        if(align_bm & AlignBitmaskVerCenter) return -1;
        return -2; // BitmaskTop
    } else if(font == GuiFontBf5x7 || font == GuiFontBf5x7CondensedNumerals) {
        if(align_bm & AlignBitmaskBottom) return 0;
        if(align_bm & AlignBitmaskVerCenter) return -1;
        return -2; // BitmaskTop
    } else if(font == GuiFontBf7x10) {
        if(align_bm & AlignBitmaskBottom) return 2;
        if(align_bm & AlignBitmaskVerCenter) return 0;
        return -2; // BitmaskTop
    } else {
        furi_crash();
    }
}

static int32_t canvas_element_nudge_y(const CanvasElement* element) {
    furi_assert(element);

    if(element->type == CanvasElementTypeText) {
        return canvas_text_nudge_y(element->text.font, element->align);
    }

    return 0;
}

static void
    canvas_element_update_generic(Widget* base, Widget* root, const CanvasElement* element) {
    furi_assert(base);
    furi_assert(element);

    int32_t x = element->x;
    int32_t y = element->y;
    canvas_element_reanchor(root, element->align, &x, &y);
    y += canvas_element_nudge_y(element);

    widget_set_align(base, element->align);
    widget_set_pos(base, x, y);
}

static bool
    canvas_element_update(CanvasApp* canvas, const char* app_id, const CanvasElement* element) {
    size_t complete_id_len = strlen(app_id) + 1 + strlen(element->app_scoped_id);
    char* complete_id = malloc(complete_id_len + 1);
    strcat(complete_id, app_id);
    strcat(complete_id, ".");
    strcat(complete_id, element->app_scoped_id);

    CanvasWidget* widget_old = CanvasWidgetsDict_get(canvas->widgets, complete_id);
    CanvasWidget widget = {0};
    if(widget_old) {
        if(widget_old->type != element->type) {
            return false;
        }
        memcpy(&widget, widget_old, sizeof(CanvasWidget));
    }

    with_gui(canvas->gui, {
        widget.type = element->type;
        widget.display = element->display;
        GuiLayer* gui_layer = gui_get_layer(canvas->gui, GuiLayerIdMain);
        Widget* root = gui_layer_get_root_widget(gui_layer, element->display);
        Widget* base = canvas_element_update_specific(&widget, root, element);
        canvas_element_update_generic(base, root, element);
    });

    uint32_t effective_timeout = 0;
    if(element->timeout > 0) {
        furi_check(element->display_until == 0);
        effective_timeout = element->timeout;
    } else if(element->display_until > 0) {
        furi_check(element->timeout == 0);
        time_t current_stamp = (time_t)furi_hal_rtc_get_timestamp(); // TODO: Y2038
        effective_timeout = MAX(0, element->display_until - current_stamp);
    }

    if((effective_timeout > 0) || (widget.timeout_timer)) {
        if(!widget.timeout_context) {
            widget.timeout_context = malloc(sizeof(CanvasWidgetTimeoutContext));
            widget.timeout_context->id = strdup(complete_id);
            widget.timeout_context->canvas = canvas;
        }
    }

    CanvasWidgetsDict_set_at(canvas->widgets, complete_id, widget);

    if((effective_timeout > 0) || (widget.timeout_timer)) {
        CanvasAppQueueEvent evt = {
            .type = CanvasAppEventSetTimeout,
            .element_id = strdup(complete_id),
            .timeout_value = effective_timeout,
        };
        furi_check(
            furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);
    }

    free(complete_id);

    return true;
}

bool canvas_show_elements(CanvasApp* canvas, const char* app_id, CanvasElementsArray_t elements) {
    furi_assert(canvas);
    furi_assert(app_id);

    UNUSED(elements);

    bool success = true;
    furi_mutex_acquire(canvas->widget_list_mutex, FuriWaitForever);
    CanvasElementsArray_it_t it;
    for(CanvasElementsArray_it(it, elements); !CanvasElementsArray_end_p(it);
        CanvasElementsArray_next(it)) {
        const CanvasElement* item = CanvasElementsArray_cref(it);
        if(!canvas_element_update(canvas, app_id, item)) {
            success = false;
            break;
        }
        canvas_check_back_screen_empty(canvas);
    }
    furi_mutex_release(canvas->widget_list_mutex);

    return success;
}

void canvas_delete_elements(CanvasApp* canvas, const char* app_id) {
    furi_check(canvas);

    CanvasAppQueueEvent evt = {
        .type = CanvasAppEventClearApp,
        .app_id = app_id ? strdup(app_id) : NULL,
    };
    furi_check(furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);
}
