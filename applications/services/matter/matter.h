/**
 * @file matter.h
 * API for Matter service on u5.
 */

#pragma once

#include "matter_common.h"

#include <time.h>

#include <core/string.h>
#include <core/pubsub.h>
#include <core/state.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_MATTER "matter"

typedef struct Matter Matter;

/**
 * @brief Type of service event
 */
typedef enum {
    MatterEventTypeCommissioning, //<! Started, completed or failed commissioning
    MatterEventTypeFabricCountChanged, //<! Number of fabric the device is commissioned into has changed
} MatterEventType;

/**
 * @brief Event of type `MatterEventTypeCommissioning`
 */
typedef struct {
    MatterCommissioningStatus status;
} MatterCommissioningEvent;

/**
 * @brief Complete service event
 */
typedef struct {
    MatterEventType type;
    union {
        MatterCommissioningEvent commissioning;
        size_t fabric_count;
    };
} MatterEvent;

/**
 * @brief Commissioning information struct
 */
typedef struct {
    uint32_t count; //<! Number of fabric the device is commissioned into
    MatterCommissioningStatus last_status; //<! Latest status update from the service PubSub
    time_t last_status_at; //<! UTC Unix millisecond timestamp of latest status update..
} MatterCommissionedFabrics;

/**
 * @brief Get a PubSub to listen to Matter events.
 *
 * The returned FuriPubSub object witll have an underlying type of MatterEvent.
 * Use furi_pubsub_subscribe() to get notifications about new events.
 * 
 * @param[in,out] instance pointer to the service instance
 *
 * @returns pointer to a FuriPubSub object
 */
FuriPubSub* matter_get_pubsub(Matter* instance);

/**
 * @brief Get the switch state object that supports change notifications.
 *
 * The returned FuriState object will have an underlying type of MatterSwitchState.
 * Use furi_state_subscribe() to get notifications about changes in the current switch state.
 *
 * This function never blocks.
 *
 * @param[in,out] instance pointer to the service instance
 *
 * @returns pointer to a FuriState object
 */
FuriState* matter_get_switch_state(Matter* instance);

/**
 * @brief Set the state of the Matter switch.
 *
 * @note Passing @c MatterSwitchStateUnknown as @c switch_state does nothing.
 *
 * @param[in,out] instance pointer to the service instance
 * @param[in] switch_state required Matter switch state
 *
 * @returns @c MatterStatusOk on success, any other value from @c MatterStatus enum on error
 */
MatterStatus matter_set_switch_state(Matter* instance, MatterSwitchState switch_state);

/**
 * @brief Set the startup mode of the Matter switch
 *
 * @param[in,out] instance pointer to the service instance
 * @param[in] mode required Matter switch startup mode
 *
 * @returns @c MatterStatusOk on success, any other value from @c MatterStatus enum on error
 */
MatterStatus matter_set_switch_startup_mode(Matter* instance, MatterSwitchStartupMode mode);

/**
 * @brief Enable Matter commissioning (open the commissioning window).
 * 
 * @param[in,out] instance pointer to the service instance
 * @param[out] info pointer to the object to contain the information about the current commissioning window
 *
 * @returns @c MatterStatusOk on success, any other value from @c MatterStatus enum on error
 */
MatterStatus matter_enable_commissioning(Matter* instance, MatterCommissioningInfo* info);

/**
 * @brief Get the brief information about the commissioning status.
 * 
 * @param[in,out] instance pointer to the service instance
 * @param[out] fabrics pointer to the object to contain the information about commissioned fabrics
 * 
 * @returns @c MatterStatusOk on success, any other value from @c MatterStatus enum on error
 */
MatterStatus matter_get_commissioned_fabrics(Matter* instance, MatterCommissionedFabrics* fabrics);

/**
 * @brief Delete all Matter pairing data.
 *
 * @param[in,out] instance pointer to the service instance
 *
 * @returns @c MatterStatusOk on success, any other value from @c MatterStatus enum on error
 */
MatterStatus matter_factory_reset(Matter* instance);

/**
 * @brief Get the currently selected Certification Declaration.
 * 
 * @param[in] instance Service instance
 * 
 * @returns Name of the Certification Declaration variant currently marked as "wanted"
 */
const char* matter_get_wanted_cd_selection(Matter* instance);

/**
 * @brief Set the currently selected Certification Declaration.
 * 
 * @param[in,out] instance pointer to the service instance
 * @param[in] selection Wanted selection
 * 
 * @warning Changes will only apply after a reboot
 * 
 * @returns true on success
 */
MatterStatus matter_set_wanted_cd_selection(Matter* instance, const char* selection);

/**
 * @brief Get the currently de-facto active Certification Declaration.
 * 
 * @param[in,out] instance pointer to the service instance
 * 
 * @returns name of the Certification Declaration variant that Matter is currently using
 */
const char* matter_get_de_facto_cd_selection(Matter* instance);

#ifdef __cplusplus
}
#endif
