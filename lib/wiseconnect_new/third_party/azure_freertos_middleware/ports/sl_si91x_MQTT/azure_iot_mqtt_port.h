/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_mqtt_port.h
 * @brief Defines Azure IoT MQTT port based on SL MQTT
 *
 */

#ifndef AZURE_IOT_MQTT_PORT_H
#define AZURE_IOT_MQTT_PORT_H

#include <stdio.h>

#include "sl_mqtt_client_types.h"

/* Maps SL MQTT Client Context directly to AzureIoTMQTT */
typedef sl_mqtt_client_t AzureIoTMQTT_t;

#endif /* AZURE_IOT_MQTT_PORT_H */
