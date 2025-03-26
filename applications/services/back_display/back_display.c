#include <furi.h>
#include <furi_hal_resources.h>
#include <ssd1320/ssd1320.h>
#include "back_display.h"

#define BACK_DISPLAY_BUFFERS 2

typedef enum {
    BackDisplayEventDraw = 1 << 0,
    BackDisplayEventTearing = 1 << 1,
} BackDisplayEvent;

struct BackDisplaySrv {
    FuriEventLoop* event_loop;
    uint8_t data[BACK_DISPLAY_BUFFERS][BACK_DISPLAY_BUF_SIZE];

    FuriMutex* buffers_mutex;
    uint8_t* send_buffer;
    uint8_t* draw_buffer;

    bool dirty;
};

static void buffer_l8_to_l4(uint8_t* dst_l4, const uint8_t* src_l8) {
    for(uint32_t i = 0; i < BACK_DISPLAY_BUF_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst_l4[i] = (src_l8[draw_idx] >> 4) | (src_l8[draw_idx + 1] & 0xF0);
    }
}

static void back_display_swap_buffers(BackDisplaySrv* instance) {
    furi_check(instance);

    furi_mutex_acquire(instance->buffers_mutex, FuriWaitForever);
    uint8_t* tmp = instance->send_buffer;
    instance->send_buffer = instance->draw_buffer;
    instance->draw_buffer = tmp;
    furi_mutex_release(instance->buffers_mutex);
}

static void back_display_event_callback(uint32_t events, void* context) {
    BackDisplaySrv* instance = context;
    furi_check(instance);

    if(events & BackDisplayEventDraw) {
        back_display_swap_buffers(instance);
        instance->dirty = true;
    }

    if(instance->dirty && (events & BackDisplayEventTearing)) {
        ssd1320_draw(instance->draw_buffer);
        instance->dirty = false;
    }
}

static void back_display_tearing_callback(void* context) {
    BackDisplaySrv* instance = context;
    furi_check(instance);

    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventTearing);
}

static BackDisplaySrv* back_display_alloc(void) {
    BackDisplaySrv* instance = malloc(sizeof(BackDisplaySrv));
    furi_check(instance);

    instance->event_loop = furi_event_loop_alloc();

    instance->buffers_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->send_buffer = instance->data[0];
    instance->draw_buffer = instance->data[1];
    instance->dirty = false;

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, back_display_event_callback, instance);

    furi_record_create(RECORD_BACK_DISPLAY, instance);

    ssd1320_init();
    furi_hal_gpio_init_simple(&gpio_oled_fr, GpioModeInterruptRise);
    furi_hal_gpio_add_int_callback(&gpio_oled_fr, back_display_tearing_callback, instance);

    furi_thread_set_current_priority(FuriThreadPriorityHigh);

    return instance;
}

void back_display_draw(BackDisplaySrv* instance, const uint8_t* buf) {
    furi_check(instance);
    furi_check(buf);

    furi_mutex_acquire(instance->buffers_mutex, FuriWaitForever);
    buffer_l8_to_l4(instance->draw_buffer, buf);
    furi_mutex_release(instance->buffers_mutex);

    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventDraw);
}

int32_t back_display_srv(void* arg) {
    UNUSED(arg);

    BackDisplaySrv* instance = back_display_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
