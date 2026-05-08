#pragma once

#include "mqtt_config.h"

#include <cjson/cJSON.h>

bool mqtt_config_serialize_raw(const MqttConfig* config, cJSON* json);

bool mqtt_config_deserialize_raw(MqttConfig* config, const cJSON* json);
