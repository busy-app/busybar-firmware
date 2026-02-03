#include "mqtt_i.h"

static const MqttPropertyDesc mqtt_property_desc_table[MqttPropertyTypeMax];

void mqtt_property_to_raw(const MqttProperty* property, struct mg_mqtt_prop* raw_property) {
    const MqttPropertyDesc* desc = &mqtt_property_desc_table[property->type];

    raw_property->id = desc->raw_id;

    if(desc->value_type == MqttPropertyValueTypeInteger) {
        raw_property->iv = property->value.integer;
    } else if(desc->value_type == MqttPropertyValueTypeString) {
        raw_property->val = mg_str(property->value.string);
    } else {
        furi_crash("Invalid MqttPropertyValueType value");
    }
}

const void* mqtt_message_get_data(const MqttMessage* message, size_t* data_size) {
    furi_check(message);
    const struct mg_str data = TO_RAW_MESSAGE(message)->data;

    if(data_size) {
        *data_size = data.len;
    }

    return data.buf;
}

static bool mqtt_message_get_raw_property(
    const struct mg_mqtt_message* raw_message,
    uint8_t raw_id,
    struct mg_mqtt_prop* out_prop) {
    bool is_found = false;

    for(size_t prop_offs = 0;;) {
        struct mg_mqtt_prop prop = {};
        // NOTE: mg_mqtt_next_prop() does NOT mutate data pointed to by *msg
        prop_offs = mg_mqtt_next_prop((struct mg_mqtt_message*)raw_message, &prop, prop_offs);

        if(prop_offs <= 0) {
            break;
        }

        if(prop.id == raw_id) {
            *out_prop = prop;
            is_found = true;
            break;
        }
    }

    return is_found;
}

static struct mg_str mqtt_message_trim_response_topic(struct mg_str response_topic) {
    struct mg_str captures[3]; // NOTE: Should be number of captures + 1
    const struct mg_str pattern =
        mg_str(MQTT_SESSION_ROOT_TOPIC "/*/" MQTT_DIRECTION_UP "/" MQTT_API_VERSION "/#");

    if(mg_match(response_topic, pattern, captures)) {
        return captures[COUNT_OF(captures) - 2];
    } else {
        FURI_LOG_E(TAG, "Malformed response topic");
        return response_topic;
    }
}

bool mqtt_message_get_string_property(
    const MqttMessage* message,
    MqttPropertyType property_type,
    FuriString* value) {
    furi_check(message);
    furi_check(value);
    furi_check(property_type < MqttPropertyTypeMax);

    bool success = false;

    const MqttPropertyDesc* desc = &mqtt_property_desc_table[property_type];
    furi_check(desc->value_type == MqttPropertyValueTypeString);

    do {
        struct mg_mqtt_prop string_prop;

        if(!mqtt_message_get_raw_property(TO_RAW_MESSAGE(message), desc->raw_id, &string_prop)) {
            break;
        }

        struct mg_str raw_val = string_prop.val;

        if(raw_val.len == 0) {
            break;
        }
        // Special case
        if(property_type == MqttPropertyTypeResponseTopic) {
            raw_val = mqtt_message_trim_response_topic(raw_val);
        }

        furi_string_printf(value, "%.*s", raw_val.len, raw_val.buf);

        success = true;
    } while(false);

    return success;
}

bool mqtt_message_get_integer_property(
    const MqttMessage* message,
    MqttPropertyType property_type,
    uint32_t* value) {
    furi_check(message);
    furi_check(value);
    furi_check(property_type < MqttPropertyTypeMax);

    const MqttPropertyDesc* desc = &mqtt_property_desc_table[property_type];
    furi_check(desc->value_type == MqttPropertyValueTypeInteger);

    struct mg_mqtt_prop integer_prop;

    const bool success =
        mqtt_message_get_raw_property(TO_RAW_MESSAGE(message), desc->raw_id, &integer_prop);

    if(success) {
        *value = integer_prop.iv;
    }

    return success;
}

// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901029
static const MqttPropertyDesc mqtt_property_desc_table[MqttPropertyTypeMax] = {
    [MqttPropertyTypeExpiryInterval] =
        {
            .raw_id = MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,
            .value_type = MqttPropertyValueTypeInteger,
        },
    [MqttPropertyTypeResponseTopic] =
        {
            .raw_id = MQTT_PROP_RESPONSE_TOPIC,
            .value_type = MqttPropertyValueTypeString,
        },
    [MqttPropertyTypeCorrelationData] =
        {
            .raw_id = MQTT_PROP_CORRELATION_DATA,
            .value_type = MqttPropertyValueTypeString,
        },
    // Add more property types as needed
};
