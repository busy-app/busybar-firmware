#include <furi.h>
#include <furi_hal_resources.h>
#include <ssd1320/ssd1320.h>
#include <light_sensor/light_sensor.h>
#include "back_display.h"

#define TAG "BackDisplaySrv"

// Contrast curve parameters
#define CONTRAST_CURVE_BASE        (1.324264735f)
#define CONTRAST_CURVE_COEFFICIENT (7.143377489f)
#define CONTRAST_MIN_VALUE         (SSD1320_CONTRAST_MIN)
#define CONTRAST_MAX_VALUE         (118)

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
} BackDisplayEvent;

struct BackDisplaySrv {
    FuriEventLoop* event_loop;
    uint8_t data[2][SSD1320_BUF_SIZE];

    FuriMutex* buffers_mutex;
    uint8_t* send_buffer;
    uint8_t* draw_buffer;

    bool dirty;

    FuriPubSub* light_sensor_events;
    uint8_t sensor_contrast, brightness_override;
};

static uint8_t back_display_sensor_level_to_contrast(uint8_t level) {
    if(level > LIGHT_SENSOR_LIGHT_LEVEL_MAX) {
        level = LIGHT_SENSOR_LIGHT_LEVEL_MAX;
    }

    float normalized_level = (float)level / LIGHT_SENSOR_LIGHT_LEVEL_MAX;

    // Apply the curve: y = CONTRAST_CURVE_COEFFICIENT * (CONTRAST_CURVE_BASE ^ (normalized_level * LIGHT_SENSOR_LIGHT_LEVEL_MAX))
    uint8_t result =
        (uint8_t)(CONTRAST_CURVE_COEFFICIENT *
                  powf(CONTRAST_CURVE_BASE, normalized_level * LIGHT_SENSOR_LIGHT_LEVEL_MAX));

    if(result < CONTRAST_MIN_VALUE) {
        result = CONTRAST_MIN_VALUE;
    } else if(result > CONTRAST_MAX_VALUE) {
        result = CONTRAST_MAX_VALUE;
    }

    return result;
}

static uint8_t back_display_brightness_to_contrast(uint8_t brightness) {
    if(brightness > BACK_DISPLAY_BRIGHTNESS_MAX) {
        brightness = BACK_DISPLAY_BRIGHTNESS_MAX;
    }
    return (brightness * (CONTRAST_MAX_VALUE - CONTRAST_MIN_VALUE)) / BACK_DISPLAY_BRIGHTNESS_MAX +
           SSD1320_CONTRAST_MIN;
}

static void back_display_update_brightness(BackDisplaySrv* instance) {
    uint8_t constrast_level = instance->sensor_contrast;
    if(instance->brightness_override != BACK_DISPLAY_BRIGHTNESS_AUTO) {
        constrast_level = back_display_brightness_to_contrast(instance->brightness_override);
    }

    ssd1320_set_contrast(constrast_level);
}

static void back_display_light_sensor_event(const void* message, void* context) {
    UNUSED(message);
    furi_assert(context);

    BackDisplaySrv* instance = context;

    const LightSensorEvent* event = message;
    if(event->type != LightSensorEventTypeLightLevelChanged) {
        return;
    }

    instance->sensor_contrast = back_display_sensor_level_to_contrast(event->light_level);
    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventLightLevelUpdate);
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

    instance->brightness_override = BACK_DISPLAY_BRIGHTNESS_AUTO;
    instance->sensor_contrast = back_display_sensor_level_to_contrast(0);

    instance->light_sensor_events = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(
        instance->light_sensor_events, back_display_light_sensor_event, instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, back_display_event_callback, instance);

    ssd1320_init();
    back_display_update_brightness(instance);

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

void back_display_set_brightness(BackDisplaySrv* instance, uint8_t brightness) {
    furi_check(instance);

    instance->brightness_override = brightness;
    furi_event_loop_set_custom_event(instance->event_loop, BackDisplayEventLightLevelUpdate);
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
