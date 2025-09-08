#pragma once

#include <furi.h>
#include <mongoose.h>

#define MQTT_SERVER_ADDR     "mqtts://mqtt.cloud.dev.busy.app:8883"
#define MQTT_ROOT_TOPIC      "api"
#define MQTT_SUBSCRIBE_TOPIC MQTT_ROOT_TOPIC "/#"
#define MQTT_TOPIC_PREFIX    MQTT_ROOT_TOPIC "/"
#define MQTT_RECONNECT_DELAY (2000)
#define MQTT_QOS             (2)

#define HTTP_HOST           "http://127.0.0.1"
#define HTTP_URI_API_PREFIX "/api/"

bool mqtt_parse_topic(struct mg_str* topic, FuriString* http_req, FuriString* response_topic);

void mqtt_tls_init(struct mg_connection* conn, const struct mg_tls_opts* opts);
