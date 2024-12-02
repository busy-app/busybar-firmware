#include <furi.h>
#include <gui/gui.h>
#include <furi_hal_i2c.h>
#include <input/input.h>
// #include <input/input_new.h> // TODO:

#define TAG "ALS Test"

#define ALS_I2C_ADDR    0x52
#define ALS_I2C_TIMEOUT 50
#define ALS_TIMING_VAL  0xDA // TIMING reg value, 100~150 ms
#define ALS_T_INT_US    2.8f //uS, typical, from datasheet

typedef struct {
    FuriMutex* mutex;
    float value;
} LighSensorTestState;

typedef enum {
    LighSensorTestEventExit,
    LighSensorTestEventTick,
} LighSensorTestEventType;

typedef struct {
    LighSensorTestEventType type;
} LighSensorTestEvent;

static void als_bh1730_init(FuriHalI2cBusHandle* handle) {
    furi_hal_i2c_acquire(handle);

    // Reg 0: CONTROL
    furi_hal_i2c_write_reg_8(handle, ALS_I2C_ADDR, 0x80 | 0, 0x03, ALS_I2C_TIMEOUT);
    // Reg 1: CONTROL
    furi_hal_i2c_write_reg_8(handle, ALS_I2C_ADDR, 0x80 | 1, ALS_TIMING_VAL, ALS_I2C_TIMEOUT);
    // Reg 7: GAIN (x1)
    furi_hal_i2c_write_reg_8(handle, ALS_I2C_ADDR, 0x80 | 7, 0x00, ALS_I2C_TIMEOUT);

    furi_hal_i2c_release(handle);
}

static float als_bh1730_read_lux(FuriHalI2cBusHandle* handle) {
    uint8_t data_buf[4];

    furi_hal_i2c_acquire(handle);
    furi_hal_i2c_read_mem(handle, ALS_I2C_ADDR, 0x80 | 0x14, data_buf, 4, ALS_I2C_TIMEOUT);
    furi_hal_i2c_release(handle);

    int16_t adc0_val = (data_buf[1] << 8) | data_buf[0];
    int16_t adc1_val = (data_buf[3] << 8) | data_buf[1];
    uint8_t gain = 1;

    float itime_ms = (ALS_T_INT_US * 964.f * (256.f - ALS_TIMING_VAL)) / 1000.f;

    float lux = 0.f;
    if(adc0_val != 0) {
        if(adc1_val / adc0_val < 0.26f) {
            lux = (1.290f * adc0_val - 2.733f * adc1_val) / gain * 102.6f / itime_ms;
        } else if(adc1_val / adc0_val < 0.55f) {
            lux = (0.795f * adc0_val - 0.859f * adc1_val) / gain * 102.6f / itime_ms;
        } else if(adc1_val / adc0_val < 1.09f) {
            lux = (0.510f * adc0_val - 0.345f * adc1_val) / gain * 102.6f / itime_ms;
        } else if(adc1_val / adc0_val < 2.13f) {
            lux = (0.276f * adc0_val - 0.130f * adc1_val) / gain * 102.6f / itime_ms;
        }
    }

    return lux;
}

static void als_test_render_callback(Canvas* canvas, void* ctx) {
    LighSensorTestState* state = ctx;
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "ALS test");

    canvas_set_font(canvas, FontSecondary);

    char temp_str[16];
    snprintf(temp_str, 16, "%.1f lx", (double)state->value);
    canvas_draw_str(canvas, 10, 40, temp_str);

    furi_mutex_release(state->mutex);
}

static void als_test_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    if((input_event->key == InputKeyBack) && (input_event->type == InputTypeShort)) {
        LighSensorTestEvent event = {.type = LighSensorTestEventExit};
        furi_message_queue_put(event_queue, &event, FuriWaitForever);
    }
}

static void als_test_tick_callback(void* context) {
    furi_assert(context);
    FuriMessageQueue* event_queue = context;

    LighSensorTestEvent event = {.type = LighSensorTestEventTick};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

int32_t lightsensor_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(32, sizeof(LighSensorTestEvent));
    furi_check(event_queue);

    als_bh1730_init(&furi_hal_i2c_handle_1);

    LighSensorTestState state = {0};
    state.mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, als_test_render_callback, &state);
    view_port_input_callback_set(view_port, als_test_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(als_test_tick_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, 500);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    LighSensorTestEvent event;
    while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        furi_mutex_acquire(state.mutex, FuriWaitForever);

        if(event.type == LighSensorTestEventExit) {
            furi_mutex_release(state.mutex);
            break;
        } else if(event.type == LighSensorTestEventTick) {
            state.value = als_bh1730_read_lux(&furi_hal_i2c_handle_1);
        }

        furi_mutex_release(state.mutex);
        view_port_update(view_port);
    }

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_timer_free(timer);
    furi_message_queue_free(event_queue);
    furi_mutex_free(state.mutex);

    furi_record_close(RECORD_GUI);

    return 0;
}
