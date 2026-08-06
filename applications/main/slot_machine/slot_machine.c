#include "slot_machine_i.h"

#include <storage/storage.h>

#define SLOT_SOUND_PATH(name) EXT_PATH("apps_assets/slot_machine/sounds/" name)

static void slot_machine_start_spin(SlotMachine* app) {
    furi_assert(app);
    if(app->state == SlotSpinning) return;

    slot_machine_game_choose_result(app);
    app->state = SlotSpinning;
    app->spin_tick = 0;
    app->flash_tick = 0;
    for(uint8_t i = 0; i < COUNT_OF(app->phase); i++) {
        app->phase[i] = 0;
    }

    slot_machine_view_update_back_label(app);
    audio_play_file(app->audio, SLOT_SOUND_PATH("spin.snd"));
    slot_machine_view_redraw(app);
}

static void slot_machine_finish_spin(SlotMachine* app) {
    furi_assert(app);
    const bool won = app->target[0] == app->target[1] && app->target[1] == app->target[2];
    app->state = won ? SlotWon : SlotLost;
    app->flash_tick = 0;
    slot_machine_view_update_back_label(app);
    audio_play_file(app->audio, won ? SLOT_SOUND_PATH("coins.snd") : SLOT_SOUND_PATH("lose.snd"));
}

static void slot_machine_tick(SlotMachine* app) {
    furi_assert(app);
    bool overlay_changed = false;
    if(app->chance_overlay_ticks) {
        app->chance_overlay_ticks--;
        overlay_changed = true;
    }

    if(app->state == SlotSpinning) {
        static const uint8_t stop_at[3] = {14, 20, 26};
        app->spin_tick++;
        for(uint8_t reel = 0; reel < COUNT_OF(app->visible); reel++) {
            if(app->spin_tick < stop_at[reel]) {
                app->phase[reel] += 4;
                if(app->phase[reel] >= SLOT_MACHINE_DISPLAY_HEIGHT) {
                    app->phase[reel] = 0;
                    app->visible[reel] = (app->visible[reel] + 1) % SLOT_MACHINE_SYMBOL_COUNT;
                }
            } else if(app->spin_tick == stop_at[reel]) {
                app->visible[reel] = app->target[reel];
                app->phase[reel] = 0;
            }
        }
        if(app->spin_tick >= stop_at[COUNT_OF(stop_at) - 1]) {
            slot_machine_finish_spin(app);
        }
        slot_machine_view_redraw(app);
    } else if(app->state == SlotWon && app->flash_tick < 24) {
        app->flash_tick++;
        slot_machine_view_redraw(app);
    } else if(overlay_changed) {
        slot_machine_view_redraw(app);
    }
}

static bool slot_machine_input_callback(const InputEvent* input, void* context) {
    furi_assert(input);
    furi_assert(context);
    SlotMachine* app = context;
    SlotEvent event;

    if((input->type == InputTypeShort || input->type == InputTypeLong) &&
       input->key == InputKeyBack) {
        event = SlotEventExit;
    } else if(
        input->type == InputTypeShort &&
        (input->key == InputKeyOk || input->key == InputKeyStart)) {
        event = SlotEventSpin;
    } else if(input->type == InputTypeShort && input->key == InputKeyUp) {
        event = SlotEventChanceUp;
    } else if(input->type == InputTypeShort && input->key == InputKeyDown) {
        event = SlotEventChanceDown;
    } else {
        return false;
    }

    if(furi_message_queue_put(app->queue, &event, 0) != FuriStatusOk) {
        FURI_LOG_E(SLOT_MACHINE_TAG, "Failed to queue input event");
    }
    return true;
}

static void slot_machine_timer_callback(void* context) {
    furi_assert(context);
    SlotMachine* app = context;
    const SlotEvent event = SlotEventTick;
    /* A dropped animation tick is harmless and will be recovered by the next one. */
    furi_message_queue_put(app->queue, &event, 0);
}

static void slot_machine_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    SlotMachine* app = context;
    furi_check(object == app->queue);

    SlotEvent event;
    while(furi_message_queue_get(app->queue, &event, 0) == FuriStatusOk) {
        if(event == SlotEventExit) {
            audio_stop(app->audio);
            desktop_replace_current_app(app->desktop, "apps_menu", "slot_machine");
        } else if(event == SlotEventSpin) {
            slot_machine_start_spin(app);
        } else if(event == SlotEventChanceUp) {
            if(app->state != SlotSpinning) {
                if(app->odds_index > 0) app->odds_index--;
                app->chance_overlay_ticks = 27;
                slot_machine_view_update_back_label(app);
                slot_machine_view_redraw(app);
            }
        } else if(event == SlotEventChanceDown) {
            if(app->state != SlotSpinning) {
                if(app->odds_index < slot_machine_win_odds_count - 1) app->odds_index++;
                app->chance_overlay_ticks = 27;
                slot_machine_view_update_back_label(app);
                slot_machine_view_redraw(app);
            }
        } else if(event == SlotEventTick) {
            slot_machine_tick(app);
        }
    }
}

static bool slot_machine_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);
    furi_assert(context);
    SlotMachine* app = context;
    if(signal == FuriSignalExit) {
        audio_stop(app->audio);
        furi_event_loop_stop(app->loop);
        return true;
    }
    return signal == FuriSignalAboutToExit;
}

static SlotMachine* slot_machine_alloc(void) {
    SlotMachine* app = calloc(1, sizeof(*app));
    furi_check(app);
    slot_machine_game_init(app);

    app->loop = furi_event_loop_alloc();
    app->queue = furi_message_queue_alloc(16, sizeof(SlotEvent));
    furi_event_loop_subscribe_message_queue(
        app->loop, app->queue, FuriEventLoopEventIn, slot_machine_queue_callback, app);
    app->timer = furi_event_loop_timer_alloc(
        app->loop, slot_machine_timer_callback, FuriEventLoopTimerTypePeriodic, app);
    furi_thread_set_signal_callback(furi_thread_get_current(), slot_machine_signal_callback, app);

    app->gui = furi_record_open(RECORD_GUI);
    app->desktop = furi_record_open(RECORD_DESKTOP);
    app->audio = furi_record_open(RECORD_AUDIO);
    desktop_pin_current_app(app->desktop, false);
    audio_enable(app->audio);

    with_gui(app->gui, {
        GuiLayer* main_layer = gui_get_layer(app->gui, GuiLayerIdMain);
        Widget* front = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        app->canvas = canvas_alloc(front, SLOT_MACHINE_DISPLAY_WIDTH, SLOT_MACHINE_DISPLAY_HEIGHT);
        Widget* back = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        app->back_label = label_alloc(back);
        widget_set_align(label_get_base(app->back_label), AlignCenter);
        GuiLayer* input_layer = gui_get_layer(app->gui, GuiLayerIdSystem);
        gui_layer_add_input_callback(input_layer, slot_machine_input_callback, app);
    });

    slot_machine_view_update_back_label(app);
    slot_machine_view_redraw(app);
    furi_event_loop_timer_start(app->timer, furi_ms_to_ticks(SLOT_MACHINE_TICK_MS));
    return app;
}

static void slot_machine_free(SlotMachine* app) {
    furi_assert(app);
    audio_stop(app->audio);
    audio_disable(app->audio);
    with_gui(app->gui, {
        GuiLayer* input_layer = gui_get_layer(app->gui, GuiLayerIdSystem);
        gui_layer_remove_input_callback(input_layer, slot_machine_input_callback);
        canvas_free(app->canvas);
        label_free(app->back_label);
    });
    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);
    furi_event_loop_unsubscribe(app->loop, app->queue);
    furi_event_loop_timer_free(app->timer);
    furi_message_queue_free(app->queue);
    furi_event_loop_free(app->loop);
    furi_thread_set_signal_callback(furi_thread_get_current(), NULL, NULL);
    free(app);
}

int32_t slot_machine_entry(void* argument) {
    UNUSED(argument);
    SlotMachine* app = slot_machine_alloc();
    furi_event_loop_run(app->loop);
    slot_machine_free(app);
    return 0;
}
