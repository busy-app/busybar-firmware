#include "led_display_i.h"

#include <furi.h>

#define TAG "DotMatrixSrv"

#define REFRESH_PERIOD_MS (33)

typedef enum {
    DotMatrixSrvEventMessage = 1UL << 0,
    DotMatrixSrvEventVsync = 1UL << 1,
} DotMatrixSrvEvent;

typedef enum {
    DotMatrixSrvEventFlagReady = 1UL << 0,
    DotMatrixSrvEventFlagDone = 1UL << 1,
} DotMatrixSrvEventFlag;

typedef enum {
    DotMatrixSrvMessageTypeDraw,
} DotMatrixSrvMessageType;

typedef struct {
    DotMatrixSrvMessageType type;
    union {
        const uint8_t* frame_buffer;
    };
} DotMatrixSrvMessage;

struct DotMatrixSrv {
    FuriEventLoop* event_loop;
    FuriEventFlag* event_flag;
    FuriEventLoopTimer* refresh_timer;
    LedDisplayDriver* driver;
    const DotMatrixSrvMessage* message;
};

void dot_matrix_reset(DotMatrixSrv* instance) {
    furi_check(instance);
}

static void dot_matrix_send_message(DotMatrixSrv* instance, const DotMatrixSrvMessage* message) {
    uint32_t flags;

    flags = furi_event_flag_wait(
        instance->event_flag, DotMatrixSrvEventFlagReady, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags == DotMatrixSrvEventFlagReady);

    instance->message = message;
    furi_event_loop_set_custom_event(instance->event_loop, DotMatrixSrvEventMessage);

    flags = furi_event_flag_wait(
        instance->event_flag, DotMatrixSrvEventFlagDone, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags == DotMatrixSrvEventFlagDone);
}

void dot_matrix_draw(DotMatrixSrv* instance, const uint8_t* frame_buffer) {
    furi_check(instance);
    furi_check(frame_buffer);

    const DotMatrixSrvMessage message = {
        .type = DotMatrixSrvMessageTypeDraw,
        .frame_buffer = frame_buffer,
    };

    dot_matrix_send_message(instance, &message);
}

static void led_display_vsync_callback(void* context) {
    DotMatrixSrv* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, DotMatrixSrvEventVsync);
}

static void led_display_srv_custom_event_callback(uint32_t events, void* context) {
    DotMatrixSrv* instance = context;

    if(events == DotMatrixSrvEventMessage) {
        const DotMatrixSrvMessage* message = instance->message;
        const DotMatrixSrvMessageType message_type = message->type;

        if(message_type == DotMatrixSrvMessageTypeDraw) {
            led_display_driver_send_frame(message->frame_buffer);
            furi_event_loop_timer_restart(instance->refresh_timer);
            furi_event_flag_set(instance->event_flag, DotMatrixSrvEventFlagDone);
        }

    } else if(events == DotMatrixSrvEventVsync) {
        furi_event_flag_set(instance->event_flag, DotMatrixSrvEventFlagReady);

    } else {
        furi_crash(TAG ": Multiple events");
    }
}

static void led_display_srv_refresh_timer_callback(void* context) {
    DotMatrixSrv* instance = context;

    const uint32_t flags =
        furi_event_flag_wait(instance->event_flag, DotMatrixSrvEventFlagReady, FuriFlagWaitAll, 0);

    if(flags == DotMatrixSrvEventFlagReady) {
        led_display_driver_resend_frame();
    }
}

static DotMatrixSrv* led_display_srv_alloc(void) {
    DotMatrixSrv* instance = malloc(sizeof(DotMatrixSrv));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_flag = furi_event_flag_alloc();
    instance->refresh_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        led_display_srv_refresh_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, led_display_srv_custom_event_callback, instance);
    furi_event_loop_timer_start(instance->refresh_timer, REFRESH_PERIOD_MS);

    furi_hal_gpio_init_simple(&gpio_led_power_en, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_led_power_en, true);
    furi_delay_ms(10);

    led_display_scan_init();
    led_display_driver_init();
    furi_delay_ms(1);

    led_display_scan_set_vsync_callback(led_display_vsync_callback, instance);
    led_display_scan_start();

    furi_event_flag_set(instance->event_flag, DotMatrixSrvEventFlagReady);

    furi_record_create(RECORD_DOT_MATRIX, instance);
    return instance;
}

int32_t led_display_srv(void* p) {
    UNUSED(p);

    DotMatrixSrv* instance = led_display_srv_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
