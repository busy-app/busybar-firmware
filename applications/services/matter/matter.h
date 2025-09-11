/**
 * @file matter.h
 * API for Matter service on f20.
 */

#pragma once

#include <furi.h>
#include "matter_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_MATTER "matter"

typedef struct MatterSrv MatterSrv;

/**
 * @brief Type of service event
 */
typedef enum {
    MatterEventTypeStateUpdate, // <! State of a virtual device changed
} MatterEventType;

/**
 * @brief Event of type `MatterEventTypeStateUpdate`
 */
typedef struct {
    MatterVirtualDeviceState new_state;
} MatterUpdateEvent;

/**
 * @brief Complete service event
 */
typedef struct {
    MatterEventType type;
    union {
        MatterUpdateEvent update;
    };
} MatterEvent;

/**
 * @brief Gets a PubSub to listen to Matter events
 * 
 * @param[in] matter Service instance
 * 
 * @returns FuriPubSub handle. Events are of type `MatterEvent`.
 */
FuriPubSub* matter_get_pubsub(MatterSrv* matter);

/**
 * @brief Gets the state of a virtual Matter device
 * 
 * @param[in] matter Service instance
 * @param[in] device Virtual device selector
 * 
 * @returns Current state of the queried device
 */
MatterVirtualDeviceState matter_get_state(MatterSrv* matter, MatterVirtualDevice device);

/**
 * @brief Sets the state of a virtual Matter device
 * 
 * @param[in] matter Service instance
 * @param[in] state Virtual device selector along with its state
 */
void matter_set_state(MatterSrv* matter, MatterVirtualDeviceState state);

/**
 * @brief Deletes all Matter data
 * 
 * @param[in] matter Service instance
 */
void matter_factory_reset(MatterSrv* matter);

// TODO: matter_enable_commissioning

#ifdef __cplusplus
}
#endif
