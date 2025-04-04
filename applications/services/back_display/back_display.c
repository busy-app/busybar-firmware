#include <furi.h>
#include <furi_hal_resources.h>
#include <ssd1320/ssd1320.h>
#include <light_sensor/light_sensor.h>
#include "back_display.h"

#define TAG "BackDisplaySrv"

#define BACK_DISPLAY_CONTRAST_UPDATES_PER_SECOND (10)

// #define BACK_DISPLAY_DEBUG_ENABLE

#ifdef BACK_DISPLAY_DEBUG_ENABLE
#define BACK_DISPLAY_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define BACK_DISPLAY_DEBUG(...)
#endif

typedef enum {
    BackDisplayEventDraw = 1 << 0,
    BackDisplayEventTearing = 1 << 1,
    BackDisplayEventLightLevelUpdate = 1 << 2,
    BackDisplayEventContrastUpdate = 1 << 3,
} BackDisplayEvent;

struct BackDisplaySrv {
    FuriEventLoop* event_loop;
    uint8_t data[2][SSD1320_BUF_SIZE];

    FuriMutex* buffers_mutex;
    uint8_t* send_buffer;
    uint8_t* draw_buffer;

    bool dirty;

    FuriPubSub* light_sensor_events;

    uint8_t current_level;
    uint8_t target_level;
    FuriEventLoopTimer* contrast_timer;
};

// y = 7,143377489 * (1,324264735 ^ x), x = 1..10
static const uint8_t contrast_table[LIGHT_SENSOR_LIGHT_LEVEL_MAX] =
    {9, 13, 17, 22, 29, 39, 51, 68, 89, 118};

static void buffer_l8_to_l4(uint8_t* dst_l4, const uint8_t* src_l8) {
    for(uint32_t i = 0; i < SSD1320_BUF_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst_l4[i] = (src_l8[draw_idx] >> 4) | (src_l8[draw_idx + 1] & 0xF0);
    }
}

static int32_t get_value_step(uint32_t current, uint32_t target, uint32_t max_step) {
    if(current < target) {
        return (target - current) > max_step ? max_step : (target - current);
    } else if(current > target) {
        return (current - target) > max_step ? -max_step : (target - current);
    } else {
        return 0;
    }
}

static void back_display_update_brightness(BackDisplaySrv* instance) {
    uint8_t light_level = light_sensor_get_light_level();
    if(light_level >= LIGHT_SENSOR_LIGHT_LEVEL_MAX) {
        light_level = LIGHT_SENSOR_LIGHT_LEVEL_MAX - 1;
    }

    instance->target_level = light_level;
}

static void back_display_light_sensor_callback(const void* message, void* context) {
    UNUSED(message);
    furi_assert(context);

    BackDisplaySrv* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventLightLevelUpdate);
}

static void back_display_contrast_timer_callback(void* context) {
    furi_check(context);
    BackDisplaySrv* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventContrastUpdate);
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

    if(events & BackDisplayEventLightLevelUpdate) {
        back_display_update_brightness(instance);
    }

    if(events & BackDisplayEventContrastUpdate) {
        if(instance->current_level != instance->target_level) {
            int8_t step = get_value_step(instance->current_level, instance->target_level, 1);
            instance->current_level += step;
            uint8_t contrast = contrast_table[instance->current_level];
            ssd1320_set_contrast(contrast);
            BACK_DISPLAY_DEBUG(
                "Back display level: %d -> %d (step: %d) (contrast: %d)",
                instance->current_level,
                instance->target_level,
                step,
                contrast);
        }
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

    instance->light_sensor_events = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(
        instance->light_sensor_events, back_display_light_sensor_callback, instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, back_display_event_callback, instance);

    ssd1320_init();
    back_display_update_brightness(instance);

    furi_hal_gpio_init_simple(&gpio_oled_fr, GpioModeInterruptRise);
    furi_hal_gpio_add_int_callback(&gpio_oled_fr, back_display_tearing_callback, instance);

    furi_thread_set_current_priority(FuriThreadPriorityHigh);

    instance->contrast_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        back_display_contrast_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    furi_event_loop_timer_start(
        instance->contrast_timer, 1000 / BACK_DISPLAY_CONTRAST_UPDATES_PER_SECOND);

    furi_record_create(RECORD_BACK_DISPLAY, instance);

    return instance;
}

void back_display_draw(BackDisplaySrv* instance, const uint8_t* data) {
    furi_check(instance);
    furi_check(data);

    furi_mutex_acquire(instance->buffers_mutex, FuriWaitForever);
    buffer_l8_to_l4(instance->draw_buffer, data);
    furi_mutex_release(instance->buffers_mutex);

    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventDraw);
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
