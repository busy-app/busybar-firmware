#include <furi.h>

#include <storage/storage.h>
#include <gui/gui.h>
#include <gui/modules/image.h>
#include <gui/modules/anim_player.h>
#include <gui/modules/label.h>
#include <gui/modules/countdown.h>
#include <m-dict.h>
#include <toolbox/m_cstr_dup.h>
#include <toolbox/api_lock.h>
#include <furi_hal_rtc.h>
#include "canvas.h"
#include <gui/modules/front_display_mirror.h>

typedef struct {
    enum {
        CanvasAppEventUpdate,
        CanvasAppEventClear,
    } type;
    FuriApiLock lock;
    bool* result;
    char* app_id;
    union {
        CanvasElementsArray_t elements;
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
        AnimPlayer* anim_player;
        Label* text;
        Countdown* countdown;
    };
} CanvasWidget;

DICT_DEF2(CanvasWidgetsDict, const char*, M_CSTR_DUP_OPLIST, CanvasWidget, M_POD_OPLIST);

struct CanvasApp {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    Gui* gui;
    CanvasWidgetsDict_t widgets;
    DisplayMirror* display_mirror;
    char* app_id;
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

    char* id = ((CanvasWidgetTimeoutContext*)context)->id;

    CanvasWidget* widget = CanvasWidgetsDict_get(canvas->widgets, id);
    furi_assert(widget);

    furi_event_loop_timer_free(widget->timeout_timer);

    with_gui(canvas->gui, {
        if(widget->type == CanvasElementTypeImage) {
            furi_assert(widget->image);
            image_free(widget->image);
        } else if(widget->type == CanvasElementTypeAnimPlayer) {
            furi_assert(widget->anim_player);
            anim_player_free(widget->anim_player);
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
        } else if(widget->type == CanvasElementTypeAnimPlayer) {
            anim_player_free(widget->anim_player);
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

static void canvas_app_clear_all(CanvasApp* canvas) {
    furi_assert(canvas);

    canvas_widget_destroy_all(canvas);
    CanvasWidgetsDict_reset(canvas->widgets);

    canvas_check_back_screen_empty(canvas);

    if(CanvasWidgetsDict_empty_p(canvas->widgets)) {
        furi_event_loop_stop(canvas->event_loop);
    }
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

    } else if(widget->type == CanvasElementTypeAnimPlayer) {
        if(!widget->anim_player) {
            widget->anim_player = anim_player_alloc(root);
        }
        if(anim_player_set_source(
               widget->anim_player, furi_string_get_cstr(element->anim_player.file_path))) {
            AnimFile* file = anim_player_get_file(widget->anim_player);
            bool success = anim_file_set_section(
                file,
                element->anim_player.flags,
                furi_string_get_cstr(element->anim_player.section));
            UNUSED(success);
        }
        return anim_player_get_base(widget->anim_player);

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
            label_set_long_content_mode(
                widget->text, LabelLongContentModeScrollCircular, scroll_dur);
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

static bool canvas_element_update(CanvasApp* canvas, const CanvasElement* element) {
    CanvasWidget* widget_old = CanvasWidgetsDict_get(canvas->widgets, element->id);
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
        time_t current_stamp = furi_hal_rtc_get_timestamp();
        effective_timeout = MAX(1, element->display_until - current_stamp);
    }

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
    } else if((widget.timeout_timer) && (effective_timeout == 0)) {
        furi_event_loop_timer_free(widget.timeout_timer);
        widget.timeout_timer = NULL;
    }

    CanvasWidgetsDict_set_at(canvas->widgets, element->id, widget);

    return true;
}

static bool canvas_update_all(CanvasApp* canvas, CanvasElementsArray_t elements) {
    bool success = true;

    CanvasElementsArray_it_t it;
    for(CanvasElementsArray_it(it, elements); !CanvasElementsArray_end_p(it);
        CanvasElementsArray_next(it)) {
        const CanvasElement* item = CanvasElementsArray_cref(it);
        if(!canvas_element_update(canvas, item)) {
            success = false;
            break;
        }
    }
    canvas_check_back_screen_empty(canvas);
    return success;
}

static void canvas_app_queue_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    CanvasApp* canvas = context;
    furi_check(object == canvas->event_queue);

    CanvasAppQueueEvent event;
    furi_check(furi_message_queue_get(canvas->event_queue, &event, 0) == FuriStatusOk);

    bool success = false;

    if(event.type == CanvasAppEventUpdate) {
        if(canvas->app_id) {
            bool new_id = (strcmp(event.app_id, canvas->app_id) != 0);
            if(new_id) {
                canvas_widget_destroy_all(canvas);
                CanvasWidgetsDict_reset(canvas->widgets);
                free(canvas->app_id);
                canvas->app_id = strdup(event.app_id);
            }
        } else {
            canvas->app_id = strdup(event.app_id);
        }
        success = canvas_update_all(canvas, event.elements);
        CanvasElementsArray_clear(event.elements);

    } else if(event.type == CanvasAppEventClear) {
        if(event.app_id && canvas->app_id) {
            bool id_match = (strcmp(event.app_id, canvas->app_id) == 0);
            if(id_match) {
                canvas_app_clear_all(canvas);
                success = true;
            }
        } else {
            canvas_app_clear_all(canvas);
            success = true;
        }
    }
    if(event.app_id) {
        free(event.app_id);
        event.app_id = NULL;
    }

    *event.result = success;
    api_lock_unlock(event.lock);
}

static CanvasApp* canvas_app_alloc() {
    CanvasApp* canvas = malloc(sizeof(CanvasApp));
    canvas->event_loop = furi_event_loop_alloc();
    canvas->event_queue = furi_message_queue_alloc(8, sizeof(CanvasAppQueueEvent));
    furi_event_loop_subscribe_message_queue(
        canvas->event_loop,
        canvas->event_queue,
        FuriEventLoopEventIn,
        canvas_app_queue_event_callback,
        canvas);

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
    furi_record_close(RECORD_GUI);

    CanvasWidgetsDict_clear(canvas->widgets);

    furi_event_loop_unsubscribe(canvas->event_loop, canvas->event_queue);

    // Flush & free all pending events
    CanvasAppQueueEvent event;
    while(furi_message_queue_get(canvas->event_queue, &event, 0) == FuriStatusOk) {
        if(event.type == CanvasAppEventUpdate) {
            CanvasElementsArray_clear(event.elements);
        }
        if(event.app_id) free(event.app_id);
    }
    furi_message_queue_free(canvas->event_queue);

    furi_event_loop_free(canvas->event_loop);
    free(canvas->app_id);
    free(canvas);
}

int32_t canvas_app(void* arg) {
    UNUSED(arg);
    CanvasApp* canvas = canvas_app_alloc();
    furi_record_create(RECORD_CANVAS, canvas);

    furi_event_loop_run(canvas->event_loop);

    while(!furi_record_destroy(RECORD_CANVAS)) {
        // Wait before all users close the record
        furi_delay_ms(1);
    }

    canvas_app_free(canvas);

    return 0;
}

bool canvas_show_elements(CanvasApp* canvas, const char* app_id, CanvasElementsArray_t elements) {
    furi_assert(canvas);
    furi_assert(app_id);

    bool success = false;

    CanvasAppQueueEvent evt = {
        .lock = api_lock_alloc_locked(),
        .type = CanvasAppEventUpdate,
        .app_id = strdup(app_id),
        .result = &success,
    };

    CanvasElementsArray_init_set(evt.elements, elements);
    furi_check(furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(evt.lock);
    return success;
}

bool canvas_delete_elements(CanvasApp* canvas, const char* app_id) {
    furi_check(canvas);

    bool success = false;

    CanvasAppQueueEvent evt = {
        .lock = api_lock_alloc_locked(),
        .type = CanvasAppEventClear,
        .app_id = app_id ? strdup(app_id) : NULL,
        .result = &success,
    };
    furi_check(furi_message_queue_put(canvas->event_queue, &evt, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(evt.lock);
    return success;
}
