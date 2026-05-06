#include "mqtt_profile.h"

#include <cjson/cJSON.h>

#define MQTT_URL_PREFIX     "mqtt://"
#define MQTT_URL_TLS_PREFIX "mqtts://"

bool mqtt_profile_serialize_raw(const MqttProfile* profile, cJSON* json);

bool mqtt_profile_deserialize_raw(MqttProfile* profile, const cJSON* json);
