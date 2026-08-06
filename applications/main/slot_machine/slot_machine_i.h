#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/canvas.h>
#include <gui/modules/label.h>
#include <desktop/desktop.h>
#include <audio/audio.h>

#define SLOT_MACHINE_TAG "SlotMachine"

#define SLOT_MACHINE_DISPLAY_WIDTH  72
#define SLOT_MACHINE_DISPLAY_HEIGHT 16
#define SLOT_MACHINE_SYMBOL_COUNT   7
#define SLOT_MACHINE_TICK_MS        75

typedef enum {
    SlotEventTick,
    SlotEventSpin,
    SlotEventChanceUp,
    SlotEventChanceDown,
    SlotEventExit,
} SlotEvent;

typedef enum {
    SlotIdle,
    SlotSpinning,
    SlotWon,
    SlotLost,
} SlotState;

typedef struct {
    FuriEventLoop* loop;
    FuriMessageQueue* queue;
    FuriEventLoopTimer* timer;
    Gui* gui;
    Desktop* desktop;
    Audio* audio;
    Canvas* canvas;
    Label* back_label;
    SlotState state;
    uint8_t visible[3];
    uint8_t target[3];
    uint8_t phase[3];
    uint8_t spin_tick;
    uint8_t flash_tick;
    uint8_t odds_index;
    uint8_t chance_overlay_ticks;
    char back_text[96];
} SlotMachine;

extern const uint8_t slot_machine_win_odds[];
extern const size_t slot_machine_win_odds_count;

void slot_machine_game_init(SlotMachine* app);
void slot_machine_game_choose_result(SlotMachine* app);

void slot_machine_view_redraw(SlotMachine* app);
void slot_machine_view_update_back_label(SlotMachine* app);
