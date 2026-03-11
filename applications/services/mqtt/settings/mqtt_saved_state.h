#pragma once

#include "mqtt_saved_state_interface_v1.h"

typedef MqttSavedStateV1 MqttSavedState;

bool mqtt_saved_state_reset(MqttSavedState* saved_state);
bool mqtt_saved_state_load(MqttSavedState* saved_state);
bool mqtt_saved_state_save(const MqttSavedState* saved_state);
bool mqtt_saved_state_is_valid(const MqttSavedState* saved_state);
