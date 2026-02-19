#include "brightness_control.h"
#include <furi/furi.h>
#include <light_sensor/light_sensor.h>

#define TAG "BrightCtrl"

#define MAX_MESSAGES 4
#define LIGHT_SENSOR_UPDATE_TIMEOUT 10

struct BrightnessControl {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    FuriPubSub* light_sensor_events;

    bool is_auto;
    uint8_t brightness;
};

typedef enum {
	MessageTypeSetBrightness,
	MessageTypeLightSensor,

    MessageTypesCount,
} MessageType;

typedef struct Message {
	MessageType type;

	uint8_t value;
} Message;

typedef void (*MessageHandler)(BrightnessControl* instance, Message* message);

static const MessageHandler message_handlers[];

static const uint8_t light_sensor_to_brightness[] = {25,25,28,31,37,43,52,61,73,85,100};

static_assert(COUNT_OF(light_sensor_to_brightness) == LIGHT_SENSOR_LIGHT_LEVEL_MAX + 1);

static void light_sensor_event(const void* message, void* context);

uint8_t brightness_control_get_brightness(BrightnessControl* instance);

void brightness_control_set_auto_brightness(BrightnessControl* instance);

void brightness_control_set_manual_brightness(BrightnessControl* instance, uint8_t brightness);

FuriState* brightness_control_get_state(BrightnessControl* instance);

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    BrightnessControl* inst = context;
    UNUSED(inst);

    Message message;
    furi_check(furi_message_queue_get(inst->message_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](inst, &message);
}

static BrightnessControl* brightness_control_alloc(void) {
	BrightnessControl* inst = malloc(sizeof(BrightnessControl));

	inst->event_loop = furi_event_loop_alloc();
    inst->message_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));
	furi_event_loop_subscribe_message_queue(
        inst->event_loop,
        inst->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        inst);

#if defined(SRV_LIGHT_SENSOR)
    inst->light_sensor_events = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(
        inst->light_sensor_events, light_sensor_event, inst);
#else
    UNUSED(light_sensor_event);
#endif

    inst->is_auto = false;
    inst->brightness = 100;

	return inst;
}

int brightness_control_srv(void* arg) {
	UNUSED(arg);

	BrightnessControl *inst = brightness_control_alloc();
    furi_record_create(RECORD_BRIGHTNESS_CONTROL, inst);
	furi_event_loop_run(inst->event_loop);

	return 0;
}

static void light_sensor_event(const void* message, void* context) {
    UNUSED(message);
    furi_assert(context);

    BrightnessControl* inst = context;

    const LightSensorEvent* event = message;
    if(event->type != LightSensorEventTypeLightLevelChanged) {
        return;
    }

    uint8_t level = MIN(LIGHT_SENSOR_LIGHT_LEVEL_MAX, event->light_level);
    uint8_t brightness = light_sensor_to_brightness[level];

    Message msg = {
    	.type = MessageTypeLightSensor,
    	.value = brightness,
    };

    furi_message_queue_put(inst->message_queue, &msg, LIGHT_SENSOR_UPDATE_TIMEOUT);
}

static void apply_brightness(BrightnessControl* inst) {
	UNUSED(inst);
}

static void do_set_brightness(BrightnessControl* inst, Message* message) {
	furi_assert(message->type == MessageTypeSetBrightness);

	inst->is_auto = false;
	inst->brightness = message->value;
	apply_brightness(inst);
}

static void do_process_light_sensor(BrightnessControl* inst, Message* message) {
	furi_assert(message->type == MessageTypeLightSensor);

	FURI_LOG_I(TAG, "Light sensor brightness: %hhu", message->value);
	if(inst->is_auto) {
		inst->brightness = message->value;
		apply_brightness(inst);
	}
}

static const MessageHandler message_handlers[] = {
	[MessageTypeSetBrightness] = do_set_brightness,
	[MessageTypeLightSensor] = do_process_light_sensor,
};
