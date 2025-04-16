#include "led_display_i.h"

#include <furi.h>

#include <power/power_service/power.h>

#define TAG "DotMatrixSrv"

#define REFRESH_PERIOD_MS (100)

typedef enum {
    DotMatrixSrvEventMessage = 1UL << 0,
    DotMatrixSrvEventUpdateDone = 1UL << 1,
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
    Power* power;
    FuriEventLoop* event_loop;
    FuriSemaphore* power_ready_sem;
    FuriEventFlag* event_flag;
    const uint8_t* frame_buf_ptr;
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

static void led_display_update_done_callback(void* context) {
    DotMatrixSrv* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, DotMatrixSrvEventUpdateDone);
}

static void led_display_srv_custom_event_callback(uint32_t events, void* context) {
    DotMatrixSrv* instance = context;

    if(events == DotMatrixSrvEventMessage) {
        const DotMatrixSrvMessage* message = instance->message;
        const DotMatrixSrvMessageType message_type = message->type;

        if(message_type == DotMatrixSrvMessageTypeDraw) {
            led_display_driver_send_frame(message->frame_buffer);
            furi_event_flag_set(instance->event_flag, DotMatrixSrvEventFlagDone);
            if(instance->frame_buf_ptr == NULL) instance->frame_buf_ptr = message->frame_buffer;
        }

    } else if(events == DotMatrixSrvEventUpdateDone) {
        furi_event_flag_set(instance->event_flag, DotMatrixSrvEventFlagReady);

    } else {
        furi_crash(TAG ": Multiple events");
    }
}

static void led_display_srv_power_event(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    DotMatrixSrv* instance = context;

    if(event->type == PowerEventReady) {
        furi_semaphore_release(instance->power_ready_sem);
    }
    // TODO: React on overheat or low power budget by limiting brightness
}

static DotMatrixSrv* led_display_srv_alloc(void) {
    DotMatrixSrv* instance = malloc(sizeof(DotMatrixSrv));

    // Must be first to ensure that power subsystem is OK
    instance->power = furi_record_open(RECORD_POWER);
    if(!power_is_battery_ready(instance->power)) {
        instance->power_ready_sem = furi_semaphore_alloc(1, 0);
        furi_pubsub_subscribe(
            power_get_pubsub(instance->power), led_display_srv_power_event, instance);
        furi_check(
            furi_semaphore_acquire(instance->power_ready_sem, FuriWaitForever) == FuriStatusOk);
        furi_semaphore_free(instance->power_ready_sem);
    }
    instance->frame_buf_ptr = NULL;

    instance->event_loop = furi_event_loop_alloc();
    instance->event_flag = furi_event_flag_alloc();

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, led_display_srv_custom_event_callback, instance);

    furi_hal_gpio_init_simple(&gpio_led_power_en, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_led_power_en, true);
    furi_delay_ms(50);

    led_display_scan_init();
    led_display_driver_init();
    led_display_driver_set_update_callback(led_display_update_done_callback, instance);

    led_display_scan_start();
    led_display_driver_start();

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
