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
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#ifndef __MQTT_CLIENT_C_
#define __MQTT_CLIENT_C_

#include "MQTTPacket.h"

#if defined(SLI_SI91X_OFFLOAD_NETWORK_STACK) || defined(sl_si91x_internal_stack)
#include "MQTTSi91x.h" //Platform specific implementation header file
#elif defined(SLI_SI91X_LWIP_HOSTED_NETWORK_STACK) || defined(sl_si91x_lwip_stack)
#include "MQTTSi91x_lwip.h" //Platform specific implementation header file
#endif

#define MAX_PACKET_ID        65535
#define MAX_MESSAGE_HANDLERS 5

enum QoS { QOS0, QOS1, QOS2 };

// all failure return codes must be negative
enum returnCode { BUFFER_OVERFLOW = -2, FAILURE = -1, SUCCESS = 0 };

typedef struct MQTTMessage MQTTMessage;

typedef struct MessageData MessageData;

struct MQTTMessage {
  enum QoS qos;
  char retained;
  char dup;
  unsigned short id;
  void *payload;
  size_t payloadlen;
};

struct MessageData {
  MQTTMessage *message;
  MQTTString *topicName;
};

typedef void (*messageHandler)(MessageData *);

typedef struct Client Client;

int MQTTConnect(Client *, MQTTPacket_connectData *);
int MQTTPublish(Client *, const char *, MQTTMessage *);
int MQTTSubscribe(Client *, char *, enum QoS, messageHandler);
int MQTTUnsubscribe(Client *, const char *);
int MQTTDisconnect(Client *);
int MQTTYield(Client *, int);

void setDefaultMessageHandler(Client *, messageHandler);

void MQTTClient(Client *, Network *, unsigned int, unsigned char *, size_t, unsigned char *, size_t);

struct Client {
  unsigned int next_packetid;
  unsigned int command_timeout_ms;
  size_t buf_size, readbuf_size;
  unsigned char *buf;
  unsigned char *readbuf;
  unsigned int keepAliveInterval;
  char ping_outstanding;
  int isconnected;

  struct MessageHandlers {
    char *topicFilter;
    void (*fp)(MessageData *);
  } messageHandlers[MAX_MESSAGE_HANDLERS]; // Message handlers are indexed by subscription topic

  void (*defaultMessageHandler)(MessageData *);

  Network *ipstack;
  Timer ping_timer;
};

#define DefaultClient               \
  {                                 \
    0, 0, 0, 0, NULL, NULL, 0, 0, 0 \
  }
/* Macro Definition: Defines the timeout value to read the entire payload of one TCP Packet.
  * for ex:
  *        i. One TCP Packet i.e. MQTT PUBLISH 1st byte (MQTT Header)
  *           + 2nd byte(MSG LEN)+3rd byte(MSG LEN)
  *           + 4th byte to Nth byte(MQTT PUBLISH Payload)
  *           Note: one more example i.e. MQTT PING RESPONSE
  *           1st byte(MQTT Header)+2nd byte(MSG LEN)
  *        ii. Once host application starts reading the 1st byte(i.e. MQTT Header),
  *           remaining bytes(of single TCP Packet) to be read by host mandatorily.
  *           Otherwise, Host will not be able to identify the subsequent bytes properly.
  * Here, the Wait time is considered based on the logic present in rsi_socket_recvfrom() */
#define SINGLE_PKT_TCP_STREAM_TIMEOUT WAIT_TIMEOOUT + SLI_SOCKET_RECVFROM_RESPONSE_WAIT_TIME
#endif