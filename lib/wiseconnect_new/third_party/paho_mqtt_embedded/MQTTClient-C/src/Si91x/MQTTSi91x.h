/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#ifndef MQTTSI91X_H
#define MQTTSI91X_H

#include <stdint.h>
#include "sl_utility.h"
#include "cmsis_os2.h"

#define MQTT_WITH_WEBSOCKET_HEADER_LEN         3
#define WAIT_TIMEOOUT                          5000
#define SLI_917_WAIT_FOREVER                   0
#define WIFI_BLOCKED_TIMEOUT_SF                1
#define DEFAULT_TIMEOUT                        SLI_TX_EVENT_WAIT_TIME
#define SLI_TX_EVENT_WAIT_TIME                 (30000 + TX_WAIT_TIME)
#define TX_WAIT_TIME                           0
#define SLI_SOCKET_RECVFROM_RESPONSE_WAIT_TIME ((SLI_917_WAIT_FOREVER * WIFI_BLOCKED_TIMEOUT_SF) + (DEFAULT_TIMEOUT))
#define SINGLE_PKT_TCP_STREAM_TIMEOUT          WAIT_TIMEOOUT + SLI_SOCKET_RECVFROM_RESPONSE_WAIT_TIME
#define NETWORK_ERROR_NULL_STRUCTURE           -1 // Error: NULL network structure
#define NETWORK_ERROR_NULL_ADDRESS             -2 // Error: NULL address
#define NETWORK_ERROR_INVALID_TYPE             -3 // Error: Invalid transport type

// MQTT TLS Configuration
#ifndef MQTT_TLS_AVAILABLE
#define MQTT_TLS_AVAILABLE 1 // Set to 1 to enable TLS support, 0 to disable
#endif

typedef struct Timer Timer;
struct Timer {
  uint32_t systick_period;
  uint32_t end_time;
};

typedef enum { MQTT_TRANSPORT_TCP, MQTT_TRANSPORT_WEBSOCKET } mqtt_transport_t;

/**
 * @brief Represents the network abstraction layer for MQTT communication.
 *
 * @details
 * The `Network` structure provides an abstraction for network operations required by the MQTT client.
 * It supports both TCP and WebSocket transport types and includes function pointers for transport-specific
 * operations such as reading, writing, and disconnecting.
 *
 * Fields:
 * - `socket`: The socket descriptor used for the network connection.
 *   - For TCP, this represents the file descriptor of the TCP socket.
 *   - For WebSocket, this represents the WebSocket connection handle.
 *
 * - `mqttread`: Function pointer for reading data from the network.
 *
 * - `mqttwrite`: Function pointer for writing data to the network.
 *
 * - `disconnect`: Function pointer for disconnecting the network connection.
 *
 * - `transport_type`: Enum indicating the transport type used for the connection.
 *   - Possible values:
 *     - `MQTT_TRANSPORT_TCP`: Indicates that the connection uses TCP transport.
 *     - `MQTT_TRANSPORT_WEBSOCKET`: Indicates that the connection uses WebSocket transport.
 *
 * @note
 * - The `Network` structure must be initialized using the `NetworkInit` function before use.
 * - The transport-specific function pointers (`mqttread`, `mqttwrite`, and `disconnect`) are assigned
 *   during initialization based on the `transport_type`.
 */

typedef struct Network Network;
struct Network {
  int socket;
  int (*mqttread)(Network *, unsigned char *, int, int);
  int (*mqttwrite)(Network *, unsigned char *, int, int);
  void (*disconnect)(Network *);
  mqtt_transport_t transport_type;
};

uint32_t osKernelGetTickCount(void);
void InitTimer(Timer *);
char expired(Timer *);
void countdown_ms_mqtt(Timer *, unsigned int);
void countdown(Timer *, unsigned int);
int left_ms_mqtt(Timer *);

/**
 * @brief Initializes the network structure and sets up transport-specific callbacks.
 *
 * @param[in] n Pointer to the Network structure to be initialized.
 *
 * @details
 * - This function initializes the `Network` structure by setting the `socket` to `-1`.
 * - It assigns transport-specific function pointers (`mqttread`, `mqttwrite`, and `disconnect`) 
 *   based on the `transport_type` field in the `Network` structure.
 * - Supported transport types are:
 *   - `MQTT_TRANSPORT_TCP`: Initializes for TCP transport.
 *   - `MQTT_TRANSPORT_WEBSOCKET`: Initializes for WebSocket transport.
 *
 * @note
 * - Ensure that the `Network` structure is allocated and valid before calling this function.
 * - If the `transport_type` is invalid, the function does nothing.
 */
void NetworkInit(Network *n);

/**
 * @brief Establishes a connection to the MQTT broker using the specified transport type.
 *
 * @param[in] n         Pointer to the initialized Network structure.
 * @param[in] flags     Connection flags (reserved for future use).
 * @param[in] addr      Pointer to the broker's IP address (IPv4 or IPv6).
 * @param[in] dst_port  Destination port of the MQTT broker.
 * @param[in] src_port  Source port for the connection.
 * @param[in] ssl       Boolean flag to enable/disable SSL for the connection.
 *
 * @return
 * - `0` on success.
 * - `NETWORK_ERROR_NULL_STRUCTURE (-1)` if the `Network` structure is `NULL`.
 * - `NETWORK_ERROR_NULL_ADDRESS (-2)` if the `addr` is `NULL`.
 * - `NETWORK_ERROR_INVALID_TYPE (-3)` if the `transport_type` is invalid.
 * - Transport-specific error codes for connection failures.
 *
 * @details
 * - This function establishes a connection to the MQTT broker using either TCP or WebSocket transport.
 * - It delegates the connection logic to transport-specific handlers:
 *   - `mqtt_tcpconnection_handler` for TCP.
 *   - `mqtt_websocketconnection_handler` for WebSocket.
 * - SSL can be enabled for secure connections by setting the `ssl` parameter to `true`.
 *
 * @note
 * - Ensure that the `Network` structure is initialized using `NetworkInit` before calling this function.
 * - The `addr` parameter must point to a valid IP address string.
 */
int NetworkConnect(Network *n, uint8_t flags, char *addr, int dst_port, int src_port, bool ssl);

/**
 * @brief Cleans up and closes the network connection.
 *
 * @param[in] n Pointer to the initialized Network structure.
 *
 * @details
 * - This function invokes the transport-specific `disconnect` callback to clean up the connection.
 * - For TCP, it closes the socket using `mqtt_tcp_disconnect`.
 * - For WebSocket, it deinitializes the WebSocket connection using `mqtt_ws_disconnect`.
 *
 * @note
 * - Ensure that the `Network` structure is valid and initialized before calling this function.
 * - If the `disconnect` callback is not set, the function does nothing.
 */
void NetworkDisconnect(Network *n);
#endif
