#include <furi.h>
#include <furi_hal_pwm.h>

#define TAG "StatusLightsSrv"

#define STATUS_LIGHTS_TIMER_TICKS 16 //ms

typedef union {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct {
        uint8_t h;
        uint8_t s;
        uint8_t v;
    };
} Color;

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    Color color;
} StatusLightstSrv;

// https://stackoverflow.com/questions/24152553/hsv-to-rgb-and-back-without-floating-point-math-in-python
static void status_lights_hsv_to_rgb(const Color* hsv, Color* rgb) {
    if(hsv->s == 0) {
        rgb->r = hsv->v;
        rgb->g = hsv->v;
        rgb->b = hsv->v;
        return;
    }

    const uint8_t region = hsv->h / 43;
    const uint8_t remainder = (hsv->h % 43) * 6;

    const uint16_t s = hsv->s;
    const uint16_t v = hsv->v;

    const uint16_t p = (v * (255 - hsv->s)) >> 8;
    const uint16_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    const uint16_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch(region) {
    case 0:
        rgb->r = v;
        rgb->g = t;
        rgb->b = p;
        break;

    case 1:
        rgb->r = q;
        rgb->g = v;
        rgb->b = p;
        break;

    case 2:
        rgb->r = p;
        rgb->g = v;
        rgb->b = t;
        break;

    case 3:
        rgb->r = p;
        rgb->g = q;
        rgb->b = v;
        break;

    case 4:
        rgb->r = t;
        rgb->g = p;
        rgb->b = v;
        break;

    default:
        rgb->r = v;
        rgb->g = p;
        rgb->b = q;
        break;
    }
}

static void status_lights_timer_callback(void* context) {
    furi_assert(context);
    StatusLightstSrv* instance = context;

    Color rgb;

    status_lights_hsv_to_rgb(&instance->color, &rgb);
    furi_hal_pwm_set_rgb(rgb.r, rgb.g, rgb.b);

    instance->color.h++;
}

void status_lights_srv(void* p) {
    UNUSED(p);
    FURI_LOG_D(TAG, "Starting");

    StatusLightstSrv* instance = malloc(sizeof(StatusLightstSrv));
    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        status_lights_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    instance->color.s = 255;
    instance->color.v = 16;

    furi_hal_pwm_start();

    furi_event_loop_timer_start(instance->timer, STATUS_LIGHTS_TIMER_TICKS);

    // Start StatusLights Service
    furi_event_loop_run(instance->event_loop);
}
