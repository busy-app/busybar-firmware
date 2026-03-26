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

typedef struct MatterSrv MatterSrv;

/**
 * @brief Type of service event
 */
typedef enum {
    MatterEventTypeCommissioning, //<! Started, completed or failed commissioning
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
 */
FuriState* matter_get_switch_state(MatterSrv* matter);

/**
 * @brief Sets the state of the Matter switch
 */
bool matter_set_switch_state(MatterSrv* matter, MatterSwitchState switch_state);

/**
 * @brief Set the startup mode of the Matter switch
 *
 * @param[in] matter Service instance
 * @param[in] mode Desired Matter switch startup mode
 * 
 * @returns true on success
 */
bool matter_set_switch_startup_mode(MatterSrv* matter, MatterSwitchStartupMode mode);

/**
 * @brief Deletes all Matter data
 * 
 * @param[in] matter Service instance
 * 
 * @returns true on success
 */
bool matter_factory_reset(MatterSrv* matter);

/**
 * @brief Enables Matter commissioning
 * 
 * @param[in] matter Service instance
 * @param[out] qr_code String to fill with onboarding QR code payload
 * @param[out] manual_code String to fill with manual pairing code
 * 
 * @returns Time (in seconds) that commissioning has been enabled for.
 *          0 indicates an error.
 */
size_t
    matter_enable_commissioning(MatterSrv* matter, FuriString* qr_code, FuriString* manual_code);

/**
 * @brief Commissioning information struct
 */
typedef struct {
    size_t count; //<! Number of fabric the device is commissioned into
    MatterCommissioningStatus last_status; //<! Latest status update from the service PubSub
    time_t
        last_status_at; //<! UTC Unix millisecond timestamp of latest status update. `0` means no updates have been issued yet.
} MatterCommissionedFabrics;

/**
 * @brief Brief information about the commissioning status
 * 
 * @param[in] matter Service instance
 * 
 * @returns Commissioning information struct
 */
MatterCommissionedFabrics matter_commissioned_fabrics(MatterSrv* matter);

/**
 * @brief Gets the currently selected Certification Declaration
 * 
 * @param[in] matter Service instance
 * 
 * @returns Name of the Certification Declaration variant currently marked as "wanted"
 */
const char* matter_get_wanted_cd_selection(MatterSrv* matter);

/**
 * @brief Sets the currently selected Certification Declaration
 * 
 * @param[in] matter Service instance
 * @param[in] selection Wanted selection
 * 
 * @warning Changes will only apply after a reboot
 * 
 * @returns true on success
 */
bool matter_set_wanted_cd_selection(MatterSrv* matter, const char* selection);

/**
 * @brief Gets the currently de-facto active Certification Declaration
 * 
 * @param[in] matter Service instance
 * 
 * @returns name of the Certification Declaration variant that Matter is currently using
 */
const char* matter_get_de_facto_cd_selection(MatterSrv* matter);

#ifdef __cplusplus
}
#endif
