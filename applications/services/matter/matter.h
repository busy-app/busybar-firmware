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
    MatterEventTypeWillReboot, //<! A reboot will be performed shortly
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
    time_t last_status_at; //<! UTC Unix millisecond timestamp of latest status update
} MatterCommissionedFabrics;

/**
 * @brief Enumeration of available certification types.
 *
 * @warning The order of the elements denotes the fallback order.
 *          Do NOT change it unless necessary.
 */
typedef enum {
    MatterCertificationTypeProduction, /**< Certification declaration for production devices */
    MatterCertificationTypeDevelopment, /**< Certification declaration for development */
    MatterCertificationTypeProvisional, /**< Certification declaration for lab testing */
    MatterCertificationTypeMax, /**< Special value, invalid or internal use */
} MatterCertificationType;

/**
 * @brief Structure describing certification declaration settings.
 */
typedef struct {
    MatterCertificationType wanted; /**< Preferred certification type */
    MatterCertificationType actual; /**< Actual certification type based on installed files */
} MatterCertificationConfig;

/**
 * @brief Get a PubSub to listen to Matter events.
 *
 * The returned FuriPubSub object will have an underlying type of MatterEvent.
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
 * @brief Delete all Matter pairing data, then reboot on success
 *
 * @param[in,out] instance pointer to the service instance
 *
 * @returns @c MatterStatus enum on error. @c MatterStatusOk is never returned.
 */
MatterStatus matter_factory_reset(Matter* instance);

/**
 * @brief Set the preferred certification config.
 *
 * @warning Changes will only apply after a reboot
 *
 * @param[in,out] instance pointer to the service instance
 * @param[in] cert_type value corresponding to the preferred certification type
 *
 * @returns @c MatterStatusOk on success, any other value from @c MatterStatus enum on error
 */
MatterStatus matter_set_certification_config(Matter* instance, MatterCertificationType cert_type);

/**
 * @brief Get the certification configuration, including preferred and actual configs.
 *
 * @param[in,out] instance pointer to the service instance
 * @param[out] cert_config pointer to the object to contain the config (must be allocated)
 *
 * @returns @c MatterStatusOk on success, any other value from @c MatterStatus enum on error
 */
MatterStatus
    matter_get_certification_config(Matter* instance, MatterCertificationConfig* cert_config);

#ifdef __cplusplus
}
#endif
