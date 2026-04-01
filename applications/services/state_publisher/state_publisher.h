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
#include <mlib/m-array.h>
#include <mlib/m-shared.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_STATE_PUBLISHER "state_publisher"

typedef struct StatePublisher StatePublisher;

typedef enum StatePublisherTransportClass {
    StatePublisherTransportClassWebSocket,
    StatePublisherTransportClassBLE,
    StatePublisherTransportClassMQTT,

    StatePublisherTransportClassMax,
} StatePublisherTransportClass;

ARRAY_DEF(ByteArray, uint8_t);

SHARED_PTR_DEF(SharedByteArray, ByteArray_t, ARRAY_OPLIST(ByteArray));

typedef void (*StatePublisherPublishCb)(const SharedByteArray_t data, void* context);

typedef size_t StatePublisherTransportHandle;

#define STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID ((StatePublisherTransportHandle) - 1)

/**
 * Add transport (sink) to receive serialized updates.
 *
 * @param transport transport class.
 * @param frame_interval_ms minimum frame interval for this transport class.
 * @return handle to be used in state_publisher_del_transport.
 */
StatePublisherTransportHandle state_publisher_add_transport(
    StatePublisher* instance,
    StatePublisherTransportClass transport_class,
    uint32_t frame_interval_ms,
    StatePublisherPublishCb cb,
    void* context);

/**
 * Delete transport (sink).
 *
 * @param handle transport handle received from state_publisher_add_transport.
 */
void state_publisher_del_transport(StatePublisher* instance, StatePublisherTransportHandle handle);

#ifdef __cplusplus
}
#endif
