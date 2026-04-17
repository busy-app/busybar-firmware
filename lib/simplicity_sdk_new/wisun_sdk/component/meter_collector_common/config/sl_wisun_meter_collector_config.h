/***************************************************************************//**
 * @file sl_wisun_meter_collector_config.h
 * @brief Wi-SUN Meter - Collector common configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#ifndef SL_WISUN_METER_COLLECTOR_CONFIG_H
#define SL_WISUN_METER_COLLECTOR_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Wi-SUN Meter - Collector common configuration
// <s SL_WISUN_METER_COLLECTOR_TOKEN> Token string. (Not used in CoAP sample apps)
// <i> Default: "abcd"
// <i> This token must be send to meter to get measurement packet as response.
// <i> The length should be lower than SL_WISUN_METER_COLLECTOR_TOKEN_MAX_SIZE.
// <i> Whitespace characters are not allowed.
#define SL_WISUN_METER_COLLECTOR_TOKEN                                  "abcd"

// <o SL_WISUN_METER_COLLECTOR_TOKEN_MAX_SIZE> Allocated token buffer size [bytes]. (Not used in CoAP sample apps)
// <i> Default: 64
#define SL_WISUN_METER_COLLECTOR_TOKEN_MAX_SIZE                         64U

// <o SL_WISUN_METER_COLLECTOR_PORT> Collector receiver port
// <i> Default: 5683
// <i> This is the port number where Collector is listening
// <1-65536>
#define SL_WISUN_METER_COLLECTOR_PORT                                   5683U
// </h>

// <h>Wi-SUN Meter and CoAP Meter common configuration
// <o SL_WISUN_METER_MAX_REG_COLLECTOR> Maximum number of registerable Collectors
// <i> Default: 3
#define SL_WISUN_METER_MAX_REG_COLLECTOR                                3U

// <o SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE> Maximum number of stored measurements
// <i> Default: 5
#define SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE                          5U

// <o SL_WISUN_METER_SCHEDULE_TIME> Time in ms between two measurement cycles
// <i> Default: 5000
#define SL_WISUN_METER_SCHEDULE_TIME                                    5000U
// </h>

// <h>Wi-SUN CoAP Meter - CoAP Collector configuration
// <o SL_WISUN_COAP_COLLECTOR_MAX_REG_METER> Maximum number of registerable Meters
// <i> Default: 32
#define SL_WISUN_COAP_COLLECTOR_MAX_REG_METER                           32U

// <o SL_WISUN_COAP_COLLECTOR_MAX_ASYNC_METER> Maximum number of async Meters
// <i> Default: 32
#define SL_WISUN_COAP_COLLECTOR_MAX_ASYNC_METER                         32U

// <o SL_WISUN_COAP_COLLECTOR_REQUEST_TIMEOUT> Maximum waiting time [ms] for requests
// <i> Default: 360000
#define SL_WISUN_COAP_COLLECTOR_REQUEST_TIMEOUT                         360000U

// <s SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH> Meter measurement (all sensor) resource URI path
// <i> Default: "/sensor/all"
// <i> This is the shared resource URI path to get measurement results
#define SL_WISUN_COAP_METER_COLLECTOR_MEASUREMENT_URI_PATH              "/sensor/all"

// <s SL_WISUN_COAP_METER_COLLECTOR_TEMPERATURE_SENSOR_URI_PATH> Meter temperature sensor resource URI path
// <i> Default: "/sensor/temperature"
// <i> This is the shared resource URI path to get temperature measurement results
#define SL_WISUN_COAP_METER_COLLECTOR_TEMPERATURE_SENSOR_URI_PATH       "/sensor/temperature"

// <s SL_WISUN_COAP_METER_COLLECTOR_HUMIDITY_SENSOR_URI_PATH> Meter relative humidity sensor resource URI path
// <i> Default: "/sensor/humidity"
// <i> This is the shared resource URI path to get relative humidity measurement results
#define SL_WISUN_COAP_METER_COLLECTOR_HUMIDITY_SENSOR_URI_PATH          "/sensor/humidity"

// <s SL_WISUN_COAP_METER_COLLECTOR_LIGHT_SENSOR_URI_PATH> Meter light sensor resource URI path
// <i> Default: "/sensor/light"
// <i> This is the shared resource URI path to get light measurement results
#define SL_WISUN_COAP_METER_COLLECTOR_LIGHT_SENSOR_URI_PATH             "/sensor/light"

// <s SL_WISUN_COAP_METER_COLLECTOR_LED_TOGGLE_URI_PATH> Meter LEDs resource URI path
// <i> Default: "/gpio/led"
// <i> This is the shared resource URI path to toggle LEDs
#define SL_WISUN_COAP_METER_COLLECTOR_LED_TOGGLE_URI_PATH               "/gpio/led"

// <s SL_WISUN_COAP_METER_COLLECTOR_HELP_URI_PATH> Meter help URI path
// <i> Default: "/meter/help"
// <i> This is the resource URI path to display help
#define SL_WISUN_COAP_METER_COLLECTOR_HELP_URI_PATH                     "/meter/help"

// <o SL_WISUN_COAP_METER_COLLECTOR_DEFAULT_MESSAGE_ID> Default message ID for coap packets [bytes]
// <i> Default: 7
#define SL_WISUN_COAP_METER_COLLECTOR_DEFAULT_MESSAGE_ID                7U
// </h>

// <<< end of configuration section >>>

#endif // SL_WISUN_METER_COLLECTOR_CONFIG_H
