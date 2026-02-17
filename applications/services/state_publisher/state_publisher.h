/**
 * @file state_publisher.h
 * @brief State publisher - publish state changes across various channels.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_STATE_PUBLISHER "state_publisher"

typedef struct StatePublisher StatePublisher;

void state_publisher_publish(StatePublisher* app);

#ifdef __cplusplus
}
#endif
