#pragma once

#include <furi.h>
#include <mongoose.h>

bool mqtt_parse_topic(struct mg_str* topic, FuriString* http_req, FuriString* response_topic);
