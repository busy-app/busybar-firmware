#include <furi.h>
#include <furi_hal_resources.h>
#include <ssd1320/ssd1320.h>
#include "back_display.h"

#define TAG "BackDisplaySrv"

#define CONTRAST_DEFAULT 25

// #define BACK_DISPLAY_DEBUG_ENABLE

#ifdef BACK_DISPLAY_DEBUG_ENABLE
#define BACK_DISPLAY_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define BACK_DISPLAY_DEBUG(...)
#endif

typedef enum {
    BackDisplayEventDraw = 1 << 0,
    BackDisplayEventTearing = 1 << 1,
    BackDisplayEventUpdateContrast = 1 << 2,
    BackDisplayEventUpdateGamma = 1 << 3,
    BackDisplayEventUpdateSleep = 1 << 4,
} BackDisplayEvent;

struct BackDisplaySrv {
    FuriEventLoop* event_loop;
    uint8_t data[2][SSD1320_BUF_SIZE];

    FuriMutex* buffers_mutex;
    uint8_t* send_buffer;
    uint8_t* draw_buffer;

    bool dirty;

    FuriMutex* property_mutex;
    BackDisplayGammaTable gamma_table;
    uint8_t sensor_contrast;
    size_t sleep_holders;
};

static_assert(
    sizeof(BackDisplayGammaTable) == sizeof(SSD1320GrayscaleTable),
    "BackDisplayGammaTable and SSD1320GrayscaleTable should be the same size");

static void back_display_update_contrast(BackDisplaySrv* instance) {
    ssd1320_set_contrast(instance->sensor_contrast);
}

static void back_display_update_gamma(BackDisplaySrv* instance) {
    ssd1320_set_grayscale_table((SSD1320GrayscaleTable*)&instance->gamma_table);
}

static void back_display_event_callback(uint32_t events, void* context) {
    BackDisplaySrv* instance = context;
    furi_check(instance);

    if(events & BackDisplayEventDraw) {
        // swap buffers
        furi_mutex_acquire(instance->buffers_mutex, FuriWaitForever);
        uint8_t* tmp = instance->send_buffer;
        instance->send_buffer = instance->draw_buffer;
        instance->draw_buffer = tmp;
        furi_mutex_release(instance->buffers_mutex);

        // mark as dirty, to be drawn on next tearing event
        instance->dirty = true;
    }

    // tearing event
    if(events & BackDisplayEventTearing) {
        // draw the screen, if needed
        if(instance->dirty) {
            ssd1320_draw(instance->send_buffer);
            instance->dirty = false;
        }
    }

    furi_check(furi_mutex_acquire(instance->property_mutex, FuriWaitForever) == FuriStatusOk);

    if(events & BackDisplayEventUpdateContrast) {
        back_display_update_contrast(instance);
    }

    if(events & BackDisplayEventUpdateGamma) {
        back_display_update_gamma(instance);
    }

    if(events & BackDisplayEventUpdateSleep) {
        ssd1320_sleep_mode(instance->sleep_holders > 0);
    }

    furi_check(furi_mutex_release(instance->property_mutex) == FuriStatusOk);
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

    instance->property_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->sensor_contrast = CONTRAST_DEFAULT;

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, back_display_event_callback, instance);

    ssd1320_init();
    back_display_update_contrast(instance);

    furi_hal_gpio_init_simple(&gpio_back_display_fr, GpioModeInterruptRise);
    furi_hal_gpio_add_int_callback(&gpio_back_display_fr, back_display_tearing_callback, instance);

    furi_thread_set_current_priority(FuriThreadPriorityHigh);

    furi_record_create(RECORD_BACK_DISPLAY, instance);

    return instance;
}

static void buffer_l8_to_l4(uint8_t* dst_l4, const uint8_t* src_l8) {
    for(uint32_t i = 0; i < SSD1320_BUF_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst_l4[i] = (src_l8[draw_idx] >> 4) | (src_l8[draw_idx + 1] & 0xF0);
    }
}

void back_display_draw(BackDisplaySrv* instance, const uint8_t* data) {
    furi_check(instance);
    furi_check(data);

    furi_mutex_acquire(instance->buffers_mutex, FuriWaitForever);
    buffer_l8_to_l4(instance->draw_buffer, data);
    furi_mutex_release(instance->buffers_mutex);

    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventDraw);
}

void back_display_sleep_mode(BackDisplaySrv* instance, bool sleep) {
    furi_check(instance);

    furi_check(furi_mutex_acquire(instance->property_mutex, FuriWaitForever) == FuriStatusOk);
    if(sleep) {
        instance->sleep_holders++;
    } else {
        instance->sleep_holders--;
    }
    furi_check(furi_mutex_release(instance->property_mutex) == FuriStatusOk);

    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventUpdateSleep);
}

void back_display_set_contrast(BackDisplaySrv* instance, BackDisplayContrast contrast) {
    furi_check(instance);

    furi_check(furi_mutex_acquire(instance->property_mutex, FuriWaitForever) == FuriStatusOk);
    instance->sensor_contrast = contrast.val;
    furi_check(furi_mutex_release(instance->property_mutex) == FuriStatusOk);

    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventUpdateContrast);
}

void back_display_set_gamma_table(
    BackDisplaySrv* instance,
    const BackDisplayGammaTable* gamma_table) {
    furi_check(instance);
    furi_check(gamma_table);

    furi_check(furi_mutex_acquire(instance->property_mutex, FuriWaitForever) == FuriStatusOk);
    instance->gamma_table = *gamma_table;
    furi_check(furi_mutex_release(instance->property_mutex) == FuriStatusOk);

    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventUpdateGamma);
}

size_t back_display_get_width(void) {
    return SSD1320_W;
}

size_t back_display_get_height(void) {
    return SSD1320_H;
}

int32_t back_display_srv(void* arg) {
    UNUSED(arg);

    BackDisplaySrv* instance = back_display_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
