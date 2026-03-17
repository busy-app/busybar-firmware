/**
 * @file state_publisher.h
 * @brief State publisher - publish state changes across various channels.
 */
#pragma once

#include <stdint.h>
#include <ble/ble.h>
#include <wifi/wifi_common.h>
#include <power/power_service/power.h>
#include <matter/matter.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_STATE_PUBLISHER "state_publisher"

typedef struct StatePublisher StatePublisher;

typedef enum StatePublisherTransport {
    StatePublisherTransportWebSocket,
    StatePublisherTransportBLE,
    StatePublisherTransportMQTT,

    StatePublisherTransportMax,
} StatePublisherTransport;

typedef void (*StatePublisherPublishCb)(const void* data, size_t data_size, void* context);

typedef size_t StatePublisherTransportHandle;

StatePublisherTransportHandle state_publisher_add_transport(
    StatePublisher* instance,
    StatePublisherTransport transport,
    uint32_t frame_interval_ms,
    StatePublisherPublishCb cb,
    void* context);
void state_publisher_del_transport(StatePublisher* instance, StatePublisherTransportHandle handle);

#ifdef __cplusplus
}
#endif
