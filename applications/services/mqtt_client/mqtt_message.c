#include "mqtt_client_i.h"

static const uint8_t mqtt_property_type_table[MqttPropertyTypeMax] = {
    [MqttPropertyTypeResponseTopic] = MQTT_PROP_RESPONSE_TOPIC,
    [MqttPropertyTypeCorrelationData] = MQTT_PROP_CORRELATION_DATA,
};

const void* mqtt_message_get_data(const MqttMessage* message, size_t* data_size) {
    furi_check(message);
    const struct mg_str data = TO_RAW_MESSAGE(message)->data;

    if(data_size) {
        *data_size = data.len;
    }

    return data.buf;
}

bool mqtt_message_get_string_property(
    const MqttMessage* message,
    MqttPropertyType property_type,
    FuriString* value) {
    furi_check(message);
    furi_check(property_type < MqttPropertyTypeMax);

    bool is_found = false;

    for(size_t prop_offs = 0;;) {
        struct mg_mqtt_prop prop = {};
        const struct mg_mqtt_message* raw_message = TO_RAW_MESSAGE(message);
        // NOTE: mg_mqtt_next_prop() does NOT mutate data pointed to by *msg
        prop_offs = mg_mqtt_next_prop((struct mg_mqtt_message*)raw_message, &prop, prop_offs);

        if(prop_offs <= 0) {
            break;
        }

        if(prop.id == mqtt_property_type_table[property_type]) {
            if(value && prop.val.len) {
                furi_string_printf(value, "%.*s", prop.val.len, prop.val.buf);
                is_found = true;
                break;
            }
        }
    }

    return is_found;
}
