#pragma once

#include "mqtt_settings_interface_v1.h"

typedef MqttSettingsV1 MqttSettings;

void mqtt_settings_init(MqttSettings* settings);

void mqtt_settings_reset(MqttSettings* settings);

void mqtt_settings_load(MqttSettings* settings);

bool mqtt_settings_save(const MqttSettings* settings);
