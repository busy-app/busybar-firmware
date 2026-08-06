#include "slot_machine_i.h"

static const Color black = {.r = 0, .g = 0, .b = 0};
static const Color white = {.r = 255, .g = 255, .b = 255};
static const Color green = {.r = 20, .g = 235, .b = 80};
static const Color yellow = {.r = 255, .g = 205, .b = 0};
static const Color cyan = {.r = 0, .g = 210, .b = 255};
static const Color dim = {.r = 55, .g = 60, .b = 70};

/* Lemon, cherries, coin, heart, diamond, club and spade. */
static const uint8_t symbol_masks[SLOT_MACHINE_SYMBOL_COUNT][7] = {
    {6, 15, 30, 31, 15, 6, 0},
    {3, 6, 12, 10, 21, 21, 10},
    {14, 17, 21, 21, 21, 17, 14},
    {0, 10, 31, 31, 14, 4, 0},
    {4, 14, 31, 31, 14, 4, 0},
    {4, 14, 4, 21, 31, 4, 14},
    {4, 14, 31, 31, 4, 4, 14},
};

typedef struct {
    char ch;
    uint8_t rows[5];
} SmallGlyph;

static const SmallGlyph small_glyphs[] = {
    {'0', {7, 5, 5, 5, 7}}, {'1', {2, 6, 2, 2, 7}}, {'2', {7, 1, 7, 4, 7}}, {'3', {7, 1, 7, 1, 7}},
    {'4', {5, 5, 7, 1, 1}}, {'5', {7, 4, 7, 1, 7}}, {'6', {7, 4, 7, 5, 7}}, {'7', {7, 1, 1, 1, 1}},
    {'8', {7, 5, 7, 5, 7}}, {'9', {7, 5, 7, 1, 7}}, {'A', {2, 5, 7, 5, 5}}, {'C', {3, 4, 4, 4, 3}},
    {'E', {7, 4, 6, 4, 7}}, {'H', {5, 5, 7, 5, 5}}, {'I', {7, 2, 2, 2, 7}}, {'N', {5, 7, 7, 7, 5}},
    {'W', {5, 5, 7, 7, 5}}, {'/', {1, 1, 2, 4, 4}}, {' ', {0, 0, 0, 0, 0}},
};

static const uint8_t* slot_machine_glyph_rows(char ch) {
    for(size_t i = 0; i < COUNT_OF(small_glyphs); i++) {
        if(small_glyphs[i].ch == ch) return small_glyphs[i].rows;
    }
    return small_glyphs[COUNT_OF(small_glyphs) - 1].rows;
}

static void
    slot_machine_draw_small_text(Canvas* canvas, int x, int y, const char* text, Color color) {
    while(*text) {
        const uint8_t* rows = slot_machine_glyph_rows(*text++);
        for(int row = 0; row < 5; row++) {
            for(int col = 0; col < 3; col++) {
                if(rows[row] & (1U << (2 - col))) {
                    canvas_draw_pixel(canvas, x + col, y + row, color);
                }
            }
        }
        x += 4;
    }
}

static Color slot_machine_symbol_color(uint8_t symbol) {
    static const Color colors[SLOT_MACHINE_SYMBOL_COUNT] = {
        {.r = 255, .g = 225, .b = 0},
        {.r = 255, .g = 35, .b = 48},
        {.r = 255, .g = 175, .b = 0},
        {.r = 255, .g = 35, .b = 48},
        {.r = 255, .g = 35, .b = 48},
        {.r = 235, .g = 240, .b = 255},
        {.r = 235, .g = 240, .b = 255},
    };
    return colors[symbol % SLOT_MACHINE_SYMBOL_COUNT];
}

static void slot_machine_draw_block(Canvas* canvas, int x, int y, Color color) {
    for(int py = 0; py < 2; py++) {
        for(int px = 0; px < 2; px++) {
            const int dx = x + px;
            const int dy = y + py;
            if(dx >= 0 && dx < SLOT_MACHINE_DISPLAY_WIDTH && dy >= 0 &&
               dy < SLOT_MACHINE_DISPLAY_HEIGHT) {
                canvas_draw_pixel(canvas, dx, dy, color);
            }
        }
    }
}

static void slot_machine_draw_symbol(Canvas* canvas, int x, int y, uint8_t symbol) {
    symbol %= SLOT_MACHINE_SYMBOL_COUNT;
    const Color color = slot_machine_symbol_color(symbol);
    const uint8_t* mask = symbol_masks[symbol];
    for(int row = 0; row < 7; row++) {
        for(int col = 0; col < 5; col++) {
            if(mask[row] & (1U << (4 - col))) {
                Color pixel = color;
                if(symbol == 0 && row == 0) pixel = green;
                if(symbol == 1 && row < 3) pixel = green;
                if(symbol == 2 && row == 2 && col == 2) pixel = white;
                slot_machine_draw_block(canvas, x + col * 2, y + row * 2, pixel);
            }
        }
    }
}

static bool slot_machine_reel_is_spinning(const SlotMachine* app, uint8_t reel) {
    static const uint8_t stop_at[3] = {14, 20, 26};
    return app->state == SlotSpinning && app->spin_tick < stop_at[reel];
}

static void slot_machine_view_redraw_locked(SlotMachine* app) {
    canvas_draw_begin(app->canvas);
    canvas_clear(app->canvas);
    canvas_set_fill_color(app->canvas, black);
    canvas_fill(app->canvas);

    if(app->chance_overlay_ticks && app->state != SlotSpinning) {
        char chance[8];
        snprintf(chance, sizeof(chance), "1/%u", slot_machine_win_odds[app->odds_index]);
        const char* title = "WIN CHANCE";
        const int title_x = (SLOT_MACHINE_DISPLAY_WIDTH - ((int)strlen(title) * 4 - 1)) / 2;
        const int chance_x = (SLOT_MACHINE_DISPLAY_WIDTH - ((int)strlen(chance) * 4 - 1)) / 2;
        slot_machine_draw_small_text(app->canvas, title_x, 1, title, white);
        slot_machine_draw_small_text(app->canvas, chance_x, 10, chance, yellow);
        canvas_draw_end(app->canvas);
        return;
    }

    for(uint8_t reel = 0; reel < COUNT_OF(app->visible); reel++) {
        const int box_x = 1 + reel * 24;
        const bool moving = slot_machine_reel_is_spinning(app, reel);
        const bool flash = app->state == SlotWon && (app->flash_tick / 2) % 2 == 0;
        const Color frame = moving ? cyan : flash ? yellow : white;
        canvas_set_line_color(app->canvas, frame);
        canvas_set_line_width(app->canvas, 1);
        canvas_draw_rect(app->canvas, box_x, 0, 22, 16, false);

        if(moving) {
            const int offset = app->phase[reel];
            slot_machine_draw_symbol(app->canvas, box_x + 6, 1 + offset, app->visible[reel]);
            slot_machine_draw_symbol(
                app->canvas,
                box_x + 6,
                1 + offset - 16,
                (app->visible[reel] + 1) % SLOT_MACHINE_SYMBOL_COUNT);
            for(int y = 2; y < 15; y += 4) {
                canvas_draw_pixel(app->canvas, box_x + 3, y, dim);
                canvas_draw_pixel(app->canvas, box_x + 18, y + 1, dim);
            }
        } else {
            slot_machine_draw_symbol(app->canvas, box_x + 6, 1, app->visible[reel]);
        }
    }

    canvas_draw_end(app->canvas);
}

void slot_machine_view_redraw(SlotMachine* app) {
    furi_assert(app);
    with_gui(app->gui, { slot_machine_view_redraw_locked(app); });
}

void slot_machine_view_update_back_label(SlotMachine* app) {
    furi_assert(app);
    const uint8_t odds = slot_machine_win_odds[app->odds_index];
    if(app->state == SlotSpinning) {
        snprintf(app->back_text, sizeof(app->back_text), "SLOT MACHINE\nSPINNING...\nGOOD LUCK!");
    } else if(app->state == SlotWon) {
        snprintf(
            app->back_text,
            sizeof(app->back_text),
            "JACKPOT! COINS!\nWIN CHANCE 1/%u\nWHEEL TO ADJUST",
            odds);
    } else if(app->state == SlotLost) {
        snprintf(
            app->back_text,
            sizeof(app->back_text),
            "NO LUCK - TRY AGAIN\nWIN CHANCE 1/%u\nWHEEL TO ADJUST",
            odds);
    } else {
        snprintf(
            app->back_text,
            sizeof(app->back_text),
            "OK / START TO SPIN\nWIN CHANCE 1/%u\nWHEEL TO ADJUST",
            odds);
    }
    with_gui(app->gui, { label_set_text(app->back_label, app->back_text); });
}
