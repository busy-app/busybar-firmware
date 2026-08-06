#include "slot_machine_i.h"

#include <furi_hal_random.h>

const uint8_t slot_machine_win_odds[] = {2, 4, 6, 8, 12, 20, 50};
const size_t slot_machine_win_odds_count = COUNT_OF(slot_machine_win_odds);

static uint32_t slot_machine_random_bounded(uint32_t bound) {
    furi_assert(bound > 0);

    /* Rejection sampling avoids the modulo bias of random % bound. */
    const uint32_t threshold = (uint32_t)(-bound) % bound;
    uint32_t value;
    do {
        value = furi_hal_random_get();
    } while(value < threshold);

    return value % bound;
}

void slot_machine_game_init(SlotMachine* app) {
    furi_assert(app);

    app->odds_index = 2; /* Friendly default: jackpot probability 1/6. */
    for(uint8_t i = 0; i < COUNT_OF(app->visible); i++) {
        app->visible[i] = slot_machine_random_bounded(SLOT_MACHINE_SYMBOL_COUNT);
    }
}

void slot_machine_game_choose_result(SlotMachine* app) {
    furi_assert(app);
    furi_assert(app->odds_index < slot_machine_win_odds_count);

    /* Non-winning rolls are regenerated if they accidentally form a triple. */
    if(slot_machine_random_bounded(slot_machine_win_odds[app->odds_index]) == 0) {
        const uint8_t symbol = slot_machine_random_bounded(SLOT_MACHINE_SYMBOL_COUNT);
        for(uint8_t i = 0; i < COUNT_OF(app->target); i++) {
            app->target[i] = symbol;
        }
    } else {
        do {
            for(uint8_t i = 0; i < COUNT_OF(app->target); i++) {
                app->target[i] = slot_machine_random_bounded(SLOT_MACHINE_SYMBOL_COUNT);
            }
        } while(app->target[0] == app->target[1] && app->target[1] == app->target[2]);
    }
}
